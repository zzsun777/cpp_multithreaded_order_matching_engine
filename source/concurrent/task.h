#ifndef _TASK_H_
#define _TASK_H_

#include <memory>
#include <functional>
#include <string>
#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <memory/aligned.hpp>

namespace concurrent
{

using ReturnType = boost::optional<boost::any>;
    
class Task : public memory::Aligned<>
{
    public :
        // Constructor
        Task() = default;
        // Copy constructor
        Task(const Task& rhs) = default;
        // Assignment operator
        Task& operator= (const Task &cSource) = default;
        // Destructor
        ~Task() = default;

        template<typename Function, typename ...Args>
        Task(Function f, Args... args) : m_callback{std::bind(f, args...)}
        { }

        void execute()
        {
            // Execute callback & Transfer return value buffer to Task via boost::optional<boost::any>
            m_returnValue = m_callback();
        }

        ReturnType getReturnValue()const
        {
            return m_returnValue;
        }
       
    private:
        std::function<void*()> m_callback;
        ReturnType m_returnValue;
};

using TaskPtr = std::unique_ptr<Task>;
    
}//namespace

#endif