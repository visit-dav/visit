/*
  read in amr file data
*/
#ifndef AMR_READER_H
#define AMR_READER_H
    
#include <math.h>

#include <string>
using std::string;

#include <hdf5.h>

#include <EOS.h>


class AMRreader
{
 public:
  AMRreader() { init(); }
  ~AMRreader() { freedata(); init(); }

  void init();
  int  freedata();
  int  getInfo( const char* filename );


 public:
  int    GetNumCycles() const { return ncycle_; }
  double GetSimuTime() const { return tttttt_; }

  int GetNumberOfBlocks() const { return nblks_; };
  int GetBlockDimensions( int* dim ) const {
    dim[0] = blkdim_[0];
    dim[1] = blkdim_[1];
    dim[2] = blkdim_[2];
    return 0;
  }
  int GetBlockSize() const { return blksz_;  }
  int GetBlockMesh( int bid, float* xs, float* dx );
  int GetBlockVariable( int bid, int vid, float* dat );

  void GetInterfaceSizes( int* np, int* ne ) const { 
    *np = nvert_;
    *ne = nrect_;
  }
  int GetInterfaceVariable( int vid, void* dat );


 protected:
  int getAMRinfo( hid_t gid );
  int getIntfInfo( hid_t gid );
  int readAMRmesh();
  int readAMRdata();
  int readAMRadditionData();
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


 protected:
  string  filename_;
  int     ncycle_; // number of cycles
  double  tttttt_;   // simulation time

  int     nblks_; // number of leaf blocks
  int     blkdim_[4];  // number of dimensions in each block
  int     blksz_;

  float*  blkxs_;
  float*  blkdx_;
  float*  datbuf_;
  float*  prebuf_;
  float*  sndbuf_;
  float*  tmpbuf_;

  int  nrect_, nvert_;

  EOS* eos_;

public:
  static const int v_dens = 0;
  static const int v_uvel = 1;
  static const int v_vvel = 2;
  static const int v_wvel = 3;
  static const int v_pres = 4;
  static const int v_temp = 5;
  static const int v_sndv = 6;
  static const int v_xmnt = 7;
  static const int v_ymnt = 8;
  static const int v_zmnt = 9;
  static const int v_etot =10;
  static const int v_eint =11;
  static const int v_eknt =12;
  static const int v_velo =13;

  static const int i_coor=20;
  static const int i_velo=21;
  static const int i_pres=22;
  static const int i_segt=23;
/*   static const int i_uvel=24; */
/*   static const int i_vvel=25; */
/*   static const int i_wvel=26; */


protected:
  static const char* amr_grpname;
  static const char* amr_dimname;
  static const char* amr_time_name;
  static const char* amr_iter_name;
  static const char* amr_keyname;
  static const char* amr_crdname;
  static const char* amr_stpname;
  static const char* amr_datname;
  static const char* amr_prename;
  static const char* amr_sndname;
  static const char* amr_tmpname;
  static const char* amr_idealname;
  static const char* amr_jwlname;
  static const char* amr_sesamename;


  static const char* intf_grp_name;
  static const char* intf_np_name;
  static const char* intf_ne_name;
  static const char* intf_coor_name;
  static const char* intf_velo_name;
  static const char* intf_pres_name;
  static const char* intf_segt_name;
};





#endif
