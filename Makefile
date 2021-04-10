IPL			= ipl
ASMHEAD		= asmhead
BASE		= haribote
BUILD_DIR	= build
NASKFUNC	= naskfunc
NASKFUNC_NAS	= $(NASKFUNC).nas
NASKFUNC_O	= $(NASKFUNC).o
IPL_NAME	= $(IPL).nas
ASMHEAD_NAS = $(ASMHEAD).nas
BOOTPACK	= bootpack
BOOTPACK_SRC	= $(BOOTPACK).c
BOOTPACK_HRB	= $(BUILD_DIR)/$(BOOTPACK).hrb
ASMHEAD_BIN	= $(BUILD_DIR)/$(ASMHEAD).bin
IMAGE_NAME	= $(BUILD_DIR)/$(IPL).img
SYS_NAME	= $(BUILD_DIR)/$(BASE).sys
BIN_NAME	= $(BUILD_DIR)/$(IPL).bin

.PHONY: boot

boot: $(IMAGE_NAME)
	qemu-system-i386 -drive file=$(IMAGE_NAME),format=raw,if=floppy -boot a

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_NAME): $(IPL_NAME) $(BUILD_DIR)
	nasm $(IPL_NAME) -o $(BIN_NAME)

$(ASMHEAD_BIN): $(ASMHEAD_NAS) $(BUILD_DIR)
	nasm $(ASMHEAD_NAS) -o $(ASMHEAD_BIN)

$(NASKFUNC_O): $(NASKFUNC_NAS)
	nasm -g -f elf $(NASKFUNC_NAS) -o $(NASKFUNC_O)

$(BOOTPACK_HRB): $(BOOTPACK_SRC)
	i386-elf-gcc -march=i486 -m32 -nostdlib -T hrb.ld $(BOOTPACK_SRC) -o $(BOOTPACK_HRB)

$(SYS_NAME): $(ASMHEAD_BIN) $(BOOTPACK_HRB)
	cat $(ASMHEAD_BIN) $(BOOTPACK_HRB) > $(SYS_NAME)

$(IMAGE_NAME): $(BIN_NAME) $(SYS_NAME)
	mformat -f 1440 -C -B $(BIN_NAME) -i $(IMAGE_NAME) ::
	mcopy -i $(IMAGE_NAME) $(SYS_NAME) ::
