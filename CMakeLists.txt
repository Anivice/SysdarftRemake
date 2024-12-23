cmake_minimum_required(VERSION 3.29)

project(Sysdarft C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Universal compiler and linker flags
set(compiler_options
        # Warnings and diagnostics
        -Wall                                # Enable common warnings
        -Wextra                              # Enable extra warnings
        -Wpedantic                           # Strict compliance with the standard
        -Wunused                             # Warn about unused variables, functions, etc.
        -Wuninitialized                      # Warn if variables are used uninitialized
        -fdiagnostics-show-option            # Show which option triggered the warning
        -fdiagnostics-color=always           # Enable colored diagnostics for better readability

        # Debugging and stack protection
        -g3                                  # Maximum debug information, including macro expansions
        -O0
        -fstack-usage                        # Generate stack usage info for each function
        -fstack-protector-all                # Protect all functions with a stack canary to prevent stack overflow attacks
        #-D_FORTIFY_SOURCE=2                  # Buffer overflow detection on safer libc functions (e.g., memcpy).
        # You need to enable optimization for _FORTIFY_SOURCE to work!
        -gdwarf-4                            # Generate DWARF version 4 debug information

        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # Sanitize memory and thread issues
        -fsanitize=address                   # Detect illegal memory access such as buffer overflows and use-after-free
        -fsanitize=undefined                 # Detect undefined behavior like integer overflows and null dereferencing
        # Uncomment if debugging threading issues:
        # -fsanitize=thread                   # Ensure thread safety by detecting data races

        # Code coverage options
        -fprofile-arcs                       # Enable code coverage instrumentation
        -ftest-coverage                      # Generate coverage test data
        -lasan -lubsan -fPIC --pie
)

set(linker_options
        # Linker options for memory safety, thread safety, and verbose debugging
        -Wl,--no-omagic                         # Prevent the generation of object files in memory; useful for debugging
        -Wl,--as-needed                         # Only link libraries that are actually needed to reduce binary size
        -Wl,--fatal-warnings                    # Treat all linker warnings as errors to catch issues early
        -Wl,-z,relro                            # Read-only relocations to prevent certain memory exploits (optional)
        -Wl,-z,now                              # Fully resolve all symbols during the link time for extra safety
        -Wl,-z,noexecstack                      # Prevent execution of code on the stack (security hardening)
        -Wl,-z,defs                             # Ensure all symbols are defined, and prevent undefined symbols
        -Wl,-O0

        -gdwarf-4                               # Generate detailed debug information for the linker
        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # AddressSanitizer and UndefinedBehaviorSanitizer linking
        -fsanitize=address                      # Link the AddressSanitizer runtime for memory integrity
        -fsanitize=undefined                    # Link the UndefinedBehaviorSanitizer for detecting undefined behavior
        # Uncomment if debugging threading issues:
        # -fsanitize=thread                       # Link the ThreadSanitizer runtime for thread safety

        # Stack protection
        -fstack-protector-all                   # Link with stack protection for all functions

        # Code coverage options
        -fprofile-arcs                          # Enable code coverage instrumentation
        -ftest-coverage                         # Generate coverage test data
        -lasan -lubsan -fPIC --pie --whole-file
)

add_compile_options(${compiler_options})
add_link_options(${linker_options})

# Read and process the information file
set(information_file "${CMAKE_CURRENT_SOURCE_DIR}/sysdarft_info.txt")
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${information_file}")
file(READ "${information_file}" SYSDARFT_INFO_CONTENT)
string(REPLACE "\n" "\\n" SYSDARFT_INFO_CONTENT_ESCAPED "${SYSDARFT_INFO_CONTENT}")

add_compile_definitions(SYSDARFT_VERSION="0.0.1")
add_compile_definitions(SYSDARFT_INFORMATION="${SYSDARFT_INFO_CONTENT_ESCAPED}")

# Use ccache to significantly speed up compiling process
find_program(CCACHE_PROGRAM ccache REQUIRED)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# Define include directories globally (can be refined per target)
include_directories(src/include)

# Debug:
add_library(sysdarft_debug OBJECT src/debug/debug.cpp)
target_include_directories(sysdarft_debug PUBLIC src/include)

# Message Map:
add_library(sysdarft_msg_map OBJECT src/modular/msg_map.cpp)
target_include_directories(sysdarft_msg_map PUBLIC src/include)

# Modular:
add_library(sysdarft_modular OBJECT
        src/modular/module.cpp)
target_include_directories(sysdarft_modular PUBLIC src/include)

add_subdirectory(example_modules)

# Config:
add_library(sysdarft_config OBJECT src/config/config.cpp)
target_include_directories(sysdarft_config PUBLIC src/include)
add_custom_target(CopyConfigFiles ALL
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/tests/good_example.conf" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/tests/bad_example.conf" "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
        DEPENDS tests/good_example.conf tests/bad_example.conf
        COMMENT "Copying config files...")

# Global Event:
add_library(sysdarft_global OBJECT src/modular/global.cpp)
target_include_directories(sysdarft_global PUBLIC src/include)

# CLI:
find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)
add_library(sysdarft_cli OBJECT
        src/cli/cli.cpp
        src/cli/auto_completion.cpp
        src/cli/utils.cpp
        src/cli/ModuleForest.cpp)
target_include_directories(sysdarft_cli PUBLIC src/include ${READLINE_INCLUDE_DIRS})
target_link_libraries(sysdarft_cli PUBLIC ${READLINE_LIBRARIES})

# Curses:
find_package(Curses REQUIRED)
add_library(sysdarft_ncurses OBJECT
        src/ui/ui_curses.cpp)
target_include_directories(sysdarft_ncurses PUBLIC src/include)
target_link_libraries(sysdarft_ncurses PUBLIC ${CURSES_LIBRARIES})

# Sysdarft Shared Library:
add_library(sysdarft SHARED src/sysdarft.cpp)
target_include_directories(sysdarft PUBLIC src/include)
target_link_libraries(sysdarft PUBLIC
        sysdarft_debug
        sysdarft_msg_map
        sysdarft_modular
        sysdarft_config
        sysdarft_global
        sysdarft_cli
        sysdarft_ncurses)

enable_testing()

# Function to add unit tests
function(add_unit_test
        TARGET_NAME
        TEST_FILE
)
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TARGET_NAME} PUBLIC sysdarft)
    add_test(NAME "unit_test_${TARGET_NAME}" COMMAND $<TARGET_FILE:${TARGET_NAME}>)
endfunction()

add_unit_test(test.log tests/test.log.cpp)
add_unit_test(test.sysdarft_error tests/test.sysdarft_error.cpp)
add_unit_test(test.msgmap tests/test.msgmap.cpp)
add_unit_test(test.modular tests/test.modular.cpp)
add_unit_test(test.config tests/test.config.cpp)
add_unit_test(test.cli tests/test.cli.cpp)
add_unit_test(test.curses tests/test.curses.cpp)
add_unit_test(test.pure.curse tests/test.pure.curses.cpp)

# Console Executable:
add_executable(sysdarft-system src/main.cpp)
target_link_libraries(sysdarft-system PUBLIC sysdarft)

# Assembler
add_executable(assembler
        utils/shared/instruction.h
        utils/Assembler/encode.target.cpp
        utils/Assembler/encode.instruction.cpp
        utils/Assembler/encode.misc.cpp
)
target_include_directories(assembler PUBLIC utils/shared src/include)
target_link_libraries(assembler PUBLIC sysdarft)
#add_unit_test(test.encode.target utils/tests/test.encode.target.cpp)
#target_include_directories(test.encode.target PUBLIC utils/shared)

# Modules
link_libraries(sysdarft)
add_subdirectory(gui_module)
