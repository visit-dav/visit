/*
  read in amr file data
*/
#ifndef AMR_READER_H
#define AMR_READER_H
#include <AMRreaderInterface.h>
#include <EOS.h>
#include <hdf5.h>
#include <string>

class AMRreader : public AMRreaderInterface
{
public:
    AMRreader() : AMRreaderInterface()
    {
        init();
    }
    virtual ~AMRreader()
    {
        freedata();
        init();
    }

    void init();
    virtual int  freedata();
    virtual int  getInfo( const char* filename );

    virtual void BuildMetaData() { }

public:
    virtual int    GetNumCycles() const
    {
        return ncycle_;
    }
    virtual double GetSimuTime() const
    {
        return tttttt_;
    }

    virtual int GetNumberOfBlocks() const
    {
        return nblks_;
    };
    virtual int GetBlockDimensions( int bid, int* dim ) const
    {
        dim[0] = blkdim_[0];
        dim[1] = blkdim_[1];
        dim[2] = blkdim_[2];
        return 0;
    }

    virtual int GetBlockDefaultDimensions(int *dim) const
    {
        dim[0] = blkdim_[0];
        dim[1] = blkdim_[1];
        dim[2] = blkdim_[2];
        return 0;
    }

    virtual int GetBlockHierarchicalIndices(int bid, int *level,
                                            int *ijk_start, int *ijk_end)
    {
        *level = 0;
        // We could compute these off of the block key... later.
        ijk_start[0] = ijk_start[1] = ijk_start[2] = 0;
        ijk_end[0] = ijk_end[1] = ijk_end[2] = 0;
        return 0;
    }

    virtual int GetNumberOfLevels()
    {
        return 1;
    }

    virtual int GetBlockSize( int bid ) const
    {
        return blksz_;
    }
    virtual int GetBlockMesh( int bid, float* xs, float* dx );
    virtual int GetBlockVariable( int bid, int vid, float* dat );

    virtual OctKey GetBlockKey(int bid);

    virtual void GetInterfaceSizes( int* np, int* ne ) const
    {
        *np = nvert_;
        *ne = nrect_;
    }
    virtual int GetInterfaceVariable( int vid, void* dat );

    virtual bool HasTag() const
    {
        return blktag_==1;
    }


protected:
    int getAMRinfo( hid_t gid );
    int getAMRtagInfo( hid_t gid );
    int getIntfInfo( hid_t gid );
    int readAMRmesh();
    int readAMRdata();
    int readAMRadditionData();
    int readAMRtagData();
    int PreprocessData();

    /*   virtual int  readblk( int bid ); */

protected:
    int compvar( int vid, float* blk, float* buf, int sz );
    int  comp_dens( float*, float*, int sz );
    int  comp_uvel( float*, float*, int sz );
    int  comp_vvel( float*, float*, int sz );
    int  comp_wvel( float*, float*, int sz );
    int  comp_pres( float*, float*, int sz );
    int  comp_temp( float*, float*, int sz );
    int  comp_sndv( float*, float*, int sz );
    int  comp_xmnt( float*, float*, int sz );
    int  comp_ymnt( float*, float*, int sz );
    int  comp_zmnt( float*, float*, int sz );
    int  comp_etot( float*, float*, int sz );
    int  comp_eint( float*, float*, int sz );
    int  comp_eknt( float*, float*, int sz );
    int  comp_velo( float*, float*, int sz );
    int  comp_tags( float*, float*, int sz );


protected:
    std::string  filename_;
    int     ncycle_; // number of cycles
    double  tttttt_;   // simulation time
    int     blktag_;

    int     nblks_; // number of leaf blocks
    int     blkdim_[4];  // number of dimensions in each block
    int     blksz_;

    float*  blkxs_;
    float*  blkdx_;
    OctKey *blkkey_;
    float*  datbuf_;
    float*  prebuf_;
    float*  sndbuf_;
    float*  tmpbuf_;
    float*  tagbuf_;

    int  nrect_, nvert_;

    EOS* eos_;
};


extern int CompareKid2Kid0(const float* xs0, const float* xe0, const float* dx0,
                           int kid, const float* xsk, const float* dxk );
extern int isSeqEightSibling( const int* blkdim, const float*blkxs, const float* blkdx,
                              const int fst );
extern int ConsolidateBlocks(int nblks, const int* blkdim,
                             const float* blkxs, const float* blkdx,
                             int* ncb, int* sft );


#endif
