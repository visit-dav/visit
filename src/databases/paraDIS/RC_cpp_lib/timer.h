#ifndef TIMER_H
#define TIMER_H

 /*!
   VisIt has its quirks
 */
#ifdef RC_CPP_VISIT_BUILD
#include <visit-config.h>
 // this does not work on chaos machines outside of VisIt, maybe it's a VisIt thing?  
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#else
 // if not in VisIt, then just include sys/time.h
#include <sys/time.h>
#endif
#include <stdio.h>
#include <ctime>
#ifdef WIN32
#include <sys/timeb.h>
#include <Winsock2.h>
#endif
#include <iostream>
#include <iomanip>
#include "stringutil.h"
#define INVALID_TIME_STRING "INVALID_TIME_STRING"

using namespace std; 
string GetStandardTimeStringFromString(string s);
bool GetTimeFromString(string s, struct tm &tms); 

int Progress(class timer &iTimer, double iNum, double iMax, 
             double &oPercent, double iPercentDelta, 
             double &oTime, double iTimeDelta,  std::string iMsg);



// ======================================================================== 
// Get current date/time, format "%Y-%m-%d.%X" means YYYY-MM-DD.HH:mm:ss
static const inline std::string timestamp(const char *format=NULL) {
  string fmtstring; 
  if (!format || string(format)=="") {
    fmtstring = "%Y-%m-%d.%X";
  } else {
    fmtstring = format; 
  }
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[1000];
  tstruct = *localtime(&now);
  // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
  // for more information about date/time format
  strftime(buf, sizeof(buf), fmtstring.c_str(), &tstruct);
  
  return buf;
}

// ======================================================================== 
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
  
*/
class timer
{
 friend std::ostream& operator<<(std::ostream& os, timer& t);
 private:
  bool running;
  double start_clock;
  double acc_time;
  double time_per_tick; 
  bool mUseWallTime; 
 public:
  static std::string GetExactSecondsString(void) {
    //return QTime::currentTime().toString("ssss.zzz").toStdString(); 
    std::string s = doubleToString(GetExactSeconds(), 3);
    int first = s.size() -8; 
    if (first<0) first = 0; 
    return s.substr(first, 8); 
  }

  static double GetExactSeconds(void) {
#ifndef WIN32
    struct timeval t; 
    gettimeofday(&t, NULL); 
    return t.tv_sec + (double)t.tv_usec/1000000.0; 
#else
    struct _timeb t; 
    _ftime(&t); 
   return t.time + (double)t.millitm/1000.0;
#endif
  }
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


#endif // TIMER_H

