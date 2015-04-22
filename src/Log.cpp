#include "LambdaStew/Log.hpp"

std::mutex &LambdaStew::log_mutex()
{
    static mutex m;
    return m;
}

#ifdef ENABLE_SYSLOG

bool LambdaStew::log_to_syslog( bool set, bool value )
{
    static bool current_value = false;
    if ( set )
    {
        current_value = value;
    }
    return current_value;
}

bool LambdaStew::log_facility( bool set, int facility )
{
    static int current_facility = LOG_DAEMON;
    if ( set )
    {
        current_facility = facility;
    }
    return current_facility;
}

#endif
