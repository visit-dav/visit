    
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


    void ReadSolidBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
    {
        PVLD_Reader::ReadSolidBlockMesh( nb, vcrd, elmt );
    }
    bool ReadSolidBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data )
    {
        return PVLD_Reader::ReadSolidBlockData( varname, nb, dims, data );
    }
    void ReadSolidMaterial()
    {
        /*void*/
    }
    void ReadSolidBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );


    void ReadShellBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
    {
        PVLD_Reader::ReadShellBlockMesh( nb, vcrd, elmt );
    }
    bool ReadShellBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data )
    {
        return PVLD_Reader::ReadShellBlockData( varname, nb, dims, data );
    }
    void ReadShellMaterial()
    {
        /*void*/
    }
    void ReadShellBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );


    void ReadSurfaceBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
    {
        PVLD_Reader::ReadSurfaceBlockMesh( nb, vcrd, elmt );
    }
    bool ReadSurfaceBlockData( const char* varname, int nb, vector<int>& dims, vector<float>& data )
    {
        return PVLD_Reader::ReadSurfaceBlockData( varname, nb, dims, data );
    }
    void ReadSurfaceMaterial()
    {
        /*void*/
    }
    void ReadSurfaceBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data );


protected:
    void ReadSolidBlockMesh( hid_t file_id, int blkInd,
                             vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    void ReadSolidBlockData( hid_t file_id, int varInd, int blkInd,
                             vector<int>& dims, vector<float>& edat );
    void ReadSolidBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    vector<int>& dims, vector<float>& data );

    void ReadShellBlockMesh( hid_t file_id, int blkInd,
                             vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    void ReadShellBlockData( hid_t file_id, int varInd, int blkInd,
                             vector<int>& dims, vector<float>& edat );
    void ReadShellBlockHistoryData( hid_t fid, int blkInd, int varInd,
                                    vector<int>& dims, vector<float>& data );

    void ReadSurfaceBlockMesh( hid_t file_id, int blkInd,
                               vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt );
    void ReadSurfaceBlockData( hid_t file_id, int varInd, int blkInd,
                               vector<int>& dims, vector<float>& edat );


protected:
    void ReadMaterialInfo( hid_t fid, const char* grpname,
                           const vector<string>& dsname,
                           int npart, int ne,
                           vector<int>& part,
                           vector<int>& prtsft, vector<int>& mat,
                           vector<int>& matsft, vector<int>& matidx );
    void ReadHistoryDataInfo( hid_t fid, const char* meshname,
                              const vector<int>& part,
                              int& mxnb, vector<int>& hvpart );
    // void ReadMaterialHistoryInfo( hid_t fid, const char* meshname,
    //                 const vector<int>& part,
    //                 const vector<int>& eidx,
    //                 int& mxnb,
    //                 vector<int>& hvprt,
    //                 vector<int>& hvsft );

    void ReadRawData( hid_t fid, const char* grpname, const char* varname,
                      const vector<int>& dims,
                      const vector<int>& idx,
                      vector<int>& data );
    void ReadRawData( hid_t fid, const char* grpname, const char* varname,
                      const vector<int>& dims,
                      const vector<int>& idx,
                      vector<float>& data );
    void ReadRawHistoryData( hid_t fid,
                             const char* grpname,
                             const vector<int>& idx,
                             vector<int>& sft, vector<float>& dat );
    void ReadElementIndex( hid_t fid,
                           const char* grpname,
                           const int ne,
                           const vector<int>& mat,
                           const vector<int>& matsft,
                           vector<int>& matidx );



protected:
    // number of engines
    //int                 nengines_;

    // solid
    vector<int>         solid_prtsft_;
    vector<int>         solid_matsft_;
    vector<int>         solid_matidx_;
    vector<int>         solid_hvesft_;
    vector<int>         solid_elmdef_;
    string              solid_varnam_;
    vector<float>       solid_elmvar_;

    // shell
    vector<int>         shell_prtsft_;
    vector<int>         shell_matsft_;
    vector<int>         shell_matidx_;
    vector<int>         shell_hvesft_;
    vector<int>         shell_elmdef_;
    string              shell_varnam_;
    vector<float>       shell_elmvar_;

    // surface
    vector<int>         surf_prtsft_;
    vector<int>         surf_matsft_;
    vector<int>         surf_matidx_;
    vector<int>         surf_elmdef_;
    string              surf_varnam_;
    vector<float>       surf_elmvar_;


};

#endif
