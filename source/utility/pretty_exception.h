#ifndef __PRETTY_EXCEPTION__
#define __PRETTY_EXCEPTION__

#include <exception>
#include <string>

// For throwing exceptions with messages that contain the file name and the line number info
// We could use boost::exception , however in that we have to catch boost::exception class

// Implemented this "pretty_exception" macro using macro indirection in order to stick with
// std::runtime_error

// Note for MSVC : std::exception implementation of Microsoft has an overloaded constructor
// but this is not standard. Therefore using std::runtime_error

#ifdef _WIN32
#define NEW_LINE "\r\n"
#elif __linux__
#define NEW_LINE "\n"
#else
#define NEW_LINE "\n"
#endif

//Macro technique used : http://stackoverflow.com/questions/19343205/c-concatenating-file-and-line-macros
//and http://stackoverflow.com/questions/2670816/how-can-i-use-the-compile-time-constant-line-in-a-string

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)
#define PRETTY_EXCEPTION_LOCATION "File : "  __FILE__ NEW_LINE "Line:" STRINGIFY(__LINE__) 

//Following .Net/Java exception message convention , first the message then its details...
#define THROW_PRETTY_EXCEPTION(msg) throw std::runtime_error( (std::string((msg).c_str() ) +  NEW_LINE PRETTY_EXCEPTION_LOCATION).c_str() );

#endif