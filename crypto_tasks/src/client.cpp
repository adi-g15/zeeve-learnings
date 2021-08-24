#include "argparse.hpp"
#include <cstdint>
#include <stdexcept>
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <algorithm>

#include "actions.hpp"
#include "payload.pb.h"

using std::string;

string to_lower_case(string&& str) {
	string lower(str);
	std::for_each(lower.begin(), lower.end(), [](char& c){ c = tolower(c); });

	return lower;
}

string encode_payload( const string &action, const string& msg ) {
	auto payload = Payload();
	
	if ( action == "encode" ) {
		payload.set_action(AppliedAction::ENCODE);
		
		auto str = message::encode(msg);
		payload.set_payload_str(str);

	} else if ( action == "hash" ) {
		payload.set_action(AppliedAction::HASH);

		auto str = message::hash(msg);
		payload.set_payload_str(str);
	} else if ( action == "sign" ) {
		payload.set_action(AppliedAction::SIGN);

		auto str = message::sign(msg);
		payload.set_payload_str(str);
	} else if ( action == "encrypt" ) {
		payload.set_action(AppliedAction::ENCRYPT);

		auto encrypted_bytes = message::encrypt_bytes({
			reinterpret_cast<const uint8_t*>(msg.data()),
			reinterpret_cast<const uint8_t*>(msg.data()) + msg.size()
		});
		payload.set_payload_str( util::bytes_to_hex_string( encrypted_bytes.message ));
		auto metadata = payload.mutable_metadata();
		metadata->insert({
			{"key", util::bytes_to_hex_string(encrypted_bytes.key )},
			{"iv", util::bytes_to_hex_string(encrypted_bytes.IV) }
		});
	} else throw std::runtime_error("No Such Action !");

	return payload.SerializeAsString();
}

int main (int argc, const char *argv[]) {
	/*Create a simple arg parser to take two args: action, message*/
	argparse::ArgumentParser program("client");

	program.add_argument("action")
		.help("Chose action among: encode, hash, sign, encrypt");

	program.add_argument("message")
		.help("The message data (a string, can pipe a file)");

	try {
		program.parse_args(argc, argv);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		std::clog << program;
		exit(0);
	}

	auto action = to_lower_case( program.get<string>("action") );
	auto message = program.get<string>("message");

	/*Now, we create the zmq context, and send the message*/
	auto context = zmq::context_t{1};
	/*This is a req (request) socket*/
	auto socket = zmq::socket_t{context, zmq::socket_type::req};

	socket.connect("tcp://localhost:15035");

	auto payload = encode_payload(action, message);

	socket.send(zmq::buffer(payload), zmq::send_flags::none);
}
