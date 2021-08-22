#include <cstdint>
#include <ios>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <string_view>
#include <cassert>
#include <map>
#include <vector>

#include "rust-ffi.h"

namespace util {
	std::string bytes_to_hex_string(const std::vector<uint8_t>& bytes) {
		std::stringstream ss;
		ss << std::hex;

		for (auto& byte: bytes) {
			ss << std::setw(2) << std::setfill('0') << (int)byte;
		}

		return ss.str();
	}
}

namespace message {
	bool _is_encodable(const std::string& str) {
		/*current encoder decorder don't support messages with these chars as it disrupts message structure*/
		return std::none_of(str.cbegin(), str.cend(), [](char c) {
				return c == ';' || c == '}' || c == '{';
				});
	}

	std::string encode (const std::string& str) { return str; }
	std::string encode (const std::map<std::string,std::string>& mapping) {
		std::string encoded = "";
		for( auto& p : mapping ) {
			/*assert that the data is okay to encode and can be decoded exactly same later*/
			assert(_is_encodable(p.first) && _is_encodable(p.second) );
			encoded.append("{" + p.first + ";" + p.second + "}");
		}

		return encoded;
	}

	std::map<std::string,std::string> decode (const std::string& str) {
		std::map<std::string, std::string> out;

		for(auto i = str.cbegin(); i != str.cend(); ++i ) {
			// TODO	
		}

		return out;
	}

	std::string encrypt(const std::string& str) {
		std::string out(str);
		std::for_each(out.begin(), out.end(), [](char& c){ c += 1; });
		return out;
	}

	std::string decrypt(const std::string& str) {
		std::string out(str);
		std::for_each(out.begin(), out.end(), [](char& c){ c -= 1; });
		return out;
	}

	std::string hash(const std::string& str) {
		uint8_t hash_buffer[64];
		get_hash(str.data(), hash_buffer);

		return util::bytes_to_hex_string({hash_buffer, hash_buffer+64});
	}

	std::string sign(const std::string& str) {
		// TODO: I don't know the sizes required
		uint8_t public_key_buffer[128];
		std::vector<char> signed_msg(str.size() * 2 + 1);	// allocating 2*length + 1 size for the signature

		sign_str(str.data(), signed_msg.data(), public_key_buffer.data());

		return util::bytes_to_hex_string(signed_msg.data());
	}
}

