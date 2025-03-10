objs =start.o main.o
ld :=arm-linux-gnueabihf-ld
gcc :=arm-linux-gnueabihf-gcc
objcopy :=arm-linux-gnueabihf-objcopy
objdump :=arm-linux-gnueabihf-objdump

ledc.bin:$(objs)
	$(ld) -T imx6ull.lds $^ -o ledc.elf
	$(objcopy) -O binary -g -S ledc.elf $@
	$(objdump) -D -m arm ledc.elf > ledc.dis

%.o:%.c
	$(gcc) -g -c -Wall -O2 -nostdlib $< -o $@ 
%.o:%.s
	$(gcc) -g -c -Wall -O2 -nostdlib $< -o $@ 

clean:
	rm -rf *.o ledc.bin ledc.elf ledc.dis