#ifndef _SINGLETON_H_
#define _SINGLETON_H_

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
};

}
#endif