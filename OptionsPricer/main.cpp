#pragma once

using namespace std;

#include <iostream>
#include "Pricer.h"

#include "SPSCTest.h"
#include "MPSCQueueTest.h"
#include "SPSCConflatedTest.h"
#include "SpinLockTest.h"

int main()
{
    /*Pricer p;
    p.go();*/

    tests::testSPSC();
   /* tests::testSPSCConflation();
    for (int i = 0; i < 2; i++)
    {
        std::this_thread::sleep_for(1s);        
        tests::testSpinLockAtomicFlag();
        tests::testSpinLockAtomicBool();
    }*/

    tests::testMPSC();

    getchar();
    return 1;
}
