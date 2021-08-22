cmake_minimum_required(VERSION 3.18)

project(encryption_n_more)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(CMakeCargo)

enable_language(Rust)

include(CPM)
CPMAddPackage("gh:p-ranav/argparse@2.1")
CPMAddPackage(
	GITHUB_REPOSITORY zeromq/cppzmq
	VERSION 4.7.1
	OPTIONS "CPPZMQ_BUILD_TESTS NO"
	)

add_executable(client "src/client.cpp")
add_executable(server "src/server.cpp")

add_subdirectory(rust-lib)

include_directories("include")
include_directories("rust-lib")

# These additional dependencies need to be linked for the C library or something... or atleast this CMake-Rust to work
if (WIN32)
target_link_libraries(client ws2_32 userenv advapi32)
target_link_libraries(server ws2_32 userenv advapi32)
elseif (APPLE)
target_link_libraries(client m c System resolv)
target_link_libraries(server m c System resolv)
endif()

find_package(openssl)
find_package(ssl)
find_package(crypto)
find_package(dl)	# dlsym symbol not found error
find_package(Threads REQUIRED)

target_link_libraries(client dl Threads::Threads ssl crypto cppzmq rust-lib)
target_link_libraries(server Threads::Threads ssl crypto cppzmq rust-lib)

