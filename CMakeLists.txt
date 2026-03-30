cmake_minimum_required(VERSION 3.15)
project(CFRProject VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
add_executable(main src/main.cpp)
add_executable(dudo src/dudo.cpp)

# Install target (optional)
install(TARGETS main RUNTIME DESTINATION bin)
install(TARGETS dudo RUNTIME DESTINATION bin)
