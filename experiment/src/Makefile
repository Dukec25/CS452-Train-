SRC		= main.c first.c add.c
TARGET_ELF	= main.elf
ASM_SRC		= switch.s
ASM_OBJ		= build/switch.o

all: build $(ASM_OBJ) $(TARGET_ELF) copy

include ../../common/common.mk

INCLUDE += -I../../include

$(ASM_OBJ): $(ASM_SRC)
	$(AS) $(ASFLAGS) $< -o $@
