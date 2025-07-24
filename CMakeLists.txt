cmake_minimum_required(VERSION 3.10)
project(SocketServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find thread library
find_package(Threads REQUIRED)

# Server executable
add_executable(server
    src/server.cpp
    src/services.cpp
    src/interceptors.cpp
    src/server_main.cpp
)

target_link_libraries(server Threads::Threads)

# Client executable
add_executable(client
    src/client.cpp
    src/interceptors.cpp
    src/client_main.cpp
)

target_link_libraries(client Threads::Threads)

# Include directories
target_include_directories(server PRIVATE include)
target_include_directories(client PRIVATE include)

# Compiler flags
target_compile_options(server PRIVATE -Wall -Wextra)
target_compile_options(client PRIVATE -Wall -Wextra)

# Set output directories
set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Print configuration info
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Directory: ${CMAKE_BINARY_DIR}/bin") 