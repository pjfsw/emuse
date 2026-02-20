cmake_minimum_required(VERSION 3.20)
project(emuse C)

# Generate compile_commands.json for VS Code / Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)

# 1. SDL3 Setup
# Ensure SDL3 is installed on your system (via brew, apt, or vcpkg)
find_package(SDL3 REQUIRED)

# 2. VASM Integration
# This assumes 'vasmm68k_mot' is in your PATH. 
# We output a raw binary (.bin) for the emulator to load into its RAM buffer.
set(ASM_SRC "${CMAKE_CURRENT_SOURCE_DIR}/asm/boot.asm")
set(ASM_BIN "${CMAKE_CURRENT_BINARY_DIR}/boot.bin")

add_custom_command(
    OUTPUT ${ASM_BIN}
    COMMAND vasmm68k_mot -Fbin -nosym -spaces ${ASM_SRC} -o ${ASM_BIN}
    DEPENDS ${ASM_SRC}
    COMMENT "Assembling 68k boot code with vasm"
)

# Create a target for the binary so it builds automatically
add_custom_target(generate_boot_code DEPENDS ${ASM_BIN})

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