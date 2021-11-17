CROSS_COMPILE := riscv64-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=rv32ifd -mabi=ilp32 -mcmodel=medany
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf32lriscv

AM_SRCS := npc/start.S \
           npc/trm.c \
		   npc/sys.c \
		   npc/libgcc/muldi3.S \
           npc/libgcc/div.S

CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/platform/nemu.ld --defsym=_pmem_start=0x0 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/npc/trm.c

RISCV_OBJCOPY ?= $(RISCV_PREFIX)objcopy -O verilog
RISCV_HEXGEN ?= '{ gsub("\r","",$$(NF)); if ($$1~/@/) { gsub("@","0x",$$1); addr=strtonum($$1); printf "@%08x\n",(addr%262144)/4;} else {for(i=1;i<NF;i+=4) print $$(i+3)$$(i+2)$$(i+1)$$i;}}'
RISCV_MIFGEN ?= 'BEGIN{printf "WIDTH=32;\nDEPTH=%d;\n\nADDRESS_RADIX=HEX;\nDATA_RADIX=HEX;\n\nCONTENT BEGIN\n",depth; addr=0;} { gsub("\r","",$$(NF)); if ($$1 ~/@/) {gsub("@","0x",$$1);addr=strtonum($$1);} else {printf "%04X : %s;\n",addr, $$1; addr=addr+1;}} END{print "END\n";}'

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	$(RISCV_OBJCOPY) $< $(IMAGE).tmp
	awk $(RISCV_HEXGEN) $(IMAGE).tmp > $(IMAGE).hex
	awk -v depth=65536 $(RISCV_MIFGEN) $(IMAGE).hex > $(IMAGE).mif
