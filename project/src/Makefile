LIB_SRC		= cli_server.c  train_server.c train_server_helper.c train_task.c park_server.c workers.c
TEST_SRC	= milestone1_test.c
SRC			= $(LIB_SRC) user_tasks.c main.c
TARGET_ELF 	= main.elf

all: build $(TARGET_ELF) copy

include ../../common/common.mk

LIB_OBJ	= $(addprefix $(BUILD)/, $(patsubst %.c, %.o, $(LIB_SRC)))
lib:
	mkdir -p ../lib
	$(AR) $(ARFLAGS) ../lib/libtrain.a $(LIB_OBJ)
