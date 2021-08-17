#include "encryption.h"

namespace RSAEncryption {
    std::string get_str_key(unsigned long key, unsigned long prd)
    {
        std::stringstream stream;
        stream << std::hex << key << '.' << std::hex << prd;
        return stream.str();
    }

    std::vector<unsigned int> erathostenes(int n)
    {
        bool arr[n+1];
        std::vector<unsigned int> primes;

        memset(arr, true, sizeof(arr));

        for (int p = 2; p * p <= n; p++)
            if (arr[p] == true)
                for (int i = p * p; i <= n; i += p)
                    arr[i] = false;

        for (int i = 2; i <= n; i++)
            if (arr[i])
                primes.push_back(i);

        return primes;
    }

    std::pair<unsigned int, unsigned int> get_two_primes(int max)
    {
        std::vector<unsigned int> primes = erathostenes(max);
        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<int> gen(primes.size() / 2, primes.size() - 3);
        int idx = gen(rng);

        return std::make_pair(primes.at(idx), primes.at(idx + 2));
    }

    unsigned long pow_mod(uint128_t n, uint128_t pow, uint128_t mod)
    {
        uint128_t y;
        
        if (pow == 1)
            return n % mod;
        else
        {
            if (pow % 2 == 0)
            {
                y = pow_mod(n % mod, pow / 2, mod);
                return y % mod * y % mod;
            } else {
                y = pow_mod(n, (pow - 1) / 2, mod);
                return (n % mod * y % mod) % mod * y % mod;
            }
        }
    }

    std::tuple<unsigned long, unsigned long, unsigned long> gen_key_pairs(unsigned int prime_max)
    {
        std::pair<unsigned int, unsigned int> primes = get_two_primes(prime_max);
        unsigned long phi = (primes.first - 1) * (primes.second - 1);   
        unsigned long q = primes.first * primes.second;
        unsigned long d = 1;
        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<unsigned long> gen(3, phi / 4);
        unsigned long e = gen(rng);

        while (std::gcd(e, phi) != 1)
            e--;

        while (((d * phi) + 1) % e != 0)
            d++;
        d = ((d * phi) + 1) / e;

        return std::make_tuple(e, d, q);
    }

    unsigned long encrypt(int msg, uint128_t public_key, uint128_t product)
    {
        return pow_mod(msg, public_key, product);
    }

    unsigned long decrypt(unsigned long msg, uint128_t private_key, uint128_t product)
    {
        return pow_mod(msg, private_key, product);
    }
};

// int main(void)
// {
//     std::pair<unsigned int, unsigned int> primes = RSAEncryption::get_two_primes(0xFFFF);
//     auto[pub_key, priv_key, prd] = RSAEncryption::gen_key_pairs(0xFFFF);

//     std::cout << pub_key << " " << priv_key << " " << prd << std::endl;

//     unsigned long encrypted = RSAEncryption::encrypt(6666, pub_key, prd);
//     std::cout << encrypted << std::endl;

//     encrypted = RSAEncryption::decrypt(encrypted, priv_key, prd);
//     std::cout << encrypted << std::endl;

//     return 0;
// }