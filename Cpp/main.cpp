#include <iostream>

using namespace std;

#include "sharedpointer.h"
#include "../common/ConflatedQueue.h"
#include "../common/FIFOQueue.h"
#include "move_test.h"
#include "nums.h"
#include "conversion.h"

int main()
{

   // testConflationQueue();
   // testFIFOQueue();   
    //testSharedPtr();
   // sharedtest::testSharedPtrMain();
    //sharedtest::testSharedPtrinMultiThreadingEnv();   
    //sharedtest::benchmark();
    movetest::test();
    nums::test();
    conv::test();
    getchar();
}