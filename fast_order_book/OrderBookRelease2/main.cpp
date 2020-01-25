#include <iostream>
#include "FastOrderBook.hpp"
#include "TextFileFeedParser.hpp"
#include "LatencyMonitor.hpp"
#include "HighFrequencyFeedSimulator.hpp"
#include "AgnosticFeedHandler.hpp"
#include "UnifiedMsg.hpp"
#include "FastLogger.hpp"

int main(int argc, char *argv[])
{    
    std::string feedFileName;
    {
        if (argc > 1) 
        {
            feedFileName.assign(argv[1]);
        }

        if (feedFileName.empty())
        {
            std::cerr << "Program usage: orderbook dummy_feed.txt\n\n";
            return -1;
        }
    }
    
    try
    {
        LatencyMonitor::instance(); //warm up

        FastOrderBook book(ORDER_BOOK_PRICE_RANGE,
                           ORDER_BOOK_TICK_SIZE);

        Summary stats = {0, 0, 0, 0, 0, 0};

        cpu_set_t affinity[2]; 
        getAffinity(affinity, 2);

        #ifdef HIGH_FREQUENCY_ON
            int bookDumpAtNThMsg = 300000;
            SPSCBoundedQueue<UnifiedMsg> q(HIGH_FREQ_SIMULATOR_Q_SIZE);
            HighFrequencyFeedSimulator simulator(q, &affinity[1], 10000/*N*/);
        #endif

        #ifndef HIGH_FREQUENCY_ON
            int bookDumpAtNThMsg = 10;
            SPSCBoundedQueue<UnifiedMsg> q(TEXT_FILE_PARSER_MSG_Q_SIZE);
            TextFileFeedParser fileFeed(q, feedFileName, &affinity[1], stats);
        #endif

        AgnosticFeedHandler handler(book, stats, q, std::cout, &affinity[0], bookDumpAtNThMsg);
        handler.start();
        sleep(1);

        #ifdef HIGH_FREQUENCY_ON
            simulator.start();
        #endif

        #ifndef HIGH_FREQUENCY_ON
            fileFeed.start();
        #endif

        //did not have enough time
        //to sort out nice shut down
        sleep(1); 
        handler.stop();
        sleep(1); 

        stats.dump(std::cout);
        LatencyMonitor::instance().dumpStat(std::cout);
        std::cout << std::endl;

        handler.stop();
        sleep(1);
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n\nUnfourtunately program throwned an exception: [%s]\n\n"
                  << e.what();
    }
    catch (...)
    {
        std::cerr << "\n\nEven worse...Caught unknown exception\n\n";
    }

    return 0;
}

