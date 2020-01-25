#ifndef _CONSTANTS_HPP_
#define _CONSTANTS_HPP_

#define TRACE_LATENCY
#define HIGH_FREQUENCY_ON
#undef HIGH_FREQUENCY_ON 
#undef TRACE_LATENCY

const double PRICE_EPSILON = 0.000001; 
const double INVALID_PRICE = 0.0;
const double INVALID_POS = 123456789;

const int OSTREAM_PRECISION = 6;
const int OSTREAM_QTY_WIDTH = 8;
const int OSTREAM_PRICE_WIDTH = 13;	

const int MAX_MSG_SIZE = 1024;
const int CACHE_LINE_SIZE = 64;

const int FIXED_ALLOCATOR_SIZE = 32000;
const int LATENCY_MONITOR_ALLOC_SIZE = 1024;

const int FAST_LOGGER_MSG_Q_SIZE = 32000;
const int TEXT_FILE_PARSER_MSG_Q_SIZE = 1; 
const int HIGH_FREQ_SIMULATOR_Q_SIZE = 1;

const double ORDER_BOOK_PRICE_RANGE = 50;
const double ORDER_BOOK_TICK_SIZE = 0.0001;
const int ORDER_BOOK_ORDER_MAPPING_SIZE = 16384;
const int ORDER_BOOK_TRADE_Q_SIZE = 256;

#endif // _CONSTANTS_HPP_
