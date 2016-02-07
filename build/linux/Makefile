#Compiler
CXX=g++
#Source Directories
SOURCE_DIR=../../source
SOURCE_DIR_MEMORY= $(SOURCE_DIR)/memory
SOURCE_DIR_CONCURRENT= $(SOURCE_DIR)/concurrent
SOURCE_DIR_ORDER_MATCHER= $(SOURCE_DIR)/order_matcher
SOURCE_DIR_UTILITY= $(SOURCE_DIR)/utility
SOURCE_DIR_SERVER= $(SOURCE_DIR)/server
SOURCES =  $(SOURCE_DIR_MEMORY)/aligned_memory.cpp \
		$(SOURCE_DIR_CONCURRENT)/thread.cpp \
		$(SOURCE_DIR_CONCURRENT)/thread_pool.cpp \
		$(SOURCE_DIR_UTILITY)/config_file.cpp \
		$(SOURCE_DIR_UTILITY)/stopwatch.cpp \
		$(SOURCE_DIR_UTILITY)/single_instance.cpp \
		$(SOURCE_DIR_UTILITY)/logger.cpp \
		$(SOURCE_DIR_ORDER_MATCHER)/order.cpp \
		$(SOURCE_DIR_ORDER_MATCHER)/order_book.cpp \
		$(SOURCE_DIR_ORDER_MATCHER)/central_order_book.cpp \
		$(SOURCE_DIR_SERVER)/server.cpp \
		$(SOURCE_DIR)/server_main.cpp
#Include Directories
INCLUDE_DIR = $(SOURCE_DIR)/
INCLUDE_DIR_BOOST = ../../dependencies/
INCLUDE_DIR_QUICKFIX = ../../dependencies/quickfix/include
#Objects
OBJECTS = $(SOURCES:.cpp=.o)
#Executable
EXECUTABLE = ../../bin/ome
#Compiler flags
CFLAGS= -I$(INCLUDE_DIR) -I$(INCLUDE_DIR_BOOST) -I$(INCLUDE_DIR_QUICKFIX) -std=c++11 -c
#Linker flags
LFLAGS= -lstdc++ -pthread -lquickfix

#Add DEBUG macro , symbol generation and show all warnings
debug: CFLAGS += -DDEBUG -g -Wall 
debug: all

#Use O2 as it is the most tested optimisation level in most software projects
release: CFLAGS +=  -O2
release: all

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
		$(CXX) $(LFLAGS) $(OBJECTS) -o $@ 
	
pch : 
	g++ -c ../../source/precompiled_header.h -o ../../source/precompiled_header.h.gch -std=c++11
	#To use precompiled header in linux :
	#1. Add pch rule as first dependency in rule "all"
	#2. precompiled_header.h should be included in all source files
	
.cpp.o: *.h
	$(CXX) $(CFLAGS) $< -o $@

clean:
	@echo Cleaning
	-rm -f $(OBJECTS) $(EXECUTABLE)
	@echo Cleaning done
	
.PHONY: all clean