#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void process_request( zmq::message_t& request ) {
	std::cout << "Received:\n" << request.to_string() << std::endl;
}

int main () {
	auto context = zmq::context_t{1};	// init the zmq context with single IO thread
	auto socket = zmq::socket_t{context, zmq::socket_type::rep};	// 'rep' (reply) type socket

	socket.bind("tcp://*:15035");

	while(true) {
		auto request = zmq::message_t{};

		socket.recv(request, zmq::recv_flags::none);
		process_request(request);
		
		std::this_thread::sleep_for(100ms);
	}
}

