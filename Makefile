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
HANKAKU		= hankaku.txt
CONV_HANKAKU	= convHankaku.c
BOOTPACK_SRC	= $(BOOTPACK).c
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

$(CONV_HANKAKU_O): $(CONV_HANKAKU)
	gcc $(CONV_HANKAKU) -o $(CONV_HANKAKU_O)

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

$(BOOTPACK_HRB): $(BOOTPACK_SRC) $(HANKAKU_C) $(NASKFUNC_O)
	i386-elf-gcc -fno-builtin -march=i486 -m32 -nostdlib -T hrb.ld -g $(BOOTPACK_SRC) $(NASKFUNC_O) $(HANKAKU_C) -o $(BOOTPACK_HRB)

$(SYS_NAME): $(ASMHEAD_BIN) $(BOOTPACK_HRB)
	cat $(ASMHEAD_BIN) $(BOOTPACK_HRB) > $(SYS_NAME)

$(IMAGE_NAME): $(BIN_NAME) $(SYS_NAME)
	mformat -f 1440 -C -B $(BIN_NAME) -i $(IMAGE_NAME) ::
	mcopy -i $(IMAGE_NAME) $(SYS_NAME) ::
