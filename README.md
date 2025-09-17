# Air OS

Air is a hobbyist x86_64 operating system written in C, built on the Limine boot protocol and designed for modularity and clarity.
It includes framebuffer and serial output, memory management, structured exception handling, and a custom 8x16 font rendered via Flanterm.

## About

Air OS currently supports:

- Serial output via 16550 UART
- Custom 8x16 bitmap font rendering
- Interrupt Descriptor Table (IDT) setup
- Physical Memory Manager (PMM)
- Virtual Memory Manager (VMM)
- Kernel Heap (Kheap) using the Buddy allocator
- Virtual File System (VFS)

## New Updates

Latest commit: i forgor

- Added IRQ handling
- Added PIT handling
- Modified kprint()
  - kprint() now uses the PIT
  - kprint() uses bright blue instead of bright green for LOG_DEBUG

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
	- [X] Add log timestamps or tick count
	- [ ] Implement log filtering by subsystem

- [ ] Exception Handling
	- [X] Dump GPRs, RIP, CS, RFLAGS, and stack
	- [X] Decode page fault error code and CR2
	- [ ] Detect and show CPU ID
	- [ ] Add basic backtrace using frame pointer

- [X] Interrupts
	- [X] Set up IDT
	- [X] Implement IRQ stubs
	- [X] Add timer interrupt handlers
        - [X] Add PIT timer interrupt handler
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

- [ ] Scheduler / Tasking / SMP
	- [ ] Basic task struct and context switch
	- [ ] Round-robin or priority-based scheduler
	- [ ] Support for kernel threads
	- [ ] Multiprocessing (SMP) initialization
	- [ ] Inter-processor interrupts (IPI)
	- [ ] Per-CPU scheduling and run queues

## License

This project is based on the `limine-c-template`
(previously available at github.com/limine-bootloader/limine-c-template, now unavailable),
which is licensed under the terms of the 0BSD license.

The `LICENSE` file in this repository contains the original license text.
All additional code in this project is also provided under the terms of 0BSD.

## Requirements

- GCC/CC
- NASM
- [Limine](https://codeberg.org/Limine/Limine) (bootloader)
- QEMU or compatible x86_64 virtual machine
- Make
