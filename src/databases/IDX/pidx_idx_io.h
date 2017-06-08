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

#include "visit_idx_io.h"
#include "visit_idx_io_types.h"

#include <PIDX.h>

#include <string>
#include <vector>
#include <cassert>

typedef std::string String;

//class DatasetImpl;
//class AccessImpl;

class PIDXIO : public IDX_IO{

public:
    
    //TODO: remove use_raw
    PIDXIO(){use_raw = false;}

    PIDXIO(bool _use_raw){ use_raw = _use_raw; };
    
    bool openDataset(const std::string filename);
    
    unsigned char* getData(const VisitIDXIO::Box box, const int timestate, const char* varname);
    
    virtual ~PIDXIO();

    bool use_raw;
  
};


#endif
