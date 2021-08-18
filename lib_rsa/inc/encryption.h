#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <vector>
#include <random>
#include <numeric>
#include <tuple>

typedef unsigned __int128 uint128_t;

namespace RSAEncryption {
    std::string get_str_key(unsigned long key, unsigned long prd);
    bool store_to_int(std::string buf, unsigned long *key, unsigned long *prd);
    unsigned long encrypt(int msg, uint128_t public_key, uint128_t product);
    unsigned long decrypt(unsigned long msg, uint128_t private_key, uint128_t product);
    std::string encrypt_msg(std::string msg, unsigned long public_key, unsigned long product);
    std::string decrypt_msg(std::string msg, unsigned long private_key, unsigned long product);
    std::tuple<unsigned long, unsigned long, unsigned long> gen_key_pairs(unsigned int prime_max);
};