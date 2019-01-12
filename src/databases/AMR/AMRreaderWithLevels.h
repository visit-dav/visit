/*
  read in amr file data
*/
#ifndef AMR_READER_WITH_LEVELS_H
#define AMR_READER_WITH_LEVELS_H
#include <AMRreaderInterface.h>
#include <vector>
#include <map>

// ****************************************************************************
// Class: AMRreaderWithLevels
//
// Purpose:
//   Provides an AMRreader interface that uses one of the existing readers to
//   read the AMR data. We also compute all of the intermediate layers in the
//   AMR hierarchy so that we can serve up hierarchical AMR data but we retain
//   the ability to combine patches via using the existing readers for the 
//   actual data reading.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Fri May 23 15:32:50 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class AMRreaderWithLevels : public AMRreaderInterface
{
public:
  AMRreaderWithLevels();
  virtual ~AMRreaderWithLevels();

  virtual int    freedata();

  virtual int    getInfo( const char* filename );

  virtual void   BuildMetaData();

  virtual int    GetNumCycles() const;
  virtual double GetSimuTime() const;

  virtual int    GetNumberOfBlocks() const;
  virtual int    GetBlockDimensions( int bid, int* dim ) const;
  virtual int    GetBlockDefaultDimensions(int *dim) const;

  virtual int    GetNumberOfLevels();
  virtual int    GetBlockHierarchicalIndices(int bid, int *level,
                                             int *ijk_start, int *ijk_end);

  virtual int    GetBlockSize( int bid ) const ;
  virtual int    GetBlockMesh( int bid, float* xs, float* dx );
  virtual int    GetBlockVariable( int bid, int vid, float* dat );

  virtual OctKey GetBlockKey(int bid);

  virtual void   GetInterfaceSizes( int* np, int* ne ) const;
  virtual int    GetInterfaceVariable( int vid, void* dat );

  virtual bool   HasTag() const;
private:
  struct Patch
  {
      int    level;
      int    ijk_start[3];
      float  xs[3];
      float  xe[3];
      OctKey key;
      int    fileBID; // The block id from the file or -1 if the block is not present in the file.
  };

  int  FindBlock(OctKey key);
  int  MaxLevels();
  void MakeOctTree();
  void first_subdivide8(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax, OctKey current, int maxLevels);
  void subdivide8(int *ijk_start, int *ijk_end, float xMin, float yMin, float zMin, float xMax, float yMax, float zMax, OctKey current, int thisLevel, int maxLevels);
  int  AssembleBlockVariable(int bid, int vid, float *dat, const int *dims);
  int  BlockKeyToBID(const OctKey &k) const;

  AMRreaderInterface  *reader;
  bool                 patchesBuilt;
  std::vector<Patch>   patches;
  std::map<OctKey,int> blkkey2bid;
};

#endif
