#include "LambdaStew/Log.hpp"

namespace LambdaStew
{

std::mutex &log_mutex()
{
    static mutex m;
    return m;
}

std::ostream *log_ostream( bool set, std::ostream *o )
{
    static mutex m;
    lock_guard<mutex> guard( m );

    ostream *output_stream = &std::clog;

    if ( set )
    {
        output_stream = o;
    }

    return output_stream;
}

bool log_error_enable( bool set, bool new_value )
{
    static std::atomic<bool> current_value( true );

    bool r = current_value;

    if ( set )
    {
        current_value.store( new_value );
        r = new_value;
    }

    return r;
}

bool log_warning_enable( bool set, bool new_value )
{
    static std::atomic<bool> current_value( true );

    bool r = current_value;

    if ( set )
    {
        current_value.store( new_value );
        r = new_value;
    }

    return r;
}

bool log_debug_enable( bool set, bool new_value )
{
    static std::atomic<bool> current_value( false );

    bool r = current_value;

    if ( set )
    {
        current_value.store( new_value );
        r = new_value;
    }

    return r;
}

bool log_info_enable( bool set, bool new_value )
{
    static std::atomic<bool> current_value( true );

    bool r = current_value;

    if ( set )
    {
        current_value.store( new_value );
        r = new_value;
    }

    return r;
}

bool log_notice_enable( bool set, bool new_value )
{
    static std::atomic<bool> current_value( true );

    bool r = current_value;

    if ( set )
    {
        current_value.store( new_value );
        r = new_value;
    }

    return r;
}

bool log_crit_enable( bool set, bool new_value )
{
    static std::atomic<bool> current_value( true );

    bool r = current_value;

    if ( set )
    {
        current_value.store( new_value );
        r = new_value;
    }

    return r;
}

#ifdef ENABLE_SYSLOG

bool log_to_syslog(
    bool set, bool new_value, const char *ident, int logopt, int facility )
{
    static std::atomic_bool current_value;

    bool r = current_value;

    if ( set )
    {
        if ( current_value )
        {
            closelog();
        }

        if ( new_value )
        {
            openlog( ident, logopt, facility );
        }

        current_value.store( new_value );
        r = new_value;
    }
    return r;
}

#endif
}
