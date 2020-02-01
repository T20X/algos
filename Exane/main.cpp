#include <functional>

#include "Problem1.h"
#include "Problem2.h"
#include "Problem3.h"
#include "Problem4.h"

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cout << "\n Usage: app <mode> "
                      << "\n <mode> can be unittest, prob1, prob2, prob3, prob4 "
                      << "\n For example: unittest prob1 will execute prob1";
        }

        std::map<std::string, std::function<void()>> commands {
            { "unittest",
               []() {
                  prob1::test::go<1000,10000>();                    
                  prob2::test::go();
                  prob3::test::go<10000>();
                  prob4::test::go();    
               }
            },
            { "prob1",
               []() {
                  prob1::IOProcessor p;
                  p.go(std::cin, std::cout);
               }
            },
            { "prob2",
               []() {
                  prob2::IOProcessor p;
                  p.go(std::cin, std::cout);
               }
            },
            { "prob3",
               []() {
                  prob3::IOProcessor p;
                  p.go(std::cin, std::cout, 10000);
               }
            },
            { "prob4",
               []() {
                  prob4::IOProcessor p;
                  p.go(std::cin, std::cout);
               }
            }
        };

        std::string mode(argv[1]);
        commands[mode]();
    }
    catch (const std::exception & e)
    {
        std::cout << "\n Exception has been thrown -> [" << e.what()
                  << " ]. Terminating...Please check your input is valid!";
    }

    return 0;
}
