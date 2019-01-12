
#ifndef  AMR_LOW_RES_BLOCK_CONSOLIDATION_H
#define  AMR_LOW_RES_BLOCK_CONSOLIDATION_H

#include <AMRreaderLowRes.h>

class AMRreaderLowResBlkConso
  : public AMRreaderLowRes
{
public:
  AMRreaderLowResBlkConso() { init(); }
  virtual ~AMRreaderLowResBlkConso(){ freedata(); }

  void init();
  virtual int  freedata();
  virtual int  getInfo( const char* filename );

  virtual int GetNumberOfBlocks() const { return nbc_; };
  virtual int GetBlockDimensions( int bid, int* dim ) const;
  virtual int GetBlockSize( int bid ) const;
  virtual int GetBlockMesh( int bid, float* xs, float* dx );
  virtual int GetBlockVariable( int bid, int vid, float* dat );

protected:
  int Consolidate();
  int CombineBlockData( int nv, float* bd[8], float* src );
  int CombineScalarArray( float*bd[8], float* scl );
  int CombineData();
  int PreprocessData();

protected:
  int nbc_;
  int *bcsft_;
  int ldata_;

};




class AMRreaderLowRes2BlkConso
  : public AMRreaderLowRes2
{
public:
  AMRreaderLowRes2BlkConso() { init(); }
  ~AMRreaderLowRes2BlkConso(){ freedata(); }

  void init();
  virtual int  freedata();
  virtual int  getInfo( const char* filename );

  virtual int GetNumberOfBlocks() const { return nbc_; };
  virtual int GetBlockDimensions( int bid, int* dim ) const;
  virtual int GetBlockSize( int bid ) const;
  virtual int GetBlockMesh( int bid, float* xs, float* dx );
  virtual int GetBlockVariable( int bid, int vid, float* dat );

protected:
  int Consolidate();
  int CombineBlockData( int nv, float* bd[8], float* src );
  int CombineScalarArray( float*bd[8], float* scl );
  int CombineData();
  int PreprocessData();

protected:
  int nbc_;
  int *bcsft_;
  int ldata_;
};

#endif
