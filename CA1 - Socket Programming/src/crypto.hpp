#ifndef CRYPTO_HPP_INCLUDE
#define CRYPTO_HPP_INCLUDE

#include <string>

namespace Crypto {

std::string SHA256(const std::string& input);
std::string base64Encode(const std::string& input);
std::string base64Decode(const std::string& encoded);

}; // namespace Crypto

#endif // CRYPTO_HPP_INCLUDE
