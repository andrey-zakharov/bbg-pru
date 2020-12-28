# move this out of makefile to cmake build system?
TARGET = test.pru0
MODEL := $(shell cat /proc/device-tree/model | sed 's/ /_/g' | tr -d '\000')
CHIP = am335x
CHIP_REV = am335x
PRU_DIR = /sys/class/remoteproc/remoteproc1
PRU_SUPPORT := /usr/lib/ti/pru-software-support-package
PRU_STARTERWARE := /usr/share/ti/starterware
PRU_CGT := /usr/share/ti/cgt-pru
COMMON := /var/lib/cloud9/common
PRUN = 0
PROC = pru
EXE = .out
LD = lnkpru
LDFLAGS = --reread_libs --warn_sections --stack_size=0x100 --heap_size=0x100 -m $(basename $(1)).map \
   -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(COMMON)/$(CHIP)_$(PROC).cmd --library=libc.a \
   --library=$(PRU_SUPPORT)/lib/rpmsg_lib.lib
CC = clpru
CFLAGS += --verbose --include_path=$(COMMON) --include_path=$(PRU_SUPPORT)/include \
   --include_path=$(PRU_SUPPORT)/include/$(CHIP_REV) \
   --include_path=$(PRU_STARTERWARE)/include --include_path=$(PRU_STARTERWARE)/include/hw \
   --include_path=$(PRU_CGT)/include -DCHIP=$(CHIP) -DCHIP_IS_$(CHIP) -DMODEL=$(MODEL) -DPROC=$(PROC) -DPRUN=$(PRUN) \
   --printf_support=minimal --display_error_number --endian=little --hardware_mac=on \
    -ppd -ppa --opt_for_speed=5 -O4 --keep_asm --symdebug:none
CXX := $(CC)
CXXFLAGS := $(CFLAGS)
# make custom
COMPILE.cc = $(CXX) $(CXXFLAGS)
OUTPUT_OPTION = --output_file $@
VPATH=..
# --c_src_interlist --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) --asm_directory=$(GEN_DIR)
all: stop install start
	@echo "MODEL   = $(MODEL)"
	@echo "PROC    = $(PROC)"
	@echo "PRUN    = $(PRUN)"
	@echo "PRU_DIR = $(PRU_DIR)"
	
# TODO: think about what we want to say if stop fails
stop:
ifneq ($(PRU_DIR),)
	@echo "-    Stopping PRU $(PRUN)"
	@echo stop > $(PRU_DIR)/state || echo Cannot stop $(PRUN)
endif

start:
ifneq ($(PRU_DIR),)
	@echo write_init_pins.sh
	@$(COMMON)/write_init_pins.sh /lib/firmware/$(CHIP)-pru$(PRUN)-fw
	@echo "-    Starting PRU $(PRUN)"
	@echo start > $(PRU_DIR)/state
else
	./$(TARGET)$(EXE)
endif


install: $(TARGET)$(EXE)
ifneq ($(PRU_DIR),)
	@echo '-	copying firmware file $(TARGET) to /lib/firmware/$(CHIP)-pru$(PRUN)-fw'
	@cp $(TARGET)$(EXE) /lib/firmware/$(CHIP)-pru$(PRUN)-fw
else
	@cp $(TARGET) ./$(TARGET)$(EXE)
endif


%.out: %.o
	@echo 'LD	$^'
	@$(LD) $^ $(LDFLAGS) $(OUTPUT_OPTION)

# test.pru0.out: test.pru0.o write24.asm.o
# 	$(LD) $^ $(LDFLAGS) $(OUTPUT_OPTION)
test.pru0.o: test.pru0.obj
	$(CP) $@ $^

test.pru0.out: test.pru0.c write24.asm
	$(COMPILE.C) $^ --run_linker $(LDFLAGS) $(OUTPUT_OPTION)

# write24.asm.o: write24.asm
# 	$(COMPILE.C) $^ $(OUTPUT_OPTION)



clean:
	$(RM) *.asm *.obj *.lst *.pp *.out *.o
