#pragma once
#include <string>

class CryptoService {
public:
    static std::string xor_encrypt(const std::string& input, const std::string& key);
    static std::string xor_decrypt(const std::string& input, const std::string& key);
};
