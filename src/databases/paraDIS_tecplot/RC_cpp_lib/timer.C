#include <cstdio>

#include "timer.h"

/*!
  Progress(theTimer, nodenum, numnodes, thePercent, 5, theTime, 60, "Reading dumpfile"); 
 if either delta is exceeded, increase oPercent & oTime and report the progress
 return 1 if something is printed out, else return 0 
*/
int Progress(timer &iTimer, double iNum, double iMax, 
             double &oPercent, double iPercentDelta, 
             double &oTime, double iTimeDelta, char *iMsg) {
  double percent = iNum/iMax*100.0;
  double newtime = iTimer.elapsed_time();
  if (iNum == iMax || percent - oPercent > iPercentDelta || newtime-oTime > iTimeDelta) {      
    double remaining = (iMax-iNum)/(iNum!=0?iNum:1)*newtime;
    double persec= iNum/newtime;
    char msg[4096]; 
    sprintf(msg, "\r%s: %g%%;  %g/%g items, %g secs, %g/sec, left: %g", /*datestring(), */iMsg, percent, iNum, iMax, newtime, persec, remaining);
    cerr << msg << endl << flush;
    oPercent=percent;
    oTime=newtime; 
    return 1; 
  }
  return 0; 
}
