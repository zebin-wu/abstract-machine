CROSS_COMPILE := riscv64-linux-gnu-
COMMON_FLAGS  := -fno-pic -march=rv64ifd -mcmodel=medany
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv

AM_SRCS := npc/start.S \
           npc/trm.c \
           npc/libgcc/muldi3.S \
           npc/libgcc/div.S \
           npc/ioe.c \
           npc/timer.c \
           npc/input.c \
           npc/cte.c \
           npc/trap.S \
           npc/vme.c \
           npc/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/platform/nemu.ld --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/npc/trm.c

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin
