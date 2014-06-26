/*
  lower resolution
*/
#ifndef  AMR_LOWER_RESOLUTION_H
#define  AMR_LOWER_RESOLUTION_H

#include <AMRreader.h>


class AMRreaderLowRes
  : public AMRreader
{
public:
  AMRreaderLowRes() { init(); }
  virtual ~AMRreaderLowRes(){ freedata(); }

  void init();
  virtual int  freedata();
  virtual int  getInfo( const char* filename );

  virtual int GetNumberOfBlocks() const { return nlows_; };
  virtual int GetBlockMesh( int bid, float* xs, float* dx );
  virtual int GetBlockVariable( int bid, int vid, float* dat );

protected:
  int FlagLowBid();
  int RestrictBlockData( int nv, float* src, float* trg );
  int RestrictScalarArray( float** scl );
  int RestrictMesh();
  int RestrictData();
  int PreprocessData();

protected:
  int nlows_;
  int *lowbid_;
  int lmesh_, ldata_;
};


class AMRreaderLowRes2
  : public AMRreaderLowRes
{
public:
  AMRreaderLowRes2() { init(); }
  ~AMRreaderLowRes2(){ freedata(); }

  void init();
  virtual int  freedata();
  virtual int  getInfo( const char* filename );

  virtual int GetNumberOfBlocks() const { return nlows_; };
  virtual int GetBlockMesh( int bid, float* xs, float* dx );
  virtual int GetBlockVariable( int bid, int vid, float* dat );

protected:
  int FlagLowBid();
  int RestrictMesh();
  int RestrictData();
  int PreprocessData();

protected:
  int nlows_;
  int *lowbid_;
  int lmesh_, ldata_;
};




#endif
