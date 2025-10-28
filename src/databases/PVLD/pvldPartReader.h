    
#ifndef PVLD_PART_READER_HPP
#define PVLD_PART_READER_HPP

#include <pvldReader.h>


class PVLD_Part_Reader
    : public PVLD_Reader
{
public:
    PVLD_Part_Reader()
        : PVLD_Reader()
    {}

    void FreeResource();
    void ReadTOC();
    void ReadHistoryTOC();


    void ReadSolidBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt )
    {
        PVLD_Reader::ReadSolidBlockMesh( nb, vcrd, elmt );
    }
    bool ReadSolidBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data, std::vector<int>& idata )
    {
        return PVLD_Reader::ReadSolidBlockData( varname, nb, dims, data, idata );
    }
    void ReadSolidMaterial()
    {
        /*void*/
    }
    void ReadSolidBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );


    void ReadShellBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt )
    {
        PVLD_Reader::ReadShellBlockMesh( nb, vcrd, elmt );
    }
    bool ReadShellBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data, std::vector<int>& idata )
    {
        return PVLD_Reader::ReadShellBlockData( varname, nb, dims, data, idata );
    }
    void ReadShellMaterial()
    {
        /*void*/
    }
    void ReadShellBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );


    void ReadSurfaceBlockMesh( int nb, std::vector<float>& vcrd, std::vector<int>& elmt )
    {
        PVLD_Reader::ReadSurfaceBlockMesh( nb, vcrd, elmt );
    }
    bool ReadSurfaceBlockData( const char* varname, int nb, std::vector<int>& dims, std::vector<float>& data, std::vector<int>& idata )
    {
        return PVLD_Reader::ReadSurfaceBlockData( varname, nb, dims, data, idata );
    }
    void ReadSurfaceMaterial()
    {
        /*void*/
    }
    void ReadSurfaceBlockMaterial( int blkInd, std::vector<int>& dims, std::vector<float>& data );


protected:
    void ReadSolidBlockMesh( hid_t file_id, int blkInd,
                             std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadSolidBlockData( hid_t file_id, int varInd, int blkInd,
                             std::vector<int>& dims, std::vector<float>& edat );
    void ReadSolidBlockData( hid_t file_id, int varInd, int blkInd,
                             std::vector<int>& dims, std::vector<int>& edat );
    void ReadSolidBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    std::vector<int>& dims, std::vector<float>& data );

    void ReadShellBlockMesh( hid_t file_id, int blkInd,
                             std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadShellBlockData( hid_t file_id, int varInd, int blkInd,
                             std::vector<int>& dims, std::vector<float>& edat );
    void ReadShellBlockData( hid_t file_id, int varInd, int blkInd,
                             std::vector<int>& dims, std::vector<int>& edat );
    void ReadShellBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    std::vector<int>& dims, std::vector<float>& data );

    void ReadSurfaceBlockMesh( hid_t file_id, int blkInd,
                               std::vector<int>& vmap, std::vector<float>& vcrd, std::vector<int>& elmt );
    void ReadSurfaceBlockData( hid_t file_id, int varInd, int blkInd,
                               std::vector<int>& dims, std::vector<float>& edat );
    void ReadSurfaceBlockData( hid_t file_id, int varInd, int blkInd,
                               std::vector<int>& dims, std::vector<int>& edat );


protected:
    void ReadMaterialInfo( hid_t fid, const char* grpname,
                           const std::vector<std::string>& dsname,
                           int npart, int ne,
                           std::vector<int>& part,
                           std::vector<int>& prtsft, std::vector<int>& mat,
                           std::vector<int>& matsft, std::vector<int>& matidx );
    void ReadHistoryDataInfo( hid_t fid, const char* meshname,
                              const std::vector<int>& part,
                              int& mxnb, std::vector<int>& hvpart );

    void ReadRawData( hid_t fid, const char* grpname, const char* varname,
                      const std::vector<int>& dims,
                      const std::vector<int>& idx,
                      std::vector<int>& data );
    void ReadRawData( hid_t fid, const char* grpname, const char* varname,
                      const std::vector<int>& dims,
                      const std::vector<int>& idx,
                      std::vector<float>& data );
    void ReadRawHistoryData( hid_t fid,
                             const char* grpname,
                             const std::vector<int>& idx,
                             std::vector<int>& sft, std::vector<float>& dat );
    void ReadElementIndex( hid_t fid,
                           const char* grpname,
                           const int ne,
                           const std::vector<int>& mat,
                           const std::vector<int>& matsft,
                           std::vector<int>& matidx );



protected:
    // number of engines
    //int                 nengines_;

    // solid
    std::vector<int>         solid_prtsft_;
    std::vector<int>         solid_matsft_;
    std::vector<int>         solid_matidx_;
    std::vector<int>         solid_hvesft_;
    std::vector<int>         solid_elmdef_;
    std::string              solid_varnam_;
    std::vector<float>       solid_elmvar_;
    std::vector<int>         solid_elmvari_;

    // shell
    std::vector<int>         shell_prtsft_;
    std::vector<int>         shell_matsft_;
    std::vector<int>         shell_matidx_;
    std::vector<int>         shell_hvesft_;
    std::vector<int>         shell_elmdef_;
    std::string              shell_varnam_;
    std::vector<float>       shell_elmvar_;
    std::vector<int>         shell_elmvari_;

    // surface
    std::vector<int>         surf_prtsft_;
    std::vector<int>         surf_matsft_;
    std::vector<int>         surf_matidx_;
    std::vector<int>         surf_elmdef_;
    std::string              surf_varnam_;
    std::vector<float>       surf_elmvar_;
    std::vector<int>         surf_elmvari_;


};

#endif
