# Air OS

Air is a hobbyist x86_64 operating system written in C, built on the Limine boot protocol and designed for modularity and clarity.
It includes framebuffer and serial output, memory management, structured exception handling, and a custom 8x16 font rendered via Flanterm.

## About

Air currently supports:

- Limine boot protocol with framebuffer, HHDM, memmap, and RSDP requests
- ANSI-formatted framebuffer logging via Flanterm
- Serial output via 16550 UART
- Custom 8x16 bitmap font rendering
- Interrupt Descriptor Table (IDT) setup
- Structured exception output with full register and state dump
- Physical Memory Manager (PMM)
- Virtual Memory Manager (VMM)
- Kernel heap (kheap) with automatic initialization
- Inline ASCII and hex character table rendering
- Memory map debugging via memmap_dump()

## New Updates

Latest commit: `Moved most printk usage to kprint, added debug mode`

It's what the commit said.

## TODO

- [X] Memory
	- [X] Parse Limine memory map
    - [X] Use memory map
	- [X] Physical Memory Manager
	- [X] Virtual Memory Manager

- [X] Heap
	- [X] Implement buddy allocator

- [ ] Logging and Output
	- [X] Framebuffer output via Flanterm
	- [X] Serial UART output
	- [X] kprint() with log levels
	- [ ] Add log timestamps or tick count
	- [ ] Implement log filtering by subsystem

- [ ] Exception Handling
	- [X] Dump GPRs, RIP, CS, RFLAGS, and stack
	- [X] Decode page fault error code and CR2
	- [ ] Detect and show CPU ID
	- [ ] Add basic backtrace using frame pointer

- [ ] Interrupts
	- [X] Set up IDT
	- [ ] Implement IRQ stubs
	- [ ] Add timer interrupt handlers
        - [ ] Add PIT timer interrupt handler
        - [ ] Add HPET timer interrupt handler

- [X] Virtual File System (VFS)
	- [X] Design abstract VFS interface
	- [ ] Implement several filesystems
		- [X] RamFS
			- [X] In-memory storage
			- [X] File creation
			- [X] File deletion
			- [X] Directory creation
			- [X] Directory deletion
			- [X] File read
			- [X] File write
		- [ ] FAT12
			- [ ] Boot sector parsing
			- [ ] FAT table handling
			- [ ] Root directory navigation
			- [ ] File read
		- [ ] FAT16
			- [ ] Boot sector parsing
			- [ ] FAT table handling
			- [ ] Root directory navigation
			- [ ] File read
			- [ ] File write
		- [ ] FAT32
			- [ ] Extended BPB parsing
			- [ ] Cluster chain traversal
			- [ ] Long File Name (LFN) support
			- [ ] File read
			- [ ] File write
	- [X] Add file and directory structures
	- [X] Add support for path resolution
	- [X] Integrate with existing heap/memory system
	- [ ] Add simple VFS test harness or init-time mounts

## License

This project is based on the `limine-c-template`
(previously available at github.com/limine-bootloader/limine-c-template, now unavailable),
which is licensed under the terms of the 0BSD license.

The `LICENSE` file in this repository contains the original license text.
All additional code in this project is also provided under the terms of 0BSD.

## Requirements

- GCC cross-compiler targeting x86_64-elf
- NASM
- Limine (as submodule or manually installed)
- QEMU or compatible x86_64 virtual machine
- Make (project uses GNUmakefile)
