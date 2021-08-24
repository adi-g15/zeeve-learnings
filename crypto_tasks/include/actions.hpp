#include <cstdint>
#include <ios>
#include <memory>
#include <openssl/ossl_typ.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <string_view>
#include <cassert>
#include <map>
#include <vector>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "rust-ffi.h"

namespace util {
	std::vector<uint8_t> get_random_bytes(int n) {
		std::vector<uint8_t> buff(n);
		RAND_poll();
		RAND_bytes(buff.data(), n);

		return buff;
	}

	/*
	 * Note: 1 byte is represented by 2 characters in hex string
	 *
	 * So the string returned has 2*bytes.size() characters
	 * */
	std::string bytes_to_hex_string(const std::vector<uint8_t>& bytes) {
		std::stringstream ss;
		ss << std::hex;

		for (auto& byte: bytes) {
			ss << std::setw(2) << std::setfill('0') << (int)byte;
		}

		return ss.str();
	}

	/*We use the fact that, two chars in hex represent one byte*/
	std::vector<uint8_t> hex_string_to_bytes(const std::string& data_str) {
		auto bytes = std::vector<uint8_t>();

		if( data_str.size() % 2 != 0 ) throw std::runtime_error("Invalid hex string for conversion into bytes, length MUST be even");

		for (auto i = 0; i < data_str.size() - 1; i+=2)
		{
			bytes.push_back( std::strtol( data_str.substr(i,2).data(), nullptr, 16 ) );
		}
		
		return bytes;
	}

	std::vector<uint8_t> hex_string_to_bytes(const std::vector<uint8_t>& data_str_bytes) {
		auto c_str = reinterpret_cast<const char*>(data_str_bytes.data());

		return hex_string_to_bytes( std::string( c_str, c_str + data_str_bytes.size()) );
	}
}

namespace message {
	bool _is_encodable(const std::string& str) {
		/*current encoder decorder don't support messages with these chars as it disrupts message structure*/
		return std::none_of(str.cbegin(), str.cend(), [](char c) {
				return c == ';' || c == '}' || c == '{';
				});
	}

	std::string serialise_map (const std::map<std::string,std::string>& mapping) {
		std::string encoded = "";
		for( auto& p : mapping ) {
			/*assert that the data is okay to encode and can be decoded exactly same later*/
			assert(_is_encodable(p.first) && _is_encodable(p.second) );
			encoded.append("{" + p.first + ";" + p.second + "}");
		}

		return encoded;
	}

	std::map<std::string,std::string> deserialise (const std::string& str) {
		std::map<std::string, std::string> out;

		for(auto i = str.cbegin(); i != str.cend(); ++i ) {
			// TODO	
		}

		return out;
	}

	std::string encode(const std::string& str) {
		std::string out(str);
		std::for_each(out.begin(), out.end(), [](char& c){ c += 1; });
		return out;
	}

	std::string decode(const std::string& str) {
		std::string out(str);
		std::for_each(out.begin(), out.end(), [](char& c){ c -= 1; });
		return out;
	}

	struct _EncryptedData {
		std::vector<uint8_t> key;
		std::vector<uint8_t> IV;
		std::vector<uint8_t> message;
	};
	_EncryptedData encrypt_bytes(const std::vector<uint8_t>& bytes) {
		ERR_load_crypto_strings();	// Load human readable error strings for libcrypto
		EVP_add_cipher( EVP_aes_256_cbc() );	// Load the necessary cipher

		constexpr int KEY_SIZE = 32;	// 256 BIT key... 32 bytes
		constexpr int IV_SIZE = 16;	// 128 BIT... 16 bytes
		// constexpr int MAX_PADDING = BLOCK_SIZE - 1;

		auto key = util::get_random_bytes(KEY_SIZE); 
		auto iv = util::get_random_bytes(IV_SIZE);	// Initialisation vector

		auto cipher_text = std::vector<uint8_t>(bytes.size() + 16);	// Cipher text expands upto BLOCK_SIZE
		
		using CIPHER_CTX = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

		// Encryption starts here...
		auto context = CIPHER_CTX{ EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free };
		if( !context ) throw std::runtime_error("COULDN'T CREATE CONTEXT");

		/*Initialise the encryption operation*/
		if(
			EVP_EncryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data())
			!= 1
		  ) throw std::runtime_error("Failed to initialise encryption operation");

		int len1 = cipher_text.size(), len2;
		if(
				/*WARN: the third argument must not be just .size(), since it is to be modified variable*/
			EVP_EncryptUpdate(context.get(), cipher_text.data(), &len1, bytes.data(), bytes.size())
			!= 1
		) throw std::runtime_error("Failed to encrypt");

		len2 = cipher_text.size() - len1;	// ie. the additional space we still have in the buffer... I think it's for "padding"

		if (
			EVP_EncryptFinal_ex(context.get(), cipher_text.data() + len1, &len2)	/*I think it's for padding, they did state it, but not directly here,... https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption*/
			!= 1
		) throw std::runtime_error("Failed to finalize encryption");

		cipher_text.resize(len1 + len2);

		EVP_cleanup();

		return {
			key,
			iv,
			cipher_text
		};
	}

	std::vector<uint8_t> decrypt_bytes(const std::vector<uint8_t> &encrypted_bytes, const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv) {
		ERR_load_crypto_strings();
		EVP_add_cipher(EVP_aes_256_cbc());

		std::vector<uint8_t> decrypted( encrypted_bytes.size() );

		using CIPHER_CTX = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

		auto context = CIPHER_CTX{ EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free };

		if (!context) throw std::runtime_error("Couldn't create context");

		if ( 
			EVP_DecryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data())
			!= 1
		) throw std::runtime_error("Couldn't initialise decrypt operation");

		int len1 = decrypted.size(), len2;
		if(
			EVP_DecryptUpdate(context.get(), decrypted.data(), &len1, encrypted_bytes.data(), encrypted_bytes.size())
			!= 1
		) throw std::runtime_error("Couldn't decrypt data");

		len2 = len1;

		if(
			EVP_DecryptFinal_ex(context.get(), decrypted.data() + len1, &len2)
			!= 1
		) throw std::runtime_error("Couldn't finalise decryption");

		decrypted.resize(len1 + len2);
		EVP_cleanup();

		return decrypted;
	}

	std::string hash(const std::string& str) {
		uint8_t hash_buffer[64];
		get_hash(str.data(), hash_buffer);

		return util::bytes_to_hex_string({hash_buffer, hash_buffer+64});
	}

	std::string sign(const std::string& str) {
		// TODO: I don't know the sizes required
		char public_key_buffer[128];
		std::vector<char> signed_msg(str.size() * 2 + 1);	// allocating 2*length + 1 size for the signature

		auto signed_msg_len = sign_str(str.data(), signed_msg.data(), public_key_buffer);
		signed_msg.resize(signed_msg_len);

		return util::bytes_to_hex_string({signed_msg.begin(),signed_msg.end()});
	}
}

