IPL			= ipl
ASMHEAD		= asmhead
BUILD_DIR	= build
IPL_NAME	= $(IPL).nas
OS_MAIN		= $(ASMHEAD).nas
SYS_NAME	= $(BUILD_DIR)/$(ASMHEAD).sys
IMAGE_NAME	= $(BUILD_DIR)/$(IPL).img
BIN_NAME	= $(BUILD_DIR)/$(IPL).bin

.PHONY: boot

boot: $(IMAGE_NAME)
	qemu-system-i386 -drive file=$(IMAGE_NAME),format=raw,if=floppy -boot a

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_NAME): $(IPL_NAME) $(BUILD_DIR)
	nasm $(IPL_NAME) -o $(BIN_NAME)

$(SYS_NAME): $(OS_MAIN) $(BUILD_DIR)
	nasm $(OS_MAIN) -o $(SYS_NAME)

$(IMAGE_NAME): $(BIN_NAME) $(SYS_NAME)
	mformat -f 1440 -C -B $(BIN_NAME) -i $(IMAGE_NAME) ::
	mcopy -i $(IMAGE_NAME) $(SYS_NAME) ::
