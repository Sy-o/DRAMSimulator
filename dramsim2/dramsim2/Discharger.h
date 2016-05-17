#pragma once
#include <random>
#include <vector>

using namespace std;
class default_random_engine;
class exponential_distribution;

class Discharger
{
public:
    Discharger();
    ~Discharger();

public:
    void Initialize();
    vector<int> GetRandomAddressList(int curCycle);

private:
    std::random_device rd;
    std::mt19937* gen;
    std::uniform_int_distribution<>* uniformDist;
};