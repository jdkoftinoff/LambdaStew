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

///
/// \brief The Signaler class
///
/// Allows signalling between threads with atomic counting
///
class Signaler
{
  public:
    using signal_count_type = uint32_t;

    ///
    /// \brief get_count
    ///
    /// Get the current count of signals sent
    ///
    /// \return unsigned count of number of signals sent
    ///
    signal_count_type get_count() const;

    ///
    /// \brief send_signal_all
    ///
    /// Use the condition variable to signal all threads waiting on it
    ///
    void send_signal_all();

    ///
    /// \brief send_signal_one
    ///
    /// Use the condition variable to signal only one thread waiting on it
    ///
    void send_signal_one();

    ///
    /// \brief send_signal
    ///
    /// Use the condition variable to signal either all or one thread
    /// waiting on it, depending on value of notify_all parameter
    ///
    /// \param notify_all bool true to notify all threads, false to notify only
    /// one
    ///
    void send_signal( bool notify_all )
    {
        if ( notify_all )
        {
            send_signal_all();
        }
        else
        {
            send_signal_one();
        }
    }

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
