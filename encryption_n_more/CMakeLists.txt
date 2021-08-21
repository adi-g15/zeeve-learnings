cmake_minimum_required(VERSION 3.18)

project(encryption_n_more)

include(cmake/CPM.cmake)
CPMAddPackage("gh:p-ranav/argparse@2.1")
CPMAddPackage(
	GITHUB_REPOSITORY zeromq/cppzmq
	VERSION 4.7.1
	OPTIONS "CPPZMQ_BUILD_TESTS NO"
	)

add_executable(client "src/client.cpp")
add_executable(server "src/server.cpp")

include_directories("include")
target_link_libraries(client cppzmq)
target_link_libraries(server cppzmq)

