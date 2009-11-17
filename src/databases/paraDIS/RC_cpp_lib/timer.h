#ifndef TIMER_H
#define TIMER_H

/* thanks to
   Kenneth Wilder
   Department of Statistics
   The University of Chicago
   5734 South University Avenue
   Chicago, IL 60637 
   
   Office: Eckhart 105
   Phone: (773) 702-8325
   Email: wilder@galton.uchicago.edu
   http://oldmill.uchicago.edu/~wilder/

   example usage: 
 int main()
{
  timer t;

  // Start the timer and then report the time required to
  // run a big loop.  The string argument to start() is
  // optional.
  t.start("Timer started");
  for (int j = 0; j < 2; ++j)
    for (unsigned int i = 0; i < 2000000000; ++i)
      ;
  cout << "Check 1: " << t << endl;

  // Restart the timer and time another loop.
  t.restart("Timer restarted");
  for (int i = 0; i < 1000000000; ++i)
    ;
  cout << "Check 2: " << t << endl;

  // Stop the timer and repeat the loop.  The third
  // timer check should report the same value as the
  // second.
  t.stop("Timer stopped");
  for (int i = 0; i < 1000000000; ++i)
    ;
  cout << "Check 3: " << t << endl;

  // Start the timer again.  Since there is no restart,
  // the timer will start from where it left off the
  // last time it was stopped.
  t.start("Timer started, not restarted");
  for (int i = 0; i < 1000000000; ++i)
    ;
  // The 'check' member function gives a default
  // method of printing the current elapsed time.
  t.check("Check 4");

  cout << "Now sleeping for 2 hours...\n";
  sleep(60*60*2);
  cout << "Check 5: " << t << endl;


  return 0;
}

  
*/
#include <visit-config.h>
#include <ctime>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef WIN32
#include <sys/timeb.h>
#include <Winsock2.h>
#endif
#include <iostream>
#include <iomanip>

class timer; 
int Progress(timer &iTimer, double iNum, double iMax, 
             double &oPercent, double iPercentDelta, 
             double &oTime, double iTimeDelta, char *iMsg);


using namespace std; 
class timer
{
 friend std::ostream& operator<<(std::ostream& os, timer& t);
 private:
  bool running;
  double start_clock;
  double acc_time;
  double time_per_tick; 
  bool mUseWallTime; 
#ifndef WIN32
  double getExactSeconds(void) {
    struct timeval t; 
    gettimeofday(&t, NULL); 
    return t.tv_sec + (double)t.tv_usec/1000000.0; 
  }
#else
  double getExactSeconds(void) {
    struct _timeb t; 
    _ftime(&t); 
   return t.time + (double)t.millitm/1000.0;
  }
#endif
 public:
  // 'running' is initially false.  A timer needs to be explicitly started
  // using 'start' or 'restart'
  timer() : running(false), start_clock(0),  acc_time(0), mUseWallTime(true) { 
    time_per_tick = 1.0/(double)CLOCKS_PER_SEC;
  }
 void useWallTime(bool use=true) {mUseWallTime=use;}
  double elapsed_time(void);
  double total_time(void);
  void start(const char* msg = 0);
  void restart(const char* msg = 0);
  void stop(const char* msg = 0);
  void check(const char* msg = 0);

}; // class timer

// __________________________________________________________________________

inline double timer::total_time(void) {
  return acc_time + elapsed_time(); 
}
// __________________________________________________________________________
// Return the time that has passed since the timer was started last.  If the
// total time is less than a minute, the time is reported to two decimal
// places.

inline double timer::elapsed_time(void)
{  
  if (running) {
    if (mUseWallTime) {
      return (time(NULL) - start_clock); 
    } else {
      return getExactSeconds() - start_clock; 
      //      return (((double)clock() - start_clock) * time_per_tick);
    }
  }
  return 0; 

} // timer::elapsed_time

// __________________________________________________________________________
// Start a timer.  If it is already running, let it continue running.
// Print an optional message.

inline void timer::start(const char* msg)
  {
  // Report the timer message
  if (msg)
    std::cout << msg << std::endl;

  // Return immediately if the timer is already running
  if (running)
    return;

  // Change timer status to running
  running = true;
  
  // Set the start time;
  if (mUseWallTime) {
    start_clock = time(NULL); 
  } else {
    start_clock=getExactSeconds(); 
    //    start_clock = clock();
  }
  
} // timer::start

// __________________________________________________________________________
// Turn the timer off and start it again from 0.  Print an optional message.

inline void timer::restart(const char* msg)
{
  // Report the timer message
  if (msg)
    std::cout << msg << std::endl;

  // Set the timer status to running
  running = true;

  // Set the accumulated time to 0 and the start time to now
  acc_time = 0;
  if (mUseWallTime) {
    start_clock = time(NULL); 
  } else {
    start_clock = getExactSeconds(); 
    //  start_clock = clock();
  }

} // timer::restart

// __________________________________________________________________________
// Stop the timer and print an optional message.

inline void timer::stop(const char* msg)
{
  // Report the timer message
  if (msg)
    std::cout << msg << std::endl;

  // Recalculate and store the total accumulated time up until now
  if (running)
    acc_time += elapsed_time();

  running = false;

} // timer::stop


// __________________________________________________________________________
// Allow timers to be printed to ostreams using the syntax 'os << t'
// for an ostream 'os' and a timer 't'.

inline std::ostream& operator<<(std::ostream& os, timer& t)
{
  /*std::ios::fmtflags saveFlags = os.flags(); 
  long savePrec = os.precision(); 
  return os << std::setprecision(4) << std::setiosflags(std::ios::fixed)
  << t.total_time() << std::setprecision(savePrec) << std::setiosflags(saveFlags); */
  return os << t.total_time();
}

#endif // TIMER_H

