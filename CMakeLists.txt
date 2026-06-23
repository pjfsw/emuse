cmake_minimum_required(VERSION 3.20)
project(emuse C)

# Generate compile_commands.json for VS Code / Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

add_compile_definitions(_XOPEN_SOURCE=600)

add_compile_options(
    -Wunused-value
    -Werror=return-type
    -Werror
)

# 1. SDL3 Setup
# Ensure SDL3 is installed on your system (via brew, apt, or vcpkg)
find_package(SDL3 REQUIRED)

# 2. VASM Integration
# This assumes 'vasmm68k_mot' is in your PATH. 
# We output a raw binary (.bin) for the emulator to load into its RAM buffer.
set(ASM_SRC "${CMAKE_CURRENT_SOURCE_DIR}/asm/boot.asm")
set(ASM_BIN "${CMAKE_CURRENT_BINARY_DIR}/boot.bin")
set(ASM_SYM "${CMAKE_CURRENT_BINARY_DIR}/boot.sym")

add_custom_command(
    OUTPUT ${ASM_BIN}
    COMMAND vasmm68k_mot -ignore-mult-inc -Fbin -nosym -no-opt -spaces -L ${ASM_SYM} ${ASM_SRC} -o ${ASM_BIN}
    DEPENDS ${ASM_SRC}
    COMMENT "Assembling 68k boot code with vasm"
)

# Create a target for the binary so it builds automatically
add_custom_target(generate_boot_code DEPENDS ${ASM_BIN})

set(ROM_BIN ${CMAKE_BINARY_DIR}/boot.bin)
set(ROM_ODD ${CMAKE_BINARY_DIR}/boot_odd.bin)
set(ROM_EVEN ${CMAKE_BINARY_DIR}/boot_even.bin)

add_custom_command(
    OUTPUT ${ROM_EVEN} ${ROM_ODD}
    COMMAND python3 ${CMAKE_SOURCE_DIR}/split_rom.py
            ${ROM_BIN}
            ${ROM_EVEN}
            ${ROM_ODD}
    DEPENDS ${ROM_BIN}
    COMMENT "Splitting ROM into HI/LO images"
)

add_custom_target(split_rom ALL
    DEPENDS ${ROM_EVEN} ${ROM_ODD}
)

# 3. Executable Setup
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} generate_boot_code)

# 4. Includes & Linking
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Define the path to the boot bin so the C code knows where to find it
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    BOOT_BIN_PATH="${ASM_BIN}"
)