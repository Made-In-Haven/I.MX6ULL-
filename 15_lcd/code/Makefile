CROSS_COMPILE ?= arm-linux-gnueabihf-
TARGET := lcd
GCC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

LIBPATH := -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4

INCLUDEDIRS := imx6ull \
			bsp/clk \
			bsp/led \
			bsp/delay \
			bsp/beep \
			bsp/key \
			bsp/gpio \
			bsp/int  \
			bsp/exti \
			bsp/epit \
			bsp/key_filter \
			bsp/uart \
			stdio/include \
			bsp/lcd \
			project

SRCDIRS :=  project \
			bsp/clk \
			bsp/led \
			bsp/beep \
			bsp/key \
			bsp/gpio \
			bsp/int \
			bsp/exti \
			bsp/delay \
			bsp/key_filter \
			bsp/uart \
			stdio/lib \
			bsp/lcd \
			bsp/epit 

INCLUDE := $(patsubst %, -I %,$(INCLUDEDIRS))
CFILES  := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
SFILES  := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.s))
SFILESDIR := $(notdir $(SFILES))
CFILESDIR := $(notdir $(CFILES))
SOBJS := $(patsubst %.s, obj/%.o, $(SFILESDIR))
COBJS := $(patsubst %.c, obj/%.o, $(CFILESDIR))
OBJS := $(SOBJS)$(COBJS)

VPATH := $(SRCDIRS)

.PHONY:clean



$(TARGET).bin:$(OBJS)
	$(LD) -T imx6ull.lds $^ -o $(TARGET).elf $(LIBPATH)
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS): obj/%.o : %.s
	$(GCC) -Wall -nostdlib -fno-builtin -c -O2  $(INCLUDE) -o $@ $<

$(COBJS): obj/%.o : %.c
	$(GCC) -Wall -Wa,-mimplicit-it=thumb -nostdlib -fno-builtin -c -O2 $(INCLUDE) $< -o $@

print:
	@echo INCLUDE = $(INCLUDE)
	@echo SFILES = $(SFILES)
	@echo SFILESDIR = $(SFILESDIR)
	@echo OBJS = $(OBJS)

clean:
	rm -rf *.elf *.bin *.dis obj/*.o