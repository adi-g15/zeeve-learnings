#include <string>
#include <algorithm>
#include <string_view>
#include <cassert>
#include <map>

#include "rust-ffi.h"

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

		return str;
	}

	std::string sign(const std::string& str) {
		return str;
	}
}

