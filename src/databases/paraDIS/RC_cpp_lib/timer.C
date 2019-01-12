#include <cstdio>
#include "timer.h"
#include <iostream>
#include <string> 
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
using namespace boost; 
using namespace std;


/*!
  Progress(theTimer, nodenum, numnodes, thePercent, 5, theTime, 60, "Reading dumpfile"); 
 if either delta is exceeded, increase oPercent & oTime and report the progress
 return 1 if something is printed out, else return 0 
*/
int Progress(timer &iTimer, double iNum, double iMax, 
             double &oPercent, double iPercentDelta, 
             double &oTime, double iTimeDelta,  string iMsg) {
  double percent = iNum/iMax*100.0;
  double newtime = iTimer.elapsed_time();
  if (iNum == iMax || percent - oPercent > iPercentDelta || newtime-oTime > iTimeDelta) {      
    double remaining = (iMax-iNum)/(iNum!=0?iNum:1)*newtime;
    double persec= iNum/newtime;
    char msg[4096]; 
    sprintf(msg, "\r%s: %6.5g%%, %8g/%-8g, %6.5g secs, %8.5g/sec, %8.5g sec left", /*datestring(), */iMsg.c_str(), percent, iNum, iMax, newtime, persec, remaining);
    cerr << msg << flush;
    oPercent=percent;
    oTime=newtime; 
    return 1; 
  }
  return 0; 
}

// __________________________________________________________________________

double timer::total_time(void) {
  return acc_time + elapsed_time(); 
}
// __________________________________________________________________________
// Return the time that has passed since the timer was started last.  If the
// total time is less than a minute, the time is reported to two decimal
// places.

double timer::elapsed_time(void)
{  
  if (running) {
    if (mUseWallTime) {
      return (time(NULL) - start_clock); 
    } else {
      return GetExactSeconds() - start_clock; 
      //      return (((double)clock() - start_clock) * time_per_tick);
    }
  }
  return 0; 

} // timer::elapsed_time

// __________________________________________________________________________
// Start a timer.  If it is already running, let it continue running.
// Print an optional message.

void timer::start(const char* msg)
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
    start_clock=GetExactSeconds(); 
    //    start_clock = clock();
  }
  
} // timer::start

// __________________________________________________________________________
// Turn the timer off and start it again from 0.  Print an optional message.

void timer::restart(const char* msg)
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
    start_clock = GetExactSeconds(); 
    //  start_clock = clock();
  }

} // timer::restart

// __________________________________________________________________________
// Stop the timer and print an optional message.

void timer::stop(const char* msg)
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

std::ostream& operator<<(std::ostream& os, timer& t)
{
  /*std::ios::fmtflags saveFlags = os.flags(); 
  long savePrec = os.precision(); 
  return os << std::setprecision(4) << std::setiosflags(std::ios::fixed)
  << t.total_time() << std::setprecision(savePrec) << std::setiosflags(saveFlags); */
  return os << t.total_time();
}


  
// =================================================================
/* Search the entire known universe for a time string match.  
   This is awful but it works. 
*/ 
bool GetTimeFromString(string s, struct tm &tms) {
  
  boost::trim(s);
  //cerr << "Checking time \"" << s << "\"" << endl;
  vector<string> delims; 
  delims.push_back(" "); 
  delims.push_back(" , "); 

  vector<string> yearformats; 
  yearformats.push_back(" ");
  yearformats.push_back("%Y");
  yearformats.push_back("%Y -");
  yearformats.push_back("- %Y");
  yearformats.push_back("%Y /");
  yearformats.push_back("/ %Y");

  vector<string> weekdays; 
  weekdays.push_back(" ");
  weekdays.push_back("%A"); 
  weekdays.push_back("%a"); 

  vector<string> dateformats; 
  dateformats.push_back(" "); 
  dateformats.push_back("%m / %d ");
  dateformats.push_back("%m - %d");
  dateformats.push_back("%e %b"); 
  dateformats.push_back("%b %e"); 
  dateformats.push_back("%B %e"); 

  vector<string> prefixes; 
  prefixes.push_back(" "); 
  prefixes.push_back(" : "); 
  prefixes.push_back(" - "); 

  vector<string> bases; 
  bases.push_back(" "); 
  bases.push_back("%H : %M : %S"); 
  bases.push_back("%H : %M"); 
  bases.push_back("%H"); 

  vector<string> mods; 
  mods.push_back(" "); 
  mods.push_back("%p"); 
  mods.push_back("%P");

  vector<string>zones;
  zones.push_back(" "); 
  //zones.push_back("%Z"); // does not work
  zones.push_back("%z");

  vector<string> timeformats; 
  for (vector<string>::iterator prefix = prefixes.begin(); prefix != prefixes.end(); prefix++) {
    for (vector<string>::iterator base = bases.begin(); base != bases.end(); base++) {
      for (vector<string>::iterator mod = mods.begin(); mod != mods.end(); mod++) {
        for (vector<string>::iterator zone = zones.begin(); zone != zones.end(); zone++) {
          timeformats.push_back(str(boost::format("%1% %2% %3% %4%")%(*prefix)%(*base)%(*mod)%(*zone)));
        }
      }
    }
  }

  time_t now = time(NULL); 
  boost::trim(s);
  char *m = 0; 
  uint32_t numattempts = 0; 
  for (vector<string>::iterator yearformat = yearformats.begin(); yearformat != yearformats.end(); yearformat++) {
    for (vector<string>::iterator dateformat = dateformats.begin(); dateformat != dateformats.end(); dateformat++) {
      for (vector<string>::iterator weekday = weekdays.begin(); weekday != weekdays.end(); weekday++) {
        for (vector<string>::iterator timeformat = timeformats.begin(); timeformat != timeformats.end(); timeformat++) {
          if (*yearformat == " " && *dateformat == " " && *weekday == " " && *timeformat == " ") {
            continue;
          }
          vector<string>timestrings; 
          timestrings.push_back(str(boost::format("%1% %2% %3% %4%")%(*weekday)%(*dateformat)%(*yearformat)%(*timeformat))); 
          timestrings.push_back(str(boost::format("%1% %2% %3% %4%")%(*weekday)%(*dateformat)%(*timeformat)%(*yearformat))); 
          for (vector<string>::iterator de1 = delims.begin(); de1 != delims.end(); de1++) {
            for (vector<string>::iterator de2 = delims.begin(); de2 != delims.end(); de2++) {
              if (*de1 != " " || *de2 != " ") {
                timestrings.push_back(str(boost::format("%1% %2% %3% %4% %5% %6%")%(*weekday)%(*de1)%(*dateformat)%(*de2)%(*yearformat)%(*timeformat))); 
                timestrings.push_back(str(boost::format("%1% %2% %3% %4% %5%")%(*weekday)%(*dateformat)%(*timeformat)%(*de2)%(*yearformat))); 
              }
            }
            for (uint32_t i = 0; i < timestrings.size(); i++) {
              numattempts++; 
              tms.tm_year = localtime(&now)->tm_year;
              tms.tm_hour = tms.tm_min = tms.tm_sec = 0; 
              tms.tm_mon = tms.tm_wday = tms.tm_yday = 0; 
              tms.tm_mday = 1; 
              const char *sp = s.c_str(); 
              m = strptime(sp, timestrings[i].c_str(), &tms);
              if (m && !*m) {
                cerr << "matched format " << numattempts << ": " << timestrings[i] << endl; 
                return true;           
              }
            }
          }
        }
      }
    }
  }
  //cerr << "no format matches out of " << numattempts << endl;
  return false; 
}


// =================================================================
/*bool GetTimeFromString(string s, struct tm &tms) {
  vector<string> timestrings = GetTimeStrings();
  time_t now = time(NULL); 
  boost::trim(s);
  char *m = 0; 
  for (uint32_t i = 0; i < timestrings.size(); i++) {
    tms.tm_year = localtime(&now)->tm_year; 
    const char *sp = s.c_str(); 
    m = strptime(sp, timestrings[i].c_str(), &tms);
    if (m && !*m) {
      cout << "matched format " << timestrings[i] << endl; 
      return true; 
    }
  }
  return false; 
  }*/ 

// =================================================================
string GetStandardTimeStringFromString(string s) {
  struct tm ts;
  ts.tm_year = 0; 
  if  (!GetTimeFromString(s, ts)) {
    return INVALID_TIME_STRING;
  }
  s = asctime(&ts);
  boost::trim (s); 
  return s;
}
  

