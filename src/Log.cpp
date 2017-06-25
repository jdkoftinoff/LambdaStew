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
    static std::atomic<ostream *> output_stream{&std::clog};

    if ( set )
    {
        output_stream.store( o );
    }

    return output_stream.load();
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

bool log_trace_enable( bool set, bool new_value )
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
    lock_guard<mutex> guard( log_mutex() );

    static std::atomic<bool> current_value( false );

    bool r = current_value;

    if ( set )
    {
        if ( r )
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
