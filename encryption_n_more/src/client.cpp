#include <argparse/argparse.hpp>
#include <stdexcept>
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <algorithm>

#include "actions.hpp"

using std::string;

string to_lower_case(string&& str) {
	string lower(str);
	std::for_each(lower.begin(), lower.end(), [](char& c){ c = tolower(c); });

	return lower;
}

string encode_payload( const string &action, const string& msg ) {
	auto payload = "\"" + action + "\"";	// ie. payload is "<action>"message ... so it's easy to parse later

	if ( action == "encode" ) {
		payload.append(message::encode(msg));
	} else if ( action == "hash" ) {
		payload.append(message::hash(msg));
	} else if ( action == "sign" ) {
		payload.append(message::sign(msg));
	} else if ( action == "encrypt" ) {
		payload.append(message::encode(msg));
	} else throw std::runtime_error("No Such Action !");

	return payload;
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
