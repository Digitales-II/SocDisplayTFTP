BUILD_DIR=../build/
SOC_DIRECTORY=../../../../litex/litex/litex/soc/
include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak
OBJECTS	=	main.o \
			crt0.o 
vpath %.a $(PACKAGES:%=../%)

all: boot.bin
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	chmod -x $@

boot.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) \
		-T linker.ld \
		-N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-Wl,--whole-archive \
		-Wl,--gc-sections \
		-Wl,-Map,$@.map \
		$(LIBS:lib%=-l%)
	chmod -x $@

main.o: main.c
	$(compile)


crt0.o: $(CPU_DIRECTORY)/crt0.S
	$(assemble)
%.o: %.c
	$(compile)
%.o: %.S
	$(assemble)
clean:
	$(RM) $(OBJECTS) *.elf *.bin .~ *~ *.map *.d
.PHONY: clean