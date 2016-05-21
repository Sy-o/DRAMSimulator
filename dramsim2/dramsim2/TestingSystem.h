#include "stdint.h"
#include "MemorySystem.h"

class TestingSystem
{
    struct Trans
    {
        bool Write;
        unsigned Address;
        uint16_t Data;

        Trans(bool write, unsigned address, uint16_t data = 0) : Write(write), Address(address), Data(data){};
    };

public:
    TestingSystem(int cycles = 4000);
    ~TestingSystem();

private:
    unsigned numCycles;
    DRAMSim::MemorySystem *memory;

private:
    void Initialize();
    int AlignAddress(int address);

public:
    void start();

    void read_complete(uint64_t address, uint16_t data, size_t);
    void write_complete(unsigned id, uint64_t address, uint64_t cycle);
};