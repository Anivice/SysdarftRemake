# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(EmbedPythonHTTPServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        -gdwarf-4                            # Generate DWARF version 4 debug information

        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # Code coverage options
        -fprofile-arcs                       # Enable code coverage instrumentation
        -ftest-coverage                      # Generate coverage test data
        -fPIC --pie
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

        # Stack protection
        -fstack-protector-all                   # Link with stack protection for all functions

        # Code coverage options
        -fprofile-arcs                          # Enable code coverage instrumentation
        -ftest-coverage                         # Generate coverage test data
        -fPIC --pie
)

add_compile_options(${compiler_options})
add_link_options(${linker_options})

# Set CMake policy CMP0167 to NEW to use BoostConfig.cmake instead of FindBoost
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Find Boost libraries without specifying 'process' as a separate component
find_package(Boost 1.70 REQUIRED COMPONENTS system filesystem thread)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please install boost-devel.")
endif()

# Find nlohmann_json
find_package(nlohmann_json 3.2.0 REQUIRED)

# Find libcurl
find_package(CURL REQUIRED)

# Find Threads
find_package(Threads REQUIRED)

function(add_object OBJECTNAME FILE)
    add_library(${OBJECTNAME} OBJECT ${FILE})
    target_include_directories(${OBJECTNAME} PUBLIC ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../src/include)
endfunction()

add_object(websocket_session src/websocket_session.cpp)
add_object(http src/http.cpp)
add_object(backend src/backend.cpp)
add_object(fallback src/fallback.cpp)
add_object(input src/input.cpp)

# Add the executable target
add_library(sysdarft_backend SHARED src/main.cpp)
target_link_libraries(sysdarft_backend PUBLIC websocket_session http backend fallback input)

# Include directories
target_include_directories(sysdarft_backend PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/include
)

# Link libraries
target_link_libraries(sysdarft_backend PRIVATE
        Boost::system
        Boost::filesystem
        Boost::thread
        nlohmann_json::nlohmann_json
        CURL::libcurl
        Threads::Threads
)

add_custom_target(CopyPythonFiles ALL
        COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/resources/"
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/resources/index.html" "${CMAKE_CURRENT_BINARY_DIR}/resources/"
        VERBATIM
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/index.html
        COMMENT "Copying WebPage files...")
