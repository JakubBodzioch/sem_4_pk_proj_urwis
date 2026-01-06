#pragma once
#include <string>
#include <iostream>

class AuthService {
public:
    static bool verify_credentials(const std::string& email, const std::string& password, std::string& out_role);
};
