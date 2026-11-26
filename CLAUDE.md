# Project Overview

The overall project is a 68000-based SBC that I have built, comprised of a 68000 CPU running
at 14.7 MHz, 1 MB RAM (located at $000000-$0FFFFF), 64 KB ROM (located at $F00000-$FFFFFF, mirrored several times). It has an 16C550 UART for serial and main communication with the user. It 
has an MMC/SDCard slot, connected to the hardware via SPI-interface, which is bitbanged in firmwre.

## Repo overview
The repo itself has three main purposes:
- Firmware, OS and applications for the SBC (Jofmodore SixtyEight), located in jofmodorese/. It's mostly 68000 assembly, but C code is compiled using vbcc/vasm/vlink.
  - jofmodorese/include/: shared target include files (rootlib.i, hardware.i, ...)
  - jofmodorese/bios/: ROM firmware (read-only storage support, kept stable)
  - jofmodorese/boot/: system.asm, the SYSTEM.BIN loader started by the BIOS (raw image at $1000)
  - jofmodorese/legacy/shell/: the legacy shell (SHELL.EXE, hunk format), frozen
  - jofmodorese/kernel/: the new kernel (planned), mainly C
  - jofmodorese/apps/: applications (edit/ in C, ehbasic/, utils/)
- An emulator of the SBC, written in C and SDL3, located in emu/ (emu/src, emu/include), that mimics
the hardware as closely as possible. It sets up a pts so that the enduser can communicate with the emulator using a terminal application such as picocom. The MMC is also emulated, and currently a dump of the FAT-16 image used in the hardware is stored in sdcard.bin. When I launch the 
emulator manually, i use the command line "emuse.sh -m ./sdcard.bin" to attach the SDCard image.
- Build, upload and EEPROM burn scripts are located in tools/ and are run from the repository root (e.g. tools/hunk.sh jofmodorese/legacy/shell/shell.asm).
- Additional help files for the hardware, such as PLD descriptions, located in hw/. This folder can be ignored.

The whole project can be run using CMake and is also integrated with VS Code.

## Emulator architecture
- CPU instruction decoding and execution in C. 
- Each 68k-instruction is executed atomically, and the number of cycles are 
  added after the execution, rather than per cycle emulation.
- Group 0 exceptions push the address of the next instruction as PC; the instruction completes before the exception is raised.
- Emulated memory and memory-mapped peripherals.
- UART emulation with terminal integration.
- Interrupt and exception handling.

## Design Principles
- Prioritize correctness over performance.
- Preserve existing architecture and coding style.
- Avoid introducing external dependencies.
- Do not implement additional CPU instructions unless requested.
- Prefer fixing root causes over adding workarounds.

# Development Guidelines

## General behavior
- Prefer minimal, targeted changes.
- Do not refactor unrelated code.
- Do not introduce new dependencies without approval.
- Do not modify build configurations unless explicitly requested.
- Preserve existing coding conventions and architecture.

## Before modifying code
- Read and understand the relevant implementation.
- Explain the suspected problem and proposed solution.
- Ask for approval before making substantial changes.
- Do not assume missing requirements.

## Implementation
- Make one logical change at a time.
- Avoid unnecessary abstractions or defensive code.
- Do not replace working implementations merely for stylistic reasons.
- Preserve compatibility with existing interfaces.

## Verification
- Never claim code has been tested unless tests were actually executed.
- Report what changed and what remains unverified.
- Do not silently fix unrelated issues.

## Safety
- Never delete files or discard changes without explicit approval.
- Never commit or push automatically.
- Never execute destructive commands without approval.
- Never modify files outside the project directory without approval.

## Programming style
- Adher to the existing formatting style of the code

