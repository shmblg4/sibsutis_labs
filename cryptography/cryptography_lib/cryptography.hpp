#ifndef CRYPTOGRAPHY_HPP
#define CRYPTOGRAPHY_HPP

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#define BMP_H_SIZE 54

typedef unsigned char byte_t;
typedef long long ll;

ll pow_mod(ll base, ll exp, ll mod);
ll ferm_test(ll n);
void Euclid();
int Euler_algorithm(int n);
ll gen_prime();
ll gcd(ll a, ll b);
std::vector<ll> algorithm_Euclid(ll a, ll b);
ll gen_random();

class BMPworker {
public:
    std::vector<ll> read(std::string filename);
    void write(std::vector<ll> data, std::string filename);

private:
    std::vector<ll> header;
};

class Fileworker {
public:
    std::vector<ll> read(std::string filename);
    void write(std::vector<ll> data, std::string filename);
};

class RSAcoder {
public:
    RSAcoder() {
        configure();
    }

    std::vector<ll> encode(std::vector<ll> data);
    std::vector<ll> decode(std::vector<ll> data);

private:
    ll p;
    ll q;
    ll n;
    ll phi;
    ll d;
    ll c; // Изменено с double на ll

    void configure();
    void gen_keys();
};

#endif