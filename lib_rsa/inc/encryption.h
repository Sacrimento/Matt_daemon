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
    unsigned long encrypt(int msg, uint128_t public_key, uint128_t product);
    unsigned long decrypt(unsigned long msg, uint128_t private_key, uint128_t product);
    std::tuple<unsigned long, unsigned long, unsigned long> gen_key_pairs(unsigned int prime_max);
};