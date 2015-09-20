#include "LambdaStew/MessageQueue.hpp"
#include <random>

using namespace LambdaStew;

static std::random_device rd;
static std::uniform_int_distribution<int> dis;
static std::mt19937_64 rng( rd() );
using std::string;
using std::vector;
using std::atomic_uint;
using std::this_thread::sleep_for;
using std::this_thread::get_id;
using std::shared_ptr;
using std::async;
using std::future;
using std::make_shared;

///
/// \brief maybe_sleep
///
/// Sleep for 3 seconds if a random number is a multiple of 32
///
static void maybe_sleep( string name )
{
    if ( ( dis( rng ) % 32 ) == 0 )
    {
        log_info( "sleeping for 1: ", get_id() );
        sleep_for( std::chrono::seconds( 1 ) );
    }
}

///
/// \brief red_count
///
/// Atomic count of the number of times consumer_red() is called
///
atomic_uint red_count( 0 );

///
/// \brief consumer_red
///
/// log information about thread context, increase red_count, and sleep one
/// second once in a while
///
/// \param from string thread name
///
void consumer_red( string from )
{
    log_info( "Consumer Red   called in context: ", get_id(), " from ", from );
    red_count++;
    maybe_sleep( from );
}

///
/// \brief green_count
///
/// Atomic count of the number of times consumer_green() is called
///
atomic_uint green_count( 0 );

///
/// \brief consumer_green
///
/// log information about thread context, increase green_count, and sleep one
/// second once in a while
///
/// \param from string thread name
///
void consumer_green( string from )
{
    log_info( "Consumer Green called in context: ", get_id(), " from ", from );
    green_count++;
    maybe_sleep( from );
}

///
/// \brief blue_count
///
/// Atomic count of the number of times consumer_blue() is called
///
atomic_uint blue_count( 0 );

///
/// \brief consumer_blue
///
/// log information about thread context, increase blue_count, and sleep one
/// second once in a while.  Once every 16 calls, throw a runtime error
///
/// \param from string thread name
///
void consumer_blue( string from )
{
    log_info( "Consumer Blue  called in context: ", get_id(), " from ", from );
    maybe_sleep( from );
    blue_count++;
    ///
    /// \brief count_to_exception
    ///
    /// Throw an exception every 16 calls
    ///
    static atomic_uint count_to_exception( 0 );
    if ( ( ++count_to_exception & 0xf ) == 0 )
    {
        throw std::runtime_error( "blue buzz 16" );
    }
}

///
/// \brief consumer_thread
///
/// The consumer thread function.  Waits on the MesasageQueue for functions
/// to execute.
///
/// \param name string name of the consumer thread
/// \param q the MessageQueue to consume
/// \param consumed_count reference to an atomic counter to increment on
/// consumption
///
void
    consumer_thread( string name, MessageQueue &q, atomic_uint &consumed_count )
{
    log_info( name, " thread: ", get_id() );

    log_trace(
        name, " thread: ", get_id(), " consumed_count: ", consumed_count );

    Signaler::signal_count_type last_signal_count = 0;

    // Consume until the PleaseStopException is thrown by a function
    try
    {
        while ( true )
        {
            // if the queue is empty, wait for up to 10 seconds
            if ( q.empty() )
            {
                log_trace( name,
                           " thread: ",
                           get_id(),
                           " waiting, last_signal_count: ",
                           last_signal_count );
                last_signal_count = q.signaler().wait_for_signal_for(
                    last_signal_count, std::chrono::seconds( 10 ) );
            }

            try
            {
                // invoke the next function in the queue
                if ( q.invoke() )
                {
                    // increment the consumed_count counter
                    consumed_count++;
                    log_trace( name,
                               " thread: ",
                               get_id(),
                               " consumed_count: ",
                               consumed_count );
                }
            }
            catch ( std::runtime_error &e )
            {
                // a normal runtime error is caught, log it and
                // count it as consumed
                log_warning( name,
                             " thread: ",
                             get_id(),
                             " caught runtime_error: ",
                             e.what() );
                consumed_count++;
            }
        }
    }
    catch ( MessageQueue::PleaseStopException )
    {
        // a PleaseStopException is not consumed. Log the info and re-throw it.
        log_info( name, " thread: ", get_id(), " Asked to stop" );
        throw;
    }
    catch ( const std::exception &e )
    {
        // any other unknown exception is logged and re-thrown
        log_error(
            name, " thread: ", get_id(), " caught exception: ", e.what() );
        throw;
    }
}

///
/// \brief producer_a
///
/// The first producer puts red, green, and blue consumer calls into the
/// MessageQueue
/// with varying sleeps between them, incrementing the produced_count each time.
///
/// Pushes 100 red, 100 green, and 100 blue for a total of 300 items
///
/// \param name Name of the producer thread
/// \param q MessageQueue to submit functions to
/// \param produced_count reference to atomic counter of production count
///
void producer_a( string name, MessageQueue &q, atomic_uint &produced_count )
{
    log_info( name, " thread: ", get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        log_trace(
            name, " thread: ", get_id(), " produced_count: ", produced_count );

        sleep_for( std::chrono::milliseconds( 150 ) );

        log_trace( name, " thread: ", get_id(), " pushing red" );
        q.push_back( [name]()
                     {
                         consumer_red( name );
                     } );
        produced_count++;

        sleep_for( std::chrono::milliseconds( 180 ) );
        log_trace( name, " thread: ", get_id(), " pushing green" );
        q.push_back( [name]()
                     {
                         consumer_green( name );
                     } );
        produced_count++;

        sleep_for( std::chrono::milliseconds( 290 ) );
        log_trace( name, " thread: ", get_id(), " pushing blue" );
        q.push_back( [name]()
                     {
                         consumer_blue( name );
                     } );
        produced_count++;
    }
}

///
/// \brief producer_a
///
/// The first producer puts blue, green and red consumer calls into the
/// MessageQueue
/// with varying sleeps between them, incrementing the produced_count each time
///
/// Pushes 100 red, 100 green, and 100 blue for a total of 300 items
///
/// \param name Name of the producer thread
/// \param q MessageQueue to submit functions to
/// \param produced_count reference to atomic counter of production count
///
void producer_b( string name, MessageQueue &q, atomic_uint &produced_count )
{
    log_info( name, " thread: ", get_id() );

    for ( int i = 0; i < 100; ++i )
    {
        sleep_for( std::chrono::milliseconds( 75 ) );
        log_trace( name, " thread: ", get_id(), " pushing blue" );
        q.push_back( [name]()
                     {
                         consumer_blue( name );
                     } );
        produced_count++;

        sleep_for( std::chrono::milliseconds( 110 ) );
        log_trace( name, " thread: ", get_id(), " pushing green" );
        q.push_back( [name]()
                     {
                         consumer_green( name );
                     } );
        produced_count++;

        sleep_for( std::chrono::milliseconds( 90 ) );
        log_trace( name, " thread: ", get_id(), " pushing red" );
        q.push_back( [name]()
                     {
                         consumer_red( name );
                     } );
        produced_count++;
    }
}

///
/// \brief test_producers_and_consumers
///
/// Main test fucntion:
///
/// Creates N consumer threads, M*2 producer threads, and atomic counters for
/// each
///
/// Creates counters for each
///
/// \return
///
bool test_producers_and_consumers( int num_consumers, int num_producers )
{
    log_debug( "Creating MessageQueue: consumers = ",
               num_consumers,
               " Total Producers = ",
               num_producers * 2 );

    MessageQueue q;

    vector<shared_ptr<atomic_uint> > consumer_counts;
    vector<future<void> > consumers;

    for ( int consumer = 0; consumer < num_consumers; ++consumer )
    {
        consumer_counts.push_back( make_shared<std::atomic_uint>( 0 ) );
        consumers.push_back( async( std::launch::async,
                                    consumer_thread,
                                    print_to_string( "Consumer", consumer + 1 ),
                                    std::ref( q ),
                                    std::ref( *consumer_counts.back() ) ) );
    }

    std::vector<std::shared_ptr<std::atomic_uint> > producer_counts;
    std::vector<std::future<void> > producers;

    for ( int producer = 0; producer < num_producers; ++producer )
    {
        producer_counts.push_back( make_shared<std::atomic_uint>( 0 ) );
        producers.push_back(
            async( std::launch::async,
                   producer_a,
                   print_to_string( "ProducerA", producer + 1 ),
                   std::ref( q ),
                   std::ref( *producer_counts.back() ) ) );
    }

    for ( int producer = 0; producer < num_producers; ++producer )
    {
        producer_counts.push_back( make_shared<std::atomic_uint>( 0 ) );
        producers.push_back(
            async( std::launch::async,
                   producer_b,
                   print_to_string( "ProducerB", producer + num_producers + 1 ),
                   std::ref( q ),
                   std::ref( *producer_counts.back() ) ) );
    }

    log_info( "Waiting for producers to end" );
    for ( auto &producer : producers )
    {
        producer.wait();
    }

    log_info( "Pause 2 seconds before showing producer counts" );
    sleep_for( std::chrono::seconds( 2 ) );

    log_info( "Items in queue:", q.size() );

    {
        size_t item = 0;
        for ( auto const &producer_count : producer_counts )
        {
            log_info( "Producer", item + 1, " total:", *producer_count );
        }
    }

    {
        size_t item = 0;
        for ( auto const &consumer_count : consumer_counts )
        {
            log_info( "Consumer", item + 1, " total:", *consumer_count );
        }
    }

    log_debug( "Sending please stop to MessageQueue" );
    q.push_back_please_stop();

    log_debug( "Waiting for consumers to end" );
    for ( auto &consumer : consumers )
    {
        consumer.wait();
    }

    log_info( "Report" );

    log_info( "Signal Count:", q.signaler().get_count() );

    unsigned int producer_total = 0;
    {
        size_t item = 0;
        for ( auto const &producer_count : producer_counts )
        {
            log_trace( "Producer", ++item, " total:", *producer_count );
            producer_total += *producer_count;
        }
    }
    log_info( "Producer total:", producer_total );

    unsigned int consumer_total = 0;

    {
        size_t item = 0;
        for ( auto const &consumer_count : consumer_counts )
        {
            log_trace( "Consumer", ++item, " total:", *consumer_count );
            consumer_total += *consumer_count;
        }
    }

    log_info( "Consumer total:", consumer_total );

    log_info( "Red Count:", red_count );
    log_info( "Green count", green_count );
    log_info( "Blue Count:", blue_count );

    log_info( "Items in queue (should be 1):", q.size() );

    if ( q.size() != 1 )
    {
        log_error(
            "FAILURE: items in queue is not 1 (the PleaseStopException)" );
        return false;
    }

    if ( producer_total == consumer_total )
    {
        log_info( "SUCCESS: Produced messages equals consumed messages" );
        return true;
    }
    else
    {
        log_error(
            "FAILURE: Produced messages does not equal consumed messages" );
        return false;
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
    log_trace_enable( true, false );
    log_debug_enable( true, true );

    log_info( "Starting LambdaStew example1" );

    (void)argc;
    (void)argv;

    if ( test_producers_and_consumers( 20, 3 ) )
    {
        return 0;
    }
    else
    {
        return 255;
    }
}
