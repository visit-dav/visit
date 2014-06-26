/*
  read in amr file data
*/
#ifndef AMR_READER_INTERFACE_H
#define AMR_READER_INTERFACE_H
#include <OctKey.h>
#include <AMRconfigure.h>

// ****************************************************************************
// Class: AMRreaderInterface
//
// Purpose:
//   Abstract base class that provides an interface for returning AMR data.
//
// Notes:    The bulk of the API was taken from the old AMRreader class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 23 15:35:10 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class AMRreaderInterface
{
public:
  AMRreaderInterface();
  virtual ~AMRreaderInterface();

  virtual int    freedata() = 0;

  virtual int    getInfo( const char* filename ) = 0;

  virtual void   BuildMetaData() = 0;

  virtual int    GetNumCycles() const = 0;
  virtual double GetSimuTime() const = 0;

  virtual int    GetNumberOfBlocks() const = 0;
  virtual int    GetBlockDimensions( int bid, int* dim ) const = 0;
  virtual int    GetBlockDefaultDimensions(int *dim) const = 0;

  virtual int    GetNumberOfLevels() = 0;
  virtual int    GetBlockHierarchicalIndices(int bid, int *level,
                                             int *ijk_start, int *ijk_end) = 0;

  virtual int    GetBlockSize( int bid ) const  = 0;
  virtual int    GetBlockMesh( int bid, float* xs, float* dx ) = 0;
  virtual int    GetBlockVariable( int bid, int vid, float* dat ) = 0;

  virtual OctKey GetBlockKey(int bid) = 0;

  virtual void   GetInterfaceSizes( int* np, int* ne ) const = 0;
  virtual int    GetInterfaceVariable( int vid, void* dat ) = 0;

  virtual bool   HasTag() const = 0;

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
  static const int v_tags =14;

  static const int i_coor=20;
  static const int i_velo=21;
  static const int i_pres=22;
  static const int i_segt=23;
  static const int i_matl=24;
/*   static const int i_uvel=24; */
/*   static const int i_vvel=25; */
/*   static const int i_wvel=26; */

  static const int kid2i[8];
  static const int kid2j[8];
  static const int kid2k[8];

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
  static const char* amr_tagname;
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
  static const char* intf_matl_name;
};

// Function to return instances of the various interfaces.
AMRreaderInterface *InstantiateAMRreader();

#endif
