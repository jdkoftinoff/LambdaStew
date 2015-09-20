#ifndef LAMBDASTEW_LOG_HPP
#define LAMBDASTEW_LOG_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <atomic>

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

///
/// template helper function to print a list of parameters to a string
///
template <typename... FirstT>
std::string print_to_string( FirstT &&... args )
{
    std::ostringstream ostr;
    print( ostr, args... );
    return ostr.str();
}

/// \brief log_mutex
///
/// Get a reference to the single mutex used for logging
///
/// \return the logging mutex
///
mutex &log_mutex();

std::ostream *log_ostream( bool set = false, std::ostream *o = &std::clog );

bool log_error_enable( bool set = false, bool new_value = true );

bool log_warning_enable( bool set = false, bool new_value = true );

bool log_trace_enable( bool set = false, bool new_value = true );

bool log_debug_enable( bool set = false, bool new_value = true );

bool log_info_enable( bool set = false, bool new_value = true );

bool log_notice_enable( bool set = false, bool new_value = true );

bool log_crit_enable( bool set = false, bool new_value = true );

#ifdef ENABLE_SYSLOG
/// \brief log_to_syslog
///
/// get or change the syslog logging mode
///
/// call with set=true to change the logging mode
///
/// \param set set to true to change the logging mode via value
/// \param new_value syslog enable flag
/// \param ident const char * identifier for syslog
/// \param logopt Syslog option flags defaults to LOG_CONS | LOG_NDELAY |
/// LOG_PERROR | LOG_PID
/// \param facility Syslog facility defaults to LOG_DAEMON
/// \return current suslog enable flag
///
bool log_to_syslog( bool set = false,
                    bool new_value = false,
                    const char *ident = "daemon",
                    int logopt = LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID,
                    int facility = LOG_DAEMON );

#endif

///
/// @brief log_info
///
/// print a list of parameters to the log file as
/// an informational line
///
template <typename FirstT, typename... RestT>
void log_info( FirstT &&first, RestT &&... rest )
{
    if ( log_info_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_INFO, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "INFO   :", first, rest... ) << std::endl;
        }
    }
}

/// log_trace
///
/// print a list of parameters to the log file as
/// a trace (debug) line
///
template <typename FirstT, typename... RestT>
void log_trace( FirstT &&first, RestT &&... rest )
{
    if ( log_trace_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_DEBUG, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "trace  :", first, rest... ) << std::endl;
        }
    }
}

/// log_debug
///
/// print a list of parameters to the log file as
/// a debug line
///
template <typename FirstT, typename... RestT>
void log_debug( FirstT &&first, RestT &&... rest )
{
    if ( log_debug_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_DEBUG, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "DEBUG  :", first, rest... ) << std::endl;
        }
    }
}

/// log_error
///
/// print a list of parameters to the log file as
/// a error line
///
template <typename FirstT, typename... RestT>
void log_error( FirstT &&first, RestT &&... rest )
{
    if ( log_error_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_ERR, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "ERROR  :", first, rest... ) << std::endl;
        }
    }
}

///
/// log_crit
///
/// print a list of parameters to the log file as
/// a critical error line
///
template <typename FirstT, typename... RestT>
void log_crit( FirstT &&first, RestT &&... rest )
{
    if ( log_crit_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_CRIT, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "CRIT   :", first, rest... ) << std::endl;
        }
    }
}

///
/// log_notice
///
/// print a list of parameters to the log file as
/// a notice line
///
template <typename FirstT, typename... RestT>
void log_notice( FirstT &&first, RestT &&... rest )
{
    if ( log_notice_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_NOTICE, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "NOTICE :", first, rest... ) << std::endl;
        }
    }
}

///
/// log_warning
///
/// print a list of parameters to the log file as
/// a warning line
///
template <typename FirstT, typename... RestT>
void log_warning( FirstT &&first, RestT &&... rest )
{
    if ( log_warning_enable() )
    {
#ifdef ENABLE_SYSLOG
        if ( log_to_syslog() )
        {
            std::stringstream o;
            print( o, first, rest... );
            lock_guard<mutex> guard( log_mutex() );
            syslog( LOG_WARNING, "%s", o.str().c_str() );
        }
        else
#endif
        {
            lock_guard<mutex> guard( log_mutex() );
            print( *log_ostream(), "WARNING:", first, rest... ) << std::endl;
        }
    }
}
}

#endif // LAMBDASTEW_LOG_HPP
