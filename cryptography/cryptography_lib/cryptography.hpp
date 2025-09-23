#ifndef CRYPTOGRAPHY_HPP
#define CRYPTOGRAPHY_HPP

#include <iostream>
#include <exception>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>

typedef struct Step {
    int index;
    int val;
} Step;

typedef struct User {
    int secret_key;
    int open_key;
} User;

typedef char byte_t;

int pow_mod(int base, int exp, int mod);
int ferm_test(int n);
void Euclid();
std::vector<int> algorithm_Euclid(int a, int b);
void disp_Euclid_result(std::vector<int> result, int a, int b);
void small_big_step();
std::vector<int> algorithm_small_big_step(int a, int y, int p);
std::vector<int> open_key_system(User &user1, User &user2);
int Euler_algorithm(int n);
#endif