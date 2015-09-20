#include "LambdaStew/Signaler.hpp"

namespace LambdaStew
{

Signaler::signal_count_type Signaler::get_count() const
{
    unique_lock<mutex> guard( m_cv_mutex );
    return m_signal_count;
}

void Signaler::send_signal_all()
{
    unique_lock<mutex> guard( m_cv_mutex );
    ++m_signal_count;
    m_cv.notify_all();
}

void Signaler::send_signal_one()
{
    unique_lock<mutex> guard( m_cv_mutex );
    ++m_signal_count;
    m_cv.notify_one();
}

Signaler::signal_count_type Signaler::wait_for_signal(
    Signaler::signal_count_type last_signal_count ) const
{
    std::unique_lock<mutex> guard( m_cv_mutex );
    if ( m_signal_count == last_signal_count )
    {
        m_cv.wait( guard );
    }
    return m_signal_count;
}
}
