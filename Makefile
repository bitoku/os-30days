BASE_NAME	= helloos
BUILD_DIR	= build
NAS_NAME	= $(BASE_NAME).nas
IMAGE_NAME	= $(BUILD_DIR)/$(BASE_NAME).img
BIN_NAME	= $(BUILD_DIR)/$(BASE_NAME).bin

.PHONY: boot

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_NAME): $(NAS_NAME) $(BUILD_DIR)
	nasm $(NAS_NAME) -o $(BIN_NAME)

$(IMAGE_NAME): $(BIN_NAME)
	mformat -f 1440 -C -B $(BIN_NAME) -i $(IMAGE_NAME) ::

boot: $(IMAGE_NAME)
	qemu-system-i386 -drive file=$(IMAGE_NAME),format=raw,if=floppy -boot a

