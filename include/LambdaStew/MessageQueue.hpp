#ifndef LAMDBASTEW_MESSAGEQUEUE
#define LAMDBASTEW_MESSAGEQUEUE

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

class MessageQueue
{
  public:
    ///
    /// \brief operator ()
    ///
    /// Call all functions in the queue and remove them
    ///
    /// \return count of functions invoked
    ///
    int operator()() { return invoke(); }

    ///
    /// \brief operator ()
    ///
    /// Call all functions in the queue and remove them
    ///
    /// \return count of functions invoked
    ///
    int invoke();

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

    Signaler &signaler() { return m_signaler; }

  private:
    queue<function<void()> > m_items;
    mutable mutex m_items_mutex;

    Signaler m_signaler;
};
}

#endif // LAMBDAQUEUE1
