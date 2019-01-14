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

#ifndef _pidx_idx_io_h
#define _pidx_idx_io_h

#include <string>
#include <vector>
#include <cassert>

#include <PIDX.h>

#include "visit_idx_io.h"
#include "visit_idx_io_types.h"

typedef std::string String;

class PIDXIO : public IDX_IO{

public:
    
    PIDXIO(){}
  
    bool openDataset(const std::string filename);
    
    unsigned char* getData(const VisitIDXIO::Box box, const int timestate, const char* varname);
    
    std::vector<int> getGlobalSize();

    virtual ~PIDXIO();

};


#endif
