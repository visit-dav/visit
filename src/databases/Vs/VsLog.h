/**
 * @file VsLog.h
 *      
 *  @class VsLog
 *  @brief Holds output streams for different layers of debugging info
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VSLOG_H_
#define VSLOG_H_

// std includes
#include <iostream>
#include <string>
#include <vector>

class VsLog {
public:
  
  /**
   * Initialize logger
   * @param d3 low level debug messages
   * @param d4 medium level debug messages
   * @param d5 fine grain debug messages
   */
  static void initialize(std::ostream& d3, std::ostream& d4, std::ostream& d5) {
    if (singleton) {
      return;
    }
    
    singleton = new VsLog(d3, d4, d5);
  }

  /**
   * finalize logger
   */
  static void finalize() {
    if (singleton) delete singleton;
    singleton = NULL;
  }

  static std::ostream& errorLog();
  static std::ostream& warningLog();
  static std::ostream& debugLog();
  
private:
  //Constructor & destructor are never called, this is a static-only class
  VsLog(std::ostream& d3, std::ostream& d4, std::ostream& d5);
  ~VsLog() {};
  
  static VsLog* singleton;
    
  std::ostream& debugLevel3;
  std::ostream& debugLevel4;
  std::ostream& debugLevel5;
};

#endif /* VSLOG_H_ */
