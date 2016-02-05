#Compiler
CXX=g++
#Source Directories
SOURCE_DIR=../../source
SOURCE_DIR_UTILITY= $(SOURCE_DIR)/utility
SOURCES = client.cpp \
			client_application.cpp \
			client_request.cpp
		
#Include Directories
INCLUDE_DIR = $(SOURCE_DIR)/
INCLUDE_DIR_BOOST = ../../dependencies/
INCLUDE_DIR_QUICKFIX = ../../dependencies/quickfix/include
#Objects
OBJECTS = $(SOURCES:.cpp=.o)
#Executable
EXECUTABLE = ../client
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

.cpp.o: *.h
	$(CXX) $(CFLAGS) $< -o $@

clean:
	@echo Cleaning
	-rm -f $(OBJECTS) $(EXECUTABLE)
	@echo Cleaning done
	
.PHONY: all clean