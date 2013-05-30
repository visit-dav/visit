#include <cstdio>
#include "timer.h"
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
    sprintf(msg, "\r%s: %6.5g%%, %8g/%-8g items, %6.5g secs elapsed, %8.5g/sec,%8.5g secs left", /*datestring(), */iMsg.c_str(), percent, iNum, iMax, newtime, persec, remaining, " ");
    cerr << msg << flush;
    oPercent=percent;
    oTime=newtime; 
    return 1; 
  }
  return 0; 
}
