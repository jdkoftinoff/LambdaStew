#include "LambdaStew/Signaler.hpp"

LambdaStew::Signaler::signal_count_type LambdaStew::Signaler::get_count() const
{
    unique_lock<mutex> guard( m_cv_mutex );
    return m_signal_count;
}

void LambdaStew::Signaler::send_signal()
{
    unique_lock<mutex> guard( m_cv_mutex );
    ++m_signal_count;
    m_cv.notify_all();
}

LambdaStew::Signaler::signal_count_type LambdaStew::Signaler::wait_for_signal(
    LambdaStew::Signaler::signal_count_type last_signal_count ) const
{
    std::unique_lock<mutex> guard( m_cv_mutex );
    if ( m_signal_count == last_signal_count )
    {
        m_cv.wait( guard );
    }
    return m_signal_count;
}
