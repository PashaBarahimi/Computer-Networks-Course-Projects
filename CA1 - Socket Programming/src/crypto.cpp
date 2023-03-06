#include "crypto.hpp"

#include <crypto++/base64.h>
#include <crypto++/hex.h>
#include <crypto++/sha.h>

namespace crypto {

std::string SHA256(const std::string& input) {
    CryptoPP::SHA256 hash;
    std::string digest;

    // clang-format off
    CryptoPP::StringSource(input, true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::Base64Encoder(
                new CryptoPP::StringSink(digest), false
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
            new CryptoPP::StringSink(encoded), false
        )
    );
    // clang-format on

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

} // namespace crypto
