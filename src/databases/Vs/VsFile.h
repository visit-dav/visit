/**
 * @file VsFile.h
 *      
 *  @class VsFile
 *  @brief Represents an hdf5 data file
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VS_FILE_H_
#define VS_FILE_H_

#include <string>
#include "VsGroup.h"
#include "VsObject.h"
#include <hdf5.h>

class VsFile : public VsObject {
public:
  VsFile(VsRegistry* r, std::string name, hid_t id);
  virtual ~VsFile();
  
  std::string getName() const;
  void write() const;

private:
  std::string fileName;
};

#endif /* VS_FILE_H_ */
