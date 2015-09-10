#ifndef LAMDBASTEW_MESSAGEQUEUE_HPP
#define LAMDBASTEW_MESSAGEQUEUE_HPP

#include "Log.hpp"
#include "Signaler.hpp"

#include <functional>
#include <vector>
#include <algorithm>
#include <thread>
#include <queue>
#include <future>

namespace LambdaStew
{
using std::function;
using std::queue;
using std::vector;
using std::swap;

///
/// \brief The MessageQueue class
///
class MessageQueue
{
  public:
    struct PleaseStopException
    {
    };

    ///
    /// \brief make_please_stop_item
    ///
    /// Creates a lambda that when added to the MessageQueue and processed
    /// will cause the thread to cleanly exit
    ///
    /// \return function which throws an appropriate exception to trigger end of
    /// thread
    ///
    std::function<void()> make_please_stop_item() const;

    ///
    /// \brief push_back_please_stop
    ///
    /// Put a 'please_stop' item into the queue to ask all consumers to stop
    /// processing cleanly
    ///
    void push_back_please_stop();

    ///
    /// \brief operator ()
    ///
    /// Call all functions in the queue and remove them
    ///
    /// \return count of functions invoked
    ///
    int operator()() { return invoke_all(); }

    ///
    /// \brief invoke_all ()
    ///
    /// Call all functions in the queue and remove them
    ///
    /// \return count of functions invoked
    ///
    int invoke_all();

    ///
    /// \brief invoke
    ///
    /// Call one function from the queue and remove it
    ///
    void invoke();

    ///
    /// \brief empty
    ///
    /// \return true if the queue is currently empty
    ///
    bool empty() const;

    ///
    /// \brief push_back
    ///
    /// Add item to the queue
    ///
    /// \param func callable function which takes no parameters and returns void
    ///
    void push_back( function<void()> func );

    ///
    /// \brief skip_next
    ///
    /// Remove the next item from the queue without calling it
    ///
    void skip_next()
    {
        lock_guard<mutex> guard( m_items_mutex );
        m_items.pop();
    }

    ///
    /// \brief signaler
    ///
    /// get the signaller for the queue
    ///
    /// \return A Reference to the Signaler object
    ///
    Signaler &signaler() { return m_signaler; }

  private:
    ///
    /// \brief m_items
    ///
    /// The queue of functions to executed in a different thread context
    ///
    queue<function<void()> > m_items;

    ///
    /// \brief m_items_mutex
    ///
    /// The mutex for the function queue
    ///
    mutable mutex m_items_mutex;

    ///
    /// \brief m_signaler
    ///
    /// The signaler used to wake up any threads waiting for an item
    /// to be added to the queue
    ///
    Signaler m_signaler;
};
}

#endif // LAMDBASTEW_MESSAGEQUEUE_HPP
