//
// This code was contributed to the VisIt project by Corvid Technologies
// on February 10, 2010.
//

/*
  read in velodyne plot files
*/
#ifndef VELODYNE_READER_HPP
#define VELODYNE_READER_HPP

#include <set>
using std::set;
#include <string>
using std::string;
#include <hdf5.h>


class VelodyneReader
{
 public:
  VelodyneReader() {
    file_id=node_id,solid_id,shell_id,particle_id=surf_id=tied_id=-1;
    nhhv=nshv=nsphhv=0;
  }
  ~VelodyneReader() {
    close();
    mat_solid.clear();
    mat_shell.clear();
    mat_surface.clear();
    mat_particle.clear();
    delete [] mat_titles;
  }

  int open( const char* filename );
  int close();

  int GetNodeSize() const { return nnode; };
  int GetSolidSize() const { return nsolid; };
  int GetShellSize() const { return nshell; };
  int GetParticleSize() const { return npart; };
  int GetSurfaceSize() const { return nsurf; };
  int GetSPHsize() const { return nsph; };

  int GetNumMaterials() const { return nmmat; }
  int GetNumCycles() const { return ncycle; }
  float GetSimuTime() const { return time; }
  const string& GetMaterialTitle(int ind) const { return mat_titles[ind-1]; }

  int GetDataSetInfo( int run, int grp,
              char* name, int strlen,
              hsize_t* dims, int* ndim );
  int GetDatasetDims( int grp,
              const char* name,
              int* ndim, int* dims );

  int GetMeshSize( int grp ) const;
  int readMeshMaterialSet( int grp );
  const set<int>& GetMeshMatInfo( int grp ) const;

  int readMeshIntArray( int grp, const char* name,
            int bufsz, int* val,
            int* ndim=NULL, int* dims=NULL );
  int readMeshFltArray( int grp, const char* name, 
            int bufsz, float* val,
            int* ndim=NULL, int* dims=NULL );
  int GetHVsize( int grp ) const;


 protected:
  int openGroups();
  int closeGroups();
  int getMeshSizes();
  int getGeneralData();
  int getMaterialTitles( hid_t gid );
  int getNumberOfHV( hid_t gid );
  hid_t openDataSet( hid_t gid, const char* name );
  int readIntArray( hid_t gpid, const char* dsetname,
            int bufsz, int* buf,
            int* ndim=NULL, int* dims=NULL );
  int readFltArray( hid_t gpid, const char* dsetname,
            int bufsz, float* buf,
            int* ndim=NULL, int* dims=NULL );
  int readHVarray( hid_t gpid, int ind, 
           int bufsz, float* buf, 
           int* ndim=NULL, int* rdms=NULL );
  int getMaterialSet( hid_t gpid, const char* matname, set<int>& mset );


 protected:
  hid_t  file_id;
  hid_t  node_id,solid_id,shell_id,surf_id,particle_id,tied_id,sph_id;
  int    nnode,nsolid,nshell,npart,nsurf,ntied,nsph;

  int nmmat;  // number of materials
  int ncycle; // number of cycles
  float time;   // simulation time

  set<int> mat_solid;
  set<int> mat_shell;
  set<int> mat_surface;
  set<int> mat_particle;
  set<int> mat_sph;
  set<int> mat_null;

  string*  mat_titles;

  int nhhv,nshv,nsphhv;

 public:
  static const int node_type    =0;
  static const int solid_type   =1;
  static const int shell_type   =2;
  static const int particle_type=3;
  static const int surface_type =4;
  static const int tiednode_type=5;
  static const int sph_type=6;

}; // class VelodyneReader


#endif 
