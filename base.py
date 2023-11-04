#!/usr/bin/env python3
from migen import *
from migen.genlib.resetsync import AsyncResetSynchronizer

from litex.gen import *

from litex.build.generic_platform import IOStandard, Subsignal, Pins
from platforms import tarjeta_luis as tarjeta

from litex.build.io import DDROutput

from litex_boards.platforms import colorlight_5a_75b, colorlight_5a_75e, colorlight_i5a_907

from litex.soc.cores.clock import *
from litex.soc.integration.soc_core import *
from litex.soc.integration.builder import *
from litex.soc.cores.led import LedChaser

from litedram.modules import M12L16161A, M12L64322A
from litedram.phy import GENSDRPHY, HalfRateGENSDRPHY

from liteeth.phy.ecp5rgmii import LiteEthPHYRGMII

from ios import Led
from module import per_pantalla

_serial = [
    ("serial", 0,
        Subsignal("tx", Pins("C4")),  # J1.1
        Subsignal("rx", Pins("D4")),  # J1.2
        IOStandard("LVCMOS33")
     ),
]
_leds = [
    ("user_led", 0, Pins("P13"), IOStandard("LVCMOS33")),  # LED en la placa
    ("user_led", 1, Pins("F3"), IOStandard("LVCMOS33")),  # LED externo
]
_pantalla = [
    ("pads_pantalla", 0,
        Subsignal("o_data_clock", Pins("M3")),  # 
        Subsignal("o_data_latch", Pins("N1")),  # 
        Subsignal("o_data_blank", Pins("M4")),  # 
            
        Subsignal("led", Pins("T6")),  # J1.1
        Subsignal("o_row_select", Pins("N5 N3 P3 P4 N4")),  # J1.2
        
        Subsignal("o_data_r", Pins("P7 R8")),
        Subsignal("o_data_g", Pins("M7 M8")),
        Subsignal("o_data_b", Pins("P8 M9")),
        
        Subsignal("o_data_r2", Pins("T3 P5")),
        Subsignal("o_data_g2", Pins("R4 N6")),
        Subsignal("o_data_b2", Pins("M5 N7")),

        Subsignal("o_data_r3", Pins("L4 R2")),
        Subsignal("o_data_g3", Pins("L5 T2")),
        Subsignal("o_data_b3", Pins("P2 R3")),
        IOStandard("LVCMOS33")
     ),
]
# CRG ----------------------------------------------------------------------------------------------

class _CRG(LiteXModule):
    def __init__(self, platform, sys_clk_freq, use_internal_osc=False, with_usb_pll=False, with_rst=True, sdram_rate="1:1"):
        self.rst    = Signal()
        self.cd_sys = ClockDomain()
        if sdram_rate == "1:2":
            self.cd_sys2x    = ClockDomain()
            self.cd_sys2x_ps = ClockDomain()
        else:
            self.cd_sys_ps = ClockDomain()

        # # #

        # Clk / Rst
        if not use_internal_osc:
            clk = platform.request("clk25")
            clk_freq = 25e6
        else:
            clk = Signal()
            div = 5
            self.specials += Instance("OSCG",
                                p_DIV = div,
                                o_OSC = clk)
            clk_freq = 310e6/div

        rst_n = platform.request("user_btn_n", 0)

        # PLL
        self.pll = pll = ECP5PLL()
        self.comb += pll.reset.eq(~rst_n | self.rst)
        pll.register_clkin(clk, clk_freq)
        pll.create_clkout(self.cd_sys,    sys_clk_freq)
        if sdram_rate == "1:2":
            pll.create_clkout(self.cd_sys2x,    2*sys_clk_freq)
            pll.create_clkout(self.cd_sys2x_ps, 2*sys_clk_freq, phase=180) # Idealy 90° but needs to be increased.
        else:
           pll.create_clkout(self.cd_sys_ps, sys_clk_freq, phase=180) # Idealy 90° but needs to be increased.

        # USB PLL
        if with_usb_pll:
            self.usb_pll = usb_pll = ECP5PLL()
            self.comb += usb_pll.reset.eq(~rst_n | self.rst)
            usb_pll.register_clkin(clk, clk_freq)
            self.cd_usb_12 = ClockDomain()
            self.cd_usb_48 = ClockDomain()
            usb_pll.create_clkout(self.cd_usb_12, 12e6, margin=0)
            usb_pll.create_clkout(self.cd_usb_48, 48e6, margin=0)

        # SDRAM clock
        sdram_clk = ClockSignal("sys2x_ps" if sdram_rate == "1:2" else "sys_ps")
        self.specials += DDROutput(1, 0, platform.request("sdram_clock"), sdram_clk)

# BaseSoC ------------------------------------------------------------------------------------------

class BaseSoC(SoCCore):
    def __init__(self, sys_clk_freq=60e6, toolchain="trellis",
        with_ethernet    = True,
        with_etherbone   = False,
        with_uartbone    = False,
        eth_ip           = "192.168.1.50",
        eth_phy          = 0,
        with_led_chaser  = True,
        use_internal_osc = True,
        sdram_rate       = "1:1",
        with_spi_flash   = False,
        with_rst = True,
        **kwargs):
        
        platform = tarjeta.Platform()
        platform.add_extension(_serial)
        platform.add_extension(_leds)
        platform.add_extension(_pantalla)
        platform.add_source("module/ram/ram.v") #buscar los archivos .v
        platform.add_source("module/led_control.v")

        # CRG --------------------------------------------------------------------------------------

	
        with_usb_pll = kwargs.get("uart_name", None) == "usb_acm"
        self.crg = _CRG(platform, sys_clk_freq,
            use_internal_osc = use_internal_osc,
            with_usb_pll     = with_usb_pll,
            with_rst         = with_rst,
            sdram_rate       = sdram_rate
        )

        # SoCCore ----------------------------------------------------------------------------------
        SoCCore.__init__(
        	self, platform, cpu_type = "lm32",
        	clk_freq = int(sys_clk_freq), 
        	ident="LiteX SoC on Colorlight ", 
        	ident_version=True, 
        	integrated_rom_size = 0x9800,
        	**kwargs)

        # SDR SDRAM --------------------------------------------------------------------------------
        self.submodules.sdrphy = GENSDRPHY(platform.request("sdram"))
        sdram_cls  = M12L64322A
        self.add_sdram("sdram",
           phy                     = self.sdrphy,
           module                  = sdram_cls(sys_clk_freq, sdram_rate),
           l2_cache_size           = kwargs.get("l2_size", 8192),
           l2_cache_full_memory_we = False,
        )

        # Ethernet / Etherbone ---------------------------------------------------------------------
        self.ethphy = LiteEthPHYRGMII(
           clock_pads = self.platform.request("eth_clocks", eth_phy),
           pads       = self.platform.request("eth", eth_phy),
           tx_delay   = 0)
        self.add_ethernet(phy=self.ethphy, data_width=32)

        # Leds -------------------------------------------------------------------------------------
        user_leds = Cat(*[platform.request("user_led", i) for i in range(1)])
        self.submodules.leds = Led(user_leds)
        self.add_csr("leds")
        
        SoCCore.add_csr(self,"pantalla")
        self.submodules.pantalla = per_pantalla.PANTALLA(platform.request("pads_pantalla",0))
        

# Build --------------------------------------------------------------------------------------------

soc = BaseSoC()
builder = Builder(soc, output_dir="build", csr_csv="csr.csv", csr_svd="csr.svd")
builder.build()
