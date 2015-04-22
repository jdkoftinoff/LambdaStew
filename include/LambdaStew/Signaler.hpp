#ifndef LAMBDASTEW_SIGNALER_HPP
#define LAMBDASTEW_SIGNALER_HPP

#include "Log.hpp"

#include <condition_variable>
#include <chrono>
#include <mutex>

namespace LambdaStew
{
using std::lock_guard;
using std::unique_lock;
using std::condition_variable;

class Signaler
{
  public:
    using signal_count_type = uint32_t;

    signal_count_type get_count() const;

    ///
    /// \brief send_signal
    /// Use the condition variable to signal all threads waiting on it
    ///
    void send_signal();

    ///
    /// \brief wait_for_signal
    ///
    /// Use the condition variable to wait for a signal
    ///
    signal_count_type
        wait_for_signal( signal_count_type last_signal_count ) const;

    ///
    /// \brief wait_for_signal_for
    ///
    /// Use the condition variable to wait for a signal for a specific amount of
    /// time
    ///
    template <typename TimeT>
    signal_count_type wait_for_signal_for( signal_count_type last_signal_count,
                                           TimeT t ) const
    {
        std::unique_lock<mutex> guard( m_cv_mutex );
        if ( m_signal_count == last_signal_count )
        {
            m_cv.wait_for( guard, t );
        }
        return m_signal_count;
    }

  private:
    mutable condition_variable m_cv;
    mutable mutex m_cv_mutex;
    signal_count_type m_signal_count = 0;
};
}

#endif // LAMBDASTEW_SIGNALER_HPP
