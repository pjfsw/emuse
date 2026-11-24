cmake_minimum_required(VERSION 3.20)

project(kernel NONE)

set(VBCC      vbccm68k)
set(VASM      vasmm68k_mot)
set(VLINK     vlink)

# entry.asm MUST be first: the loader starts execution at the
# beginning of the first code hunk.
set(ASM_SOURCES
    arch/m68k/entry.asm
    arch/m68k/bios.asm
)

set(C_SOURCES
    src/main.c
)

set(ROOT_ASM_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set(KERNEL_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# vbcc runtime helpers (e.g. 32-bit division) are taken from libvc
set(VBCC_LIB_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../apps/edit/lib
)

file(GLOB KERNEL_HEADERS ${KERNEL_INCLUDE}/*.h)

set(OBJECTS)

# Hand-written ASM -> object
foreach(src ${ASM_SOURCES})
    get_filename_component(name ${src} NAME_WE)

    set(obj ${CMAKE_CURRENT_BINARY_DIR}/${name}.o)

    add_custom_command(
        OUTPUT ${obj}
        COMMAND ${VASM}
                -quiet
                -I${ROOT_ASM_INCLUDE}
                -Fhunk
                -o ${obj}
                ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${src}
        VERBATIM
    )

    list(APPEND OBJECTS ${obj})
endforeach()

# C -> ASM -> object
foreach(src ${C_SOURCES})
    get_filename_component(name ${src} NAME_WE)

    set(generated_asm ${CMAKE_CURRENT_BINARY_DIR}/${name}.asm)
    set(obj           ${CMAKE_CURRENT_BINARY_DIR}/${name}.o)

    add_custom_command(
        OUTPUT ${generated_asm}
        COMMAND ${VBCC}
                -I${KERNEL_INCLUDE}
                -quiet
                -c99
                -cpu=68000
                -O=1
                -o=${generated_asm}
                ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${src} ${KERNEL_HEADERS}
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${obj}
        COMMAND ${VASM}
                -quiet
                -no-opt
                -Fhunk
                -o ${obj}
                ${generated_asm}
        DEPENDS ${generated_asm}
        VERBATIM
    )

    list(APPEND OBJECTS ${obj})
endforeach()

# Objects -> HUNK executable
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.exe
    COMMAND ${VLINK}
            -s
            -bamigahunk
            -M${CMAKE_CURRENT_BINARY_DIR}/kernel.map
            -o ${CMAKE_CURRENT_BINARY_DIR}/kernel.exe
            ${OBJECTS}
            -L${VBCC_LIB_DIR}
            -lvc
    DEPENDS ${OBJECTS}
    VERBATIM
)

# HUNK executable -> hex file for the boot menu serial loader
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.hex
    COMMAND python3
            ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/mkhex.py
            ${CMAKE_CURRENT_BINARY_DIR}/kernel.exe
            ${CMAKE_CURRENT_BINARY_DIR}/kernel.hex
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.exe
    VERBATIM
)

add_custom_target(kernel ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.exe ${CMAKE_CURRENT_BINARY_DIR}/kernel.hex
)
