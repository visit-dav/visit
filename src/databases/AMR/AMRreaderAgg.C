    
#include <math.h>

#include <iomanip>
using std::setw;

#include <AMRreaderAgg.h>


const int kid2i[8] = {0,1,0,1,0,1,0,1};
const int kid2j[8] = {0,0,1,1,0,0,1,1};
const int kid2k[8] = {0,0,0,0,1,1,1,1};

const unsigned long LeafTemp=0x7;


#define WP(x,y,z)  << x << "  " << y << "  " << z << "\n"
void draw( ostream& out, float* xs, float* xe )
{
  out << "\n\n\n"
    WP( xs[0], xs[1], xs[2] )
    WP( xe[0], xs[1], xs[2] )
    WP( xe[0], xe[1], xs[2] )
    WP( xs[0], xe[1], xs[2] )
    WP( xs[0], xs[1], xs[2] )

    WP( xs[0], xs[1], xe[2] )
    WP( xe[0], xs[1], xe[2] )
    WP( xe[0], xe[1], xe[2] )
    WP( xs[0], xe[1], xe[2] )
    WP( xs[0], xs[1], xe[2] )

    WP( xe[0], xs[1], xe[2] )
    WP( xe[0], xs[1], xs[2] )
    WP( xe[0], xe[1], xs[2] )
    WP( xe[0], xe[1], xe[2] )
    WP( xs[0], xe[1], xe[2] )
    WP( xs[0], xe[1], xs[2] );
}

#define MeshEqual( x0, x1, dx )  ( fabs(x0-x1)<1.e-3*dx )
int CompareKid2Kid0( const float* xs0, const float* xe0, const float* dx0,
             int kid, const float* xsk, const float* dxk )
{
  int res=1;
  if( !MeshEqual( dx0[0], dxk[0], dx0[0] ) ||
      !MeshEqual( dx0[1], dxk[1], dx0[1] ) ||
      !MeshEqual( dx0[2], dxk[2], dx0[2] ) )
    return res;

  switch( kid ) {
  case(1):
    if( MeshEqual( xe0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xs0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xs0[2], xsk[2], dx0[2] ) )
      res=0;
    break;

  case(2):
    if( MeshEqual( xs0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xe0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xs0[2], xsk[2], dx0[2] ) )
      res=0;
    break;

  case(3):
    if( MeshEqual( xe0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xe0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xs0[2], xsk[2], dx0[2] ) )
      res=0;
    break;

  case(4):
    if( MeshEqual( xs0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xs0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xe0[2], xsk[2], dx0[2] ) )
      res=0;
    break;

  case(5):
    if( MeshEqual( xe0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xs0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xe0[2], xsk[2], dx0[2] ) )
      res=0;
    break;

  case(6):
    if( MeshEqual( xs0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xe0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xe0[2], xsk[2], dx0[2] ) )
      res=0;
    break;

  case(7):
    if( MeshEqual( xe0[0], xsk[0], dx0[0] ) &&
    MeshEqual( xe0[1], xsk[1], dx0[1] ) &&
    MeshEqual( xe0[2], xsk[2], dx0[2] ) )
      res=0;
    break;
  }
  return res;
}




int AMRreaderAgg::
readkey( void* buf )
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

  hid_t tpid = H5Dopen1( gid, amr_keyname );
  if( tpid<0 ) {
    debug1 << "Failed to open block key in " << filename_ << ".\n";
    return -3;
  }
  herr_t herr = H5Dread( tpid, H5T_STD_U32LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf );
  H5Dclose(tpid);
  if( herr<0 ) {
    debug1 << "Failed to read block key in " << filename_ << ".\n";
    return -4;
  }

  H5Gclose( gid );
  H5Fclose( file_id );
  return 0;
}



#ifdef USING_OCTKEY
//
// using OctKey to find leaf sibling. Not working. key???
//
int AMRreaderAgg::
genAggInfo()
{
  OctKey* key = new OctKey[nblks_];
  if( key==NULL ) {
    debug1 << "Failed to allocate OctKey for " << filename_ << ".\n";
    return -1;
  }

  if( readkey(&(key[0].fb[0]))!=0 ) {
    debug1 << "Failed to read in Octkey in file " << filename_ << ".\n";
    return -2;
  }
//   // write out OctKey to check
//   debug2 << "write in 64b\n";
//   for( int i=0; i<nblks_; i++ ) 
//     debug2 << setw(8) << i << std::hex 
//        << setw(20) << key[i].eb << std::dec << "\n";
//   debug2 << "write in 32\n";
//   for( int i=0; i<nblks_; i++ ) 
//     debug2 << setw(8) << i << std::hex 
//        << setw(12) << key[i].fb[1] 
//        << setw(8) << key[i].fb[0] << std::dec << "\n";

  aggbid_ = new int[nblks_];
  aggsft_ = new long[nblks_];
  if( aggbid_==NULL || aggsft_==NULL ) {
    debug1 << "Failed to allocate aggbid_,  aggsft_ for " << filename_ << ".\n";
    return -1;
  }

  unsigned long pk,ck;
  int pos=0;
  int cnt=0;
  while(pos+7<nblks_) {
    pk = key[pos].eb;
    if( (pk&LeafTemp) == 0 ) {
      int j;
      for( j=1; j<7; j++ ) {
    ck = key[pos+j].eb;
    if( pk+1 != ck ) break;
    pk=ck;
      }

      if( j>=7 ) {
    aggbid_[cnt++] = -(pos+1);
    pos+=8;
    continue;
      }
    }
    aggbid_[cnt++] = pos++;
  }
  while(pos<nblks_) 
    aggbid_[cnt++] = pos++;
  nagg_ = cnt;

  for( int i=0; i<nagg_; i++ ) {
    pos = aggbid_[i];
    if( pos<0 ) pos = -(pos+1);
    aggsft_[i] = (long)pos*(long)bvsz_;
  }
  debug2 << "after aggregation, nblks= " << nagg_ << " instead of " << nblks_ << ".\n";

  delete [] key;
  return 0;
}

#else

//
// using block coordinates to identify siblings. Works. But expensive
//
int AMRreaderAgg::
genAggInfo()
{
  aggbid_ = new int[nblks_];
  aggsft_ = new long[nblks_];
  if( aggbid_==NULL || aggsft_==NULL ) {
    debug1 << "Failed to allocate aggbid_,  aggsft_ for " << filename_ << ".\n";
    return -1;
  }

  if( blkxs_==NULL ) {
    if( readAMRmesh()!=0 ) {
      debug1 << "Failed to read AMR mesh.\n";
      return -1;
    }
  }

  float xs[3],dx[3],xe[3],xk[3],dk[3];
  int pos=0;
  int cnt=0;
  while(pos<nblks_) {
    if( pos+7<nblks_ ) {
    xs[0] = blkxs_[3*pos];
    xs[1] = blkxs_[3*pos+1];
    xs[2] = blkxs_[3*pos+2];

    dx[0] = blkdx_[3*pos];
    dx[1] = blkdx_[3*pos+1];
    dx[2] = blkdx_[3*pos+2];

    xe[0] = xs[0] + float(blkdim_[0])*dx[0];
    xe[1] = xs[1] + float(blkdim_[1])*dx[1];
    xe[2] = xs[2] + float(blkdim_[2])*dx[2];

    int j;
    for( j=1; j<8; j++ ) {
      int ttt=pos+j;

      xk[0] = blkxs_[3*ttt];
      xk[1] = blkxs_[3*ttt+1];
      xk[2] = blkxs_[3*ttt+2];

      dk[0] = blkdx_[3*ttt];
      dk[1] = blkdx_[3*ttt+1];
      dk[2] = blkdx_[3*ttt+2];

      if( CompareKid2Kid0( xs,xe,dx, j,xk,dk )!=0 ) 
    break;
    }

    if( j>=8 ) {
      aggbid_[cnt++] = -(pos+1);
      pos+=8;
      continue;
    }}
    aggbid_[cnt++] = pos++;      
  }
  nagg_ = cnt;

  for( int i=0; i<nagg_; i++ ) {
    pos = aggbid_[i];
    if( pos<0 ) pos = -(pos+1);
    aggsft_[i] = (long)pos*(long)bvsz_;
  }
  debug2 << "after aggregation, nblks= " << nagg_ << " instead of " << nblks_ << ".\n";

  return 0;
}

#endif

int AMRreaderAgg::
genAggData()
{
  float* bd[8];
  for( int k=0; k<8; k++ ) {
    bd[k] = new float[bvsz_];
    if( bd[k]==NULL ) {
      debug1 << "Failed to allocate memory for kids node data.\n";
      return -1;
    }
  }

  long sft;
  int bid,lsft,ksft;
  int is,js,ks,ii,jj,kk;
  float* dbf;

  for( int i=0; i<nagg_; i++ ) {
    bid = aggbid_[i];
    sft = aggsft_[i];

    if( bid<0 ) {
      dbf = datbuf_ + sft;

      lsft=0;
      for( int k=0; k<8; k++ ) {
    for( int j=0; j<bvsz_; j++ )
      bd[k][j] = dbf[lsft++];
      }

      for( int k=0; k<8; k++ ) {
    is = kid2i[k]*blkdim_[0];
    js = kid2j[k]*blkdim_[1];
    ks = kid2k[k]*blkdim_[2];

    ksft=0;
    for( int n=0; n<blkdim_[2]; n++ ) {
      kk = (ks+n) * dbdim_[1]*dbdim_[0];
      for( int m=0; m<blkdim_[1]; m++ ) {
        jj = (js+m) * dbdim_[0];
        for( int l=0; l<blkdim_[0]; l++ ) {
          ii = is+l;
          lsft = 5*( ii + jj + kk );

          dbf[lsft++] = bd[k][ksft++];
          dbf[lsft++] = bd[k][ksft++];
          dbf[lsft++] = bd[k][ksft++];
          dbf[lsft++] = bd[k][ksft++];
          dbf[lsft  ] = bd[k][ksft++];
        }}}
      }
    }
  }

  for( int k=0; k<8; k++ ) 
    delete [] bd[k];

  aggdat_=1;
  return 0;
}



int AMRreaderAgg::
genAggScalar( float* dat )
{
  float* bd[8];
  for( int k=0; k<8; k++ ) {
    bd[k] = new float[blksz_];
    if( bd[k]==NULL ) {
      debug1 << "Failed to allocate memory for kids node data.\n";
      return -1;
    }
  }

  long sft;
  int bid,lsft,ksft;
  int is,js,ks,ii,jj,kk;
  float* dbf;

  for( int i=0; i<nagg_; i++ ) {
    bid = aggbid_[i];
    sft = aggsft_[i];

    if( bid<0 ) {
      dbf = dat + sft/5;

      lsft=0;
      for( int k=0; k<8; k++ ) {
    for( int j=0; j<blksz_; j++ )
      bd[k][j] = dbf[lsft++];
      }

      for( int k=0; k<8; k++ ) {
    is = kid2i[k]*blkdim_[0];
    js = kid2j[k]*blkdim_[1];
    ks = kid2k[k]*blkdim_[2];

    ksft=0;
    for( int n=0; n<blkdim_[2]; n++ ) {
      kk = (ks+n) * dbdim_[1]*dbdim_[0];
      for( int m=0; m<blkdim_[1]; m++ ) {
        jj = (js+m) * dbdim_[0];
        for( int l=0; l<blkdim_[0]; l++ ) {
          ii = is+l;
          lsft = ( ii + jj + kk );
          dbf[lsft] = bd[k][ksft++];
        }}}
      }
    }
  }

  for( int k=0; k<8; k++ ) 
    delete [] bd[k];

  return 0;
}



int AMRreaderAgg::
genAggAdditionData()
{
  if( genAggScalar( prebuf_ )!=0 ) {
    debug1 << "Failed to aggregate pressure variable\n";
    return -1;
  }
  if( genAggScalar( sndbuf_ )!=0 ) {
    debug1 << "Failed to aggregate soundspeed variable\n";
    return -2;
  }
  if( genAggScalar( tmpbuf_ )!=0 ) {
    debug1 << "Failed to aggregate temperature variable\n";
    return -3;
  }
  return 0;
}





int AMRreaderAgg::
GetBlockMesh( int bid, float* xs, float* dx )
{
  if( blkxs_==NULL ) {
    if( readAMRmesh()!=0 ) {
      debug1 << "Failed to read AMR mesh.\n";
      return -1;
    }
  }

  int tid = aggbid_[bid];
  if( tid<0 ) { tid = -(tid+1); }

  int sft=3*tid;
  xs[0] = blkxs_[sft];
  xs[1] = blkxs_[sft+1];
  xs[2] = blkxs_[sft+2];

  dx[0] = blkdx_[sft];
  dx[1] = blkdx_[sft+1];
  dx[2] = blkdx_[sft+2];
  return 0;
}



int AMRreaderAgg::
GetBlockVariable( int bid, int vid, float* dat ) 
{
  //debug5 << "AMRreaderAgg::GetBlockVariable( " << bid << ", " << vid << " )" << endl;
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

  //debug5 << "AMRreaderAgg::genAggAdditionData()\n";
  if( aggdat_==0 ) {
    if( genAggData()!=0 ) {
      debug1 << "Failed to aggregate blocks in AMR data.\n";
      return -2;
    }
    if( eos_->EOStype()==SesameEOS_type ) {
      if( genAggAdditionData()!=0 ) {
    debug1 << "Failed to aggregate blocks in additional AMR data.\n";
    return -2;
      }}
  }

  int tid = aggbid_[bid];
  long sft = aggsft_[bid];
  int sz  = blksz_;
  if( tid<0 ) { sz*=8; }

  //debug5 << "AMRreaderAgg::compvar(): sft= " << sft << "  sz= " << sz << endl;
  int ierr = compvar( vid, datbuf_+sft, dat, sz );
  if( ierr!=0 ) {
    debug1 << "Failed to compute requested variable: " << vid << " .\n";
    return ierr;
  }
  return 0;
}



