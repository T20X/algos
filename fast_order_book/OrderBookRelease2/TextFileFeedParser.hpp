#ifndef _TEXT_FILE_FEED_PARSER_HPP_
#define _TEXT_FILE_FEED_PARSER_HPP_

#include <string>
#include "Summary.hpp"
#include "Common.hpp"
#include "UnifiedMsg.hpp"
#include "SPSCBoundedQueue.hpp"
#include "Thread.hpp"

class TextFileFeedParser : protected Thread
{
public:
    TextFileFeedParser(
        SPSCBoundedQueue<UnifiedMsg>& q,
        const std::string& filename,
        cpu_set_t* affinity,
        Summary& stats);         
    ~TextFileFeedParser();

    void start();

private:
    void* process();

    void parseMsg(char* msg);

    typedef bool (TextFileFeedParser::*ParseFunc)(char*, UnifiedMsg**);  
    bool parseN_Order(char* raw, UnifiedMsg** msg);
    bool parseM_Order(char* raw, UnifiedMsg** msg);
    bool parseD_Order(char* raw, UnifiedMsg** msg);
    bool parseTrade(char* raw, UnifiedMsg** msg);
    bool reportCorruptedMsg(char* raw, UnifiedMsg** msg);

    bool doParseOrder(char* msg, Order& o);
    bool doParseTrade(char* msg, Trade& t);    

    uint32_t strToPositiveUnsInt(const char* str);
    double strToPositiveDouble(const char* str);

private:
    SPSCBoundedQueue<UnifiedMsg>& m_q;
    std::string m_filename;
    Summary& m_stats;

    std::vector<ParseFunc> m_parsers;
};

#endif // _TEXT_FILE_FEED_PARSER_HPP_
