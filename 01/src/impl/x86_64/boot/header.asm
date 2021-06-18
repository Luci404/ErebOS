section .multiboot_header
header_start:
	; https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
	; https://www.gnu.org/software/grub/manual/multiboot2/html_node/multiboot2_002eh.html#multiboot2_002eh
	; https://intermezzos.github.io/book/first-edition/multiboot-headers.html
	
	dd 0xe85250d6 					; magic number
	dd 0 							; protected mode i386
	dd header_end - header_start	; header length

	; checksum
	dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

	; end tag
	dw 0	; type
	dw 0	; flags
	dd 8	; size
header_end: