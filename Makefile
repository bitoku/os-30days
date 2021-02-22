BASE_NAME	= helloos
IMAGE_NAME	= $(BASE_NAME).img
NAS_NAME	= $(BASE_NAME).nas

.PHONY: boot

$(IMAGE_NAME): $(NAS_NAME)
	nasm $(NAS_NAME) -o $(IMAGE_NAME)

boot: $(IMAGE_NAME)
	qemu-system-i386 -drive file=$(IMAGE_NAME),format=raw,if=floppy -boot a

