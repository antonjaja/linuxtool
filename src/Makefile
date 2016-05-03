CC      = arm-linux-gnueabi-gcc
AS      = arm-linux-gnueabi-gcc
LD      = arm-linux-gnueabi-gcc
SIZE    = arm-linux-gnueabi-size
NM      = arm-linux-gnueabi-nm
OBJDUMP = arm-linux-gnueabi-objdump
OBJCOPY = arm-linux-gnueabi-objcopy
RM      = rm

#compile include file
CFLAGS = -static -O2 -Wall -Ih
#CFLAGS += -DKERNEL_ANDROID

LDFLAGS = -static

OUTPUT   = ./out

EXE += $(OUTPUT)/mp
EXE += $(OUTPUT)/efm32fn 
EXE += $(OUTPUT)/ASIXeeprom
#The mcu.h file from xxd -i mcu.bin mcu.h, that mcu.bin is mcu update file. 
EXE += $(OUTPUT)/mcuupdate
EXE += $(OUTPUT)/rtx_uboot
EXE += $(OUTPUT)/pidalive

TARGET = rtx_setenv
OBJS += o/rtx_setenv.o
OBJS += o/datalist.o
OBJS += o/crc32.o

#depence name for $(OBJS) is rtx_setenv.o datalist.o crc32.o
o/%.o : %.c
	$(CC) -c $(CFLAGS)  $< -o $@
	
#first is default build target
all:makepath $(EXE) setenv

#makepath
makepath:
	mkdir -p o
	mkdir -p $(OUTPUT)

#depence name for $(EXE) are EXE += $(OUTPUT)/mp EXE += $(OUTPUT)/efm32fn EXE += $(OUTPUT)/ASIXeeprom EXE += $(OUTPUT)/mcuupdate
$(OUTPUT)/% : %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

#depence name setenv
setenv:$(OBJS) 
	$(LD) $(LDFLAGS) $(OBJS) -o $(OUTPUT)/$(TARGET)

clean: distclean
distclean:
	rm -rf $(OUTPUT)
	rm -rf ./o



