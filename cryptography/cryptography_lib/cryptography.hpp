#ifndef CRYPTOGRAPHY_HPP
#define CRYPTOGRAPHY_HPP

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

typedef struct Step {
    int index;
    int val;
} Step;

typedef struct User {
    int secret_key;
    int open_key;
} User;

typedef unsigned char byte_t;

int pow_mod(int base, int exp, int mod);
int ferm_test(int n);
void Euclid();
std::vector<int> algorithm_Euclid(int a, int b);
void disp_Euclid_result(std::vector<int> result, int a, int b);
void small_big_step();
std::vector<int> algorithm_small_big_step(int a, int y, int p);
std::vector<int> open_key_system(User &user1, User &user2);
int Euler_algorithm(int n);
int gen_prime();

class FileWorker {
public:
    std::vector<byte_t> read(std::string filename);
    void write(std::vector<byte_t> &bytes, std::string filename);
};

class ShamirCoder {
public:
    ShamirCoder();
    std::vector<byte_t> worker(std::vector<byte_t> &bytes);

private:
    int _P;
    int u1_keys[2]; // [Ca, Da]
    int u2_keys[2]; // [Cb, Db]

    void gen_keys();
};
#endif