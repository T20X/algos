#pragma once

using namespace std;

#include <iostream>
#include "Pricer.h"

#include "SPSCTest.h"
#include "SPSCConflatedTest.h"

int main()
{
    /*Pricer p;
    p.go();*/

    tests::testSPSC();
    tests::testSPSCConflation();
    getchar();
    return 1;
}
