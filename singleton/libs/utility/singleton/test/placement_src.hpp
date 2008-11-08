#ifndef PLACEMENT_SRC_HPP_INCLUDED
#define PLACEMENT_SRC_HPP_INCLUDED

#include <boost/utility/singleton.hpp>
#include <boost/utility/mutexed_singleton.hpp>
#include <boost/utility/thread_specific_singleton.hpp>

namespace user_project
{
    struct S1 : boost::singleton<S1>
    {
        S1(boost::restricted) { }

        void foo() {}

      private:
        BOOST_SINGLETON_PLACEMENT_DECLARATION
    };
    struct S2 : boost::mutexed_singleton<S2>
    {
        S2(boost::restricted) { }

        void foo() {}

      private:
        BOOST_SINGLETON_PLACEMENT_DECLARATION
    };
    struct S3 : boost::mutexed_singleton<S3>
    {
        S3(boost::restricted) { }

        void foo() {}

      private:
        BOOST_SINGLETON_PLACEMENT_DECLARATION
    };
}

#endif

