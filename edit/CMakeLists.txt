cmake_minimum_required(VERSION 3.20)

project(edit NONE)

set(VBCC      vbccm68k)
set(VASM      vasmm68k_mot)
set(VLINK     vlink)


set(C_SOURCES
    src/edit.c
)

set(ASM_SOURCES
    asm/console.asm
)

set(ROOT_ASM_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/../asm/include
)

set(OBJECTS)

# C -> ASM -> object
foreach(src ${C_SOURCES})
    get_filename_component(name ${src} NAME_WE)

    set(generated_asm ${CMAKE_CURRENT_BINARY_DIR}/${name}.asm)
    set(obj           ${CMAKE_CURRENT_BINARY_DIR}/${name}.o)

    add_custom_command(
        OUTPUT ${generated_asm}
        COMMAND ${VBCC}
                -quiet
                -O=1
                -o=${generated_asm}
                ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${src}
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${obj}
        COMMAND ${VASM}
                -Fhunk
                -o ${obj}
                ${generated_asm}
        DEPENDS ${generated_asm}
        VERBATIM
    )

    list(APPEND OBJECTS ${obj})
endforeach()

# Hand-written ASM -> object
foreach(src ${ASM_SOURCES})
    get_filename_component(name ${src} NAME_WE)

    set(obj ${CMAKE_CURRENT_BINARY_DIR}/${name}.o)

    add_custom_command(
        OUTPUT ${obj}
        COMMAND ${VASM}
                -I${ROOT_ASM_INCLUDE}        
                -Fhunk
                -o ${obj}
                ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${src}
        VERBATIM
    )

    list(APPEND OBJECTS ${obj})
endforeach()

# Objects -> HUNK executable
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/edit.exe
    COMMAND ${VLINK}
            -s
            -bamigahunk
            -o ${CMAKE_CURRENT_BINARY_DIR}/edit.exe
            ${OBJECTS}
    DEPENDS ${OBJECTS}
    VERBATIM
)

add_custom_target(edit ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/edit.exe
)