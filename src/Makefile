BUILD:=../build
SRC:=.
ENTRYPOINT:=0x10000 #内核程序入口地址

CFLAGS:= -m32 #32位的程序
CFLAGS+= -fno-builtin #不需要gcc内置函数
CFLAGS+= -nostdinc #不需要标准头文件
CFLAGS+= -fno-pic #不需要位置无关的代码
CFLAGS+= -fno-pie #不需要位置无关的可执行程序
CFLAGS+= -nostdlib#不需要标准库
CFLAGS+= -fno-stack-protector #不需要栈保护
CFLAGS:=$(strip $(CFLAGS))


DEBUG:=-g     #生成调试信息
INCLUDE:=-I$(SRC)/include


#编译生成bin文件
$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@)) 
	nasm -f bin $< -o $@

#编译生成start.o
$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	
	nasm -f elf32 $(DEBUG) $< -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

#编译生成kernel.bin,此时生成的kernel.bin为elf格式
$(BUILD)/kernel.bin: $(BUILD)/kernel/start.o \
		$(BUILD)/kernel/main.o	\
		$(BUILD)/kernel/io.o	\
		$(BUILD)/kernel/console.o \
		$(BUILD)/kernel/printk.o   \
		$(BUILD)/kernel/assert.o   \
		$(BUILD)/kernel/debug.o    \
		$(BUILD)/kernel/global.o    \
		$(BUILD)/kernel/schedule.o    \
		$(BUILD)/kernel/interrupt.o    \
		$(BUILD)/kernel/handler.o    \
		$(BUILD)/kernel/clock.o    \
		$(BUILD)/kernel/time.o    \
		$(BUILD)/kernel/rtc.o    \
		$(BUILD)/kernel/memory.o    \
		$(BUILD)/kernel/thread.o    \
		$(BUILD)/kernel/task.o    \
		$(BUILD)/kernel/gate.o    \
		$(BUILD)/lib/bitmap.o    \
		$(BUILD)/lib/string.o   \
		$(BUILD)/lib/vsprintf.o   \
		$(BUILD)/lib/syscall.o   \
		$(BUILD)/lib/list.o   \
		$(BUILD)/lib/stdlib.o   
	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)   
#编译生成system.bin文件，将elf heaer头去掉
$(BUILD)/system.bin:$(BUILD)/kernel.bin
	objcopy -O binary $< $@

#编译生成system.map文件，sort为排序
$(BUILD)/system.map:$(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin      \
	$(BUILD)/system.map		  \

# 创建一个 16M 的硬盘镜像
	yes | bximage -q -hd=16 -mode=create  -imgmode=flat $@
# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
# 将loader.bin写入硬盘
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 测试system.bin小于 100K ,否则需要修改下面的count
	test -n "$$(find $(BUILD)/system.bin -size -100k)"
# 将system.bin写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc   



.PHONY: test
test: $(BUILD)/master.img


.PHONY: clean
clean:
	rm -rf $(BUILD)
.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -unlock



QEMU:=	qemu-system-i386 \
	-m 32M \
	-boot c \
	-drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw \
	-rtc base=localtime 

.PHONY: qemu
qemu: $(BUILD)/master.img
	$(QEMU)

.PHONY: qemug
qemug: $(BUILD)/master.img
	$(QEMU) -s -S

# 将qemu格式的镜像转换为vmware格式的镜像
$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert  vmdk $< $@

.PHONY:vmdk
vmdk: $(BUILD)/master.vmdk