#include "LambdaStew/MessageQueue.hpp"
#include <random>

using namespace LambdaStew;

static std::random_device rd;
static std::uniform_int_distribution<int> dis;
static std::mt19937_64 rng( rd() );
using std::string;

///
/// \brief maybe_sleep
///
/// Sleep for 3 seconds if a random number is a multiple of 32
///
static void maybe_sleep()
{
    if ( ( dis( rng ) % 32 ) == 0 )
    {
        log( "sleeping for 3: ", std::this_thread::get_id() );
        std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    }
}

void consumer_red( std::string from )
{
    log( "Consumer Red   called in context: ",
         std::this_thread::get_id(),
         " from ",
         from );
    maybe_sleep();
}

void consumer_green( std::string from )
{
    log( "Consumer Green called in context: ",
         std::this_thread::get_id(),
         " from ",
         from );
    maybe_sleep();
}

void consumer_blue( std::string from )
{
    log( "Consumer Blue  called in context: ",
         std::this_thread::get_id(),
         " from ",
         from );
    maybe_sleep();

    ///
    /// \brief count
    ///
    /// Throw an exception every 20 calls
    ///
    static int count = 0;
    if ( ++count == 20 )
    {
        throw std::runtime_error( "blue buzz 20" );
    }
}

void consumer_thread( string name, MessageQueue *q, std::atomic_bool *ender )
{
    log( name, " thread: ", std::this_thread::get_id() );

    Signaler::signal_count_type last_signal_count = 0;

    // Consume until the ender flag is set
    try
    {
        while ( !ender->load() )
        {
            // if the queue is empty, wait for up to 100ms
            if ( q->empty() )
            {
                log( name, " waiting: ", std::this_thread::get_id() );
                last_signal_count = q->signaler().wait_for_signal_for(
                    last_signal_count, std::chrono::milliseconds( 10000 ) );
            }

            // if the queue is not empty, invoke all the functions
            // in the queue, skipping any that caused exceptions
            if ( !q->empty() )
            {
                q->invoke();
            }
        }
    }
    catch ( const std::exception &e )
    {
        log( name,
             " thread: ",
             std::this_thread::get_id(),
             " caught exception: ",
             e.what() );
        throw;
    }
}

void producer_a( string name, MessageQueue *q, std::atomic_bool *ender )
{
    log( name, " thread: ", std::this_thread::get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        if ( ender->load() )
        {
            break;
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );

        q->push_back( [name]()
                      {
                          consumer_red( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 180 ) );
        q->push_back( [name]()
                      {
                          consumer_green( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
        q->push_back( [name]()
                      {
                          consumer_blue( "from " + name );
                      } );
    }
}

void producer_b( string name, MessageQueue *q, std::atomic_bool *ender )
{
    log( name, " thread: ", std::this_thread::get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        if ( ender->load() )
        {
            break;
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 75 ) );

        q->push_back( [name]()
                      {
                          consumer_red( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 90 ) );
        q->push_back( [name]()
                      {
                          consumer_green( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
        q->push_back( [name]()
                      {
                          consumer_blue( "from " + name );
                      } );

        if ( i == 20 )
        {
            log( name, " ending things" );
            ender->store( true );
            q->signaler().send_signal();
        }
    }
}

int main( int argc, char *argv[] )
{
    (void)argc;
    (void)argv;

    MessageQueue q;
    std::atomic_bool ender;
    ender.store( false );

    auto consumer1 = std::async(
        std::launch::async, consumer_thread, "Consumer1", &q, &ender );
    auto consumer2 = std::async(
        std::launch::async, consumer_thread, "Consumer2", &q, &ender );
    auto consumer3 = std::async(
        std::launch::async, consumer_thread, "Consumer3", &q, &ender );
    auto consumer4 = std::async(
        std::launch::async, consumer_thread, "Consumer4", &q, &ender );
    auto producerA1 = std::async(
        std::launch::async, producer_a, "ProducerA1", &q, &ender );
    auto producerB1 = std::async(
        std::launch::async, producer_b, "ProducerB1", &q, &ender );
    auto producerA2 = std::async(
        std::launch::async, producer_a, "ProducerA2", &q, &ender );

    producerA1.wait();
    producerB1.wait();
    producerA2.wait();
    ender.store( true );
    q.signaler().send_signal();
    consumer1.wait();
    consumer2.wait();
    consumer3.wait();
    consumer4.wait();
}
