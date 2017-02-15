/***************************************************
 ** ViSUS Visualization Project                    **
 ** Copyright (c) 2010 University of Utah          **
 ** Scientific Computing and Imaging Institute     **
 ** 72 S Central Campus Drive, Room 3750           **
 ** Salt Lake City, UT 84112                       **
 **                                                **
 ** For information about this project see:        **
 ** http://www.pascucci.org/visus/                 **
 **                                                **
 **      or contact: pascucci@sci.utah.edu         **
 **                                                **
 ****************************************************/

#ifndef _idx_io_h
#define _idx_io_h

#include <string>
#include <vector>
#include <cassert>
#include "visit_idx_io_types.h"

class IDX_IO{
    
public:
    
  IDX_IO(){};
    
  virtual bool openDataset(const std::string filename) = 0;
  
  virtual unsigned char* getData(const VisitIDXIO::Box box,
                                 const int timestate, const char* varname) = 0;
  
  inline std::vector<double> getTimes() const{
    return tsteps;
  }

  inline std::vector<VisitIDXIO::Field> getFields() const{
    return fields;
  }
  
  inline int getDimension() const{
    return dims;
  }
  
  inline int getNTimesteps() const{
    return ntimesteps;
  }
  
  inline int getMaxResolution() const{
    return max_resolution;
  }
  
  inline VisitIDXIO::Box getLogicBox() const{
      return logic_box;
  }
  
  inline const double* getLogicToPhysic() const{
      return &logic_to_physic[0];
  }
  
  inline VisitIDXIO::Field getCurrField() const{
      return curr_field;
  }

  inline bool isCompressed() const{
    return compressed_dataset;
  }
  
  virtual ~IDX_IO(){};
    
protected:
  int dims;
  int ntimesteps;
  int max_resolution;
  std::vector<double> tsteps;
  std::string dataset_url;
  std::vector<VisitIDXIO::Field> fields;
  VisitIDXIO::Box logic_box;
  VisitIDXIO::Field curr_field;
  double logic_to_physic[16];
  bool compressed_dataset;
};


#endif
