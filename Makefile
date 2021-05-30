IPL			= ipl
ASMHEAD		= asmhead
BASE		= haribote
BUILD_DIR	= build
NASKFUNC	= naskfunc
NASKFUNC_NAS	= $(NASKFUNC).nas
NASKFUNC_O	= $(BUILD_DIR)/$(NASKFUNC).o
IPL_NAME	= $(IPL).nas
ASMHEAD_NAS = $(ASMHEAD).nas
BOOTPACK	= bootpack
GRAPHIC		= graphic
GRAPHIC_C	= $(GRAPHIC).c
GRAPHIC_O	= $(BUILD_DIR)/$(GRAPHIC).o
DSCTBL		= dsctbl
DSCTBL_C	= $(DSCTBL).c
DSCTBL_O	= $(BUILD_DIR)/$(DSCTBL).o
FIFO		= fifo
FIFO_C		= $(FIFO).c
FIFO_O		= $(BUILD_DIR)/$(FIFO).o
INT			= int
INT_C		= $(INT).c
INT_O		= $(BUILD_DIR)/$(INT).o
HANKAKU		= hankaku.txt
HANKAKU_O	= $(BUILD_DIR)/hankaku.o
CONV_HANKAKU	= convHankaku
CONV_HANKAKU_C	= $(CONV_HANKAKU).c
BOOTPACK_C	= $(BOOTPACK).c
BOOTPACK_O	= $(BUILD_DIR)/$(BOOTPACK).o
CONV_HANKAKU_O	= $(BUILD_DIR)/$(CONV_HANKAKU).o
HANKAKU_C		= $(BUILD_DIR)/$(HANKAKU).c
BOOTPACK_HRB	= $(BUILD_DIR)/$(BOOTPACK).hrb
ASMHEAD_BIN	= $(BUILD_DIR)/$(ASMHEAD).bin
IMAGE_NAME	= $(BUILD_DIR)/$(IPL).img
SYS_NAME	= $(BUILD_DIR)/$(BASE).sys
BIN_NAME	= $(BUILD_DIR)/$(IPL).bin

.PHONY: clean

boot: $(IMAGE_NAME)
	qemu-system-i386 -drive file=$(IMAGE_NAME),format=raw,if=floppy -boot a

clean:
	rm -rf $(BUILD_DIR)

$(CONV_HANKAKU_O): $(CONV_HANKAKU_C)
	gcc $(CONV_HANKAKU_C) -o $(CONV_HANKAKU_O)

$(HANKAKU_C): $(CONV_HANKAKU_O) $(HANKAKU)
	./$(CONV_HANKAKU_O) $(HANKAKU) $(HANKAKU_C)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_NAME): $(IPL_NAME) $(BUILD_DIR)
	nasm $(IPL_NAME) -o $(BIN_NAME)

$(ASMHEAD_BIN): $(ASMHEAD_NAS) $(BUILD_DIR)
	nasm $(ASMHEAD_NAS) -o $(ASMHEAD_BIN)

$(NASKFUNC_O): $(NASKFUNC_NAS)
	nasm -g -f elf $(NASKFUNC_NAS) -o $(NASKFUNC_O)

$(BOOTPACK_HRB): $(BOOTPACK_O) $(NASKFUNC_O) $(GRAPHIC_O) $(DSCTBL_O) $(HANKAKU_O) $(INT_O) $(FIFO_O)
	i386-elf-gcc -fno-builtin -march=i486 -m32 -nostdlib -T hrb.ld -g $(BOOTPACK_O) $(GRAPHIC_O) $(DSCTBL_O) $(NASKFUNC_O) $(HANKAKU_O) $(INT_O) $(FIFO_O) -o $(BOOTPACK_HRB)

$(BUILD_DIR)/%.o: %.c
	i386-elf-gcc -fno-builtin -march=i486 -m32 -nostdlib -c $*.c -o $@

$(HANKAKU_O): $(HANKAKU_C)
	i386-elf-gcc -fno-builtin -march=i486 -m32 -nostdlib -c $(HANKAKU_C) -o $(HANKAKU_O)

$(SYS_NAME): $(ASMHEAD_BIN) $(BOOTPACK_HRB)
	cat $(ASMHEAD_BIN) $(BOOTPACK_HRB) > $(SYS_NAME)

$(IMAGE_NAME): $(BIN_NAME) $(SYS_NAME)
	mformat -f 1440 -C -B $(BIN_NAME) -i $(IMAGE_NAME) ::
	mcopy -i $(IMAGE_NAME) $(SYS_NAME) ::
