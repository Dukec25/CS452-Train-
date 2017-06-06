ifeq ($(VERBOSE), 1)
	MSG = --verbose
endif

XCC = arm-none-eabi-gcc
AS	= arm-none-eabi-as
AR	= arm-none-eabi-ar
LD	= arm-none-eabi-ld

CFLAGS  = -c -fPIC -Wall $(INCLUDE) -mcpu=arm920t -msoft-float --no-builtin -O2
ASFLAGS	= -mcpu=arm920t -mapcs-32
ARFLAGS = rcs

INCLUDE = -I../../common/include -I../../util/include -I../../io/include
LDPATHS += -L/u/a7hu/gcc-arm-none-eabi-6-2017-q1-update/lib/gcc/arm-none-eabi/6.3.1
LDPATHS += -L/u/a7hu/gcc-arm-none-eabi-6-2017-q1-update/arm-none-eabi/lib
LDPATHS += -L../../common/lib -L../../io/lib -L../../util/lib 
LIBS 	= -lutil -lcommon -lc -lgcc
LDFLAGS = -init main -Map $(MAP) -N  -T orex.ld $(LDPATHS) $(LIBS)

BUILD	= build
ASM	= $(addprefix $(BUILD)/, $(patsubst %.c, %.s, $(SRC)))
OBJ	= $(addprefix $(BUILD)/, $(patsubst %.c, %.o, $(SRC))) $(ASM_OBJ)
MAP	= $(patsubst %.elf, %.map, $(TARGET_ELF))

$(BUILD):
	mkdir -p $@

$(BUILD)/%.s: %.c
	$(XCC) -S $(CFLAGS) $(MSG) $< -o $@

$(BUILD)/%.o: $(BUILD)/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET_LIB): $(ASM) $(OBJ)
	mkdir -p ../lib
	$(AR) $(ARFLAGS) ../lib/$@ $(OBJ)

$(TARGET_ELF): $(ASM) $(OBJ)
	$(LD) $(MSG) -o $@ $(OBJ) $(LDFLAGS) 

copy: $(TARGET_ELF)
	cp $< /u/cs452/tftp/ARM/$(shell whoami)

clean:
	rm -rf $(BUILD)
	rm -f ../lib/*.a *.elf *.map
