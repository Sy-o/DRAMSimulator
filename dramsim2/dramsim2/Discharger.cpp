#include "Discharger.h"
#include "SystemConfiguration.h"
#include <math.h>

using namespace std;

Discharger::Discharger() : gen(0), uniformDist(0)
{
    
}

Discharger::~Discharger()
{
    delete uniformDist;
}

void Discharger::Initialize()
{
    gen = new std::mt19937(rd());
    int totalBits = NUM_RANKS * NUM_BANKS * NUM_ROWS * NUM_COLS * DEVICE_WIDTH;
    uniformDist = new std::uniform_int_distribution<>(0, totalBits - 1);
}

vector<int> Discharger::GetRandomAddressList(int curCycle)
{
    vector<int> addrList;
    double number = exp(sqrt(curCycle)/5.2);
    int addrCount = (int)number;
    
    for (int i = 0; i < addrCount; i++)
    {
        addrList.push_back((*uniformDist)(*gen));
    }
    return addrList;
}