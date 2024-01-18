from migen import *
from migen.genlib.cdc import MultiReg
from litex.soc.interconnect.csr import *
from litex.soc.interconnect.csr_eventmanager import *

class PANTALLA(Module,AutoCSR):
   def __init__(self, pads):
   # Interfaz
      self.clk = ClockSignal()
      self.led = pads.led
      self.o_data_clock = pads.o_data_clock
      self.o_data_latch = pads.o_data_latch
      self.o_data_blank = pads.o_data_blank
      self.o_data_r = pads.o_data_r
      self.o_data_g = pads.o_data_g
      self.o_data_b = pads.o_data_b
      self.o_data_r2 = pads.o_data_r2
      self.o_data_g2 = pads.o_data_g2
      self.o_data_b2 = pads.o_data_b2
      self.o_data_r3 = pads.o_data_r3
      self.o_data_g3 = pads.o_data_g3
      self.o_data_b3 = pads.o_data_b3
      self.o_row_select = pads.o_row_select


   # Registros solo escritura       
      self.enable       = CSRStorage(1)
      self.wr           = CSRStorage(1)
      self.dataLine    = CSRStorage(54)
      self.addrWrite    = CSRStorage(12)
      self.RamTime      = CSRStorage(30)

   # Registro de lectura
      self.requireData  = CSRStatus(1)

   # Instanciación del módulo verilog     
      self.specials +=Instance("led_control", 
          	i_i_clk_i      = self.clk,
            i_enable       = self.enable.storage,
            i_wr           = self.wr.storage,
            i_dataLine    = self.dataLine.storage,
            i_addrWrite    = self.addrWrite.storage,
            i_RamTime      = self.RamTime.storage,
            o_led          = self.led,
            o_requireData  = self.requireData.status,
	         o_o_data_clock = self.o_data_clock,
            o_o_data_latch = self.o_data_latch,
            o_o_data_blank = self.o_data_blank,
	         o_o_data_r     = self.o_data_r,
            o_o_data_g     = self.o_data_g,
            o_o_data_b     = self.o_data_b,
            o_o_data_r2    = self.o_data_r2,
            o_o_data_g2    = self.o_data_g2,
            o_o_data_b2    = self.o_data_b2,
            o_o_data_r3    = self.o_data_r3,
            o_o_data_g3    = self.o_data_g3,
            o_o_data_b3    = self.o_data_b3,
	         o_o_row_select = self.o_row_select,
	   )	   
      
	         
