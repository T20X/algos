INCLUDES = 
LIBS = 
DEBUG = -g

CXX = g++
CXXFLAGS = -O2 -Wall -m64 -pthread -D_REENTRANT $(INCLUDES)

all: orderbook 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp:
	$(CXX) $(CXXFLAGS) $< -o $@ 

BOOK_SRC = main.cpp TextFileFeedParser.cpp AgnosticFeedHandler.cpp FastOrderBook.cpp
BOOK_OBJ = $(addsuffix .o, $(basename $(BOOK_SRC)))

orderbook: $(BOOK_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(BOOK_OBJ)

clean:
	rm -f $(BOOK_OBJ) orderbook
