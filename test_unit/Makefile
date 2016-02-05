#Compiler
CXX=g++
#Sources
SOURCE_DIR=../source
SOURCE_DIR_MEMORY= $(SOURCE_DIR)/memory
SOURCE_DIR_CONCURRENT= $(SOURCE_DIR)/concurrent
SOURCE_DIR_ORDER_MATCHER= $(SOURCE_DIR)/order_matcher
SOURCE_DIR_UTILITY= $(SOURCE_DIR)/utility
SOURCES =  test.cpp \
	   $(SOURCE_DIR_MEMORY)/aligned_memory.cpp \
	   $(SOURCE_DIR_CONCURRENT)/thread.cpp \
	   $(SOURCE_DIR_CONCURRENT)/thread_pool.cpp \
	   $(SOURCE_DIR_UTILITY)/config_file.cpp \
	   $(SOURCE_DIR_UTILITY)/stopwatch.cpp \
	   $(SOURCE_DIR_UTILITY)/logger.cpp \
	   $(SOURCE_DIR_UTILITY)/single_instance.cpp \
	   $(SOURCE_DIR_ORDER_MATCHER)/order.cpp \
	   $(SOURCE_DIR_ORDER_MATCHER)/order_book.cpp \
	   $(SOURCE_DIR_ORDER_MATCHER)/central_order_book.cpp 
#Include Directories
INCLUDE_DIR = ./
INCLUDE_DIR_PROJECT = ../source
INCLUDE_DIR_GTEST = ../dependencies/gtest-1.7.0/include/
INCLUDE_DIR_BOOST = ../dependencies/

#Objects
OBJECTS = $(SOURCES:.cpp=.o)
#Executable
EXECUTABLE = ./ome_unit_test
#Compiler flags
CFLAGS= -I$(INCLUDE_DIR) -I$(INCLUDE_DIR_GTEST)  -I$(INCLUDE_DIR_PROJECT) -I$(INCLUDE_DIR_BOOST) -std=c++11 -c -DDEBUG -g -Wall -fexceptions -frtti
#Linker flags
LFLAGS= -lstdc++ -pthread -lgtest -lgtest_main

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
		$(CXX) -o $@  $(OBJECTS)  $(LFLAGS)
# On Ubuntu LFLAGS shall be placed at the end , see http://stackoverflow.com/questions/26232237/google-testing-missing-dso

.cpp.o: *.h
	$(CXX) $< -o $@ $(CFLAGS)

clean:
	@echo Cleaning
	-rm -f *.o $(EXECUTABLE)
	@echo Cleaning done
	
.PHONY: all clean
