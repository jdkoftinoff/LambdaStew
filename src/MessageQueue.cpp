#include "LambdaStew/MessageQueue.hpp"

namespace LambdaStew
{

std::function<void()> MessageQueue::make_please_stop_item() const
{
    return []()
    {
        throw PleaseStopException();
    };
}

void MessageQueue::push_back_please_stop()
{
    push_back( make_please_stop_item() );
}

bool MessageQueue::invoke()
{
    // get the item to execute

    function<void()> item_to_execute;

    {
        lock_guard<mutex> guard( m_items_mutex );
        if ( m_items.size() > 0 )
        {
            item_to_execute = m_items.front();
            m_items.pop();
        }
    }

    if ( item_to_execute )
    {
        try
        {
            item_to_execute();
        }
        catch ( PleaseStopException const &e )
        {
            // The message was to shut down the thread
            log_info(
                "MessageQueue::invoke() asked to end thread via "
                "PleaseStopException" );

            // put the item back on the item list for other threads to receive
            push_back( item_to_execute );

            // re-throw
            throw;
        }
        catch ( std::exception const &e )
        {
            // An exception happened during the call
            // log the exception info
            log_info( "MessageQueue::invoke() caught exception: ", e.what() );

            // Re-throw the exception
            throw;
        }
        catch ( ... )
        {
            // An unknown exception
            log_info( "MessageQueue::invoke() caught exception" );

            // Re-throw the exception
            throw;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool MessageQueue::empty() const
{
    lock_guard<mutex> guard( m_items_mutex );
    return m_items.empty();
}

void MessageQueue::push_back( function<void()> func )
{
    lock_guard<mutex> guard( m_items_mutex );
    m_items.push( func );
    // send the signal to waiting threads only if the number of items
    // transitioned from 0 to 1
    if ( m_items.size() == 1 )
    {
        signaler().send_signal();
    }
}

size_t MessageQueue::size() const
{
    lock_guard<mutex> guard( m_items_mutex );
    return m_items.size();
}
}
