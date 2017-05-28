ifeq ($(VERBOSE), 1)
	MSG = --verbose
endif

XCC     = gcc
AS	= as
AR	= ar


CFLAGS  = -c -fPIC -Wall $(INCLUDE) -mcpu=arm920t -msoft-float --no-builtin
ASFLAGS	= -mcpu=arm920t -mapcs-32
ARFLAGS = rcs

INCLUDE = -I../../common/include -I../../util/include -I../../io/include
LDPATHS = -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../../common/lib -L../../io/lib -L../../util/lib 
LIBS 	= -lutil -lc -lgcc -lcommon
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
