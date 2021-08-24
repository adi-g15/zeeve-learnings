#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "actions.hpp"
#include "payload.pb.h"

using namespace std::chrono_literals;
using std::string;

string action_to_string(const AppliedAction action) {
	switch (action)
	{
		case AppliedAction::ENCODE:
			return "AppliedAction::ENCODE";
		case AppliedAction::HASH:
			return "AppliedAction::HASH";
		case AppliedAction::ENCRYPT:
			return "AppliedAction::ENCRYPT";
		case AppliedAction::SIGN:
			return "AppliedAction::SIGN";
		case AppliedAction::CERTIFICATE:
			return "AppliedAction::CERTIFICATE";
		default:
			return "";
	}
}

auto print_payload(const Payload& payload) {
	using std::cout, std::endl;

	cout << "{\n"
		 << "\taction  : \"" << action_to_string( payload.action() ) << "\"\n"
		 << "\tmessage : \"" << payload.payload_str() << "\"\n"
		 << "\tmetadata: {";

	auto metadata = payload.metadata();
	if( metadata.empty() )	cout << "}";
	else cout << '\n';

	for(const auto &p : metadata) {
		cout << "\t\t" << p.first << ": \"" << p.second << "\",\n";
	}

	if( !metadata.empty() )	cout << "\t}";

	cout << "\n}\n";
}

auto process_request( zmq::message_t& request ) {
	auto payload = Payload();
	payload.ParseFromString( request.to_string() );

	std::cout << "Received payload is: \n";
	print_payload(payload);

	switch( payload.action() ) {
		case AppliedAction::ENCODE:
			std::cout << "Decoded string is:\n\"" << message::decode( payload.payload_str() ) << '\"' << std::endl;
			break;
		case AppliedAction::HASH:
			std::cout << "Received hash is: \"" << payload.payload_str() << '\"' << std::endl;
			break;
		case AppliedAction::ENCRYPT: {	// A block is needed in switch-case when initialising creating objects, such as strings vectors
			auto cipher = payload.payload_str();

			auto metadata = payload.metadata();
			auto key = metadata["key"];
			auto iv = metadata["iv"];

			auto plaintext_bytes = message::decrypt_bytes(
				util::hex_string_to_bytes(cipher),
				util::hex_string_to_bytes(key),
				util::hex_string_to_bytes(iv)
			);

			auto plaintext = std::string(
				(char*)plaintext_bytes.data(),
				(char*)plaintext_bytes.data() + plaintext_bytes.size()
			);

			plaintext.push_back('\0');

			std::cout << "Decrypted content is:\n\""
					  << plaintext << "\"\n";
			break;
		}
		case AppliedAction::SIGN:
			break;
		case AppliedAction::CERTIFICATE:
			break;
		default:
			std::cerr << "Action not known !\n";
			std::cerr << request.to_string();
			return 1;
	}

	return 0;
}

int main () {
	auto context = zmq::context_t{1};	// init the zmq context with single IO thread
	auto socket = zmq::socket_t{context, zmq::socket_type::rep};	// 'rep' (reply) type socket

	socket.bind("tcp://*:15035");
	std::cout << "Bind socket to tcp://*:15035 succeeded...\n";

	while(true) {
		auto request = zmq::message_t{};

		socket.recv(request, zmq::recv_flags::none);
		process_request(request);

		socket.send(zmq::buffer(std::string("Received OK (200)")), zmq::send_flags::none);
	}

	return 0;
}

