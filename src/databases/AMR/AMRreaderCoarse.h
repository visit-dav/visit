/*
  coarse amr blocks
*/
#ifndef AMR_COARSE_H
#define AMR_COARSE_H

#include <AMRreaderAgg.h>

class AMRreaderCoarse
  : public AMRreaderAgg
{
public:
  AMRreaderCoarse() { init(); }
  virtual ~AMRreaderCoarse(){ freedata(); }

public:
  virtual int GetBlockDimensions( int bid, int* dim ) const {
    return AMRreader::GetBlockDimensions( bid, dim );
  }
  virtual int GetBlockSize( int bid ) const { 
    return AMRreader::GetBlockSize( bid );
  }
  virtual int GetBlockMesh( int bid, float* xs, float* dx );
  virtual int GetBlockVariable( int bid, int vid, float* dat );

 protected:
  int genCoarseMesh();
  int genCoarseData();
  int genCoarseScalar( float** dat );
  int genCoarseAdditionData();

};

#endif
