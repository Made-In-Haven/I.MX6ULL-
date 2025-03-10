objs = start.o main.o

ledc.bin:$(objs)
	arm-linux-gnueabihf-ld -T imx6ull.lds $^ -o ledc.elf
	arm-linux-gnueabihf-objcopy -O binary -S ledc.elf $@
	arm-linux-gnueabihf-objdump -D ledc.elf > ledc.dis

%.o:%.c
	arm-linux-gnueabihf-gcc -Wall -nostdlib  -g -O2 -c -o $@ $<

%.o:%.s
	arm-linux-gnueabihf-gcc -Wall -nostdlib  -g -O2 -c -o $@ $<

clean:
	rm -rf *.o ledc.bin ledc.elf ledc.dis