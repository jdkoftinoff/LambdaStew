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
        log_info( "sleeping for 3: ", std::this_thread::get_id() );
        std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    }
}

void consumer_red( std::string from )
{
    log_info( "Consumer Red   called in context: ",
              std::this_thread::get_id(),
              " from ",
              from );
    maybe_sleep();
}

void consumer_green( std::string from )
{
    log_info( "Consumer Green called in context: ",
              std::this_thread::get_id(),
              " from ",
              from );
    maybe_sleep();
}

void consumer_blue( std::string from )
{
    log_info( "Consumer Blue  called in context: ",
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

void consumer_thread( string name, MessageQueue *q, bool consume_all )
{
    log_info( name, " thread: ", std::this_thread::get_id() );

    Signaler::signal_count_type last_signal_count = 0;

    // Consume until the ender flag is set
    try
    {
        while ( true )
        {
            // if the queue is empty, wait for up to 100ms
            if ( q->empty() )
            {
                log_info( name, " waiting: ", std::this_thread::get_id() );
                last_signal_count = q->signaler().wait_for_signal_for(
                    last_signal_count, std::chrono::milliseconds( 10000 ) );
            }

            if ( consume_all )
            {
                // if the queue is not empty, invoke all the functions in the
                // queue
                if ( !q->empty() )
                {
                    int count = q->invoke_all();
                    // sometimes count is 0 because another thread beat us to it
                    log_info( name, " invoked: ", count );
                }
            }
            else
            {
                q->invoke();
            }
        }
    }
    catch ( MessageQueue::PleaseStopException )
    {
        log_info( name, " thread: ", "Asked to stop" );
        throw;
    }
    catch ( const std::exception &e )
    {
        log_info( name,
                  " thread: ",
                  std::this_thread::get_id(),
                  " caught exception: ",
                  e.what() );
        throw;
    }
}

void producer_a( string name, MessageQueue *q )
{
    log_info( name, " thread: ", std::this_thread::get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );

        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing red" );
        q->push_back( [name]()
                      {
                          consumer_red( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 180 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing green" );
        q->push_back( [name]()
                      {
                          consumer_green( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing blue" );
        q->push_back( [name]()
                      {
                          consumer_blue( "from " + name );
                      } );
    }
}

void producer_b( string name, MessageQueue *q )
{
    log_info( name, " thread: ", std::this_thread::get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 75 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing red" );
        q->push_back( [name]()
                      {
                          consumer_red( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 90 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing green" );
        q->push_back( [name]()
                      {
                          consumer_green( "from " + name );
                      } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing blue" );
        q->push_back( [name]()
                      {
                          consumer_blue( "from " + name );
                      } );
    }
}

int main( int argc, char *argv[] )
{
#ifdef ENABLE_SYSLOG
    log_to_syslog( true, true, "example1" );
#else
    log_ostream( true, &std::clog );
#endif
    log_crit_enable( true, true );
    log_warning_enable( true, true );
    log_notice_enable( true, true );
    log_info_enable( true, true );
    log_debug_enable( true, true );

    log_info( "Starting LambdaStew example1" );

    (void)argc;
    (void)argv;

    log_debug( "Creating MessageQueue" );

    MessageQueue q;

    auto consumer1 = std::async(
        std::launch::async, consumer_thread, "Consumer1", &q, false );
    auto consumer2 = std::async(
        std::launch::async, consumer_thread, "Consumer2", &q, false );
    auto consumer3 = std::async(
        std::launch::async, consumer_thread, "Consumer3", &q, false );
    auto consumer4 = std::async(
        std::launch::async, consumer_thread, "Consumer4", &q, true );
    auto producerA1
        = std::async( std::launch::async, producer_a, "ProducerA1", &q );
    auto producerB1
        = std::async( std::launch::async, producer_b, "ProducerB1", &q );
    auto producerA2
        = std::async( std::launch::async, producer_a, "ProducerA2", &q );

    log_debug( "Waiting for producerA1" );
    producerA1.wait();

    log_debug( "Waiting for producerB1" );
    producerB1.wait();

    log_debug( "Waiting for producerA2" );
    producerA2.wait();

    log_debug( "Sending please stop to MessageQueue" );
    q.push_back_please_stop();

    log_debug( "Waiting for consumer1 to end" );
    consumer1.wait();

    log_debug( "Waiting for consumer2 to end" );
    consumer2.wait();

    log_debug( "Waiting for consumer3 to end" );
    consumer3.wait();

    log_debug( "Waiting for consumer4 to end" );
    consumer4.wait();
}
