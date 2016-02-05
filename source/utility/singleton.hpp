#ifndef _SINGLETON_
#define _SINGLETON_

#include <boost/noncopyable.hpp>

namespace utility
{

template<typename T>
class Singleton : public boost::noncopyable
{
    public :
        static T& getInstance()
        {
            // Using Scott Meyers` singleton
            // Note that , the order of initialisation for statics in different translation units
            // is undefined , so this could be problematic if more singletons used in the project

            // In C++11 statics are initialized in a thread safe way
            static T single_instance;
            return single_instance;
        }
		
	private :
	
		// Move ctor deletion
        Singleton(Singleton&& other) = delete;
        // Move assignment operator deletion
        Singleton& operator=(Singleton&& other) = delete;
};

}
#endif