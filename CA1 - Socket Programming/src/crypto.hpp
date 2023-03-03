#ifndef CRYPTO_HPP_INCLUDE
#define CRYPTO_HPP_INCLUDE

#include <string>

class Crypto {
public:
    static std::string SHA256(const std::string& input);
    static std::string base64Encode(const std::string& input);
    static std::string base64Decode(const std::string& encoded);
};

#endif // CRYPTO_HPP_INCLUDE
