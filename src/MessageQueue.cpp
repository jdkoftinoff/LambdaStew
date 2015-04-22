#include "LambdaStew/MessageQueue.hpp"

int LambdaStew::MessageQueue::invoke()
{
    // The count of functions called
    int count = 0;
    // The temporary snapshot of the item queue
    queue<function<void()> > items_to_execute;

    // Swap the queue of items with our snapshot
    // so that we can execute them without blocking the
    // producers from adding items during this process
    {
        lock_guard<mutex> guard( m_items_mutex );
        swap( items_to_execute, m_items );
    }

    try
    {
        // Execute all the functions in the queue
        // as they are removed, and count the number
        // of items
        while ( !items_to_execute.empty() )
        {
            items_to_execute.front()();
            items_to_execute.pop();
            ++count;
        }
    }
    catch ( std::exception const &e )
    {
        /// An exception happened during the call
        /// log the exception info
        log( "LambdaQueue::invoke() caught exception: ", e.what() );
        items_to_execute.pop();

        {
            /// Copy any new pending items to the queue
            lock_guard<mutex> guard( m_items_mutex );
            swap( items_to_execute, m_items );
            while ( !items_to_execute.empty() )
            {
                m_items.push( items_to_execute.front() );
                items_to_execute.pop();
            }
        }
    }
    return count;
}

bool LambdaStew::MessageQueue::empty() const
{
    lock_guard<mutex> guard( m_items_mutex );
    return m_items.empty();
}

void LambdaStew::MessageQueue::push_back( function<void()> func )
{
    lock_guard<mutex> guard( m_items_mutex );
    m_items.push( func );
    signaler().send_signal();
}
