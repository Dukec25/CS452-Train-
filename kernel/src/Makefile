SRC		= kernel.c scheduler.c task_descriptor.c user_functions.c name_server.c irq.c clock_server.c uart_irq.c io_server.c
TARGET_LIB 	= libkernel.a
ASM_SRC		= switch.s
ASM_OBJ		= build/switch.o

all: build $(ASM_OBJ) $(TARGET_LIB)

include ../../common/common.mk

#CFLAGS += -O2

$(ASM_OBJ): $(ASM_SRC)
	$(AS) $(ASFLAGS) $< -o $@
