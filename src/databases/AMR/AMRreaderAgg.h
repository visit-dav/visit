/*
  aggregate amr blocks
*/
#ifndef AMR_AGG_H
#define AMR_AGG_H
    
#include <DebugStream.h>

#include <AMRreader.h>

typedef union {
  unsigned int fb[2];
  unsigned long eb;
} OctKey;


class AMRreaderAgg
  : public AMRreader
{
public:
  AMRreaderAgg() { init(); }
  ~AMRreaderAgg(){ freedata(); }

  void init() { 
    AMRreader::init();
    nagg_=aggdat_=0;
    aggbid_=NULL;
    aggsft_=NULL;
  }
  int  freedata() { 
    AMRreader::freedata();
    if( aggbid_!=NULL ) delete [] aggbid_;
    if( aggsft_!=NULL ) delete [] aggsft_; 
    nagg_=aggdat_=0;
    aggbid_=NULL;
    aggsft_=NULL;
    return 0;
  }
  int  getInfo( const char* filename ) {
    int err = AMRreader::getInfo( filename );
    if( err!=0 ) {
      debug1 << "Failed to get AMR file info in AMRreaderAgg::getInfo()\n";
      return err;
    }

    bvsz_     = 5*blksz_;
    dbdim_[0] = 2*blkdim_[0];
    dbdim_[1] = 2*blkdim_[1];
    dbdim_[2] = 2*blkdim_[2];

    err = genAggInfo();
    if( err!=0 ) {
      debug1 << "Failed to generate Aggregation info in AMRreaderAgg::getInfo()\n";
      return err;
    }
    return 0;
  }

public:
  int GetNumberOfBlocks() const { return nagg_; };
  int GetBlockDimensions( int bid, int* dim ) const {
    if( aggbid_[bid]>=0 ) {
      dim[0] = blkdim_[0];
      dim[1] = blkdim_[1];
      dim[2] = blkdim_[2];
    }
    else {
      dim[0] = dbdim_[0];
      dim[1] = dbdim_[1];
      dim[2] = dbdim_[2];
    }
    return 0;
  }
  int GetBlockSize( int bid ) const { 
    if( aggbid_[bid]>=0 ) 
      return blksz_;
    else
      return 8*blksz_;
  }
  int GetBlockMesh( int bid, float* xs, float* dx );
  int GetBlockVariable( int bid, int vid, float* dat );


protected:
  int readkey( void* buf );
  int genAggInfo();
  int genAggData();
  int genAggAdditionData();

  int genAggScalar( float* dat );


protected:
  int bvsz_;

  int dbdim_[3];

  int nagg_;
  int *aggbid_;
  long *aggsft_;
  int aggdat_;

};





#endif
