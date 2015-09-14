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
        log_info( "sleeping for 1: ", std::this_thread::get_id() );
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    }
}

std::atomic_int red_count( 0 );

void consumer_red( std::string from )
{
    log_info( "Consumer Red   called in context: ",
              std::this_thread::get_id(),
              " from ",
              from );
    red_count++;
    maybe_sleep();
}

std::atomic_int green_count( 0 );

void consumer_green( std::string from )
{
    log_info( "Consumer Green called in context: ",
              std::this_thread::get_id(),
              " from ",
              from );
    green_count++;
    maybe_sleep();
}

std::atomic_int blue_count( 0 );

void consumer_blue( std::string from )
{
    log_info( "Consumer Blue  called in context: ",
              std::this_thread::get_id(),
              " from ",
              from );
    maybe_sleep();
    blue_count++;
    ///
    /// \brief count
    ///
    /// Throw an exception every 10 calls
    ///
    static std::atomic_int count( 0 );
    if ( ++count == 10 )
    {
        count = 0;
        throw std::runtime_error( "blue buzz 10" );
    }
}

void consumer_thread( string name,
                      MessageQueue &q,
                      std::atomic_int &consumed_count )
{
    log_info( name, " thread: ", std::this_thread::get_id() );

    Signaler::signal_count_type last_signal_count = 0;

    // Consume until the ender flag is set
    try
    {
        while ( true )
        {
            // if the queue is empty, wait for up to 100ms
            if ( q.empty() )
            {
                log_info( name, " waiting: ", std::this_thread::get_id() );
                last_signal_count = q.signaler().wait_for_signal_for(
                    last_signal_count, std::chrono::milliseconds( 10000 ) );
            }

            try
            {
                if ( q.invoke() )
                {
                    consumed_count++;
                }
            }
            catch ( std::runtime_error &e )
            {
                log_info( "Runtime error caught: ", e.what() );
                consumed_count++;
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

void producer_a( string name, MessageQueue &q, std::atomic_int &produced_count )
{
    log_info( name, " thread: ", std::this_thread::get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );

        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing red" );
        q.push_back( [name]()
                     {
                         consumer_red( "from " + name );
                     } );
        produced_count++;

        std::this_thread::sleep_for( std::chrono::milliseconds( 180 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing green" );
        q.push_back( [name]()
                     {
                         consumer_green( "from " + name );
                     } );
        produced_count++;

        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing blue" );
        q.push_back( [name]()
                     {
                         consumer_blue( "from " + name );
                     } );
        produced_count++;
    }
}

void producer_b( string name, MessageQueue &q, std::atomic_int &produced_count )
{
    log_info( name, " thread: ", std::this_thread::get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 75 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing red" );
        q.push_back( [name]()
                     {
                         consumer_red( "from " + name );
                     } );
        produced_count++;

        std::this_thread::sleep_for( std::chrono::milliseconds( 90 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing green" );
        q.push_back( [name]()
                     {
                         consumer_green( "from " + name );
                     } );
        produced_count++;

        std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
        log_info(
            name, " thread: ", std::this_thread::get_id(), " pushing blue" );
        q.push_back( [name]()
                     {
                         consumer_blue( "from " + name );
                     } );
        produced_count++;
    }
}

bool test_producers_and_consumers()
{
    log_debug( "Creating MessageQueue" );

    MessageQueue q;

    std::atomic_int consumer1_count( 0 );
    auto consumer1 = std::async( std::launch::async,
                                 consumer_thread,
                                 "Consumer1",
                                 std::ref( q ),
                                 std::ref( consumer1_count ) );

    std::atomic_int consumer2_count( 0 );
    auto consumer2 = std::async( std::launch::async,
                                 consumer_thread,
                                 "Consumer2",
                                 std::ref( q ),
                                 std::ref( consumer2_count ) );

    std::atomic_int consumer3_count( 0 );
    auto consumer3 = std::async( std::launch::async,
                                 consumer_thread,
                                 "Consumer3",
                                 std::ref( q ),
                                 std::ref( consumer3_count ) );

    std::atomic_int consumer4_count( 0 );
    auto consumer4 = std::async( std::launch::async,
                                 consumer_thread,
                                 "Consumer4",
                                 std::ref( q ),
                                 std::ref( consumer4_count ) );

    std::atomic_int producer1_count( 0 );
    auto producer1 = std::async( std::launch::async,
                                 producer_a,
                                 "Producer1",
                                 std::ref( q ),
                                 std::ref( producer1_count ) );

    std::atomic_int producer2_count( 0 );
    auto producer2 = std::async( std::launch::async,
                                 producer_b,
                                 "Producer2",
                                 std::ref( q ),
                                 std::ref( producer2_count ) );

    std::atomic_int producer3_count( 0 );
    auto producer3 = std::async( std::launch::async,
                                 producer_a,
                                 "Producer3",
                                 std::ref( q ),
                                 std::ref( producer3_count ) );

    log_debug( "Waiting for producer1 to finish" );
    producer1.wait();

    log_debug( "Waiting for producer2 to finish" );
    producer2.wait();

    log_debug( "Waiting for producer3 to finish" );
    producer3.wait();

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

    log_info( "Items in queue:", q.size() );
    log_info( "Producer1 total:", producer1_count );
    log_info( "Producer2 total:", producer2_count );
    log_info( "Producer3 total:", producer3_count );
    log_info( "Consumer1 total:", consumer1_count );
    log_info( "Consumer2 total:", consumer2_count );
    log_info( "Consumer3 total:", consumer3_count );
    log_info( "Consumer4 total:", consumer4_count );
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

    log_info( "Report" );

    log_info( "Signal Count:", q.signaler().get_count() );

    int producer_total = producer1_count + producer2_count + producer3_count;
    log_info( "Producer total:", producer_total );

    int consumer_total = consumer1_count + consumer2_count + consumer3_count
                         + consumer4_count;
    log_info( "Consumer total:", consumer_total );

    log_info( "Producer1 total:", producer1_count );
    log_info( "Producer2 total:", producer2_count );
    log_info( "Producer3 total:", producer3_count );

    log_info( "Consumer1 total:", consumer1_count );
    log_info( "Consumer2 total:", consumer2_count );
    log_info( "Consumer3 total:", consumer3_count );
    log_info( "Consumer4 total:", consumer4_count );

    log_info( "Red Count:", red_count );
    log_info( "Green count", green_count );
    log_info( "Blue Count:", blue_count );

    log_info( "Items in queue:", q.size() );

    return ( producer_total == consumer_total );
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

    if ( test_producers_and_consumers() )
    {
        return 0;
    }
    else
    {
        return 255;
    }
}
