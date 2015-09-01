#ifndef LAMBDASTEW_LOG_HPP
#define LAMBDASTEW_LOG_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>

#define ENABLE_SYSLOG

#ifdef ENABLE_SYSLOG
#include <syslog.h>
#endif

namespace LambdaStew
{
using std::mutex;
using std::ostream;
using std::lock_guard;

/// \brief print
///
/// template helper function to print one parameter to an ostream
///
/// \return ostream &
///
template <typename FirstT>
ostream &print( ostream &dest, FirstT &&first )
{
    dest << first;
    return dest;
}

///
/// template helper function to print a list of parameters to an ostream
///
template <typename FirstT, typename... RestT>
ostream &print( ostream &dest, FirstT &&first, RestT &&... rest )
{
    dest << first;
    return print( dest, rest... );
}

/// \brief log_mutex
///
/// Get a reference to the single mutex used for logging
///
/// \return the logging mutex
///
mutex &log_mutex();

#ifdef ENABLE_SYSLOG
/// \brief log_to_syslog
///
/// get or change the syslog logging mode
///
/// call with set=true to change the logging mode
///
/// \param set true to change the logging mode via value
/// \param value syslog enable flag
/// \return current suslog enable flag
///
bool log_to_syslog( bool set = false, bool value = false );

///
/// \brief log_facility
///
/// get or change the syslog logging facility
///
/// \param set true to change the facility value
/// \param facility the facility to use when set is true
/// \return true if it was set
///
bool log_facility( bool set = false, int facility = LOG_DAEMON );
#endif

///
/// @brief log
///
/// print a list of parameters to the log file
///
template <typename FirstT, typename... RestT>
void log( FirstT &&first, RestT &&... rest )
{
    lock_guard<mutex> guard( log_mutex() );
#ifdef ENABLE_SYSLOG
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

#endif // LAMBDASTEW_LOG_HPP
