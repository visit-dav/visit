/*
 * VsLog.C
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#include "VsLog.h"
#include <iostream>

VsLog* VsLog::singleton = NULL;

  VsLog::VsLog(std::ostream& d3, std::ostream& d4, std::ostream& d5):
    debugLevel3(d3),debugLevel4(d4),debugLevel5(d5) {
    
  }

  std::ostream& VsLog::errorLog() {
    return singleton->debugLevel3;
  }
  
  std::ostream& VsLog::warningLog() {
    return singleton->debugLevel4;
  }
  
  std::ostream& VsLog::debugLog() {
    return singleton->debugLevel5;
  }
  
