SRC		= main.c kernel.c user_functions.c user_tasks.c name_server.c rps.c irq.c clock_server.c
TARGET_ELF	= main.elf
ASM_SRC		= switch.s
ASM_OBJ		= build/switch.o

all: build $(ASM_OBJ) $(TARGET_ELF) copy

include ../../common/common.mk

INCLUDE += -I../../include
#CFLAGS += -O2

$(ASM_OBJ): $(ASM_SRC)
	$(AS) $(ASFLAGS) $< -o $@
