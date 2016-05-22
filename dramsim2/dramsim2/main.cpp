//#include "vld.h"

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

#include "TraceBasedSim.h"
#include "TestingSystem.h"


int main(int argc, char **argv)
{
    // TraceBasedStart(argc, argv);
    TestingSystem system_(2700);
    system_.start();
    //_CrtDumpMemoryLeaks();
    system("pause");
    return 0;
}