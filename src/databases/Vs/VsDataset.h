/**
 * @file VsDataset.h
 *      
 *  @class VsDataset
 *  @brief Represents a dataset stored in an hdf5 file.
 *  
 *  Provides accessors to retrieve attributes and data from
 *  an hdf5 file.
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VS_DATASET_H_
#define VS_DATASET_H_

#include <string>
#include "VsObject.h"
#include <hdf5.h>
#include <vector>
#include <map>

class VsAttribute;

class VsDataset : public VsObject {

public:

  /**
   * Constructor
   */
  VsDataset(VsRegistry* r, VsObject* parentObject, 
            const std::string& datasetName, hid_t id);

  /** 
   * Destructor
   */
  virtual ~VsDataset();
  
  /**
   * Write debug information to log file
   */
  void write() const;

  /**
   * Get the total number of data values
   * @return number 
   */
  size_t getLength() const;

  /**
   * Get the dimensions 
   * @return number of values along each axis
   */
  std::vector<int> getDims() const;

  /**
   * Get the number of dimensions (rank)
   * @return number
   */
  size_t getNumDims() const { return this->dims.size(); }

  /**
   * Get the hdf5 type
   * @return type
   */
  hid_t getType() const { return this->dataType; }
  
private:
  std::vector<int> dims;
  hid_t dataType;
  std::vector<VsAttribute*> attributes;

  void loadDims();
};

#endif /* VS_DATASET_H_ */
