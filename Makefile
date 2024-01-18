TARGET=tarjeta_luis
TOP=tarjeta_luis
GATE_DIR=build/gateware
SOFT_DIR=build/software




SERIAL?=/dev/ttyUSB0
	
#antes va eñ make -c
#litex_term /dev/ttyUSB0 --kernel hello_world/hello_world.bin

NEXTPNR=nextpnr-ecp5
CC=riscv64-unknown-elf-gcc

all: gateware firmware

${GATE_DIR}/${TARGET}.bit:
	./base.py

gateware: ${GATE_DIR}/${TARGET}.bit

${SOFT_DIR}/common.mak: gateware

firmware: ${SOFT_DIR}/common.mak
	$(MAKE) -C firmware/ -f Makefile all

litex_term: firmware
	litex_term ${SERIAL} --kernel firmware/boot.bin

litex_term_bootNet: firmware
	sudo cp firmware/boot.bin ~/Documents/tftp/
	litex_term ${SERIAL}

configure: ${GATE_DIR}/${TARGET}.bit
	sudo openFPGALoader -c ft232RL --pins=RXD:RTS:TXD:CTS -m ${GATE_DIR}/${TARGET}.bit 
#	sudo openFPGALoader -b colorlight-ft232rl -m ${GATE_DIR}/${TARGET}.bit 

gateware-clean:
	rm -f ${GATE_DIR}/*.svf ${GATE_DIR}/*.bit ${GATE_DIR}/*.config ${GATE_DIR}/*.json ${GATE_DIR}/*.ys *svg

firmware-clean:
	make -C firmware -f Makefile clean

clean: firmware-clean gateware-clean

.PHONY: clean
