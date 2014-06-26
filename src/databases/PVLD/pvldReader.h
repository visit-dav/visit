/*
  read in partitioned velodyne plot files
*/
#ifndef VELODYNE_READER_HPP
#define VELODYNE_READER_HPP

#include <set>
using std::set;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include <hdf5.h>

class PVLD_Reader
{
public:
    PVLD_Reader()
        : filename_(),title_(),file_id_(0),
          ncycle_(0),time_(0),nmmat_(0),
          nnode_(0),nsolid_(0),nshell_(0),nbeam_(0),nsph_(0),ntdst_(0),ncntt_(0),
          solid_lhv_(false),shell_lhv_(false),beam_lhv_(false),sph_lhv_(false),
          solid_mxhv_(0),shell_mxhv_(0),beam_mxhv_(0),sph_mxhv_(0)
    {}

    virtual ~PVLD_Reader()
    {
        CloseFile();
    }

    virtual
    void FreeResource();

    void SetFileName( const string& name )
    {
        filename_ = name;
    }

    virtual void ReadTOC();
    virtual void ReadHistoryTOC();

    void ReadMaterialType( bool add_missing_parts );

    void CheckNumberOfEngines( int npart );


    const string& GetFileName() const
    {
        return filename_;
    }
    const string &GetTitle() const
    {
        return title_;
    }
    int   GetCycle() const
    {
        return ncycle_;
    }
    float GetTime()  const
    {
        return time_;
    }
    int   GetNumOfMaterials()  const
    {
        return nmmat_;
    }
    const string& GetMaterialName( int nm ) const
    {
        return mat_titles_.at(nm-1);
    }


    bool HasNode() const
    {
        return nnode_!=0;
    }
    int  GetNumOfNodes() const
    {
        return nnode_;
    }
    int  GetNumOfNodeVariables() const
    {
        return node_dsname_.size();
    }
    const string& GetNodeVariableName( int idx ) const
    {
        return node_dsname_[idx];
    }
    const vector<int>& GetNodeVariableDims( int idx ) const
    {
        return node_dsdims_[idx];
    }


    bool HasSolid() const
    {
        return nsolid_!=0;
    }
    int  GetNumOfSolidBlocks() const
    {
        return solid_part_.size()-1;
    }
    int  GetSolidBlockSize( int nb ) const
    {
        return solid_part_[nb+1] - solid_part_[nb];
    }
    int  GetNumOfSolidVariables() const
    {
        return solid_dsname_.size();
    }
    const string& GetSolidVariableName( int idx ) const
    {
        return solid_dsname_[idx];
    }
    const vector<int>& GetSolidVariableDims( int idx ) const
    {
        return solid_dsdims_[idx];
    }
    bool HasSolidHistoryVariables() const
    {
        return solid_lhv_;
    }
    int  MaxSolidHistoryVariables() const
    {
        return solid_mxhv_;
    }
    const vector<int>& GetSolidHistoryPartitions() const
    {
        return solid_hvpart_;
    }
    const vector<int>& GetSolidMaterial() const
    {
        return solid_matid_;
    }
    const vector<int>& GetMissingSolidMaterial() const
    {
        return solid_msmat_;
    }
    const vector<int> &GetSolidBlockMeshMap(int nb) const
    {
        return solid_blkmap_[nb];
    }

    bool HasShell() const
    {
        return nshell_!=0;
    }
    int  GetNumOfShellBlocks() const
    {
        return shell_part_.size()-1;
    }
    int  GetShellBlockSize( int nb ) const
    {
        return shell_part_[nb+1] - shell_part_[nb];
    }
    int  GetNumOfShellVariables() const
    {
        return shell_dsname_.size();
    }
    const string& GetShellVariableName( int idx ) const
    {
        return shell_dsname_[idx];
    }
    const vector<int>& GetShellVariableDims( int idx ) const
    {
        return shell_dsdims_[idx];
    }
    bool HasShellHistoryVariables() const
    {
        return shell_lhv_;
    }
    int  MaxShellHistoryVariables() const
    {
        return shell_mxhv_;
    }
    const vector<int>& GetShellHistoryPartitions() const
    {
        return shell_hvpart_;
    }
    const vector<int>& GetShellMaterial() const
    {
        return shell_matid_;
    }
    const vector<int>& GetMissingShellMaterial() const
    {
        return shell_msmat_;
    }
    const vector<int> &GetShellBlockMeshMap(int nb) const
    {
        return shell_blkmap_[nb];
    }


    bool HasBeam() const
    {
        return nbeam_!=0;
    }
    int  GetNumOfBeamBlocks() const
    {
        return beam_part_.size()-1;
    }
    int  GetBeamBlockSize( int nb ) const
    {
        return beam_part_[nb+1] - beam_part_[nb];
    }
    int  GetNumOfBeamVariables() const
    {
        return beam_dsname_.size();
    }
    const string& GetBeamVariableName( int idx ) const
    {
        return beam_dsname_[idx];
    }
    const vector<int>& GetBeamVariableDims( int idx ) const
    {
        return beam_dsdims_[idx];
    }
    bool HasBeamHistoryVariables() const
    {
        return beam_lhv_;
    }
    int  MaxBeamHistoryVariables() const
    {
        return beam_mxhv_;
    }
    const vector<int>& GetBeamHistoryPartitions() const
    {
        return beam_hvpart_;
    }
    const vector<int>& GetBeamMaterial() const
    {
        return beam_matid_;
    }
    const vector<int>& GetMissingBeamMaterial() const
    {
        return beam_msmat_;
    }
    const vector<int> &GetBeamBlockMeshMap(int nb) const
    {
        return beam_blkmap_[nb];
    }


    bool HasSurface() const
    {
        return nsurf_!=0;
    }
    int  GetNumOfSurfaceBlocks() const
    {
        return surf_part_.size()-1;
    }
    int  GetSurfaceBlockSize( int nb ) const
    {
        return surf_part_[nb+1] - surf_part_[nb];
    }
    int  GetNumOfSurfaceVariables() const
    {
        return surf_dsname_.size();
    }
    const string& GetSurfaceVariableName( int idx ) const
    {
        return surf_dsname_[idx];
    }
    const vector<int>& GetSurfaceVariableDims( int idx ) const
    {
        return surf_dsdims_[idx];
    }
    const vector<int>& GetSurfaceMaterial() const
    {
        return surf_matid_;
    }
    const vector<int> &GetSurfaceBlockMeshMap(int nb) const
    {
        return surf_blkmap_[nb];
    }


    bool HasSPH() const
    {
        return nsph_!=0;
    }
    int  GetNumOfSphBlocks() const
    {
        return sph_part_.size()-1;
    }
    int  GetSphBlockSize( int nb ) const
    {
        return sph_part_[nb+1] - sph_part_[nb];
    }
    int  GetNumOfSphVariables() const
    {
        return sph_dsname_.size();
    }
    const string& GetSphVariableName( int idx ) const
    {
        return sph_dsname_[idx];
    }
    const vector<int>& GetSphVariableDims( int idx ) const
    {
        return sph_dsdims_[idx];
    }
    bool HasSphHistoryVariables() const
    {
        return sph_lhv_;
    }
    int  MaxSphHistoryVariables() const
    {
        return sph_mxhv_;
    }
    const vector<int>& GetSphHistoryPartitions() const
    {
        return sph_hvpart_;
    }
    const vector<int>& GetSphMaterial() const
    {
        return sph_matid_;
    }
    const vector<int>& GetMissingSphMaterial() const
    {
        return sph_msmat_;
    }


    bool HasTiedSet() const
    {
        return ntdst_!=0;
    }
    int  GetNumOfTiedSetBlocks() const
    {
        return tdst_part_.size()-1;
    }
    int  GetTiedSetBlockSize( int nb ) const
    {
        return tdst_part_[nb+1] - tdst_part_[nb];
    }
    //const vector<string>& GetTiedSetVariableNames() const { return tdst_dsname_; }
    int  GetNumOfTiedSetVariables() const
    {
        return tdst_dsname_.size();
    }
    const string& GetTiedSetVariableName( int idx ) const
    {
        return tdst_dsname_[idx];
    }
    const vector<int>& GetTiedSetVariableDims( int idx ) const
    {
        return tdst_dsdims_[idx];
    }


    bool HasContact() const
    {
        return ncntt_!=0;
    }
    int  GetNumOfContactBlocks() const
    {
        return cntt_part_.size()-1;
    }
    int  GetContactBlockSize( int nb ) const
    {
        return cntt_part_[nb+1] - cntt_part_[nb];
    }
    int  GetNumOfContactVariables() const
    {
        return cntt_dsname_.size();
    }
    const string& GetContactVariableName( int idx ) const
    {
        return cntt_dsname_[idx];
    }
    const vector<int>& GetContactVariableDims( int idx ) const
    {
        return cntt_dsdims_[idx];
    }


    void ReadSolidBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    bool ReadSolidBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data );
    virtual
    void ReadSolidMaterial();
    virtual
    void ReadSolidBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );

    void ReadShellBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    bool ReadShellBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data );
    virtual
    void ReadShellMaterial();
    virtual
    void ReadShellBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );

    void ReadBeamBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    bool ReadBeamBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data );
    virtual
    void ReadBeamMaterial();
    virtual
    void ReadBeamBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );

    void ReadSurfaceBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    bool ReadSurfaceBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data );
    virtual
    void ReadSurfaceMaterial();
    virtual
    void ReadSurfaceBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );

    void ReadSphBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    void ReadSphBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data );
    void ReadSphMaterial(bool allowCollective = true);
    void ReadSphBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );


    void ReadTiedSetBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    void ReadTiedSetSlaveBlockMesh( int blkInd, vector<float>& crd );
    void ReadTiedSetMasterBlockMesh( int blkInd, vector<float>& crd );
    bool ReadTiedSetBlockData( const char* varname, int nb,
                               vector<int>& dims, vector<float>& data );

    void ReadContactBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt );
    void ReadContactSlaveBlockMesh( int blkInd, vector<float>& crd );
    void ReadContactMasterBlockMesh( int blkInd, vector<float>& crd );
    bool ReadContactBlockData( const char* varname, int nb,
                               vector<int>& dims, vector<float>& data );

    void AppendMissingMaterialMesh( int type, int blkInd, vector<float>& vcrd, vector<int>& elmt );
    void AppendMissingMaterialData( int type, int blkInd, vector<int>& dims, vector<float>& data );
    void AppendMissingMaterialMaterial( int type, int blkInd, vector<int>& dims, vector<float>& data );


protected:
    void ReadGeneralInfo( hid_t file_id );
    void ReadNodeInfo( hid_t file_id );
    void ReadSolidInfo( hid_t file_id );
    void ReadShellInfo( hid_t file_id );
    void ReadBeamInfo(  hid_t file_id );
    void ReadSurfaceInfo( hid_t file_id );
    void ReadSPHInfo( hid_t file_id );
    void ReadTiedsetInfo( hid_t file_id );
    void ReadContactInfo( hid_t file_id );


    void GetMaterialTitles( hid_t gid );
    void MakeUniqueMatNames();
    void GetMaterialType( hid_t file_id, const char* grpname );
    void GenMissingMaterials();
    void AddMissingParts( int type,
                          const vector<int>& tgt,
                          const vector<int>& own,
                          vector<int>& msmat );

    void ReadInfo( hid_t file_id, const char* name,
                   int&                 num,
                   vector<int>&         part,
                   vector<string>&      dsname,
                   vector<vector<int> >& dsdims,
                   const char* num_str=NULL );

    void ReadNodeIndexCoord( hid_t file_id );
    void ReadNodeData( hid_t fid, int varInd, const vector<int>& map,
                       vector<int>& dims, vector<float>& dat );

    virtual
    void ReadSolidBlockMesh( hid_t file_id, int blkInd,
                             vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    virtual
    void ReadSolidBlockData( hid_t file_id, int varInd, int blkInd,
                             vector<int>& dims, vector<float>& edat );
    virtual
    void ReadSolidBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    vector<int>& dims, vector<float>& data );
    void CollectSolidMaterial( hid_t file_id, int varInd, int blkInd,
                               vector<int>& dims, vector<float>& edat );

    virtual
    void ReadShellBlockMesh( hid_t file_id, int blkInd,
                             vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    virtual
    void ReadShellBlockData( hid_t file_id, int varInd, int blkInd,
                             vector<int>& dims, vector<float>& edat );
    virtual
    void ReadShellBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    vector<int>& dims, vector<float>& data );

    virtual
    void ReadBeamBlockMesh( hid_t file_id, int blkInd,
                            vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    virtual
    void ReadBeamBlockData( hid_t file_id, int varInd, int blkInd,
                            vector<int>& dims, vector<float>& edat );
    virtual
    void ReadBeamBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                   vector<int>& dims, vector<float>& data );

    virtual
    void ReadSurfaceBlockMesh( hid_t file_id, int blkInd,
                               vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    virtual
    void ReadSurfaceBlockData( hid_t file_id, int varInd, int blkInd,
                               vector<int>& dims, vector<float>& edat );

    void ReadSphBlockMesh( hid_t file_id, int blkInd,
                           vector<float>& vcrd, vector<int>& elmt );
    void ReadSphBlockData( hid_t file_id, int varInd, int blkInd,
                           vector<int>& dims, vector<float>& edat );


    virtual
    void ReadHistoryDataInfo( hid_t fid, const char* meshname,
                              const vector<int>& part,
                              int& mxnb, vector<int>& hvpart );
    void ReadBlockHistoryData( hid_t fid, const char* meshname,  int blkInd, int varInd,
                               const vector<int>& part,  const vector<int>& hvpart,
                               vector<int>& hvsft, vector<int>& dims, vector<float>& data );

    void ReadTiedSetBlockMesh( hid_t file_id, int blkInd,
                               vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    void ReadTiedSetSlaveBlockMesh( hid_t file_id, int blkInd, vector<float>& crd );
    void ReadTiedSetMasterBlockMesh( hid_t file_id, int blkInd, vector<float>& crd );
    void ReadTiedSetBlockData( hid_t file_id, int varInd, int blkInd,
                               vector<int>& dims, vector<float>& edat );

    void ReadContactBlockMesh( hid_t file_id, int blkInd,
                               vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    void ReadContactSlaveBlockMesh( hid_t file_id, int blkInd, vector<float>& crd );
    void ReadContactMasterBlockMesh( hid_t file_id, int blkInd, vector<float>& crd );
    void ReadContactBlockData( hid_t file_id, int varInd, int blkInd,
                               vector<int>& dims, vector<float>& edat );



    void CollectMaterial( hid_t fid, const char* meshname, const vector<string>& dsname,
                          int ne, vector<int>& mat, bool allowCollective = true );




protected:
    hid_t OpenFile();
    void  CloseFile( hid_t fid );
    void  CloseFile();

    static hid_t OpenGroup( hid_t loc, const char* name );
    static void  CloseGroup( hid_t gid );
    static void  CollectGroupDataSets( hid_t gid, vector<string>& name, vector<vector<int> >& dims );
    static void  CollectSubgroups( hid_t gid, vector<string>& sgname );

    static hid_t OpenAttribute( hid_t loc, const char* name );
    static void  CloseAttribute( hid_t aid );
    static void  GetAttributeSize( hid_t aid, int& ndim, hsize_t* dims );  // ndim: input/output
    static void  ReadAttribute( hid_t aid, hid_t type, void* buf );
    static void  ReadAttribute( hid_t loc, const char* name, hid_t type, void* buf );

    static hid_t OpenDataSet( hid_t loc, const char* name );
    static void  CloseDataSet( hid_t dsid );
    static void  GetDataSetSize( hid_t aid, int& ndim, hsize_t* dims );  // ndim: input/output
    static void  ReadDataSet( hid_t dsid, hid_t type, void* buf,
                              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); // ndim<=0: read whole array

    // ndim<=0: read whole array
    static void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname,
                                  hid_t type, void* buf,
                                  int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL );

    static void LoadSampleDatasetNames( const string& filename, const string& grpname,
                                        vector<string>& dsname, vector<vector<int> >& dsdims,
                                        bool& lhv, int& mxhv );
    static void GenMissMesh( int mesh_type,
                             const vector<int>& msmat, const vector<float>& crd,
                             vector<float>& vcrd, vector<int>& elmt );
    static void GenMissData( const vector<int>& vdim, const vector<int>& msmat,
                             vector<int>& dims, vector<float>& data );
    static void GenMissMaterial( const vector<int>& msmat,
                                 vector<int>& dims, vector<float>& data );

    static void GenDefaultSphVariables( vector<string>& dsname, vector<vector<int> >& dsdims );


    /* void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname, int* buf, */
    /*              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); */
    /* void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname, float* buf, */
    /*              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); */
    /* void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname, double* buf, */
    /*              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); */




protected:
    //bool           hasTOCread_;
    string         filename_;
    string         title_;

    hid_t          file_id_;

    int            ncycle_; // number of cycles
    float          time_;   // simulation time

    int            nmmat_;  // number of materials
    vector<int>    mat_type_;
    vector<string> mat_titles_;

    // node
    int                    nnode_;
    vector<string>         node_dsname_;
    vector<vector<int> >    node_dsdims_;
    vector<int>            node_idx_;  // node index
    vector<float>          node_crd_;  // node coord
    int                    node_idx_mn_;
    int                    node_idx_mx_;
    vector<int>            node_map_;


    // solid
    int                 nsolid_;
    vector<int>         solid_part_;  // partition
    vector<string>      solid_dsname_;
    vector<vector<int> > solid_dsdims_;
    vector<vector<int> > solid_blkmap_;
    bool                solid_lhv_;
    int                 solid_mxhv_;
    vector<int>         solid_hvpart_;
    vector<vector<int> > solid_hvdisp_;
    vector<int>         solid_matid_;
    vector<int>         solid_msmat_;


    // shell
    int                 nshell_;
    vector<int>         shell_part_;  // partition
    vector<string>      shell_dsname_;
    vector<vector<int> > shell_dsdims_;
    vector<vector<int> > shell_blkmap_;
    bool                shell_lhv_;
    int                 shell_mxhv_;
    vector<int>         shell_hvpart_;
    vector<vector<int> > shell_hvdisp_;
    vector<int>         shell_matid_;
    vector<int>         shell_msmat_;


    // beam
    int                 nbeam_;
    vector<int>         beam_part_;  // partition
    vector<string>      beam_dsname_;
    vector<vector<int> > beam_dsdims_;
    vector<vector<int> > beam_blkmap_;
    bool                beam_lhv_;
    int                 beam_mxhv_;
    vector<int>         beam_hvpart_;
    vector<vector<int> > beam_hvdisp_;
    vector<int>         beam_matid_;
    vector<int>         beam_msmat_;


    // surface
    int                 nsurf_;
    vector<int>         surf_part_;  // partition
    vector<string>      surf_dsname_;
    vector<vector<int> > surf_dsdims_;
    vector<vector<int> > surf_blkmap_;
    vector<int>         surf_matid_;

    // sph
    int                 nsph_;
    vector<int>         sph_part_;  // partition
    vector<string>      sph_dsname_;
    vector<vector<int> > sph_dsdims_;
    bool                sph_lhv_;
    int                 sph_mxhv_;
    vector<int>         sph_hvpart_;
    vector<vector<int> > sph_hvdisp_;
    vector<int>         sph_matid_;
    vector<int>         sph_msmat_;

    // tiedset
    int                 ntdst_;
    vector<int>         tdst_part_;  // partition
    vector<string>      tdst_dsname_;
    vector<vector<int> > tdst_dsdims_;
    vector<vector<int> > tdst_blkmap_;

    // contact
    int                 ncntt_;
    vector<int>         cntt_part_;  // partition
    vector<string>      cntt_dsname_;
    vector<vector<int> > cntt_dsdims_;
    vector<vector<int> > cntt_blkmap_;



public:
    typedef struct
    {
        vector<string>                 names_;
        vector<vector<string> >         dsnames_;
        vector<vector<vector<int> > >    dsdims_;
        vector<vector<vector<double> > > dsdata_;
    } IndexVariables;

    const IndexVariables& GetSolidIndexVariables() const
    {
        return solid_idxvar_;
    }
    const IndexVariables& GetShellIndexVariables() const
    {
        return shell_idxvar_;
    }

    void ReadSolidBlockIndexVariable( const string& idxname,
                                      const string& varname,
                                      int domain,
                                      vector<int>& dims,
                                      vector<float>& data );
    void ReadShellBlockIndexVariable( const string& idxname,
                                      const string& varname,
                                      int domain,
                                      vector<int>& dims,
                                      vector<float>& data );
protected:
    void ReadIndexVariableInfo( hid_t file_id,
                                const char* mshname,
                                const char* ivname,
                                const vector<string>& dsname,
                                IndexVariables& iv );
    void ReadStringArrayAttribute( hid_t loc,
                                   const char* name,
                                   vector<string>& strings );
    void PickIndexVariable( const vector<int>&    dim,
                            const vector<double>& smp,
                            const vector<float>&  idx,
                            vector<float>&  dat );


protected:
    IndexVariables solid_idxvar_;
    IndexVariables shell_idxvar_;


public:
    static string ComposeNames( const string& m, const string& v, const char* seperator="/" );
    static void   DecomposeNames( const string& str, string& m, string& v, const char* seperator="/" );
    static void   DecomposeNames( const string& str, vector<string>& vs, const char* seperator="/" );
    static void   ConvDyna3dStreeTensor( const float* stress, float* tensor );
    static void   ConvSphStreeTensor( const float* stress, float* tensor );



public:
    //static void*        mpi_comm_ptr;
    //static MPI_Comm     mpi_comm;
    static const string general_name;
    static const string node_name;
    static const string solid_name;
    static const string shell_name;
    static const string beam_name;
    static const string surface_name;
    static const string sph_name;
    static const string tiedset_name;
    static const string tiedset_slave_name;
    static const string contact_name;
    static const string contact_slave_name;
    static const string node_index_name;
    static const string number_of_history_name;
    static const string history_name;
    static const string partition_name;

    static const int solid_elmt_type;
    static const int beam_elmt_type;
    static const int shell_elmt_type;
    static const int tshell_elmt_type;
    static const int sph_elmt_type;

    static const int default_number_of_partitions;

};


void EquallyPartition( int npart, int total, int* sft );


template<class T>
void FreeVector( vector<T>& vv )
{
    vector<T> tmp;
    tmp.swap(vv);
}

#endif
