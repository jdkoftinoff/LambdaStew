#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>

#ifdef ENABLE_SYSLOG
#include <syslog.h>
#endif

namespace LambdaStew
{
using std::mutex;
using std::ostream;
using std::lock_guard;

template <typename FirstT>
ostream &print( ostream &dest, FirstT &&first )
{
    dest << first;
    return dest;
}

template <typename FirstT, typename... RestT>
ostream &print( ostream &dest, FirstT &&first, RestT &&... rest )
{
    dest << first;
    return print( dest, rest... );
}

mutex &log_mutex();

#ifdef ENABLE_SYSLOG
bool log_to_syslog( bool set = false, bool value = false );

bool log_facility( bool set = false, int facility = LOG_DAEMON );
#endif

template <typename FirstT, typename... RestT>
void log( FirstT &&first, RestT &&... rest )
{
    lock_guard<mutex> guard( log_mutex() );
#if ENABLE_SYSLOG
    if ( log_to_syslog() )
    {
        std::stringstream o;
        print( o, first, rest... );
        syslog( log_facility(), "%s", o.str().c_str() );
    }
    else
#endif
    {
        print( std::clog, first, rest... ) << std::endl;
    }
}
}

#endif // LOG_HPP
