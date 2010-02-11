//
// This code was contributed to the VisIt project by Corvid Technologies
// on February 10, 2010.
//

#include <string.h>
#include <cstdlib>

#ifndef DEBUG_TEST
#include <DebugStream.h>
#else
#include <iostream>
using std::cout;
#define debug1 cout
#define debug2 cout 
#endif

#include <VelodyneReader.h>

int VelodyneReader::
open( const char* filename ) 
{
  file_id = H5Fopen( filename, H5F_ACC_RDONLY, H5P_DEFAULT );
  if( file_id<0 ) {
    debug1 << "Failed to open Velodyne plot file: " << filename << ".\n";
    return -1;
  }

  if( openGroups()<0 ) {
    debug1 << "Failed to open groups in " << filename << ".\n";
    return -2;
  }
  debug2 << "group ids are: node=" << node_id << "   solid=" << solid_id << "   shell="
     << shell_id << "   particle=" << particle_id << "   surface="  << surf_id
     << "   tiednode="  << tied_id << "\n";

  if( getMeshSizes()<0 ) {
    debug1 << "Failed to find size of meshes\n.";
    return -3;
  }
  debug2 << "mesh sizes are: " << nnode << "   " << nsolid << "   "
     << nshell << "   "  << npart << "   " << nsurf << "   " << ntied << "\n";

  if( getGeneralData()<0 ) {
    debug1 << "Failed to read /General data\n.";
    return -4;
  }

  if( solid_id>0 ) {
    nhhv = getNumberOfHV( solid_id );
    if( nhhv<0 ) {
      debug1 << "Failed to find out number of history variables for solid.\n";
      return -5;
    }
  }
  if( shell_id>0 ) {
    nshv = getNumberOfHV( shell_id );
    if( nshv<0 ) {
      debug1 << "Failed to find out number of history variables for shell.\n";
      return -6;
    }
  }
  if( sph_id>0 ) {
    nsphhv = getNumberOfHV( sph_id );
    if( nshv<0 ) {
      debug1 << "Failed to find out number of history variables for sph.\n";
      return -7;
    }
  }
  debug2 << "Number of history variables: solid=" << nhhv << "   shell=" << nshv << "   sph=" << nsphhv << "\n";
  return 0;
}



int VelodyneReader::
close() 
{
  closeGroups();
  if( file_id>0 ) 
    if( H5Fclose( file_id )<0 ) {
      debug1 << "Failed to close Velodyne plot file.\n";
      return -1;
    }
  return 0;
}



int VelodyneReader::
openGroups()
{
  node_id=solid_id=shell_id=particle_id=surf_id=tied_id=sph_id=-1;

  hsize_t nobjs;
  herr_t  err = H5Gget_num_objs( file_id, &nobjs );
  for( hsize_t i=0; i<nobjs; i++ ) {
    int type = H5Gget_objtype_by_idx( file_id, i );
    if( type == H5G_GROUP ) {
      char name[1000];
      ssize_t l= H5Gget_objname_by_idx( file_id, i, name, 1000 );

      hid_t gid = H5Gopen1( file_id, name );

      if( strcmp( name, "Node" )==0 ) 
    node_id = gid;
      else if( strcmp( name, "Solid" )==0 ) 
    solid_id = gid;
      else if( strcmp( name, "Shell" )==0 ) 
    shell_id = gid;
      else if( strcmp( name, "Particle" )==0 ) 
    particle_id = gid;
      else if( strcmp( name, "Surface" )==0 ) 
    surf_id = gid;
      else if( strcmp( name, "TiedNode" )==0 ) 
    tied_id = gid;
      else if( strcmp( name, "SPH" )==0 ) 
    sph_id = gid;
      else
    H5Gclose(gid);
    }
  }
  return 0;
}



int VelodyneReader::
closeGroups()
{
  if( sph_id>0 ) H5Gclose(sph_id);
  if( surf_id>0 ) H5Gclose(surf_id);
  if( node_id>0 )  H5Gclose(node_id);
  if( solid_id>0 ) H5Gclose(solid_id);
  if( shell_id>0 ) H5Gclose(shell_id);
  if( particle_id>0 ) H5Gclose(particle_id);
  if( tied_id>0 ) H5Gclose(tied_id);
  node_id=solid_id=shell_id=particle_id=surf_id=tied_id=sph_id-1;
  return 0;
}



int VelodyneReader::
getMeshSizes()
{
  hid_t aid;

  if( node_id<0 ) 
    nnode=0;
  else {
    aid = H5Aopen_name( node_id, "number" );
    if( aid<0 ) {
      nnode=0;debug1 << "Failed to find number of nodes.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &nnode );H5Aclose(aid);
    }}
  debug2 << "Number of nodes is " << nnode << "\n";

  if( solid_id<0 ) 
    nsolid = 0;
  else {
    aid = H5Aopen_name( solid_id, "number" );
    if( aid<0 ) {
      nsolid=0;debug1 << "Failed to find number of solid elements.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &nsolid );H5Aclose(aid);
    }}
  debug2 << "Number of solid elements is " << nsolid << "\n";

  if( shell_id<0 ) 
    nshell = 0;
  else {
    aid = H5Aopen_name( shell_id, "number" );
    if( aid<0 ) {
      nshell=0;debug1 << "Failed to find number of shell elements.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &nshell );H5Aclose(aid);
    }}
  debug2 << "Number of shell elements is " << nshell << "\n";

  if( particle_id<0 ) 
    npart = 0;
  else {
    aid = H5Aopen_name( particle_id, "number" );
    if( aid<0 ) {
      npart=0;debug1 << "Failed to find number of particles.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &npart );H5Aclose(aid);
    }}
  debug2 << "Number of particles is " << npart << "\n";

  if( surf_id<0 ) 
    nsurf = 0;
  else {
    aid = H5Aopen_name( surf_id, "number" );
    if( aid<0 ) {
      nsurf=0;debug1 << "Failed to find number of surface elements.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &nsurf );H5Aclose(aid);
    }}
  debug2 << "Number of surface elements is " << nsurf << "\n";

  if( tied_id<0 ) 
    ntied = 0;
  else {
    aid = H5Aopen_name( tied_id, "number" );
    if( aid<0 ) {
      ntied=0;debug1 << "Failed to find number of tied nodes.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &ntied );H5Aclose(aid);
    }}
  debug2 << "Number of tied nodes is " << ntied << "\n";

  if( sph_id<0 ) 
    nsph = 0;
  else {
    aid = H5Aopen_name( sph_id, "number" );
    if( aid<0 ) {
      nsph=0;debug1 << "Failed to find number of sph particles.\n";
    }
    else {
      H5Aread( aid, H5T_NATIVE_INT, &nsph );H5Aclose(aid);
    }}
  debug2 << "Number of sph particles is " << nsph << "\n";
  return 0;
}




int VelodyneReader::
getGeneralData()
{
  hid_t aid;

  hid_t gid = H5Gopen1( file_id, "/General" );
  if( gid<0 ) {
    debug1 << "Failed to open group named as \"/General\".\n";
    return -1;
  }

  aid = H5Aopen_name( gid, "SimuTime" );
  if( aid<0 ) {
    debug1 << "Failed to find SimuTime.\n";
    return -2;
  }
  H5Aread( aid, H5T_NATIVE_FLOAT, &time );H5Aclose(aid);
  debug2 << "SimuTime= " << time << "\n";
  
  aid = H5Aopen_name( gid, "Ncycles" );
  if( aid<0 ) {
    debug1 << "Failed to find Ncycles.\n";
    return -3;
  }
  H5Aread( aid, H5T_NATIVE_INT, &ncycle );H5Aclose(aid);
  debug2 << "Ncycles= " << ncycle << "\n";

  aid = H5Aopen_name( gid, "NumMaterials" );
  if( aid<0 ) {
    debug1 << "Failed to find NumMaterials.\n";
    return -4;
  }
  H5Aread( aid, H5T_NATIVE_INT, &nmmat );H5Aclose(aid);
  debug2 << "NumMaterials= " << nmmat << "\n";

  mat_titles = new string[nmmat];
  if( getMaterialTitles( gid )<0 ) {
    debug1 << "Failed to get material titles.\n";
    return -5;
  }

  H5Gclose(gid);
  return 0;
}



int VelodyneReader::
getMaterialTitles( hid_t gid )
{
  for( int i=0; i<nmmat; i++ ) {
    char name[200];
    sprintf(name,"PartTitle_%d",i+1);

    hid_t aid = H5Aopen_name( gid, name );
    if( aid<0 ) {
      debug1 << "Failed to find "<< name << ".\n";
      mat_titles[i] = name;
      continue;
    }

//     hid_t filetype = H5Aget_type(aid);
//     int sdim = H5Tget_size(filetype)+1; // for appending '\0'
//     H5Tclose(filetype);
    int sdim=2;

    hsize_t dims[10];
    hid_t space= H5Aget_space(aid);
    H5Sget_simple_extent_dims(space,dims,NULL);
    H5Sclose(space);
    debug2 << "title " << name << " has length " << dims[0] << "\n";

    hid_t memtype = H5Tcopy(H5T_C_S1);
    H5Tset_size(memtype,sdim);

    char** rd = new char*[ dims[0] ];
    rd[0] = new char[ sdim*dims[0] ];
    for( int j=1; j<dims[0]; j++ )
      rd[j] = rd[j-1]+sdim;

    herr_t herr = H5Aread( aid, memtype, rd[0] );
    H5Tclose(memtype);
    H5Aclose(aid);

    if( herr<0 ) {
      debug1 << "Failed to read " << name << ".\n";
      mat_titles[i] = name;
    }
    else {
      mat_titles[i] = rd[0];
      for( int j=1; j<dims[0]; j++ )
    mat_titles[i] += rd[j];
    }

    delete [] rd[0];
    delete [] rd;
    debug2 << name << "= " << mat_titles[i] << "\n";
  }
  return 0;
}




int VelodyneReader::
getNumberOfHV( hid_t gid )
{
  hsize_t nobj,idx;
  H5Gget_num_objs( gid, &nobj );

  int nn=0;
  for( idx=0; idx<nobj; idx++ ) {
    int type = H5Gget_objtype_by_idx( gid, idx );
    if( type!=H5G_DATASET ) continue;

    char name[101];
    ssize_t namelen = H5Gget_objname_by_idx( gid, idx, name, 100 );
    if( strcmp( name, "NumberOfHistoryVariables" )!=0 ) continue;

    hid_t did = H5Dopen1( gid, name );
    hid_t sid = H5Dget_space( did );
    int nds = H5Sget_simple_extent_ndims( sid );
    if( nds!=1 ) {
      debug1 << "Number of dimensions of dataset " << name << " is larger than expected(1).\n";
      H5Sclose(sid);
      H5Dclose(did);
      return -1;
    }

    hsize_t dims[10];
    H5Sget_simple_extent_dims( sid, dims, NULL );
    H5Sclose(sid);

    int *dat = new int[dims[0]];
    if( dat==NULL ) {
      debug1 << "Failed to allocate " << name << " array of size " << dims[0] << " \n";
      H5Dclose(did);
      return -2;
    }

    herr_t herr = H5Dread( did, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dat );
    if( herr<0 ) {
      debug1 << "Failed to read dataset " << name << " in group " << gid << ".\n";
      return -3;
    }
    H5Dclose(did);

    for( int i=0;i<dims[0];i++ )
      nn = nn>dat[i] ? nn : dat[i];

    delete [] dat;
    break;
  }
  return nn;
}




int VelodyneReader::
GetDataSetInfo( int run, int grp,
        char* name, int strlen,
        hsize_t* dims, int* ndim )
{
  static hsize_t nobj,idx,gid;

  if( run==0 ) {
    switch( grp ) {
    case node_type:     gid=node_id;     break;
    case solid_type:    gid=solid_id;    break;
    case shell_type:    gid=shell_id;    break;
    case particle_type: gid=particle_id; break;
    case surface_type:  gid=surf_id;     break;
    case tiednode_type: gid=tied_id;     break;
    case sph_type:      gid=sph_id;     break;
    default:
      debug1 << "Unknown group type(" << grp <<") is given.";
      return -1;
    }

    H5Gget_num_objs( gid, &nobj );
    idx=0;
    debug2 << "The " << grp << "th group has " << nobj << " members.\n";
    return 0;
  }

  while(idx<nobj) {
    int type = H5Gget_objtype_by_idx( gid, idx );
    if( type==H5G_DATASET ) break;
    idx++;
  }
  if( idx>=nobj ) return 1;

  ssize_t namelen;
  namelen = H5Gget_objname_by_idx( gid, idx, name, strlen-1 );
  idx++;

  hid_t did = H5Dopen1( gid, name );
  hid_t sid = H5Dget_space( did );
  int nds = H5Sget_simple_extent_ndims( sid );
  if( nds>*ndim ) {
    debug1 << "Number of dimensions of dataset " << name << " is larger than expected(" 
       << nds << " .vs " << *ndim << ").\n";
    return -1;
  }
  *ndim = nds;
  H5Sget_simple_extent_dims( sid, dims, NULL );
  H5Sclose(sid);
  H5Dclose(did);

  debug2 << "GetDataSetInfo: idx=" << idx << " ndim=" << *ndim << " dims=[";
  for( int d=0; d<nds; d++ ) debug2 << dims[d] << ", ";
  debug2 << "]\n";
  return 0;
}




int VelodyneReader::
GetDatasetDims( int grp,
        const char* name,
        int* ndim, int* dims )
{
  hid_t gid;
  switch( grp ) {
  case node_type:     gid=node_id;     break;
  case solid_type:    gid=solid_id;    break;
  case shell_type:    gid=shell_id;    break;
  case particle_type: gid=particle_id; break;
  case surface_type:  gid=surf_id;     break;
  case tiednode_type: gid=tied_id;     break;
  case sph_type:      gid=sph_id;     break;
  default:
    debug1 << "Unknown group type(" << grp <<") is given.";
    return -1;
  }

  hid_t dsid = openDataSet( gid, name );
  if( dsid<0 ) {
    debug1 << "Failed to find dataset " << name << " in group " << gid << ".\n";
    return -1;
  }

  int irev=0;

  hid_t spid = H5Dget_space( dsid );
  int nds = H5Sget_simple_extent_ndims( spid );
  if( nds>*ndim ) 
    irev = 1;
  else {
    hsize_t *ds=new hsize_t[nds];
    H5Sget_simple_extent_dims( spid, ds, NULL );
    for( int i=0;i<nds;i++ ) dims[i]=ds[i];
    delete [] ds;
  }

  *ndim = nds;
  H5Sclose(spid);
  H5Dclose(dsid);

  return irev;
}




hid_t VelodyneReader::
openDataSet( hid_t gid, const char* name )
{
  hsize_t nobj,idx;
  herr_t herr = H5Gget_num_objs( gid, &nobj );

  for( idx=0; idx<nobj; idx++ ) {
    int type = H5Gget_objtype_by_idx( gid, idx );

    if( type==H5G_DATASET ) {
      ssize_t namelen;
      namelen = H5Gget_objname_by_idx( gid, idx, NULL, 0 );

      char* dname = new char[namelen+1];
      namelen = H5Gget_objname_by_idx( gid, idx, dname, namelen+1 );

      if( strcmp( name, dname ) == 0 ) {
    hid_t dsid = H5Dopen1( gid, name );
    delete [] dname;
    return dsid;
      }
      delete [] dname;
    }
  }
  return -1;
}





int VelodyneReader::
readIntArray( hid_t gpid, const char* dsetname, int bufsz, int* buf, int* ndim, int* rdms )
{
  debug2 << "read integar array " << dsetname << "...\n";

  hid_t dsid = openDataSet( gpid, dsetname );
  if( dsid<0 ) {
    debug1 << "Failed to find dataset " << dsetname << " in group " << gpid << ".\n";
    return -1;
  }

  hid_t spid = H5Dget_space( dsid );
  int nds = H5Sget_simple_extent_ndims( spid );
  hsize_t *dims = new hsize_t[nds];
  H5Sget_simple_extent_dims( spid, dims, NULL );
  hsize_t nt = 1;  for(int d=0;d<nds;d++) nt*=dims[d];
  H5Sclose(spid);
  debug2 << " nds=" << nds << " \t dims=[ "; for(int d=0;d<nds; d++ ) debug2 << dims[d] << ", "; debug2<< "]\n";

  if( ndim ) *ndim=nds;
  if( rdms ) for( int d=0;d<nds;d++ ) rdms[d]=dims[d];
  delete [] dims;
  if( bufsz<=0 ) {
    H5Dclose(dsid);
    return 0;
  }

  debug2 << "bufsz=" << bufsz << " \t " << "datasize=" << nt << "\n";
  if( bufsz<nt ) {
    debug1 << "Buffer size is too small for dataset " << dsetname << " ("
       << bufsz << " vs. " << nt << ") in group " << gpid << ".\n";
    return -2;
  }

  herr_t herr = H5Dread( dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf );
  if( herr<0 ) {
    debug1 << "Failed to read dataset " << dsetname << " in group " << gpid << ".\n";
    return -3;
  }
  H5Dclose(dsid);
  return 0;
}





int VelodyneReader::
readFltArray( hid_t gpid, const char* dsetname, int bufsz, float* buf, int* ndim, int* rdms )
{
  herr_t herr;
  hid_t dsid = openDataSet( gpid, dsetname );
  if( dsid<0 ) {
    debug1 << "Failed to find dataset " << dsetname << " in group " << gpid << ".\n";
    return -1;
  }

  hid_t spid = H5Dget_space( dsid );
  int nds = H5Sget_simple_extent_ndims( spid );
  hsize_t *dims = new hsize_t[nds];
  H5Sget_simple_extent_dims( spid, dims, NULL );
  hsize_t nt = 1; for(int d=0;d<nds;d++) nt*=dims[d];
  H5Sclose(spid);
  debug2 << " nds=" << nds << " \t dims=[ "; for(int d=0;d<nds; d++ ) debug2 << dims[d] << ", "; debug2<< "]\n";

  if( ndim ) *ndim=nds;
  if( rdms ) for( int d=0;d<nds;d++ ) rdms[d]=dims[d];
  delete [] dims;
  if( bufsz<=0 ) {
    H5Dclose(dsid);
    return 0;
  }

  debug2 << "bufsz=" << bufsz << " \t " << "datasize=" << nt << "\n";
  if( bufsz<nt ) {
    debug1 << "Buffer size is too small for dataset " << dsetname << " ("
       << bufsz << " vs. " << nt << ") in group " << gpid << ".\n";
    H5Dclose(dsid);
    return -2;
  }

  int irev=0;

  hid_t tpid = H5Dget_type( dsid );
  H5T_class_t tpcl = H5Tget_class( tpid );
  if( tpcl == H5T_FLOAT ) {
    herr = H5Dread( dsid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf );
    if( herr<0 ) {
      debug1 << "Failed to read dataset " << dsetname << " in group " << gpid << ".\n";
      irev = -3;
    }
  }
  else if(  tpcl == H5T_INTEGER ) {
    int *ibuf = new int[nt];
    herr = H5Dread( dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ibuf );
    if( herr<0 ) {
      debug1 << "Failed to read integer dataset " << dsetname << " in group " << gpid << ".\n";
      irev = -4;
    }
    else {
      for( int i=0; i<nt; i++ ) 
    buf[i] = ibuf[i];
    }
    delete [] ibuf;
  }
  else {
    debug1 << "Dataset " << dsetname << "is neither float nor integer  in group " << gpid << ".\n";
    irev = -5;
  }
  H5Tclose(tpid);
  H5Dclose(dsid);
  return irev;
}




int VelodyneReader::
readHVarray( hid_t gpid, int ind, int bufsz, float* buf, int* ndim, int* rdms )
{
  herr_t herr;
  hsize_t dims[3],nt;
  // size info
  hid_t dsid = openDataSet( gpid, "NumberOfHistoryVariables" );
  if( dsid<0 ) {
    debug1 << "Failed to find dataset NumberOfHistoryVariables in group " << gpid << ".\n";
    return -1;
  }
  hid_t spid = H5Dget_space( dsid );
  int nds = H5Sget_simple_extent_ndims( spid );
  if( nds!=1 ) {
    debug1 << "Dataset NumberOfHistoryVariables has wrong number of dimensions: " << nds << ".\n";
    return -2;
  }
  H5Sget_simple_extent_dims( spid, dims, NULL );
  H5Sclose(spid);
  debug2 << "Dataset NumberOfHistoryVariables: nds=" << nds << " \t dims=[ " << dims[0] << " ]\n";

  if( ndim ) *ndim=nds;
  if( rdms ) for( int d=0;d<nds;d++ ) rdms[d]=dims[d];
  if( bufsz<=0 ) {
    H5Dclose(dsid);
    return 0;
  }
  nt=dims[0];
  if( bufsz<nt ) {
    debug1 << "Buffer size is too small for dataset History Variables" << " ("
       << bufsz << " vs. " << nt << ") in group " << gpid << ".\n";
    H5Dclose(dsid);
    return -9;
  }

  // size of hv
  int* loc = new int[dims[0]+1];
  if( loc==NULL ) {
    debug1 << "Failed to allocate array NumberOfHistoryVariables: sz=" << dims[0] << ".\n";
    return -3;
  }
  herr = H5Dread( dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, loc+1 );
  if( herr<0 ) {
    debug1 << "Failed to read dataset NumberOfHistoryVariables in group " << gpid << ".\n";
    return -4;
  }
  H5Dclose(dsid);
  // shift
  loc[0]=0;
  for( int i=0; i<dims[0]; i++ ) 
    loc[i+1] += loc[i];

  //
  // hv
  //
  dsid = openDataSet( gpid, "HistoryVariable" );
  if( dsid<0 ) {
    debug1 << "Failed to find dataset HistoryVariable in group " << gpid << ".\n";
    return -5;
  }
  spid = H5Dget_space( dsid );
  nds = H5Sget_simple_extent_ndims( spid );
  if( nds!=1 ) {
    debug1 << "Dataset HistoryVariable has wrong number of dimensions: " << nds << ".\n";
    return -6;
  }
  H5Sget_simple_extent_dims( spid, dims, NULL );
  H5Sclose(spid);
  debug2 << "Dataset HistoryVariable: nds=" << nds << " \t dims=[ " << dims[0] << " ]\n";

  float* hv = new float[dims[0]];
  if( hv==NULL ) {
    debug1 << "Failed to allocate array HistoryVariable: sz=" << dims[0] << ".\n";
    return -7;
  }
  herr = H5Dread( dsid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, hv );
  if( herr<0 ) {
    debug1 << "Failed to read dataset HistoryVariable in group " << gpid << ".\n";
    return -8;
  }
  H5Dclose(dsid);

  // assign data
  for( int i=0; i<nt; i++ ) {
    if( ind<loc[i+1]-loc[i] )
      buf[i] = hv[ loc[i]+ind ];
    else
      buf[i] = 0.;
  }

  delete [] hv;
  delete [] loc;
  return 0;
}




int VelodyneReader::
getMaterialSet( hid_t gpid, const char* matname, set<int>& mset )
{
  hid_t dsid = openDataSet( gpid, matname );
  if( dsid<0 ) {
    debug1 << "Failed to find material dataset " << matname << " in group " << gpid << ".\n"
           << "Set it to zero.\n";
    mset.insert(0);
    return 0;
  }

  hid_t spid = H5Dget_space( dsid );
  int nds = H5Sget_simple_extent_ndims( spid );
  hsize_t *dims = new hsize_t[nds];
  H5Sget_simple_extent_dims( spid, dims, NULL );
  hsize_t nt = 1;  for(int d=0;d<nds;d++) nt*=dims[d];
  delete [] dims;
  H5Sclose(spid);

  int *mat = new int[nt];

  herr_t herr = H5Dread( dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, mat );
  if( herr<0 ) {
    debug1 << "Failed to read material dataset " << matname << " in group " << gpid << ".\n";
    return -2;
  }

  for( int i=0; i<nt; i++ )
    mset.insert(mat[i]);

  delete [] mat;
  H5Dclose(dsid);
  return 0;
}



int VelodyneReader::
GetMeshSize( int grp ) const
{
  switch( grp ) {
  case node_type:     return nnode;
  case solid_type:    return nsolid;
  case shell_type:    return nshell;
  case particle_type: return npart;
  case surface_type:  return nsurf;
  case tiednode_type: return ntied;
  case sph_type:      return nsph;
  default:
    debug1 << "Unknown group type(" << grp <<") is given.";
    return -1;
  }
}



const set<int>& VelodyneReader::
GetMeshMatInfo( int grp ) const
{
  switch( grp ) {
  case solid_type:    return mat_solid;
  case shell_type:    return mat_shell;
  case particle_type: return mat_particle;
  case surface_type:  return mat_surface;
  case sph_type:      return mat_sph;
  default:
    debug1 << "Unknown group type(" << grp <<") is given.";
    return mat_null;
  }
}


int VelodyneReader::
GetHVsize( int grp ) const
{
  switch( grp ) {
  case node_type:     return 0;
  case solid_type:    return nhhv;
  case shell_type:    return nshv;
  case sph_type:      return nsphhv;
  default:
    debug1 << "Unknown group type(" << grp <<") is given.";
    return -1;
  }
}


int VelodyneReader::
readMeshMaterialSet( int grp )
{
  int ierr;
  hid_t gid;
  set<int> *pset;
  switch( grp ) {
  case solid_type:
    gid = solid_id;    pset= &mat_solid;
    break;
  case shell_type:
    gid = shell_id;    pset= &mat_shell;
    break;
  case surface_type:
    gid = surf_id;    pset= &mat_surface;
    break;
  case particle_type:
    gid = particle_id;    pset= &mat_particle;
    break;
  case sph_type:
    gid = sph_id;    pset= &mat_sph;
    break;
  default:
    debug1 << "Unknown group type(" << grp <<") is given to pick material info.\n";
    return -1;
  }

  ierr=0;
  if( pset->size()==0 )
    ierr = getMaterialSet( gid, "Material", *pset );
  return ierr;
}



int VelodyneReader::
readMeshIntArray( int grp, const char* name, int bufsz, int* val, int* ndim, int* dims )
{
  hid_t gid;
  switch( grp ) {
  case node_type:     gid=node_id;     break;
  case solid_type:    gid=solid_id;    break;
  case shell_type:    gid=shell_id;    break;
  case particle_type: gid=particle_id; break;
  case surface_type:  gid=surf_id;     break;
  case tiednode_type: gid=tied_id;     break;
  case sph_type:      gid=sph_id;      break;
  default:
    debug1 << "Unknown group type(" << grp <<") is given.";
    return -1;
  }
  return readIntArray( gid, name, bufsz, val, ndim, dims );
}


int VelodyneReader::
readMeshFltArray( int grp, const char* name, int bufsz, float* val, int* ndim, int* dims )
{
  hid_t gid;
  switch( grp ) {
  case node_type:     gid=node_id;     break;
  case solid_type:    gid=solid_id;    break;
  case shell_type:    gid=shell_id;    break;
  case particle_type: gid=particle_id; break;
  case surface_type:  gid=surf_id;     break;
  case tiednode_type: gid=tied_id;     break;
  case sph_type:      gid=sph_id;      break;
  default:
    debug1 << "Unknown group type(" << grp <<") is given.";
    return -1;
  }
  char hvn[4]="\0";
  strncat( hvn, name, 3 );
  if( strcmp( hvn, "HV_" )!=0 ) 
    return readFltArray( gid, name, bufsz, val, ndim, dims );
  else {
    int ind = atoi(name+3)-1;
    return readHVarray( gid, ind, bufsz, val, ndim, dims );
  }
}



