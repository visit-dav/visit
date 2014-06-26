/*
  block consolidation
*/
#ifndef  AMR_BLOCK_CONSOLIDATION_H
#define  AMR_BLOCK_CONSOLIDATION_H

#include <AMRreader.h>


class AMRreaderBlkConso : public AMRreader
{
public:
  AMRreaderBlkConso() { init(); }
  virtual ~AMRreaderBlkConso(){ freedata(); }

  void init();
  virtual int  freedata();
  virtual int  getInfo( const char* filename );

  virtual int GetNumberOfBlocks() const { return nbc_; };
  virtual int GetBlockDimensions( int bid, int* dim ) const;
  virtual int GetBlockSize( int bid ) const;
  virtual int GetBlockMesh( int bid, float* xs, float* dx );
  virtual int GetBlockVariable( int bid, int vid, float* dat );

  virtual OctKey GetBlockKey(int bid);

protected:
  int FlagLowBid();
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
