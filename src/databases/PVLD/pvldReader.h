/*
  read in partitioned velodyne plot files
*/
#ifndef VELODYNE_READER_HPP
#define VELODYNE_READER_HPP

#include <set>
#include <vector>
#include <string>

#include <hdf5.h>

class PVLD_Reader
{
public:
    PVLD_Reader()
        : filename_(),title_(),file_id_(0),
          ncycle_(0),time_(0),nmmat_(0),
          nnode_(0),nsolid_(0),solid_lhv_(false),solid_mxhv_(0),nshell_(0),shell_lhv_(false),shell_mxhv_(0),nbeam_(0),beam_lhv_(false),beam_mxhv_(0),nsph_(0),sph_lhv_(false),sph_mxhv_(0),ntdst_(0),ncntt_(0)
    {}

    virtual ~PVLD_Reader()
    {
        CloseFile();
    }

    virtual
    void FreeResource();

    void SetFileName( const std::string& name )
    {
        filename_ = name;
    }

    virtual void ReadTOC();
    virtual void ReadHistoryTOC();

    void ReadMaterialType( bool add_missing_parts );

    void CheckNumberOfEngines( int npart );


    const std::string& GetFileName() const
    {
        return filename_;
    }
    const std::string &GetTitle() const
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
    const std::string& GetMaterialName( int nm ) const
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
    const std::string& GetNodeVariableName( int idx ) const
    {
        return node_dsname_[idx];
    }
    const std::vector<int>& GetNodeVariableDims( int idx ) const
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
    const std::string& GetSolidVariableName( int idx ) const
    {
        return solid_dsname_[idx];
    }
    const std::vector<int>& GetSolidVariableDims( int idx ) const
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
    const std::vector<int>& GetSolidHistoryPartitions() const
    {
        return solid_hvpart_;
    }
    const std::vector<int>& GetSolidMaterial() const
    {
        return solid_matid_;
    }
    const std::vector<int>& GetMissingSolidMaterial() const
    {
        return solid_msmat_;
    }
    const std::vector<int> &GetSolidBlockMeshMap(int nb) const
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
    const std::string& GetShellVariableName( int idx ) const
    {
        return shell_dsname_[idx];
    }
    const std::vector<int>& GetShellVariableDims( int idx ) const
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
    const std::vector<int>& GetShellHistoryPartitions() const
    {
        return shell_hvpart_;
    }
    const std::vector<int>& GetShellMaterial() const
    {
        return shell_matid_;
    }
    const std::vector<int>& GetMissingShellMaterial() const
    {
        return shell_msmat_;
    }
    const std::vector<int> &GetShellBlockMeshMap(int nb) const
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
    const std::string& GetBeamVariableName( int idx ) const
    {
        return beam_dsname_[idx];
    }
    const std::vector<int>& GetBeamVariableDims( int idx ) const
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
    const std::vector<int>& GetBeamHistoryPartitions() const
    {
        return beam_hvpart_;
    }
    const std::vector<int>& GetBeamMaterial() const
    {
        return beam_matid_;
    }
    const std::vector<int>& GetMissingBeamMaterial() const
    {
        return beam_msmat_;
    }
    const std::vector<int> &GetBeamBlockMeshMap(int nb) const
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
    const std::string& GetSurfaceVariableName( int idx ) const
    {
        return surf_dsname_[idx];
    }
    const std::vector<int>& GetSurfaceVariableDims( int idx ) const
    {
        return surf_dsdims_[idx];
    }
    const std::vector<int>& GetSurfaceMaterial() const
    {
        return surf_matid_;
    }
    const std::vector<int> &GetSurfaceBlockMeshMap(int nb) const
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
    const std::string& GetSphVariableName( int idx ) const
    {
        return sph_dsname_[idx];
    }
    const std::vector<int>& GetSphVariableDims( int idx ) const
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
    const std::vector<int>& GetSphHistoryPartitions() const
    {
        return sph_hvpart_;
    }
    const std::vector<int>& GetSphMaterial() const
    {
        return sph_matid_;
    }
    const std::vector<int>& GetMissingSphMaterial() const
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
    //const vector<std::string>& GetTiedSetVariableNames() const { return tdst_dsname_; }
    int  GetNumOfTiedSetVariables() const
    {
        return tdst_dsname_.size();
    }
    const std::string& GetTiedSetVariableName( int idx ) const
    {
        return tdst_dsname_[idx];
    }
    const std::vector<int>& GetTiedSetVariableDims( int idx ) const
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
    const std::string& GetContactVariableName( int idx ) const
    {
        return cntt_dsname_[idx];
    }
    const std::vector<int>& GetContactVariableDims( int idx ) const
    {
        return cntt_dsdims_[idx];
    }


    void ReadSolidBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    bool ReadSolidBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data );
    virtual
    void ReadSolidMaterial();
    virtual
    void ReadSolidBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );

    void ReadShellBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    bool ReadShellBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data );
    virtual
    void ReadShellMaterial();
    virtual
    void ReadShellBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );

    void ReadBeamBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    bool ReadBeamBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data );
    virtual
    void ReadBeamMaterial();
    virtual
    void ReadBeamBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );

    void ReadSurfaceBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    bool ReadSurfaceBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data );
    virtual
    void ReadSurfaceMaterial();
    virtual
    void ReadSurfaceBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );

    void ReadSphBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadSphBlockData( const char* varname, int blkInd, std::vector<int>& dims, std::vector<float>& data );
    void ReadSphMaterial(bool allowCollective = true);
    void ReadSphBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );


    void ReadTiedSetBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadTiedSetSlaveBlockMesh( int blkInd, std::vector<float>& crd );
    void ReadTiedSetMasterBlockMesh( int blkInd, std::vector<float>& crd );
    bool ReadTiedSetBlockData( const char* varname, int nb,
                               std::vector<int>& dims, std::vector<float>& data );

    void ReadContactBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadContactSlaveBlockMesh( int blkInd, std::vector<float>& crd );
    void ReadContactMasterBlockMesh( int blkInd, std::vector<float>& crd );
    bool ReadContactBlockData( const char* varname, int nb,
                               std::vector<int>& dims, std::vector<float>& data );

    void AppendMissingMaterialMesh( int type, int blkInd, std::vector<float>& vcrd, std::vector<int>& elmt );

    void AppendMissingMaterialData( int type, int blkInd, std::vector<int>& dims, std::vector<float>& data );
    void AppendMissingMaterialMaterial( int type, int blkInd, std::vector<int>& dims, std::vector<float>& data );


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
                          const std::vector<int>& tgt,
                          const std::vector<int>& own,
                          std::vector<int>& msmat );

    void ReadInfo( hid_t file_id, const char* name,
                   int&                 num,
                   std::vector<int>&         part,
                   std::vector<std::string>&      dsname,
                   std::vector<std::vector<int> >& dsdims,
                   const char* num_str=NULL );

    void ReadNodeIndexCoord( hid_t file_id );
    void ReadNodeData( hid_t fid, int varInd, const std::vector<int>& map,
                       std::vector<int>& dims, std::vector<float>& dat );

    virtual
    void ReadSolidBlockMesh( hid_t file_id, int blkInd,
                             std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    virtual
    void ReadSolidBlockData( hid_t file_id, int varInd, int blkInd,
                             std::vector<int>& dims, std::vector<float>& edat );
    virtual
    void ReadSolidBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    std::vector<int>& dims, std::vector<float>& data );
    void CollectSolidMaterial( hid_t file_id, int varInd, int blkInd,
                               std::vector<int>& dims, std::vector<float>& edat );

    virtual
    void ReadShellBlockMesh( hid_t file_id, int blkInd,
                             std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    virtual
    void ReadShellBlockData( hid_t file_id, int varInd, int blkInd,
                             std::vector<int>& dims, std::vector<float>& edat );
    virtual
    void ReadShellBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    std::vector<int>& dims, std::vector<float>& data );

    virtual
    void ReadBeamBlockMesh( hid_t file_id, int blkInd,
                            std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    virtual
    void ReadBeamBlockData( hid_t file_id, int varInd, int blkInd,
                            std::vector<int>& dims, std::vector<float>& edat );
    virtual
    void ReadBeamBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                   std::vector<int>& dims, std::vector<float>& data );

    virtual
    void ReadSurfaceBlockMesh( hid_t file_id, int blkInd,
                               std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    virtual
    void ReadSurfaceBlockData( hid_t file_id, int varInd, int blkInd,
                               std::vector<int>& dims, std::vector<float>& edat );

    void ReadSphBlockMesh( hid_t file_id, int blkInd,
                           std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadSphBlockData( hid_t file_id, int varInd, int blkInd,
                           std::vector<int>& dims, std::vector<float>& edat );


    virtual
    void ReadHistoryDataInfo( hid_t fid, const char* meshname,
                              const std::vector<int>& part,
                              int& mxnb, std::vector<int>& hvpart );
    void ReadBlockHistoryData( hid_t fid, const char* meshname,  int blkInd, int varInd,
                               const std::vector<int>& part,  const std::vector<int>& hvpart,
                               std::vector<int>& hvsft, std::vector<int>& dims, std::vector<float>& data );

    void ReadTiedSetBlockMesh( hid_t file_id, int blkInd,
                               std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadTiedSetSlaveBlockMesh( hid_t file_id, int blkInd, std::vector<float>& crd );
    void ReadTiedSetMasterBlockMesh( hid_t file_id, int blkInd, std::vector<float>& crd );
    void ReadTiedSetBlockData( hid_t file_id, int varInd, int blkInd,
                               std::vector<int>& dims, std::vector<float>& edat );

    void ReadContactBlockMesh( hid_t file_id, int blkInd,
                               std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadContactSlaveBlockMesh( hid_t file_id, int blkInd, std::vector<float>& crd );
    void ReadContactMasterBlockMesh( hid_t file_id, int blkInd, std::vector<float>& crd );
    void ReadContactBlockData( hid_t file_id, int varInd, int blkInd,
                               std::vector<int>& dims, std::vector<float>& edat );



    void CollectMaterial( hid_t fid, const char* meshname, const std::vector<std::string>& dsname,
                          int ne, std::vector<int>& mat, bool allowCollective = true );




protected:
    hid_t OpenFile();
    void  CloseFile( hid_t fid );
    void  CloseFile();

    static hid_t OpenGroup( hid_t loc, const char* name );
    static void  CloseGroup( hid_t gid );
    static void  CollectGroupDataSets( hid_t gid, std::vector<std::string>& name, std::vector<std::vector<int> >& dims );
    static void  CollectSubgroups( hid_t gid, std::vector<std::string>& sgname );

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

    static void LoadSampleDatasetNames( const std::string& filename, const std::string& grpname,
                                        std::vector<std::string>& dsname, std::vector<std::vector<int> >& dsdims,
                                        bool& lhv, int& mxhv );
    static void GenMissMesh( int mesh_type,
                             const std::vector<int>& msmat, const std::vector<float>& crd,
                             std::vector<float>& vcrd, std::vector<int>& elmt );
    static void GenMissData( const std::vector<int>& vdim, const std::vector<int>& msmat,
                             std::vector<int>& dims, std::vector<float>& data );
    static void GenMissMaterial( const std::vector<int>& msmat,
                                 std::vector<int>& dims, std::vector<float>& data );

    static void GenDefaultSphVariables( std::vector<std::string>& dsname, std::vector<std::vector<int> >& dsdims );


    /* void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname, int* buf, */
    /*              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); */
    /* void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname, float* buf, */
    /*              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); */
    /* void ReadGroupDataSet( hid_t file_id, const char* gname, const char* dsname, double* buf, */
    /*              int ndim=0, const hsize_t* sft=NULL, const hsize_t* cnt=NULL ); */




protected:
    //bool           hasTOCread_;
    std::string         filename_;
    std::string         title_;

    hid_t          file_id_;

    int            ncycle_; // number of cycles
    float          time_;   // simulation time

    int            nmmat_;  // number of materials
    std::vector<int>    mat_type_;
    std::vector<std::string> mat_titles_;

    // node
    int                    nnode_;
    std::vector<std::string>         node_dsname_;
    std::vector<std::vector<int> >    node_dsdims_;
    std::vector<int>            node_idx_;  // node index
    std::vector<float>          node_crd_;  // node coord
    int                    node_idx_mn_;
    int                    node_idx_mx_;
    std::vector<int>            node_map_;


    // solid
    int                 nsolid_;
    std::vector<int>         solid_part_;  // partition
    std::vector<std::string>      solid_dsname_;
    std::vector<std::vector<int> > solid_dsdims_;
    std::vector<std::vector<int> > solid_blkmap_;
    bool                solid_lhv_;
    int                 solid_mxhv_;
    std::vector<int>         solid_hvpart_;
    std::vector<std::vector<int> > solid_hvdisp_;
    std::vector<int>         solid_matid_;
    std::vector<int>         solid_msmat_;


    // shell
    int                 nshell_;
    std::vector<int>         shell_part_;  // partition
    std::vector<std::string>      shell_dsname_;
    std::vector<std::vector<int> > shell_dsdims_;

    std::vector<std::vector<int> > shell_blkmap_;
    bool                shell_lhv_;
    int                 shell_mxhv_;
    std::vector<int>         shell_hvpart_;
    std::vector<std::vector<int> > shell_hvdisp_;
    std::vector<int>         shell_matid_;
    std::vector<int>         shell_msmat_;


    // beam
    int                 nbeam_;
    std::vector<int>         beam_part_;  // partition
    std::vector<std::string>      beam_dsname_;
    std::vector<std::vector<int> > beam_dsdims_;

    std::vector<std::vector<int> > beam_blkmap_;
    bool                beam_lhv_;
    int                 beam_mxhv_;
    std::vector<int>         beam_hvpart_;
    std::vector<std::vector<int> > beam_hvdisp_;
    std::vector<int>         beam_matid_;
    std::vector<int>         beam_msmat_;


    // surface
    int                 nsurf_;
    std::vector<int>         surf_part_;  // partition
    std::vector<std::string>      surf_dsname_;
    std::vector<std::vector<int> > surf_dsdims_;
    std::vector<std::vector<int> > surf_blkmap_;
    std::vector<int>         surf_matid_;

    // sph
    int                 nsph_;
    std::vector<int>         sph_part_;  // partition
    std::vector<std::string>      sph_dsname_;
    std::vector<std::vector<int> > sph_dsdims_;
    bool                sph_lhv_;
    int                 sph_mxhv_;
    std::vector<int>         sph_hvpart_;
    std::vector<std::vector<int> > sph_hvdisp_;
    std::vector<int>         sph_matid_;
    std::vector<int>         sph_msmat_;

    // tiedset
    int                 ntdst_;
    std::vector<int>         tdst_part_;  // partition
    std::vector<std::string>      tdst_dsname_;
    std::vector<std::vector<int> > tdst_dsdims_;
    std::vector<std::vector<int> > tdst_blkmap_;

    // contact
    int                 ncntt_;
    std::vector<int>         cntt_part_;  // partition
    std::vector<std::string>      cntt_dsname_;
    std::vector<std::vector<int> > cntt_dsdims_;
    std::vector<std::vector<int> > cntt_blkmap_;



public:
    typedef struct
    {
        std::vector<std::string>                 names_;
        std::vector<std::vector<std::string> >         dsnames_;
        std::vector<std::vector<std::vector<int> > >    dsdims_;
        std::vector<std::vector<std::vector<double> > > dsdata_;
    } IndexVariables;

    const IndexVariables& GetSolidIndexVariables() const
    {
        return solid_idxvar_;
    }
    const IndexVariables& GetShellIndexVariables() const
    {
        return shell_idxvar_;
    }

    void ReadSolidBlockIndexVariable( const std::string& idxname,
                                      const std::string& varname,
                                      int domain,
                                      std::vector<int>& dims,
                                      std::vector<float>& data );
    void ReadShellBlockIndexVariable( const std::string& idxname,
                                      const std::string& varname,
                                      int domain,
                                      std::vector<int>& dims,
                                      std::vector<float>& data );
protected:
    void ReadIndexVariableInfo( hid_t file_id,
                                const char* mshname,
                                const char* ivname,
                                const std::vector<std::string>& dsname,
                                IndexVariables& iv );
    void ReadStringArrayAttribute( hid_t loc,
                                   const char* name,
                                   std::vector<std::string>& strings );
    void PickIndexVariable( const std::vector<int>&    dim,
                            const std::vector<double>& smp,
                            const std::vector<float>&  idx,
                            std::vector<float>&  dat );


protected:
    IndexVariables solid_idxvar_;
    IndexVariables shell_idxvar_;


public:
    static std::string ComposeNames( const std::string& m, const std::string& v, const char* seperator="/" );
    static void   DecomposeNames( const std::string& str, std::string& m, std::string& v, const char* seperator="/" );
    static void   DecomposeNames( const std::string& str, std::vector<std::string>& vs, const char* seperator="/" );
    static void   ConvDyna3dStreeTensor( const float* stress, float* tensor );
    static void   ConvSphStreeTensor( const float* stress, float* tensor );



public:
    //static void*        mpi_comm_ptr;
    //static MPI_Comm     mpi_comm;
    static const std::string general_name;
    static const std::string node_name;
    static const std::string solid_name;
    static const std::string shell_name;
    static const std::string beam_name;
    static const std::string surface_name;
    static const std::string sph_name;
    static const std::string tiedset_name;
    static const std::string tiedset_slave_name;
    static const std::string contact_name;
    static const std::string contact_slave_name;
    static const std::string node_index_name;
    static const std::string number_of_history_name;
    static const std::string history_name;
    static const std::string partition_name;

    static const int solid_elmt_type;
    static const int beam_elmt_type;
    static const int shell_elmt_type;
    static const int tshell_elmt_type;
    static const int sph_elmt_type;

    static const int default_number_of_partitions;

};


void EquallyPartition( int npart, int total, int* sft );


template<class T>
void FreeVector( std::vector<T>& vv )
{
    std::vector<T> tmp;
    tmp.swap(vv);
}

#endif
