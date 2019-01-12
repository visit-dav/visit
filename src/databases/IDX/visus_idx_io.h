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

#ifndef _visus_idx_io_h
#define _visus_idx_io_h

#include "visit_idx_io.h"
#include "visit_idx_io_types.h"

#include <string>
#include <vector>
#include <cassert>

class DatasetImpl;
class AccessImpl;

// TODO generalize end extend
// Query at full resolution only

class VisusIDXIO : public IDX_IO{
    
public:
    
    VisusIDXIO(){};
    
    bool openDataset(const std::string filename);
    
    unsigned char* getData(const VisitIDXIO::Box box, const int timestate, const char* varname);
    
    virtual ~VisusIDXIO();
  
private:
  DatasetImpl* datasetImpl;
  
};


#endif
