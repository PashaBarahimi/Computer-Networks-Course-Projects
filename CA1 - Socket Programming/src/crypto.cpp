#include "crypto.hpp"

#include <crypto++/base64.h>
#include <crypto++/hex.h>
#include <crypto++/sha.h>

namespace Crypto {

std::string SHA256(const std::string& input) {
    CryptoPP::SHA256 hash;
    std::string digest;

    // clang-format off
    CryptoPP::StringSource(input, true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(digest)
            )
        )
    );
    // clang-format on

    return digest;
}

std::string base64Encode(const std::string& input) {
    std::string encoded;

    // clang-format off
    CryptoPP::StringSource(input, true,
        new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(encoded)
        )
    );
    // clang-format on

    encoded.pop_back(); // Remove the newline character
    return encoded;
}

std::string base64Decode(const std::string& encoded) {
    std::string decoded;

    // clang-format off
    CryptoPP::StringSource(encoded, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(decoded)
        )
    );
    // clang-format on

    return decoded;
}

}; // namespace Crypto
