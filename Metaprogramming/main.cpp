
#include <iostream>

using namespace std;

#include "Basics.h"
#include "tuples.h"
#include "enable_if.h"
#include "specialization.h"
#include "template_classes.h"
#include "ClassExplorer.h"
#include "concepts.h"

int main()
{
    basics::test();
    //TuplesTest().test();
    enableif::test();
    specialization::test();
    templates::test();
    explorer::quickTest();
    concepts::test();
    getchar();
    return 0;
}