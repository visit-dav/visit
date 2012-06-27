    

#include <DebugStream.h>
#include <AMRreader.h>

#include <visit-hdf5.h>

#include <string.h>

#include <string>

const char* AMRreader::amr_grpname="AMR";
const char* AMRreader::amr_dimname="BlockDim";
const char* AMRreader::amr_time_name="SimuTime";
const char* AMRreader::amr_iter_name="Ncycles";
const char* AMRreader::amr_keyname="BlockKey";
const char* AMRreader::amr_crdname="BlockCoord";
const char* AMRreader::amr_stpname="BlockSteps";
const char* AMRreader::amr_datname="BlockData";
const char* AMRreader::amr_prename="BlockPressure";
const char* AMRreader::amr_sndname="BlockSoundSpeed";
const char* AMRreader::amr_tmpname="BlockTemperature";
const char* AMRreader::amr_idealname="IdealGasEOS";
const char* AMRreader::amr_jwlname="JwlEOS";
const char* AMRreader::amr_sesamename="SesameEOStable";


const char* AMRreader::intf_grp_name="Interface";
const char* AMRreader::intf_np_name="nnode";
const char* AMRreader::intf_ne_name="nsegt";
const char* AMRreader::intf_coor_name="Coordinate";
const char* AMRreader::intf_velo_name="Velocity";
const char* AMRreader::intf_pres_name="Pressure";
const char* AMRreader::intf_segt_name="Segment";

void AMRreader::
init()
{
  filename_="";
  nblks_=0;
  nrect_=nvert_=0;
  blkxs_=blkdx_=datbuf_=NULL;
  prebuf_=sndbuf_=tmpbuf_=NULL;
  eos_=NULL;

  // Turn off error message printing.
  H5Eset_auto2(0,0,0);
}



int AMRreader::
freedata()
{
  if( blkxs_!=NULL ) delete [] blkxs_;
  if( blkdx_!=NULL ) delete [] blkdx_;
  if( datbuf_!=NULL ) delete [] datbuf_;
  if( prebuf_!=NULL ) delete [] prebuf_;
  if( sndbuf_!=NULL ) delete [] sndbuf_;
  if( tmpbuf_!=NULL ) delete [] tmpbuf_;
  if( eos_!=NULL ) delete eos_;
  nblks_=0;
  nrect_=nvert_=0;
  blkxs_=blkdx_=datbuf_=NULL;
  eos_=NULL;
  return 0;
}




int AMRreader::
getInfo( const char* filename )
{
  debug2 << "opening AMR file " << filename << "\n";

  filename_ = filename;

  hid_t file_id = H5Fopen( filename, H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id<0 ) {
    debug1 << "Failed to open AMR file: " << filename << ".\n";
    return -1;
  }

  hsize_t nobjs;
  herr_t  err = H5Gget_num_objs( file_id, &nobjs );
  debug2 << "nobjs in " << filename << " is " << nobjs << "\n";
  for( hsize_t i=0; i<nobjs; i++ ) {
    int type = H5Gget_objtype_by_idx( file_id, i );
    if( type == H5G_GROUP ) {
      char name[1000];
      ssize_t l= H5Gget_objname_by_idx( file_id, i, name, 1000 );

      hid_t gid = H5Gopen1( file_id, name );

      if( strcmp( name, amr_grpname )==0 ) {
    if( getAMRinfo( gid )!=0 ) {
      debug1 << "Failed to retrieve AMR info in " << filename << "\n";
      H5Gclose(gid);
      return -2;
    }}
      else if( strcmp( name, intf_grp_name )==0 ) {
    if( getIntfInfo( gid ) != 0 ) {
      debug1 << "Failed to retrieve Interface info in " << filename << "\n";
      return -3;
    }}
      else {
    debug1 << "Find a unrecongnized group " << name << " in " << filename << "\n";
      }

      H5Gclose(gid);
    }
  }

  H5Fclose( file_id );
  return 0;
}



int AMRreader:: 
getAMRinfo( hid_t gid )
{
  float rbuf[20];

  hid_t aid = H5Aopen_name( gid, "number" );
  if( aid<0 ) {
    nblks_=0;
    debug1 << "Failed to find number of blocks.\n";
    return -1;
  }
  else {
    H5Aread( aid, H5T_NATIVE_INT, &nblks_ );
    H5Aclose(aid);
  }
  debug2 << "Number of blocks is " << nblks_ << "\n";

  aid = H5Aopen_name( gid, amr_dimname );
  if( aid<0 ) {
    debug1 << "Failed to find dimensions of block.\n";
    return -2;
  }
  else {
    int dd[10];
    H5Aread( aid, H5T_NATIVE_INT, dd );
    H5Aclose(aid);
    blkdim_[0]=dd[0];
    blkdim_[1]=dd[1];
    blkdim_[2]=dd[2];
    blkdim_[3]=dd[3];
    blksz_ = blkdim_[0]*blkdim_[1]*blkdim_[2];
  }
  debug2 << "Dimensions of blocks are [" 
     << blkdim_[0] << "," 
     << blkdim_[1] << "," 
     << blkdim_[2] << "," 
     << blkdim_[3] << " ].\n";

  aid = H5Aopen_name( gid, amr_time_name );
  if( aid<0 ) {
    debug1 << "Failed to find SimuTime.\n";
    return -3;
  }
  H5Aread( aid, H5T_NATIVE_DOUBLE, &tttttt_ );
  H5Aclose(aid);
  debug2 << "SimuTime= " << tttttt_ << "\n";
  
  aid = H5Aopen_name( gid, amr_iter_name );
  if( aid<0 ) {
    debug1 << "Failed to find Ncycles.\n";
    return -4;
  }
  H5Aread( aid, H5T_NATIVE_INT, &ncycle_ );
  H5Aclose(aid);
  debug2 << "Ncycles= " << ncycle_ << "\n";

  // eos
  htri_t est = H5Aexists( gid, amr_idealname );
  if( est>0 ) {
    aid = H5Aopen_name( gid, amr_idealname );
    H5Aread( aid, H5T_NATIVE_FLOAT, rbuf );
    H5Aclose(aid);
    eos_ = new IdealEOS( rbuf[0], rbuf[1] );
    return 0;
  }

  est = H5Aexists( gid, amr_jwlname );
  if( est>0 ) {
    aid = H5Aopen_name( gid, amr_jwlname );
    H5Aread( aid, H5T_NATIVE_FLOAT, rbuf );
    H5Aclose(aid);
    eos_ = new JwlEOS( rbuf[0], rbuf[1], rbuf[2], rbuf[3], 
              rbuf[4], rbuf[5], rbuf[6] );
    return 0;
  }

  est = H5Aexists( gid, amr_sesamename );
  if( est>0 ) {
    eos_ = new SesameEOS();
    return 0;
  }

  eos_ = new IdealEOS( 1.4, 1.0 );
  return 0;
}




int AMRreader::
getIntfInfo( hid_t gid )
{
  hid_t aid = H5Aopen_name( gid, intf_np_name );
  if( aid<0 ) {
    nvert_=0;
    debug1 << "Failed to find number of interface points.\n";
    return -1;
  }
  else {
    H5Aread( aid, H5T_NATIVE_INT, &nvert_ );
    H5Aclose(aid);
  }
  debug2 << "nvert is " << nvert_ << "\n";

  aid = H5Aopen_name( gid, intf_ne_name );
  if( aid<0 ) {
    nrect_=0;
    debug1 << "Failed to find number of interface elements.\n";
    return -2;
  }
  else {
    H5Aread( aid, H5T_NATIVE_INT, &nrect_ );
    H5Aclose(aid);
  }
  debug2 << "nrect is " << nrect_ << "\n";

  return 0;
}



int AMRreader::
readAMRmesh()
{
  //if( blkxs_!=NULL ) return 0;

  hid_t file_id = H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id<0 ) {
    debug1 << "Failed to open AMR file: " << filename_ << " when read in mesh.\n";
    return -1;
  }

  hid_t gid = H5Gopen1( file_id, amr_grpname );
  if( gid<0 ) {
    debug1 << "Failed to open AMR group in " << filename_ << " when read in mesh.\n";
    return -2;
  }

  blkxs_ = new float[3*nblks_];
  blkdx_ = new float[3*nblks_];
  if( blkxs_==NULL ||  blkdx_==NULL ) {
    debug1 << "Failed to allocate blkxs_ or blkddx_ for " << filename_ << ".\n";
    return -3;
  }

  hid_t xsid = H5Dopen1( gid, amr_crdname );
  if( xsid<0 ) {
    debug1 << "Failed to open block coordinates in " << filename_ << ".\n";
    return -4;
  }
  herr_t herr = H5Dread( xsid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, blkxs_ );
  H5Dclose(xsid);
  if( herr<0 ) {
    debug1 << "Failed to read block coordinates in " << filename_ << ".\n";
    return -5;
  }

  hid_t dxid = H5Dopen1( gid, amr_stpname );
  if( dxid<0 ) {
    debug1 << "Failed to open block steps in " << filename_ << ".\n";
    return -6;
  }
  herr = H5Dread( dxid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, blkdx_ );
  H5Dclose(dxid);
  if( herr<0 ) {
    debug1 << "Failed to read block steps in " << filename_ << ".\n";
    return -7;
  }

  H5Gclose( gid );
  H5Fclose( file_id );
  return 0;
}



int AMRreader::
readAMRdata()
{
  //  if( datbuf_!=NULL ) return 0;

  hid_t file_id = H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id<0 ) {
    debug1 << "Failed to open AMR file: " << filename_ << " when read in mesh.\n";
    return -1;
  }

  hid_t gid = H5Gopen1( file_id, amr_grpname );
  if( gid<0 ) {
    debug1 << "Failed to open AMR group in " << filename_ << " when read in mesh.\n";
    return -2;
  }

  datbuf_ = new float[ 5*blksz_*nblks_];
  if( datbuf_==NULL ) {
    debug1 << "Failed to allocate datbuf_ for " << filename_ << ".\n";
    return -3;
  }

  hid_t datid = H5Dopen1( gid, amr_datname );
  if( datid<0 ) {
    debug1 << "Failed to open block data in " << filename_ << ".\n";
    return -4;
  }
  herr_t herr = H5Dread( datid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, datbuf_ );
  H5Dclose(datid);
  if( herr<0 ) {
    debug1 << "Failed to read block data in " << filename_ << ".\n";
    return -5;
  }

  H5Gclose( gid );
  H5Fclose( file_id );
  return 0;
}



int AMRreader::
readAMRadditionData()
{
  hid_t file_id = H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id<0 ) {
    debug1 << "Failed to open AMR file: " << filename_ << " when read in mesh.\n";
    return -1;
  }

  hid_t gid = H5Gopen1( file_id, amr_grpname );
  if( gid<0 ) {
    debug1 << "Failed to open AMR group in " << filename_ << " when read in mesh.\n";
    return -2;
  }

  prebuf_ = new float[ blksz_*nblks_ ];
  sndbuf_ = new float[ blksz_*nblks_ ];
  tmpbuf_ = new float[ blksz_*nblks_ ];
  if( prebuf_==NULL  || sndbuf_==NULL  || tmpbuf_==NULL ) {
    debug1 << "Failed to allocate additional datbuf for " << filename_ << ".\n";
    return -3;
  }

  hid_t datid = H5Dopen1( gid, amr_prename );
  if( datid<0 ) {
    debug1 << "Failed to open " << amr_prename << " data in " << filename_ << ".\n";
    return -4;
  }
  herr_t herr = H5Dread( datid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, prebuf_ );
  H5Dclose(datid);
  if( herr<0 ) {
    debug1 << "Failed to read " << amr_prename << " in " << filename_ << ".\n";
    return -5;
  }

  datid = H5Dopen1( gid, amr_sndname );
  if( datid<0 ) {
    debug1 << "Failed to open " << amr_sndname << " data in " << filename_ << ".\n";
    return -4;
  }
  herr = H5Dread( datid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, sndbuf_ );
  H5Dclose(datid);
  if( herr<0 ) {
    debug1 << "Failed to read " << amr_sndname << " in " << filename_ << ".\n";
    return -5;
  }

  datid = H5Dopen1( gid, amr_tmpname );
  if( datid<0 ) {
    debug1 << "Failed to open " << amr_tmpname << " data in " << filename_ << ".\n";
    return -4;
  }
  herr = H5Dread( datid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, tmpbuf_ );
  H5Dclose(datid);
  if( herr<0 ) {
    debug1 << "Failed to read " << amr_tmpname << " in " << filename_ << ".\n";
    return -5;
  }

  H5Gclose( gid );
  H5Fclose( file_id );
  return 0;
}





int AMRreader::
GetBlockMesh( int bid, float* xs, float* dx )
{
  if( blkxs_==NULL ) {
    if( readAMRmesh()!=0 ) {
      debug1 << "Failed to read AMR mesh.\n";
      return -1;
    }
  }

  int sft=3*bid;

  xs[0] = blkxs_[sft];
  xs[1] = blkxs_[sft+1];
  xs[2] = blkxs_[sft+2];

  dx[0] = blkdx_[sft];
  dx[1] = blkdx_[sft+1];
  dx[2] = blkdx_[sft+2];
  return 0;
}



int AMRreader::
GetBlockVariable( int bid, int vid, float* dat ) 
{
  if( datbuf_==NULL ) {
    if( readAMRdata()!=0 ) {
      debug1 << "Failed to read in AMR data.\n";
      return -1;
    }
    if( eos_->EOStype()==SesameEOS_type ) {
      if( readAMRadditionData()!=0 ) {
    debug1 << "Failed to read in AMR additional data.\n";
    return -1;
      }}
  }

  int ierr = compvar( vid, datbuf_+(5*blksz_*bid), dat, blksz_ );
  if( ierr!=0 ) {
    debug1 << "Failed to compute requested variable: " << vid << " .\n";
    return ierr;
  }
  return 0;
}




int AMRreader::
GetInterfaceVariable( int vid, void* dat ) 
{
  std::string vname;
  hid_t  mtype;
  switch(vid) {
  case(i_coor):  vname = intf_coor_name;  mtype=H5T_NATIVE_FLOAT;  break;
  case(i_velo):  vname = intf_velo_name;  mtype=H5T_NATIVE_FLOAT;  break;
  case(i_pres):  vname = intf_pres_name;  mtype=H5T_NATIVE_FLOAT;  break;
  case(i_segt):  vname = intf_segt_name;  mtype=H5T_NATIVE_INT;    break;
  default:
    debug1 << "Unknown variable id " << vid << " .\n";
    return -1;
  }

  hid_t file_id = H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id<0 ) {
    debug1 << "Failed to open AMR file: " << filename_ << " when read in mesh.\n";
    return -2;
  }

  hid_t gid = H5Gopen1( file_id, intf_grp_name );
  if( gid<0 ) {
    debug1 << "Failed to open interface group in " << filename_ << " when read in mesh.\n";
    return -3;
  }

  hid_t dtid = H5Dopen1( gid, vname.c_str() );
  if( dtid<0 ) {
    debug1 << "Failed to open interface variable " << vname << " in " << filename_ << ".\n";
    return -4;
  }
  herr_t herr = H5Dread( dtid, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, dat );
  H5Dclose(dtid);
  if( herr<0 ) {
    debug1 << "Failed to read interface variable " << vname << " in " << filename_ << ".\n";
    return -5;
  }

  H5Gclose( gid );
  H5Fclose( file_id );
  return 0;
}





int AMRreader::
compvar( int vid, float* blk, float* buf, int sz )
{
  switch( vid ) {
  case(v_dens): comp_dens( blk, buf, sz );   break;
  case(v_uvel): comp_uvel( blk, buf, sz );   break;
  case(v_vvel): comp_vvel( blk, buf, sz );   break;
  case(v_wvel): comp_wvel( blk, buf, sz );   break;
  case(v_pres): comp_pres( blk, buf, sz );   break;
  case(v_temp): comp_temp( blk, buf, sz );   break;
  case(v_sndv): comp_sndv( blk, buf, sz );   break;
  case(v_xmnt): comp_xmnt( blk, buf, sz );   break;
  case(v_ymnt): comp_ymnt( blk, buf, sz );   break;
  case(v_zmnt): comp_zmnt( blk, buf, sz );   break;
  case(v_etot): comp_etot( blk, buf, sz );   break;
  case(v_eint): comp_eint( blk, buf, sz );   break;
  case(v_eknt): comp_eknt( blk, buf, sz );   break;
  case(v_velo): comp_velo( blk, buf, sz );   break;
  default:
    debug1 << "Unknown variable id " << vid << " .\n";
    return -1;
  }
  return 0;
}




int AMRreader::
comp_dens( float* blkdt, float* buf, int sz )
{
//   for( int i=0; i<blkdim_[2]; i++ ) 
//     for( int j=0; j<blkdim_[1]; j++ ) 
//       for( int k=0; k<blkdim_[0]; k++ ) {
//     int sft = k + ( j + i*blkdim_[1] )*blkdim_[0];
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft];
  }
  return 0;
}

int AMRreader::
comp_uvel( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+1] / blkdt[5*sft];
  }
  return 0;
}

int AMRreader::
comp_vvel( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+2] / blkdt[5*sft];
  }
  return 0;
}

int AMRreader::
comp_wvel( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+3] / blkdt[5*sft];
  }
  return 0;
}

int AMRreader::
comp_pres( float* blkdt, float* buf, int sz )
{
  if( eos_->EOStype()==SesameEOS_type ) {
  long off = (blkdt - datbuf_)/5;
  float* pbuf = prebuf_ + off;
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = pbuf[sft];
  }}
  else {
  for( int sft=0; sft<sz; sft++ ) {
    float rr = blkdt[5*sft];
    float uu = blkdt[5*sft+1] / rr;
    float vv = blkdt[5*sft+2] / rr;
    float ww = blkdt[5*sft+3] / rr;
    float et = blkdt[5*sft+4] / rr;
    float ei = et - 0.5*( uu*uu + vv*vv + ww*ww );
    buf[sft] = eos_->p_from_r_e( rr, ei );
  }}
  return 0;
}

int AMRreader::
comp_temp( float* blkdt, float* buf, int sz )
{
  if( eos_->EOStype()==SesameEOS_type ) {
  long off = (blkdt - datbuf_)/5;
  float* pbuf = tmpbuf_ + off;
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = pbuf[sft];
  }}
  else {
  for( int sft=0; sft<sz; sft++ ) {
    float rr = blkdt[5*sft];
    float uu = blkdt[5*sft+1] / rr;
    float vv = blkdt[5*sft+2] / rr;
    float ww = blkdt[5*sft+3] / rr;
    float et = blkdt[5*sft+4] / rr;
    float ei = et - 0.5*( uu*uu + vv*vv + ww*ww );
    buf[sft] = eos_->T_from_r_e( rr, ei );
  }}
  return 0;
}

int AMRreader::
comp_sndv( float* blkdt, float* buf, int sz )
{
  if( eos_->EOStype()==SesameEOS_type ) {
  long off = (blkdt - datbuf_)/5;
  float* pbuf = sndbuf_ + off;
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = pbuf[sft];
  }}
  else {
  for( int sft=0; sft<sz; sft++ ) {
    float rr = blkdt[5*sft];
    float uu = blkdt[5*sft+1] / rr;
    float vv = blkdt[5*sft+2] / rr;
    float ww = blkdt[5*sft+3] / rr;
    float et = blkdt[5*sft+4] / rr;
    float ei = et - 0.5*( uu*uu + vv*vv + ww*ww );
    buf[sft] = eos_->a_from_r_e( rr, ei );
  }}
  return 0;
}

int AMRreader::
comp_xmnt( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+1];
      }
  return 0;
}

int AMRreader::
comp_ymnt( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+2];
      }
  return 0;
}

int AMRreader::
comp_zmnt( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+3];
      }
  return 0;
}

int AMRreader::
comp_etot( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    buf[sft] = blkdt[5*sft+4];
      }
  return 0;
}

int AMRreader::
comp_eint( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    float rr = blkdt[5*sft];
    float uu = blkdt[5*sft+1] / rr;
    float vv = blkdt[5*sft+2] / rr;
    float ww = blkdt[5*sft+3] / rr;
    float et = blkdt[5*sft+4] / rr;
    buf[sft] = et - 0.5*( uu*uu + vv*vv + ww*ww );
      }
  return 0;
}

int AMRreader::
comp_eknt( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    float rr = blkdt[5*sft];
    float uu = blkdt[5*sft+1] / rr;
    float vv = blkdt[5*sft+2] / rr;
    float ww = blkdt[5*sft+3] / rr;
    buf[sft] = 0.5*( uu*uu + vv*vv + ww*ww );
      }
  return 0;
}

int AMRreader::
comp_velo( float* blkdt, float* buf, int sz )
{
  for( int sft=0; sft<sz; sft++ ) {
    float rr = blkdt[5*sft];
    float uu = blkdt[5*sft+1] / rr;
    float vv = blkdt[5*sft+2] / rr;
    float ww = blkdt[5*sft+3] / rr;
    buf[3*sft]   = uu;
    buf[3*sft+1] = vv;
    buf[3*sft+2] = ww;
      }
  return 0;
}


