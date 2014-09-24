
#ifndef _WIN32
#include <dirent.h>
#endif
#include <sys/types.h>

#include <map>
#include <utility>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <algorithm>

#ifdef PARALLEL
#include <avtParallel.h>
#endif
#include <DebugStream.h>
#include <snprintf.h>
#include <pvldReader.h>

using std::set;
using std::vector;
using std::string;

static int STOI(const std::string &s)
{
    return atoi(s.c_str());
}

void OutputVectorInt( ostream& os, const string& cmt, const vector<int>& vec )
{
    os << cmt << " { ";
    for(size_t i = 0; i < vec.size(); ++i)
        os << vec[i] << " , ";
    os << " }\n";
}


void EquallyPartition( int npart, int total, int* sft )
{
    int res = total % npart;
    int avg = ( total - res )/ npart;

    sft[0] = 0;
    for( int i=0; i<npart; ++i )
    {
        if( i<res )
            sft[i+1] = sft[i] + avg + 1;
        else
            sft[i+1] = sft[i] + avg;
    }
}


void GetDirectoryName( const string& filename, string& dirname )
{
    string::size_type np = filename.rfind( "/" );
    dirname = filename.substr( 0, np );
}

#if 1
#include <FileFunctions.h>

struct file_match_struct
{
    const string   *surname;
    vector<string> *fnames;
};

void AppendMatchingFiles(void *cbData, const std::string &fileName, bool isDir, bool canAccess, long fileSize)
{
    file_match_struct *ptr = (file_match_struct *)cbData;
    string surname(*ptr->surname);

    string::size_type np= fileName.rfind( surname );
    if( np != string::npos &&
            np >=1 &&
            fileName.substr(np) == surname )
    {
        string::size_type s = fileName.rfind(VISIT_SLASH_STRING);
        if(s != string::npos)
        {
            string fn(fileName.substr(s+1,fileName.size()-s));
            ptr->fnames->push_back( fn );
        }
    }
}

void CollectFileNames( const string& surname, const string& dirname, vector<string>& fnames )
{
    file_match_struct cbData;
    cbData.surname = &surname;
    cbData.fnames = &fnames;
    FileFunctions::ReadAndProcessDirectory(dirname, AppendMatchingFiles, (void *)&cbData, false);
}
#else

void CollectFileNames( const string& surname, const string& dirname, vector<string>& fnames )
{
    DIR *dir = opendir( dirname.c_str() );
    if( dir == NULL) return;

    struct dirent *ent;
    while( (ent = readdir(dir)) != NULL )
    {
        string fn = ent->d_name;
        string::size_type np=fn.rfind( surname );
        if( np!=string::npos &&
                np>=1 &&
                fn.substr(np)==surname )
            fnames.push_back( fn );
    }
    closedir(dir);
}
#endif


extern "C"
herr_t GetAttributeName( hid_t gid, const char* name, const H5A_info_t *info, void *op_data )
{
    vector<string>* nms = reinterpret_cast<vector<string>*>(op_data);
    nms->push_back( name );
    return 0;
}


extern "C"
herr_t GetDataSetName( hid_t gid, const char* name, const H5L_info_t *info, void *op_data )
{
    H5O_info_t io;
    herr_t herr = H5Oget_info_by_name( gid, name, &io, H5P_DEFAULT );
    if( herr<0 )  return herr;

    if( io.type == H5O_TYPE_DATASET )
    {
        vector<string>* nms = reinterpret_cast<vector<string>*>(op_data);
        nms->push_back( name );
    }
    return 0;
}


extern "C"
herr_t GetSubgroupName( hid_t gid, const char* name, const H5L_info_t *info, void *op_data )
{
    H5O_info_t io;
    herr_t herr = H5Oget_info_by_name( gid, name, &io, H5P_DEFAULT );
    if( herr<0 )  return herr;

    if( io.type == H5O_TYPE_GROUP )
    {
        vector<string>* nms = reinterpret_cast<vector<string>*>(op_data);
        nms->push_back( name );
    }
    return 0;
}




//void*        PVLD_Reader::mpi_comm_ptr =NULL;
//MPI_Comm     PVLD_Reader::mpi_comm     = MPI_COMM_NULL;
const string PVLD_Reader::general_name ="General";
const string PVLD_Reader::node_name    ="Node";
const string PVLD_Reader::solid_name   ="Solid";
const string PVLD_Reader::shell_name   ="Shell";
const string PVLD_Reader::beam_name    ="Beam";
const string PVLD_Reader::surface_name ="Surface";
const string PVLD_Reader::sph_name     ="SPH";
const string PVLD_Reader::tiedset_name ="TiedNodeSet";
const string PVLD_Reader::tiedset_slave_name ="TiedNodeSetSlave";
const string PVLD_Reader::contact_name ="Contact";
const string PVLD_Reader::contact_slave_name ="ContactSlave";
const string PVLD_Reader::node_index_name="NodeIndex";
const string PVLD_Reader::number_of_history_name="NumberOfHistoryVariables";
const string PVLD_Reader::history_name="HistoryVariable";
const string PVLD_Reader::partition_name="partition";


const int PVLD_Reader::solid_elmt_type =0;
const int PVLD_Reader::beam_elmt_type  =1;
const int PVLD_Reader::shell_elmt_type =2;
const int PVLD_Reader::tshell_elmt_type=3;
const int PVLD_Reader::sph_elmt_type   =4;

const int PVLD_Reader::default_number_of_partitions=32;



void PVLD_Reader::
LoadSampleDatasetNames( const string& filename, const string& grpname,
                        vector<string>& dsname, vector<vector<int> >& dsdims,
                        bool& lhv, int& mxhv )
{
    try
    {
        // char* sphfile = getenv( "SPH_SAMPLE_VLD" );
        // if( !sphfile ) {
        //   string msg= "Undefined the Sample filename in LoadSampleDatasetNames( \"";
        //   msg+=filename; msg+="\", \""; msg+=grpname; msg+="\" ).\n";
        //   throw std::runtime_error(msg);
        // }

        hid_t fid = H5Fopen( filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        if( fid<=0 )
        {
            string msg = "Failed to open the Sample file: LoadSampleDatasetNames( \"";
            msg+=filename;
            msg+="\", \"";
            msg+=grpname;
            msg+="\" ).\n";
            throw std::runtime_error(msg);
        }

        hid_t gid = PVLD_Reader::OpenGroup( fid, grpname.c_str() );
        if( gid<=0 )
        {
            string msg = "Failed to open the Group : LoadSampleDatasetNames( \"";
            msg+=filename;
            msg+="\", \"";
            msg+=grpname;
            msg+="\" ).\n";
            throw std::runtime_error(msg);
        }

        CollectGroupDataSets( gid, dsname, dsdims );

        lhv = ( std::find( dsname.begin(), dsname.end(),
                           number_of_history_name ) != dsname.end() &&
                std::find( dsname.begin(), dsname.end(),
                           history_name ) != dsname.end() );
        mxhv=0;
        if( lhv )
        {
#ifdef PARALLEL
            if( PAR_Rank()==0 )
            {
#endif
                vector<int> cnt;
                cnt.resize( dsdims[0][0] );
                hid_t did = OpenDataSet( gid, number_of_history_name.c_str() );
                ReadDataSet( did, H5T_NATIVE_INT, &cnt[0] );
                CloseDataSet(did);
                mxhv = *(std::max_element( cnt.begin(), cnt.end() ));
#ifdef PARALLEL
            }
            BroadcastInt( mxhv );
#endif
        }

        H5Gclose(gid);
        H5Fclose(fid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg+="Failure in LoadSphDatasetNames( \"";
        msg+=filename;
        msg+="\" )!\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
GenMissMesh( int type,
             const vector<int>& msmat, const vector<float>& crd,
             vector<float>& vcrd, vector<int>& elmt )
{
    switch( type )
    {
    case( PVLD_Reader::sph_elmt_type ):
        if( !crd.empty() )
        {
            vcrd.push_back( crd[0] );
            vcrd.push_back( crd[1] );
            vcrd.push_back( crd[2] );
        }
        else
            vcrd.assign( 3, 0 );
        for(size_t i = 0; i < msmat.size(); ++i)
            elmt.push_back(0);
        break;

    default:
        throw std::runtime_error( "Unsupported element type in PVLD_Reader::GenMissMesh().\n");
    }
}


void PVLD_Reader::
GenMissData( const vector<int>& vdim, const vector<int>& msmat,
             vector<int>& dims, vector<float>& data )
{
    dims = vdim;
    dims[0]=msmat.size();

    int tot=dims[0];
    for( size_t i=1; i<dims.size(); i++ )
        tot *= dims[i];
    data.assign(tot, 0);
}


void PVLD_Reader::
GenMissMaterial( const vector<int>& msmat,
                 vector<int>& dims, vector<float>& data )
{
    dims.resize(1);
    dims[0]=msmat.size();
    for(size_t i = 0; i < msmat.size(); ++i)
        data.push_back(msmat[i]);
}






string PVLD_Reader::
ComposeNames( const string& m, const string& v, const char* seperator  )
{
    return m + seperator + v;
}

void PVLD_Reader::
DecomposeNames( const string& str, string& m, string& v, const char* seperator )
{
    int len = str.size();
    char* buf = new char[ len+1 ];
    str.copy( buf, len );
    buf[len] = '\0';

    char* first = strtok( buf,  seperator );
    if( first == NULL )
    {
        string msg = "Failed to obtain first name in DecomposeNames(): ";
        msg += "str=\"";
        msg += str;
        msg += "\"\n";
        throw std::runtime_error(msg);
    }
    m = first;

    char* second = strtok( NULL, seperator );
    if( second == NULL )
    {
        string msg = "Failed to obtain second name in DecomposeNames(): ";
        msg += "str=\"";
        msg += str;
        msg += "\"\n";
        throw std::runtime_error(msg);
    }
    v = second;

    delete [] buf;
}


void  PVLD_Reader::
DecomposeNames( const string& str, vector<string>& vs, const char* seperator )
{
    vs.clear();

    int len = str.size();
    char* buf = new char[ len+1 ];
    str.copy( buf, len );
    buf[len] = '\0';

    char *wrk = buf;
    while( char* ss = strtok( wrk, seperator ) )
    {
        vs.push_back( ss );
        wrk=NULL;
    }

    delete [] buf;
}





void PVLD_Reader::
ConvDyna3dStreeTensor( const float* stress, float* tensor )
{
    const float& c11 = stress[ 0 ];
    const float& c22 = stress[ 1 ];
    const float& c33 = stress[ 2 ];
    const float& c12 = stress[ 3 ];
    const float& c23 = stress[ 4 ];
    const float& c13 = stress[ 5 ];

    tensor[ 0 ] = c11;
    tensor[ 1 ] = c12;
    tensor[ 2 ] = c13;

    tensor[ 3 ] = c12;
    tensor[ 4 ] = c22;
    tensor[ 5 ] = c23;

    tensor[ 6 ] = c13;
    tensor[ 7 ] = c23;
    tensor[ 8 ] = c33;
}


void PVLD_Reader::
ConvSphStreeTensor( const float* stress, float* tensor )
{
    const float& c11 = stress[ 0 ];
    const float& c22 = stress[ 1 ];
    const float& c33 = stress[ 2 ];
    const float& c23 = stress[ 3 ];
    const float& c13 = stress[ 4 ];
    const float& c12 = stress[ 5 ];

    tensor[ 0 ] = c11;
    tensor[ 1 ] = c12;
    tensor[ 2 ] = c13;

    tensor[ 3 ] = c12;
    tensor[ 4 ] = c22;
    tensor[ 5 ] = c23;

    tensor[ 6 ] = c13;
    tensor[ 7 ] = c23;
    tensor[ 8 ] = c33;
}







void PVLD_Reader::
FreeResource()
{
    node_idx_.clear();
    FreeVector( node_idx_ );
    node_crd_.clear();
    FreeVector( node_crd_ );
    node_map_.clear();
    FreeVector( node_map_ );

    solid_blkmap_.clear();
    FreeVector( solid_blkmap_ );
    solid_hvdisp_.clear();
    FreeVector( solid_hvdisp_ );

    shell_blkmap_.clear();
    FreeVector( shell_blkmap_ );
    shell_hvdisp_.clear();
    FreeVector( shell_hvdisp_ );

    beam_blkmap_.clear();
    FreeVector( beam_blkmap_ );
    beam_hvdisp_.clear();
    FreeVector( beam_hvdisp_ );

    surf_blkmap_.clear();
    FreeVector( surf_blkmap_ );

    sph_hvdisp_.clear();
    FreeVector( sph_hvdisp_ );

    tdst_blkmap_.clear();
    FreeVector( tdst_blkmap_ );

    cntt_blkmap_.clear();
    FreeVector( cntt_blkmap_ );
}



void PVLD_Reader::
ReadTOC()
{
    // if( hasTOCread_ ) return;
    // hasTOCread_=true;

    H5open();
    try
    {
        hid_t file_id = OpenFile();

        vector<string> grpnames;
        CollectSubgroups( file_id, grpnames );
        bool lbeam = std::find( grpnames.begin(),
                                grpnames.end(),
                                beam_name ) != grpnames.end();

        ReadGeneralInfo( file_id );
        ReadNodeInfo( file_id );
        ReadSolidInfo( file_id );
        ReadShellInfo( file_id );
        if( lbeam )  ReadBeamInfo( file_id );
        ReadSurfaceInfo( file_id );
        ReadSPHInfo( file_id );
        ReadTiedsetInfo( file_id );
        ReadContactInfo( file_id );

        ReadIndexVariableInfo( file_id,
                               solid_name.c_str(),
                               "IndexVariables",
                               solid_dsname_,
                               solid_idxvar_ );
        ReadIndexVariableInfo( file_id,
                               shell_name.c_str(),
                               "IndexVariables",
                               shell_dsname_,
                               shell_idxvar_ );

        CloseFile( file_id );
    }
    catch( std::exception& e )
    {
        string msg("Exception caught in PVLD_Reader::ReadTOC() : \n\t");
        msg += e.what();
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
ReadHistoryTOC()
{
    H5open();
    try
    {
        hid_t file_id = OpenFile();
        if( solid_lhv_ )
        {
            ReadHistoryDataInfo( file_id, solid_name.c_str(), solid_part_,
                                 solid_mxhv_, solid_hvpart_ );
            int nblks = GetNumOfSolidBlocks();
            solid_hvdisp_.resize( nblks );
        }
        if( shell_lhv_ )
        {
            ReadHistoryDataInfo( file_id, shell_name.c_str(), shell_part_,
                                 shell_mxhv_, shell_hvpart_ );
            int nblks = GetNumOfShellBlocks();
            shell_hvdisp_.resize( nblks );
        }
        if( beam_lhv_ )
        {
            ReadHistoryDataInfo( file_id, beam_name.c_str(), beam_part_,
                                 beam_mxhv_, beam_hvpart_ );
            int nblks = GetNumOfBeamBlocks();
            beam_hvdisp_.resize( nblks );
        }
        if( sph_lhv_ )
        {
            ReadHistoryDataInfo( file_id, sph_name.c_str(), sph_part_,
                                 sph_mxhv_, sph_hvpart_ );
            int nblks = GetNumOfSphBlocks();
            sph_hvdisp_.resize( nblks );
        }
        CloseFile( file_id );
    }
    catch( std::exception& e )
    {
        string msg("Exception caught in PVLD_Reader::ReadHistoryTOC() : \n\t");
        msg += e.what();
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadMaterialType( bool add_missing_parts )
{
    try
    {
        ReadSolidMaterial();
        ReadShellMaterial();
        ReadSphMaterial();
        if( nbeam_>0 ) ReadBeamMaterial();

        solid_msmat_.clear();
        beam_msmat_.clear();
        shell_msmat_.clear();
        sph_msmat_.clear();

        if( add_missing_parts )
        {

            hid_t file_id = OpenFile();
            GetMaterialType( file_id, general_name.c_str() );
            CloseFile( file_id );

            GenMissingMaterials();
            debug1 << "solid_msmat_.sz=" << solid_msmat_.size() << "\n";

            if( !sph_msmat_.empty() && sph_dsname_.empty() )
            {
                string dirname;
                vector<string> smpnames;
                GetDirectoryName( filename_, dirname );
                CollectFileNames( ".vld", dirname, smpnames );

                for( vector<string>::const_iterator ii=smpnames.begin(); ii!=smpnames.end(); ii++ )
                {
                    string fn = dirname + "/" + *ii;
                    try
                    {
                        LoadSampleDatasetNames( fn, sph_name,
                                                sph_dsname_, sph_dsdims_,
                                                sph_lhv_,  sph_mxhv_ );
                    }
                    catch( std::exception& e )
                    {
                        // do nothing, continue the loop
                    }
                    if( !sph_dsname_.empty() ) break;
                }
                if( sph_dsname_.empty() )
                {
                    sph_lhv_=false;
                    sph_mxhv_=0;
                    GenDefaultSphVariables( sph_dsname_, sph_dsdims_ );
                }
                debug1 << "LoadSampleDatasetNames(): " << sph_dsname_.size() << ".\n";
            }
        }
    }
    catch( std::exception& e )
    {
        string msg("Exception caught in PVLD_Reader::ReadMaterialType() : \n\t");
        msg += e.what();
        throw std::runtime_error(msg);
    }
}





void PVLD_Reader::
ReadSolidBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        int nblks = GetNumOfSolidBlocks();
        if( nb >= nblks ) return;
        //if( nb >= nblks ) nb=0;
        //nb = nb<nblks ? nb : nblks-1;

        if( solid_blkmap_.size() != (size_t)nblks )
            solid_blkmap_.resize( nblks );

        vector<int>& vmap = solid_blkmap_[nb];

        hid_t fid = OpenFile();
        ReadNodeIndexCoord( fid );
        ReadSolidBlockMesh( fid, nb, vmap, vcrd, elmt );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSolidBlockMesh() public\n";
        throw std::runtime_error(msg);
    }
}

/*const vector<int> &
PVLD_Reader::GetSolidBlockMeshMap(int nb) const
{
    if(solid_blkmap_.size() != GetNumOfSolidBlocks())
    {
        throw std::runtime_error("ReadSolidBlockMesh must be called before GetSolidBlockMeshMap.");
    }

    return solid_blkmap_[nb];
}
*/

bool PVLD_Reader::
ReadSolidBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        int nblks = GetNumOfSolidBlocks();
        if( blkInd >= nblks ) return true;

        size_t ind;
        for( ind=0; ind<solid_dsname_.size(); ind++ )
            if( solid_dsname_[ind] == varname ) break;
        if( ind<solid_dsname_.size() )
        {
            hid_t fid = OpenFile();
            ReadSolidBlockData( fid, ind, blkInd, dims, data );
            CloseFile(fid);
            return true;
        }

        string ndvname = varname;
        if( ndvname == PVLD_Reader::node_index_name )  ndvname = "Index";

        for( ind=0; ind<node_dsname_.size(); ind++ )
            if( node_dsname_[ind] == ndvname ) break;
        if( ind<node_dsname_.size() )
        {
            hid_t fid = OpenFile();
            vector<int>& vmap = solid_blkmap_[blkInd];
            if( vmap.size()==0 )
            {
                vector<float> vcrd;
                vector<int>   elmt;
                ReadSolidBlockMesh( fid, blkInd, vmap, vcrd, elmt );
            }
            ReadNodeData( fid, ind, vmap, dims, data );
            CloseFile(fid);
            return false;
        }

        if( ndvname.find( history_name ) != string::npos )
        {
            string st1,st2;
            DecomposeNames( ndvname, st1, st2, "_" );
            ind = STOI( st2 ) - 1;
            hid_t fid = OpenFile();
            ReadSolidBlockHistoryData( fid, blkInd, ind, dims, data );
            // ReadBlockHistoryData( fid, solid_name.c_str(),
            //                 blkInd, ind,
            //                 solid_part_,  solid_hvpart_,
            //                 solid_hvdisp_[blkInd], dims, data );
            CloseFile(fid);
            return true;
        }

        throw std::runtime_error("Not existing solid variable is requested in PVLD_Reader::ReadSolidBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSolidBlockData()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSolidMaterial()
{
    if( solid_matid_.size()>0 ) return;
    try
    {
        hid_t fid = OpenFile();
        CollectMaterial( fid,
                         solid_name.c_str(),
                         solid_dsname_,
                         nsolid_,
                         solid_matid_ );
        CloseFile(fid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSolidMaterial()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSolidBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfSolidBlocks();
    if( blkInd >= nblks ) return;

    if( std::find( solid_dsname_.begin(),
                   solid_dsname_.end(),
                   "Material" ) != solid_dsname_.end() )
        ReadSolidBlockData( "Material", blkInd, dims, data );
    else
    {
        dims.resize(1);
        dims[0] = solid_part_.at(blkInd+1) - solid_part_.at(blkInd);
        data.assign( dims[0], nmmat_+1 );
    }
}




void PVLD_Reader::
ReadShellBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        size_t nblks = GetNumOfShellBlocks();
        if( (size_t)nb >= nblks ) return;

        if( shell_blkmap_.size() != nblks )
            shell_blkmap_.resize( nblks );

        vector<int>& vmap = shell_blkmap_[nb];

        hid_t fid = OpenFile();
        ReadNodeIndexCoord( fid );
        ReadShellBlockMesh( fid, nb, vmap, vcrd, elmt );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadShellBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}



bool PVLD_Reader::
ReadShellBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        int nblks = GetNumOfShellBlocks();
        if( blkInd >= nblks ) return true;

        size_t ind;
        for( ind=0; ind<shell_dsname_.size(); ind++ )
            if( shell_dsname_[ind] == varname ) break;
        if( ind<shell_dsname_.size() )
        {
            hid_t fid = OpenFile();
            ReadShellBlockData( fid, ind, blkInd, dims, data );
            CloseFile(fid);
            return true;
        }

        string ndvname = varname;
        if( ndvname == PVLD_Reader::node_index_name )  ndvname = "Index";

        for( ind=0; ind<node_dsname_.size(); ind++ )
            if( node_dsname_[ind] == ndvname ) break;
        if( ind<node_dsname_.size() )
        {
            hid_t fid = OpenFile();
            vector<int>& vmap = shell_blkmap_[blkInd];
            if( vmap.size()==0 )
            {
                vector<float> vcrd;
                vector<int>   elmt;
                ReadShellBlockMesh( fid, blkInd, vmap, vcrd, elmt );
            }
            ReadNodeData( fid, ind, vmap, dims, data );
            CloseFile(fid);
            return false;
        }

        if( ndvname.find( history_name ) != string::npos )
        {
            string st1,st2;
            DecomposeNames( ndvname, st1, st2, "_" );
            ind = STOI( st2 ) - 1;
            hid_t fid = OpenFile();
            ReadShellBlockHistoryData( fid, blkInd, ind, dims, data );
            // ReadBlockHistoryData( fid, shell_name.c_str(),
            //                 blkInd, ind,
            //                 shell_part_,  shell_hvpart_,
            //                 shell_hvdisp_[blkInd], dims, data );
            CloseFile(fid);
            return true;
        }

        throw std::runtime_error("Not existing shell variable is requested in PVLD_Reader::ReadShellBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadShellBlockData()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadShellMaterial()
{
    if( shell_matid_.size()>0 ) return;
    try
    {
        hid_t fid = OpenFile();
        CollectMaterial( fid,
                         shell_name.c_str(),
                         shell_dsname_,
                         nshell_,
                         shell_matid_ );
        CloseFile(fid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadShellMaterial()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadShellBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfShellBlocks();
    if( blkInd >= nblks ) return;

    if( std::find( shell_dsname_.begin(),
                   shell_dsname_.end(),
                   "Material" ) != shell_dsname_.end() )
        ReadShellBlockData( "Material", blkInd, dims, data );
    else
    {
        dims.resize(1);
        dims[0] = shell_part_.at(blkInd+1) - shell_part_.at(blkInd);
        data.assign( dims[0], nmmat_+1 );
    }
}




void PVLD_Reader::
ReadBeamBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        size_t nblks = GetNumOfBeamBlocks();
        if( (size_t)nb >= nblks ) return;

        if( beam_blkmap_.size() != nblks )
            beam_blkmap_.resize( nblks );

        vector<int>& vmap = beam_blkmap_[nb];

        hid_t fid = OpenFile();
        ReadNodeIndexCoord( fid );
        ReadBeamBlockMesh( fid, nb, vmap, vcrd, elmt );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadBeamBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}



bool PVLD_Reader::
ReadBeamBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        int nblks = GetNumOfBeamBlocks();
        if( blkInd >= nblks ) return true;

        size_t ind;
        for( ind=0; ind<beam_dsname_.size(); ind++ )
            if( beam_dsname_[ind] == varname ) break;
        if( ind<beam_dsname_.size() )
        {
            hid_t fid = OpenFile();
            ReadBeamBlockData( fid, ind, blkInd, dims, data );
            CloseFile(fid);
            return true;
        }

        string ndvname = varname;
        if( ndvname == PVLD_Reader::node_index_name )  ndvname = "Index";

        for( ind=0; ind<node_dsname_.size(); ind++ )
            if( node_dsname_[ind] == ndvname ) break;
        if( ind<node_dsname_.size() )
        {
            hid_t fid = OpenFile();
            vector<int>& vmap = beam_blkmap_[blkInd];
            if( vmap.size()==0 )
            {
                vector<float> vcrd;
                vector<int>   elmt;
                ReadBeamBlockMesh( fid, blkInd, vmap, vcrd, elmt );
            }
            ReadNodeData( fid, ind, vmap, dims, data );
            CloseFile(fid);
            return false;
        }

        if( ndvname.find( history_name ) != string::npos )
        {
            string st1,st2;
            DecomposeNames( ndvname, st1, st2, "_" );
            ind = STOI( st2 ) - 1;
            hid_t fid = OpenFile();
            ReadBeamBlockHistoryData( fid, blkInd, ind, dims, data );
            CloseFile(fid);
            return true;
        }

        throw std::runtime_error("Not existing beam variable is requested in PVLD_Reader::ReadBeamBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadBeamBlockData()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadBeamMaterial()
{
    if( beam_matid_.size()>0 ) return;
    try
    {
        hid_t fid = OpenFile();
        CollectMaterial( fid,
                         beam_name.c_str(),
                         beam_dsname_,
                         nbeam_,
                         beam_matid_ );
        CloseFile(fid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadBeamMaterial()\n";
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
ReadBeamBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfBeamBlocks();
    if( blkInd >= nblks ) return;

    if( std::find( beam_dsname_.begin(),
                   beam_dsname_.end(),
                   "Material" ) != beam_dsname_.end() )
        ReadBeamBlockData( "Material", blkInd, dims, data );
    else
    {
        dims.resize(1);
        dims[0] = beam_part_.at(blkInd+1) - beam_part_.at(blkInd);
        data.assign( dims[0], nmmat_+1 );
    }
}






void PVLD_Reader::
ReadSurfaceBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        size_t nblks = GetNumOfSurfaceBlocks();
        if( (size_t)nb >= nblks ) return;

        if( surf_blkmap_.size() != nblks )
            surf_blkmap_.resize( nblks );

        vector<int>& vmap = surf_blkmap_[nb];

        hid_t fid = OpenFile();
        ReadNodeIndexCoord( fid );
        ReadSurfaceBlockMesh( fid, nb, vmap, vcrd, elmt );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSurfaceBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}



bool PVLD_Reader::
ReadSurfaceBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        int nblks = GetNumOfSurfaceBlocks();
        if( blkInd >= nblks ) return true;

        size_t ind;
        for( ind=0; ind<surf_dsname_.size(); ind++ )
            if( surf_dsname_[ind] == varname ) break;
        if( ind<surf_dsname_.size() )
        {
            hid_t fid = OpenFile();
            ReadSurfaceBlockData( fid, ind, blkInd, dims, data );
            CloseFile(fid);
            return true;
        }

        string ndvname = varname;
        if( ndvname == PVLD_Reader::node_index_name )  ndvname = "Index";

        for( ind=0; ind<node_dsname_.size(); ind++ )
            if( node_dsname_[ind] == ndvname ) break;
        if( ind < node_dsname_.size() )
        {
            hid_t fid = OpenFile();
            vector<int>& vmap = surf_blkmap_[blkInd];
            if( vmap.size()==0 )
            {
                vector<float> vcrd;
                vector<int>   elmt;
                ReadSurfaceBlockMesh( fid, blkInd, vmap, vcrd, elmt );
            }
            ReadNodeData( fid, ind, vmap, dims, data );
            CloseFile(fid);
            return false;
        }

        throw std::runtime_error("Not existing shell variable is requested in PVLD_Reader::ReadSurfaceBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSurfaceBlockData(";
        msg+=varname;
        msg+=")\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSurfaceMaterial()
{
    if( surf_matid_.size()>0 ) return;
    try
    {
        hid_t fid = OpenFile();
        CollectMaterial( fid,
                         surface_name.c_str(),
                         surf_dsname_,
                         nsurf_,
                         surf_matid_ );
        CloseFile(fid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSurfaceMaterial()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSurfaceBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfSurfaceBlocks();
    if( blkInd >= nblks ) return;

    if( std::find( surf_dsname_.begin(),
                   surf_dsname_.end(),
                   "Material" ) != surf_dsname_.end() )
        ReadSurfaceBlockData( "Material", blkInd, dims, data );
    else
    {
        dims.resize(1);
        dims[0] = surf_part_.at(blkInd+1) - surf_part_.at(blkInd);
        data.assign( dims[0], nmmat_+1 );
    }
}




void PVLD_Reader::
ReadSphBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        if( HasSPH() )
        {
            hid_t fid = OpenFile();
            ReadSphBlockMesh( fid, nb, vcrd, elmt );
            CloseFile( fid );
        }
        else if( !sph_msmat_.empty() )
        {
            hid_t fid = OpenFile();
            ReadNodeIndexCoord( fid );
            CloseFile( fid );
            GenMissMesh( sph_elmt_type, sph_msmat_, node_crd_, vcrd, elmt );
        }
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSphBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSphBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        size_t ind;
        for( ind=0; ind<sph_dsname_.size(); ind++ )
            if( sph_dsname_[ind] == varname ) break;
        if( ind < sph_dsname_.size() )
        {
            if( HasSPH() )
            {
                hid_t fid = OpenFile();
                ReadSphBlockData( fid, ind, blkInd, dims, data );
                CloseFile(fid);
            }
            else if( !sph_msmat_.empty() )
            {
                GenMissData( sph_dsdims_[ind], sph_msmat_, dims, data );
            }
            return;
        }

        string ndvname = varname;
        if( ndvname.find( history_name ) != string::npos )
        {
            if( HasSPH() )
            {
                string st1,st2;
                DecomposeNames( ndvname, st1, st2, "_" );
                ind = STOI( st2 ) - 1;
                hid_t fid = OpenFile();
                ReadBlockHistoryData( fid, sph_name.c_str(),
                                      blkInd, ind,
                                      sph_part_,  sph_hvpart_,
                                      sph_hvdisp_[blkInd], dims, data );
                CloseFile(fid);
            }
            else if( !sph_msmat_.empty() )
            {
                vector<int> vdim;
                vdim.assign(1,0);
                GenMissData( vdim, sph_msmat_, dims, data );
            }
            return;
        }

        throw std::runtime_error("Not existing sph variable is requested in PVLD_Reader::ReadSphBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSphBlockData()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSphMaterial(bool allowCollective)
{
    try
    {
        hid_t fid = OpenFile();
        CollectMaterial( fid,
                         sph_name.c_str(),
                         sph_dsname_,
                         nsph_,
                         sph_matid_,
                         allowCollective);
        CloseFile(fid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadSphMaterial()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadSphBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    if( std::find( sph_dsname_.begin(),
                   sph_dsname_.end(),
                   "Material" ) != sph_dsname_.end() )
    {
        if( HasSPH() )
            ReadSphBlockData( "Material", blkInd, dims, data );
        else if( !sph_msmat_.empty() )
            GenMissMaterial( sph_msmat_, dims, data );
    }
    else
    {
        dims.resize(1);
        dims[0] = sph_part_.at(blkInd+1) - sph_part_.at(blkInd);
        data.assign( dims[0], nmmat_+1 );
    }
}




void PVLD_Reader::
ReadTiedSetBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        size_t nblks = GetNumOfTiedSetBlocks();
        if( (size_t)nb >= nblks ) return;

        if( tdst_blkmap_.size() != nblks )
            tdst_blkmap_.resize( nblks );

        vector<int>& vmap = tdst_blkmap_[nb];

        hid_t fid = OpenFile();
        ReadNodeIndexCoord( fid );
        ReadTiedSetBlockMesh( fid, nb, vmap, vcrd, elmt );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadTiedSetBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadTiedSetSlaveBlockMesh( int nb, vector<float>& crd )
{
    try
    {
        int nblks = GetNumOfTiedSetBlocks();
        if( nb >= nblks ) return;

        hid_t fid = OpenFile();
        ReadTiedSetSlaveBlockMesh( fid, nb, crd );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadTiedSetSlaveBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadTiedSetMasterBlockMesh( int nb, vector<float>& crd )
{
    try
    {
        int nblks = GetNumOfTiedSetBlocks();
        if( nb >= nblks ) return;

        hid_t fid = OpenFile();
        ReadTiedSetMasterBlockMesh( fid, nb, crd );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadTiedSetMasterBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}



bool PVLD_Reader::
ReadTiedSetBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        int nblks = GetNumOfTiedSetBlocks();
        if( blkInd >= nblks ) return true;

        size_t ind;
        for( ind=0; ind<tdst_dsname_.size(); ind++ )
            if( tdst_dsname_[ind] == varname ) break;
        if( ind<tdst_dsname_.size() )
        {
            hid_t fid = OpenFile();
            ReadTiedSetBlockData( fid, ind, blkInd, dims, data );
            CloseFile(fid);
            return true;
        }

        throw std::runtime_error("Not existing TiedNodeSet variable is requested in PVLD_Reader::ReadTiedSetBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadTiedSetBlockData()\n";
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
ReadContactBlockMesh( int nb, vector<float>& vcrd, vector<int>& elmt )
{
    try
    {
        size_t nblks = GetNumOfContactBlocks();
        if( (size_t)nb >= nblks ) return;

        if( cntt_blkmap_.size() != nblks )
            cntt_blkmap_.resize( nblks );

        vector<int>& vmap = cntt_blkmap_[nb];

        hid_t fid = OpenFile();
        ReadNodeIndexCoord( fid );
        ReadContactBlockMesh( fid, nb, vmap, vcrd, elmt );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadContactBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadContactSlaveBlockMesh( int nb, vector<float>& crd )
{
    try
    {
        int nblks = GetNumOfContactBlocks();
        if( nb >= nblks ) return;

        hid_t fid = OpenFile();
        ReadContactSlaveBlockMesh( fid, nb, crd );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadContactSlaveBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadContactMasterBlockMesh( int nb, vector<float>& crd )
{
    try
    {
        int nblks = GetNumOfContactBlocks();
        if( nb >= nblks ) return;

        hid_t fid = OpenFile();
        ReadContactMasterBlockMesh( fid, nb, crd );
        CloseFile( fid );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadContactMasterBlockMesh()\n";
        throw std::runtime_error(msg);
    }
}


bool PVLD_Reader::
ReadContactBlockData( const char* varname, int blkInd, vector<int>& dims, vector<float>& data )
{
    try
    {
        int nblks = GetNumOfContactBlocks();
        if( blkInd >= nblks ) return true;

        size_t ind;
        for( ind=0; ind<cntt_dsname_.size(); ind++ )
            if( cntt_dsname_[ind] == varname ) break;
        if( ind<cntt_dsname_.size() )
        {
            hid_t fid = OpenFile();
            ReadContactBlockData( fid, ind, blkInd, dims, data );
            CloseFile(fid);
            return true;
        }

        throw std::runtime_error("Not existing solid variable is requested in PVLD_Reader::ReadContactBlockData()\n");
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadContactBlockData()\n";
        throw std::runtime_error(msg);
    }
}

bool ReadStringAttribute(hid_t gid, const char *name, string &retval)
{
    retval = "";

    hid_t aid = H5Aopen_name( gid, name );
    if( aid<0 )
    {
        return false;
    }

    int sdim=2;

    hsize_t dims[10];
    hid_t space= H5Aget_space(aid);
    H5Sget_simple_extent_dims(space,dims,NULL);
    H5Sclose(space);

    hid_t memtype = H5Tcopy(H5T_C_S1);
    H5Tset_size(memtype,sdim);

    char** rd = new char*[ dims[0] ];
    rd[0] = new char[ sdim*dims[0] ];
    for( size_t j=1; j<dims[0]; j++ )
        rd[j] = rd[j-1]+sdim;

    herr_t herr = H5Aread( aid, memtype, rd[0] );
    H5Tclose(memtype);
    H5Aclose(aid);

    if( herr<0 )
    {
        delete [] rd[0];
        delete [] rd;
        return false;
    }
    else
    {
        string tmp = rd[0];
        for( size_t j=1; j<dims[0]; j++ )
            tmp += rd[j];
        retval = tmp;
        delete [] rd[0];
        delete [] rd;
    }

    return true;
}

void PVLD_Reader::
ReadGeneralInfo( hid_t file_id )
{
    herr_t herr; (void) herr;
    try
    {
        hid_t gid = OpenGroup( file_id, general_name.c_str() );
        ReadAttribute( gid, "SimuTime",      H5T_NATIVE_FLOAT,  &time_   );
        ReadAttribute( gid, "Ncycles",       H5T_NATIVE_INT,    &ncycle_ );
        ReadAttribute( gid, "NumMaterials",  H5T_NATIVE_INT,    &nmmat_  );
        GetMaterialTitles( gid );
#if 1
        string tmp;
        if(ReadStringAttribute(gid, "Title", tmp))
            title_ = tmp;
        if(ReadStringAttribute(gid, "Subtitle1", tmp))
            title_ = title_ + string("\n") + tmp;
        if(ReadStringAttribute(gid, "Subtitle2", tmp))
            title_ = title_ + string("\n") + tmp;
#endif
        CloseGroup(gid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadGeneralInfo()\n";
        throw std::runtime_error(msg);
    }
    MakeUniqueMatNames();
}



void PVLD_Reader::
ReadNodeInfo( hid_t file_id )
{
    herr_t herr; (void) herr;

    try
    {
        node_dsname_.clear();
        node_dsdims_.clear();

        hid_t gid = OpenGroup( file_id, node_name.c_str() );
        ReadAttribute( gid, "number", H5T_NATIVE_INT,  &nnode_  );
        if( nnode_!=0 )
        {
            CollectGroupDataSets( gid, node_dsname_, node_dsdims_ );
        }
        CloseGroup(gid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadNodeInfo()\n";
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
ReadSolidInfo( hid_t file_id )
{
    ReadInfo( file_id, solid_name.c_str(),
              nsolid_, solid_part_,
              solid_dsname_, solid_dsdims_ );

    OutputVectorInt( DebugStream::Stream1(), "*** solid_part_", solid_part_);

    solid_lhv_ = ( std::find( solid_dsname_.begin(), solid_dsname_.end(),
                              number_of_history_name ) != solid_dsname_.end() &&
                   std::find( solid_dsname_.begin(), solid_dsname_.end(),
                              history_name ) != solid_dsname_.end() );
}



void PVLD_Reader::
ReadShellInfo( hid_t file_id )
{
    ReadInfo( file_id, shell_name.c_str(),
              nshell_, shell_part_,
              shell_dsname_, shell_dsdims_ );
    shell_lhv_ = ( std::find( shell_dsname_.begin(), shell_dsname_.end(),
                              number_of_history_name ) != shell_dsname_.end() &&
                   std::find( shell_dsname_.begin(), shell_dsname_.end(),
                              history_name ) != shell_dsname_.end() );
}



void PVLD_Reader::
ReadBeamInfo( hid_t file_id )
{
    ReadInfo( file_id, beam_name.c_str(),
              nbeam_, beam_part_,
              beam_dsname_, beam_dsdims_ );
    beam_lhv_ = ( std::find( beam_dsname_.begin(), beam_dsname_.end(),
                             number_of_history_name ) != beam_dsname_.end() &&
                  std::find( beam_dsname_.begin(), beam_dsname_.end(),
                             history_name ) != beam_dsname_.end() );
}



void PVLD_Reader::
ReadSurfaceInfo( hid_t file_id )
{
    ReadInfo( file_id, surface_name.c_str(),
              nsurf_, surf_part_,
              surf_dsname_, surf_dsdims_ );
}





void PVLD_Reader::
ReadSPHInfo( hid_t file_id )
{
    ReadInfo( file_id, sph_name.c_str(),
              nsph_, sph_part_,
              sph_dsname_, sph_dsdims_ );
    sph_lhv_ = ( std::find( sph_dsname_.begin(), sph_dsname_.end(),
                            number_of_history_name ) != sph_dsname_.end() &&
                 std::find( sph_dsname_.begin(), sph_dsname_.end(),
                            history_name ) != sph_dsname_.end() );
}


void PVLD_Reader::
ReadTiedsetInfo( hid_t file_id )
{
    ReadInfo( file_id, tiedset_name.c_str(),
              ntdst_, tdst_part_,
              tdst_dsname_, tdst_dsdims_,
              "NumNodes" );
}


void PVLD_Reader::
ReadContactInfo( hid_t file_id )
{
    ReadInfo( file_id, contact_name.c_str(),
              ncntt_, cntt_part_,
              cntt_dsname_, cntt_dsdims_,
              "NumNodes" );
}

void PVLD_Reader::
GetMaterialTitles( hid_t gid )
{
    mat_titles_.clear();

    for( int i=0; i<nmmat_; i++ )
    {
        char name[200];
        sprintf(name,"PartTitle_%d",i+1);

        hid_t aid = H5Aopen_name( gid, name );
        if( aid<0 )
        {
            mat_titles_.push_back( name );
            continue;
        }

        int sdim=2;

        hsize_t dims[10];
        hid_t space= H5Aget_space(aid);
        H5Sget_simple_extent_dims(space,dims,NULL);
        H5Sclose(space);

        hid_t memtype = H5Tcopy(H5T_C_S1);
        H5Tset_size(memtype,sdim);

        char** rd = new char*[ dims[0] ];
        rd[0] = new char[ sdim*dims[0] ];
        for( size_t j=1; j<dims[0]; j++ )
            rd[j] = rd[j-1]+sdim;

        herr_t herr = H5Aread( aid, memtype, rd[0] );
        H5Tclose(memtype);
        H5Aclose(aid);

        if( herr<0 )
        {
            mat_titles_.push_back( name );
        }
        else
        {
            string tmp = rd[0];
            for( size_t j=1; j<dims[0]; j++ )
                tmp += rd[j];
            mat_titles_.push_back( tmp );
        }

        delete [] rd[0];
        delete [] rd;
    }
    mat_titles_.push_back( "Unknown" );
}

void PVLD_Reader::
MakeUniqueMatNames()
{
    char buf[1000];
    int *ind = new int[nmmat_];
    for( int i=0; i<nmmat_; i++ )
        ind[i]=0;

    std::map<string,int> ss;
    ss[ mat_titles_[0] ] = 0;

    for( int i=1; i<nmmat_; i++ )
    {
        std::map<string,int>::iterator j = ss.find( mat_titles_[i] );
        if( j == ss.end() )
        {
            ss[ mat_titles_[i] ] = i;
        }
        else
        {
            int k = j->second;
            sprintf( buf, "@%-d", ++(ind[k]) );
            mat_titles_[i] += buf;
        }
    }

    delete [] ind;
}


void PVLD_Reader::
GetMaterialType( hid_t file_id, const char* grpname )
{
    try
    {
        hid_t gid = OpenGroup( file_id, grpname );

        vector<int> type;
        if( H5Aexists( gid, "PartElementType" )>0 )
        {
            hid_t aid = OpenAttribute( gid, "PartElementType" );
            int ndim=1;
            hsize_t dims[1];
            GetAttributeSize( aid, ndim, dims );
            type.resize( dims[0] );
            ReadAttribute( aid, H5T_NATIVE_INT, &type[0] );
            CloseAttribute( aid );

            // verify
            if( type.size() != (size_t)nmmat_ )
                throw std::runtime_error("Unmatched number of parts in PVLD_Reader::GetMaterialType()\n");
            for( vector<int>::iterator i=solid_matid_.begin(); i!=solid_matid_.end(); ++i )
                if( type.at( *i-1 ) != solid_elmt_type )
                    throw std::runtime_error( "Unmatched solid element type is found in PVLD_Reader::GetMaterialType()\n" );
            for( vector<int>::iterator i=beam_matid_.begin(); i!=beam_matid_.end(); ++i )
                if( type.at( *i-1 ) != beam_elmt_type )
                    throw std::runtime_error( "Unmatched beam element type is found in PVLD_Reader::GetMaterialType()\n" );
            for( vector<int>::iterator i=shell_matid_.begin(); i!=shell_matid_.end(); ++i )
                if( type.at( *i-1 ) != shell_elmt_type )
                    throw std::runtime_error( "Unmatched shell element type is found in PVLD_Reader::GetMaterialType()\n" );
            for( vector<int>::iterator i=sph_matid_.begin(); i!=sph_matid_.end(); ++i )
                if( type.at( *i-1 ) != sph_elmt_type &&
                        type.at( *i-1 ) != solid_elmt_type )
                    //type.at( *i-1 ) != shell_elmt_type &&  type.at( *i-1 ) != beam_elmt_type )
                    throw std::runtime_error( "Unmatched sph element type is found in PVLD_Reader::GetMaterialType()\n" );
        }
        else
        {
            type.assign( nmmat_, solid_elmt_type ); // assuming missing parts are solid
            for( vector<int>::iterator i=beam_matid_.begin(); i!=beam_matid_.end(); ++i )
                type.at( *i-1 ) = beam_elmt_type;
            for( vector<int>::iterator i=shell_matid_.begin(); i!=shell_matid_.end(); ++i )
                type.at( *i-1 ) = shell_elmt_type;

        }

        CloseGroup(gid);

        //mat_type_.swap( type );
        mat_type_.assign(nmmat_,0);
        for( int i=0; i!=nmmat_; i++ )
        {
            mat_type_[i] |= 1<<type[i];
            if( type[i] == solid_elmt_type )
                mat_type_[i] |= (1<<sph_elmt_type);
        }
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::GetMaterialType( ";
        msg += grpname;
        msg += ")\n";
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
GenMissingMaterials()
{
    try
    {
        vector<int> tmp;
        tmp.assign(nmmat_,0);

        for( vector<int>::iterator i=solid_matid_.begin(); i!=solid_matid_.end(); ++i )
            tmp.at( *i-1 ) |= 1<<solid_elmt_type;
        for( vector<int>::iterator i=beam_matid_.begin(); i!=beam_matid_.end(); ++i )
            tmp.at( *i-1 ) |= 1<<beam_elmt_type;
        for( vector<int>::iterator i=shell_matid_.begin(); i!=shell_matid_.end(); ++i )
            tmp.at( *i-1 ) |= 1<<shell_elmt_type;
        for( vector<int>::iterator i=sph_matid_.begin(); i!=sph_matid_.end(); ++i )
            tmp.at( *i-1 ) |= 1<<sph_elmt_type;

        AddMissingParts( solid_elmt_type, mat_type_, tmp, solid_msmat_ );
        AddMissingParts( shell_elmt_type, mat_type_, tmp, shell_msmat_ );
        AddMissingParts( beam_elmt_type,  mat_type_, tmp, beam_msmat_ );
        AddMissingParts( sph_elmt_type,   mat_type_, tmp, sph_msmat_ );

        OutputVectorInt( DebugStream::Stream1(), "solid_msmat_ = ", solid_msmat_ );
        // OutputVectorInt( DebugStream::Stream1(), "shell_msmat_ = ", shell_msmat_ );
        // OutputVectorInt( DebugStream::Stream1(), "beam_msmat_ = ",  beam_msmat_ );
        // OutputVectorInt( DebugStream::Stream1(), "sph_msmat_ = ",   sph_msmat_ );
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::GenMissingMaterials()\n";
        throw std::runtime_error(msg);
    }
}

void PVLD_Reader::
AddMissingParts( int type, const vector<int>& tgt, const vector<int>& own, vector<int>& msmat )
{
    for( int i=0; i<nmmat_; i++ )
        if( tgt[i] & (1<<type) )
        {
            if( !( own[i] & (1<<type) ) )
                msmat.push_back(i+1);
        }
}



void PVLD_Reader::
ReadInfo( hid_t file_id, const char* name,
          int&                 num,
          vector<int>&         part,
          vector<string>&      dsname,
          vector<vector<int> >& dsdims,
          const char* num_str )
{
    herr_t herr; (void) herr;

    try
    {
        dsname.clear();
        dsdims.clear();

        hid_t gid = OpenGroup( file_id, name );
        if( num_str==NULL )
            ReadAttribute( gid, "number", H5T_NATIVE_INT,  &num  );
        else
            ReadAttribute( gid, num_str, H5T_NATIVE_INT,  &num  );

        int npart;
        if( num!=0 )
        {
            char* spt = getenv("PARALLEL_VELODYNE");
            if( spt==NULL )
            {
                npart = default_number_of_partitions;
                part.resize( npart+1 );
                EquallyPartition( npart, num, &part[0] );
            }
            else
            {
                npart = atoi(spt);
                if( npart>0 )
                {
                    part.resize( npart+1 );
                    EquallyPartition( npart, num, &part[0] );
                }
                else
                {
                    if( H5Aexists( gid, "partition" )>0 )
                    {
                        hid_t aid = OpenAttribute( gid, "partition" );
                        int ndim=1;
                        hsize_t dims[1];
                        GetAttributeSize( aid, ndim, dims );
                        part.resize( dims[0] );
                        ReadAttribute( aid, H5T_NATIVE_INT, &part[0] );
                        CloseAttribute( aid );
                        npart = dims[0]-1;
                    }
                    else
                    {
                        // one block
                        npart=1;
                        part.resize( npart+1 );
                        part[0] = 0;
                        part[1] = num;
                    }
                }
            }
            int np = 0;
            for( int i=1; i<npart+1; i++ )
            {
                if( part[np] != part[i] )
                    part[++np] = part[i];
            }
            part.resize( np+1 );

            CollectGroupDataSets( gid, dsname, dsdims );
        }
        CloseGroup(gid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadInfo( ";
        msg += name;
        msg += ")\n";
        throw std::runtime_error(msg);
    }
}




void PVLD_Reader::
ReadNodeIndexCoord( hid_t fid )
{
    //if( node_idx_.size() == nnode_ ) return;
    if( node_idx_.size() != 0 ) return;

    if( std::find( node_dsname_.begin(),
                   node_dsname_.end(), "Index" )
            == node_dsname_.end() )
        throw std::runtime_error( "No node Index definition is found in PVLD_Reader::ReadNodeIndexCoord()\n" );
    if( std::find( node_dsname_.begin(),
                   node_dsname_.end(), "Coordinate" )
            == node_dsname_.end() )
        throw std::runtime_error( "No node Coordinate definition is found in PVLD_Reader::ReadNodeIndexCoord()\n" );

    node_idx_.resize( nnode_ );
    node_crd_.resize( 3*nnode_ );
#ifdef PARALLEL
    {
        vector<double> dbl( 3*nnode_ );
        if( PAR_Rank()==0 )
        {
            ReadGroupDataSet( fid, node_name.c_str(), "Index",
                              H5T_NATIVE_INT, &node_idx_[0] );
            ReadGroupDataSet( fid, node_name.c_str(), "Coordinate",
                              H5T_NATIVE_DOUBLE, &dbl[0] );
        }
        //MPI_Bcast( &node_idx_[0],   nnode_, MPI_INT,   0, VISIT_MPI_COMM );
        //MPI_Bcast( &node_crd_[0], 3*nnode_, MPI_FLOAT, 0, VISIT_MPI_COMM );
        BroadcastIntArray( &node_idx_[0],   nnode_ );
        BroadcastDoubleArray( &dbl[0], 3*nnode_ );
        for( int i=0; i<3*nnode_; i++ )
            node_crd_[i] = dbl[i];
    }
    // {
    //   int myrank = PAR_Rank();
    //   int nprocs = PAR_Size();

    //   vector<int> psft(nprocs+1);
    //   EquallyPartition( nprocs, nnode_, &psft[0] );

    //   vector<int> pcnt(nprocs);
    //   for( int r=0; r<nprocs; r++ )
    //     pcnt[r] = psft[r+1] - psft[r];

    //   hsize_t sft[2],len[2];
    //   sft[1]=0;  sft[0] = psft[myrank];
    //   len[1]=3;  len[0] = pcnt[myrank];

    //   vector<int> ibuf( len[0] );
    //   ReadGroupDataSet( fid, node_name.c_str(), "Index",
    //               H5T_NATIVE_INT, &ibuf[0], 1, sft, len );

    //   vector<float> dbuf( 3*len[0] );
    //   ReadGroupDataSet( fid, node_name.c_str(), "Coordinate",
    //               H5T_NATIVE_FLOAT, &dbuf[0], 2, sft, len );

    //   MPI_Allgatherv( &ibuf[0], pcnt[myrank], MPI_INT,
    //             &node_idx_[0], &pcnt[0], &psft[0], MPI_INT,
    //             VISIT_MPI_COMM );

    //   for( auto& c : pcnt ) c*=3;
    //   for( auto& s : psft ) s*=3;
    //   MPI_Allgatherv( &dbuf[0], pcnt[myrank], MPI_DOUBLE,
    //             &node_crd_[0], &pcnt[0], &psft[0], MPI_FLOAT,
    //             VISIT_MPI_COMM );
    // }
#else
    {
        ReadGroupDataSet( fid, node_name.c_str(), "Index",
                          H5T_NATIVE_INT, &node_idx_[0] );
        ReadGroupDataSet( fid, node_name.c_str(), "Coordinate",
                          H5T_NATIVE_FLOAT, &node_crd_[0] );
    }
#endif

    // if( node_map_.size() != nnode_ )
    //   throw std::runtime_error( "Dupilicated node index is found in PVLD_Reader::ReadNodeIndexCoord()\n" );
    node_idx_mn_ = node_idx_mx_ = node_idx_[0];
    for( int i=1; i<nnode_; i++ )
    {
        node_idx_mn_ = node_idx_mn_>node_idx_[i] ? node_idx_[i] : node_idx_mn_;
        node_idx_mx_ = node_idx_mx_<node_idx_[i] ? node_idx_[i] : node_idx_mx_;
    }

    node_map_.assign( node_idx_mx_-node_idx_mn_+1, -1 );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = node_idx_[i];
        node_map_[ind-node_idx_mn_] = i;
    }
}


void PVLD_Reader::
ReadNodeData( hid_t fid, int varInd, const vector<int>& map,
              vector<int>& dims, vector<float>& dat )
{
    if( map.size()==0 ) return;

    const vector<int>& dd = node_dsdims_[varInd];
    int ndim = dd.size();
    int nele = 1;
    for( int i=1; i<ndim; i++ ) nele*=dd[i];
    int tot = nele*dd[0];

    vector<float> buf( tot );
    ReadGroupDataSet( fid, node_name.c_str(), node_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &buf[0] );
    dims = dd;
    dims[0] = map.size();

    dat.resize( nele*map.size() );
    for( size_t i=0; i<map.size(); i++ )
    {
        int sf1 = nele*i;
        int sf2 = nele*map[i];
        for( int j=0; j<nele; j++ )
            dat[sf1+j] = buf[sf2+j];
    }
}







void PVLD_Reader::
ReadSolidBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( solid_dsname_.begin(),
                   solid_dsname_.end(),
                   "Nodes" ) == solid_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadSolidBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = solid_part_.at(nb);
    len[1] = 8;
    len[0] = solid_part_.at(nb+1) - solid_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    elmt.resize( nd );
    ReadGroupDataSet( fid, solid_name.c_str(), "Nodes",
                      H5T_NATIVE_INT, &elmt[0], 2, sft, len );

    vector<int> loc( nnode_ );
    std::fill( loc.begin(), loc.end(), -1 );

    int tnp=0;
    for( vector<int>::iterator ie=elmt.begin(); ie!=elmt.end(); ie++ )
    {
        int& idx = *ie;
        int ind = node_map_[idx-node_idx_mn_];
        if( loc[ind]<0 ) loc[ind]=tnp++;
        idx = loc[ind];
    }

    vmap.resize(   tnp );
    vcrd.resize( 3*tnp );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = loc[i];
        if( ind<0 ) continue;
        vmap[  ind  ] = i;
        vcrd[3*ind  ] = node_crd_[3*i  ];
        vcrd[3*ind+1] = node_crd_[3*i+1];
        vcrd[3*ind+2] = node_crd_[3*i+2];
    }
}



void PVLD_Reader::
ReadSolidBlockData( hid_t fid, int varInd, int blkInd,
                    vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = solid_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = solid_part_.at(blkInd);
    len[0] = solid_part_.at(blkInd+1) - solid_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, solid_name.c_str(), solid_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}


void PVLD_Reader::
ReadSolidBlockHistoryData( hid_t fid, int blkInd, int varInd,
                           vector<int>& dims, vector<float>& data )
{
    ReadBlockHistoryData( fid, solid_name.c_str(),
                          blkInd, varInd,
                          solid_part_,  solid_hvpart_,
                          solid_hvdisp_[blkInd], dims, data );
}







void PVLD_Reader::
ReadShellBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( shell_dsname_.begin(),
                   shell_dsname_.end(),
                   "Nodes" ) == shell_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadShellBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = shell_part_.at(nb);
    len[1] = 4;
    len[0] = shell_part_.at(nb+1) - shell_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    elmt.resize( nd );
    ReadGroupDataSet( fid, shell_name.c_str(), "Nodes",
                      H5T_NATIVE_INT, &elmt[0], 2, sft, len );

    vector<int> loc( nnode_ );
    std::fill( loc.begin(), loc.end(), -1 );

    int tnp=0;
    for( vector<int>::iterator ie=elmt.begin(); ie!=elmt.end(); ie++ )
    {
        int& idx = *ie;
        int ind = node_map_[idx-node_idx_mn_];
        if( loc[ind]<0 ) loc[ind]=tnp++;
        idx = loc[ind];
    }

    vmap.resize(   tnp );
    vcrd.resize( 3*tnp );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = loc[i];
        if( ind<0 ) continue;
        vmap[  ind  ] = i;
        vcrd[3*ind  ] = node_crd_[3*i  ];
        vcrd[3*ind+1] = node_crd_[3*i+1];
        vcrd[3*ind+2] = node_crd_[3*i+2];
    }
}



void PVLD_Reader::
ReadShellBlockData( hid_t fid, int varInd, int blkInd,
                    vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = shell_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = shell_part_.at(blkInd);
    len[0] = shell_part_.at(blkInd+1) - shell_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, shell_name.c_str(), shell_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}



void PVLD_Reader::
ReadShellBlockHistoryData( hid_t fid, int blkInd, int varInd,
                           vector<int>& dims, vector<float>& data )
{
    ReadBlockHistoryData( fid, shell_name.c_str(),
                          blkInd, varInd,
                          shell_part_,  shell_hvpart_,
                          shell_hvdisp_[blkInd], dims, data );
}



void PVLD_Reader::
ReadBeamBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( beam_dsname_.begin(),
                   beam_dsname_.end(),
                   "Nodes" ) == beam_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadBeamBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = beam_part_.at(nb);
    len[1] = 2;
    len[0] = beam_part_.at(nb+1) - beam_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    elmt.resize( nd );
    ReadGroupDataSet( fid, beam_name.c_str(), "Nodes",
                      H5T_NATIVE_INT, &elmt[0], 2, sft, len );

    vector<int> loc( nnode_ );
    std::fill( loc.begin(), loc.end(), -1 );

    int tnp=0;
    for( vector<int>::iterator ie=elmt.begin(); ie!=elmt.end(); ie++ )
    {
        int& idx = *ie;
        if( idx<node_idx_mn_ || idx>node_idx_mx_ )
            throw std::runtime_error( "Unacceptable node index is found in PVLD_Reader::ReadBeamBlockMesh" );
        int ind = node_map_[idx-node_idx_mn_];
        if( loc[ind]<0 ) loc[ind]=tnp++;
        idx = loc[ind];
    }

    vmap.resize(   tnp );
    vcrd.resize( 3*tnp );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = loc[i];
        if( ind<0 ) continue;
        vmap[  ind  ] = i;
        vcrd[3*ind  ] = node_crd_[3*i  ];
        vcrd[3*ind+1] = node_crd_[3*i+1];
        vcrd[3*ind+2] = node_crd_[3*i+2];
    }
}



void PVLD_Reader::
ReadBeamBlockData( hid_t fid, int varInd, int blkInd,
                   vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = beam_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = beam_part_.at(blkInd);
    len[0] = beam_part_.at(blkInd+1) - beam_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, beam_name.c_str(), beam_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}



void PVLD_Reader::
ReadBeamBlockHistoryData( hid_t fid, int blkInd, int varInd,
                          vector<int>& dims, vector<float>& data )
{
    ReadBlockHistoryData( fid, beam_name.c_str(),
                          blkInd, varInd,
                          beam_part_,  beam_hvpart_,
                          beam_hvdisp_[blkInd], dims, data );
}



void PVLD_Reader::
ReadSurfaceBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( surf_dsname_.begin(),
                   surf_dsname_.end(),
                   "Nodes" ) == surf_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadSurfaceBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = surf_part_.at(nb);
    len[1] = 4;
    len[0] = surf_part_.at(nb+1) - surf_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    elmt.resize( nd );
    ReadGroupDataSet( fid, surface_name.c_str(), "Nodes",
                      H5T_NATIVE_INT, &elmt[0], 2, sft, len );

    vector<int> loc( nnode_ );
    std::fill( loc.begin(), loc.end(), -1 );

    int tnp=0;
    for( vector<int>::iterator ie=elmt.begin(); ie!=elmt.end(); ie++ )
    {
        int& idx = *ie;
        int ind = node_map_[idx-node_idx_mn_];
        if( loc[ind]<0 ) loc[ind]=tnp++;
        idx = loc[ind];
    }

    vmap.resize(   tnp );
    vcrd.resize( 3*tnp );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = loc[i];
        if( ind<0 ) continue;
        vmap[  ind  ] = i;
        vcrd[3*ind  ] = node_crd_[3*i  ];
        vcrd[3*ind+1] = node_crd_[3*i+1];
        vcrd[3*ind+2] = node_crd_[3*i+2];
    }
}



void PVLD_Reader::
ReadSurfaceBlockData( hid_t fid, int varInd, int blkInd,
                      vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = surf_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = surf_part_.at(blkInd);
    len[0] = surf_part_.at(blkInd+1) - surf_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, surface_name.c_str(), surf_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}





void PVLD_Reader::
ReadSphBlockMesh( hid_t fid, int nb, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( sph_dsname_.begin(),
                   sph_dsname_.end(),
                   "Coordinate" ) == sph_dsname_.end() )
        throw std::runtime_error( "No coordinate definition is found in PVLD_Reader::ReadSphBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1]=0;
    sft[0] = sph_part_.at(nb);
    len[1]=3;
    len[0] = sph_part_.at(nb+1) - sph_part_.at(nb);

    elmt.resize( len[0] );
    for( size_t i=0; i<len[0]; i++ )
        elmt[i] = i;

    vcrd.resize( len[0]*len[1] );
    ReadGroupDataSet( fid, sph_name.c_str(), "Coordinate",
                      H5T_NATIVE_FLOAT, &vcrd[0], 2, sft, len );
}


void PVLD_Reader::
ReadSphBlockData( hid_t fid, int varInd, int blkInd,
                  vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = sph_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = sph_part_.at(blkInd);
    len[0] = sph_part_.at(blkInd+1) - sph_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, sph_name.c_str(), sph_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}



void PVLD_Reader::
ReadHistoryDataInfo( hid_t fid, const char* meshname, const vector<int>& part, int& mxnb, vector<int>& hvpart )
{
    int np = part.size()-1;
    int ne = part[np];

    vector<int> cnt(ne);
#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        ReadGroupDataSet( fid, meshname, number_of_history_name.c_str(),
                          H5T_NATIVE_INT, &cnt[0] );
    }
#ifdef PARALLEL
    {
        //MPI_Bcast( &cnt[0], ne, MPI_INT, 0, VISIT_MPI_COMM );
        BroadcastIntArray( &cnt[0], ne );
    }
#endif

    hvpart.resize( np+1 );
    int ss=0;
    int mx=0;
    hvpart[0] = 0;
    for( int p=0; p<np; p++ )
    {
        for( int i=part[p]; i<part[p+1]; i++ )
        {
            int cc = cnt[i];
            ss += cc;
            mx = mx<cc ? cc : mx;
        }
        hvpart[p+1] = ss;
    }

    mxnb = mx;
}




void PVLD_Reader::
ReadBlockHistoryData( hid_t fid, const char* meshname, int blkInd, int varInd,
                      const vector<int>& part,  const vector<int>& hvpart,
                      vector<int>& hvsft, vector<int>& dims, vector<float>& data )
{
    hsize_t sft = part.at(blkInd);
    hsize_t len = part.at(blkInd+1) - sft;
    if( hvsft.size() != len+1 )
    {
        hvsft.resize( len+1 );
        int* d1 = &hvsft[0]+1;
        ReadGroupDataSet( fid, meshname, number_of_history_name.c_str(),
                          H5T_NATIVE_INT, d1, 1, &sft, &len );
        hvsft[0]=0;
        for( hsize_t i=0; i<len; i++ )
            hvsft[i+1] = hvsft[i+1] + hvsft[i];
    }

    hsize_t shv = hvpart.at(blkInd);
    hsize_t nhv = hvpart.at(blkInd+1) - shv;
    if( (hsize_t)hvsft[len] != nhv )
        throw std::runtime_error( "Unmatched number of history variables in PVLD_Reader::ReadBlockHistoryData()\n" );

    vector<float> buf( nhv );
    ReadGroupDataSet( fid, meshname, history_name.c_str(),
                      H5T_NATIVE_FLOAT, &buf[0], 1, &shv, &nhv );

    data.resize(len);
    std::fill( data.begin(), data.end(), 0. );

    for( hsize_t i=0; i<len; i++ )
    {
        hsize_t j = hvsft[i] + varInd;
        if( j < (hsize_t)hvsft[i+1] )
            data[i] = buf[j];
    }

    dims.resize(1);
    dims[0] = len;
}




void PVLD_Reader::
ReadTiedSetBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( tdst_dsname_.begin(),
                   tdst_dsname_.end(),
                   "MstSeg" ) == tdst_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadTiedSetBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = tdst_part_.at(nb);
    len[1] = 4;
    len[0] = tdst_part_.at(nb+1) - tdst_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    elmt.resize( nd );
    ReadGroupDataSet( fid, tiedset_name.c_str(), "MstSeg",
                      H5T_NATIVE_INT, &elmt[0], 2, sft, len );

    vector<int> loc( nnode_ );
    std::fill( loc.begin(), loc.end(), -1 );

    int tnp=0;
    for( vector<int>::iterator ie=elmt.begin(); ie!=elmt.end(); ie++ )
    {
        int& idx = *ie;
        int ind = node_map_[idx-node_idx_mn_];
        if( loc[ind]<0 ) loc[ind]=tnp++;
        idx = loc[ind];
    }

    vmap.resize(   tnp );
    vcrd.resize( 3*tnp );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = loc[i];
        if( ind<0 ) continue;
        vmap[  ind  ] = i;
        vcrd[3*ind  ] = node_crd_[3*i  ];
        vcrd[3*ind+1] = node_crd_[3*i+1];
        vcrd[3*ind+2] = node_crd_[3*i+2];
    }
}



void PVLD_Reader::
ReadTiedSetSlaveBlockMesh( hid_t fid, int nb, vector<float>& crd )
{
    if( std::find( tdst_dsname_.begin(),
                   tdst_dsname_.end(),
                   "XSlave" ) == tdst_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadTiedSetSlaveBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = tdst_part_.at(nb);
    len[1] = 3;
    len[0] = tdst_part_.at(nb+1) - tdst_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    crd.resize( nd );
    ReadGroupDataSet( fid, tiedset_name.c_str(), "XSlave",
                      H5T_NATIVE_FLOAT, &crd[0], 2, sft, len );
}



void PVLD_Reader::
ReadTiedSetMasterBlockMesh( hid_t fid, int nb, vector<float>& crd )
{
    if( std::find( tdst_dsname_.begin(),
                   tdst_dsname_.end(),
                   "XMaster" ) == tdst_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadTiedSetMasterBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = tdst_part_.at(nb);
    len[1] = 3;
    len[0] = tdst_part_.at(nb+1) - tdst_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    crd.resize( nd );
    ReadGroupDataSet( fid, tiedset_name.c_str(), "XMaster",
                      H5T_NATIVE_FLOAT, &crd[0], 2, sft, len );
}




void PVLD_Reader::
ReadTiedSetBlockData( hid_t fid, int varInd, int blkInd,
                      vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = tdst_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = tdst_part_.at(blkInd);
    len[0] = tdst_part_.at(blkInd+1) - tdst_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, tiedset_name.c_str(), tdst_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}





void PVLD_Reader::
ReadContactBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( std::find( cntt_dsname_.begin(),
                   cntt_dsname_.end(),
                   "MstSeg" ) == cntt_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadContactBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = cntt_part_.at(nb);
    len[1] = 4;
    len[0] = cntt_part_.at(nb+1) - cntt_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    elmt.resize( nd );
    ReadGroupDataSet( fid, contact_name.c_str(), "MstSeg",
                      H5T_NATIVE_INT, &elmt[0], 2, sft, len );

    vector<int> loc( nnode_ );
    std::fill( loc.begin(), loc.end(), -1 );

    int tnp=0;
    for( vector<int>::iterator ie=elmt.begin(); ie!=elmt.end(); ie++ )
    {
        int& idx = *ie;
        int ind = node_map_[idx-node_idx_mn_];
        if( loc[ind]<0 ) loc[ind]=tnp++;
        idx = loc[ind];
    }

    vmap.resize(   tnp );
    vcrd.resize( 3*tnp );
    for( int i=0; i<nnode_; i++ )
    {
        int ind = loc[i];
        if( ind<0 ) continue;
        vmap[  ind  ] = i;
        vcrd[3*ind  ] = node_crd_[3*i  ];
        vcrd[3*ind+1] = node_crd_[3*i+1];
        vcrd[3*ind+2] = node_crd_[3*i+2];
    }
}


void PVLD_Reader::
ReadContactSlaveBlockMesh( hid_t fid, int nb, vector<float>& crd )
{
    if( std::find( cntt_dsname_.begin(),
                   cntt_dsname_.end(),
                   "XSlave" ) == cntt_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadContactSlaveBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = cntt_part_.at(nb);
    len[1] = 3;
    len[0] = cntt_part_.at(nb+1) - cntt_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    crd.resize( nd );
    ReadGroupDataSet( fid, contact_name.c_str(), "XSlave",
                      H5T_NATIVE_FLOAT, &crd[0], 2, sft, len );
}


void PVLD_Reader::
ReadContactMasterBlockMesh( hid_t fid, int nb, vector<float>& crd )
{
    if( std::find( cntt_dsname_.begin(),
                   cntt_dsname_.end(),
                   "XMaster" ) == cntt_dsname_.end() )
        throw std::runtime_error( "No element definition is found in PVLD_Reader::ReadContactMasterBlockMesh()\n" );

    hsize_t sft[2],len[2];
    sft[1] = 0;
    sft[0] = cntt_part_.at(nb);
    len[1] = 3;
    len[0] = cntt_part_.at(nb+1) - cntt_part_.at(nb);

    hsize_t nd = len[0]*len[1];
    crd.resize( nd );
    ReadGroupDataSet( fid, contact_name.c_str(), "XMaster",
                      H5T_NATIVE_FLOAT, &crd[0], 2, sft, len );
}


void PVLD_Reader::
ReadContactBlockData( hid_t fid, int varInd, int blkInd,
                      vector<int>& dims, vector<float>& edat )
{
    const vector<int>& dd = tdst_dsdims_[varInd];
    int ndim = dd.size();

    vector<hsize_t> sft(ndim);
    vector<hsize_t> len(ndim);

    sft[0] = cntt_part_.at(blkInd);
    len[0] = cntt_part_.at(blkInd+1) - cntt_part_.at(blkInd);
    size_t tot = len[0];
    for( int i=1; i<ndim; i++ )
    {
        sft[i] = 0;
        len[i] = dd[i];
        tot *= len[i];
    }

    dims.resize(ndim);
    for( int i=0; i<ndim; i++ )
        dims[i]= len[i];

    edat.resize( tot );
    ReadGroupDataSet( fid, contact_name.c_str(), cntt_dsname_[varInd].c_str(),
                      H5T_NATIVE_FLOAT, &edat[0], ndim, &sft[0], &len[0] );
}






void PVLD_Reader::
AppendMissingMaterialMesh( int type, int nb, vector<float>& vcrd, vector<int>& elmt )
{
    if( nb!=0 ) return;

    int nnode;
    vector<int> msmat;
    switch( type )
    {
    case(solid_elmt_type):
        nnode=8;
        msmat=solid_msmat_;
        break;
    case( beam_elmt_type):
        nnode=2;
        msmat= beam_msmat_;
        break;
    case(shell_elmt_type):
        nnode=4;
        msmat=shell_msmat_;
        break;
    case(  sph_elmt_type):
        nnode=1;
        msmat=  sph_msmat_;
        break;
    default:
        throw std::runtime_error( "Unknow element type in PVLD_Reader::AppendMissingMaterialMesh()\n" );
    }
    if( msmat.size()==0 ) return;

    // const float alf=1.0e-3;
    // int np = vcrd.size()/3;
    // int ne = elmt.size()/8;
    // for( int i=0; i<3; i++ )
    //   vcrd.push_back( (1.0f-alf)*vcrd[i] + alf*vcrd[i+3] );
    vector<int> smp;
    if( elmt.empty() )
        smp.assign(nnode,0);
    else
    {
        for( int p=0; p<nnode; p++ )
            smp.push_back( elmt[p] );
    }

    for( vector<int>::const_iterator m=msmat.begin(); m!=msmat.end(); ++m )
        for( int p=0; p<nnode; p++ )
            elmt.push_back(smp[p]);
}


void PVLD_Reader::
AppendMissingMaterialData( int type, int nb, vector<int>& dims, vector<float>& data )
{
    if( nb!=0 ) return;

    vector<int> msmat;
    switch( type )
    {
    case(solid_elmt_type):
        msmat=solid_msmat_;
        break;
    case( beam_elmt_type):
        msmat= beam_msmat_;
        break;
    case(shell_elmt_type):
        msmat=shell_msmat_;
        break;
    case(  sph_elmt_type):
        msmat=  sph_msmat_;
        break;
    default:
        throw std::runtime_error( "Unknow element type in PVLD_Reader::AppendMissingMaterialMesh()\n" );
    }
    if( msmat.size()==0 ) return;

    int tot=1;
    for( size_t n=1; n<dims.size(); n++ )
        tot *= dims[n];

    vector<float> smp;
    if( data.empty() )
        smp.assign(tot,0);
    else
        for( int i=0; i<tot; i++ )
            smp.push_back( data[i] );

    for( vector<int>::const_iterator m=msmat.begin(); m!=msmat.end(); ++m )
    {
        for( int i=0; i<tot; i++ )
            data.push_back( smp[i] );
        dims[0]++;
    }
}



void PVLD_Reader::
AppendMissingMaterialMaterial( int type, int nb, vector<int>& dims, vector<float>& data )
{
    if( nb!=0 ) return;

    vector<int> msmat;
    switch( type )
    {
    case(solid_elmt_type):
        msmat=solid_msmat_;
        break;
    case( beam_elmt_type):
        msmat= beam_msmat_;
        break;
    case(shell_elmt_type):
        msmat=shell_msmat_;
        break;
    case(  sph_elmt_type):
        msmat=  sph_msmat_;
        break;
    default:
        throw std::runtime_error( "Unknow element type in PVLD_Reader::AppendMissingMaterialMesh()\n" );
    }
    if( msmat.size()==0 ) return;

    for( vector<int>::const_iterator m=msmat.begin(); m!=msmat.end(); ++m )
    {
        dims[0]++;
        data.push_back( *m );
    }
}











void PVLD_Reader::
CollectMaterial( hid_t fid, const char* grpname, const vector<string>& dsname,
                 int ne, vector<int>& mat, bool allowCollective)
{
    if( mat.size()>0 ) return;

    if( std::find( dsname.begin(),
                   dsname.end(),
                   "Material" )
            == dsname.end() ) return;
    // mat.resize(1); mat[0]=nmmat_+1; return
    //throw std::runtime_error( "No material data is found in PVLD_Reader::CollectMaterial()\n" );

    if(allowCollective)
    {
#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        vector<int> mdat( ne );
        ReadGroupDataSet( fid, grpname, "Material",
                          H5T_NATIVE_INT, &mdat[0] );
        set<int> mset;
        for( vector<int>::const_iterator iter=mdat.begin(); iter!=mdat.end(); iter++ )
            mset.insert( *iter );
        for( set<int>::const_iterator iter=mset.begin(); iter!=mset.end(); iter++ )
            mat.push_back( *iter );
    }
#ifdef PARALLEL
    {
        int len = mat.size();
        //MPI_Bcast( &len, 1, MPI_INT, 0, VISIT_MPI_COMM );
        BroadcastInt( len );
        mat.resize(len);
        //MPI_Bcast( &mat[0], len, MPI_INT, 0, VISIT_MPI_COMM );
        BroadcastIntArray( &mat[0], len );
    }
#endif
    }
    else
    {
        vector<int> mdat( ne );
        ReadGroupDataSet( fid, grpname, "Material",
                          H5T_NATIVE_INT, &mdat[0] );
        set<int> mset;
        for( vector<int>::const_iterator iter=mdat.begin(); iter!=mdat.end(); iter++ )
            mset.insert( *iter );
        for( set<int>::const_iterator iter=mset.begin(); iter!=mset.end(); iter++ )
            mat.push_back( *iter );
    }
}











hid_t PVLD_Reader::
OpenFile()
{
// // #ifdef PARALLEL
// //   hid_t plist_id = H5Pcreate( H5P_FILE_ACCESS );
// //   herr_t herr = H5Pset_fapl_mpio( plist_id, VISIT_MPI_COMM, MPI_INFO_NULL );
// //   if( herr<0 ) throw std::runtime_error( "Failed to set mpi comm.\n" );

// //   hid_t file_id =  H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, plist_id );
// //   H5Pclose( plist_id );
// // #else
//   hid_t file_id =  H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
// // #endif
//   if( file_id<0 ) {
//     string msg = "Failed to open Velodyne plot file: \"";
//     msg += filename_; msg+="\".\n";
//     throw std::runtime_error(msg);
//   }
//   return file_id;
    if( file_id_<=0 )
    {
        file_id_ =  H5Fopen( filename_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
        if( file_id_<0 )
        {
            string msg = "Failed to open Velodyne plot file: \"";
            msg += filename_;
            msg+="\".\n";
            throw std::runtime_error(msg);
        }
    }
    return file_id_;
}


void PVLD_Reader::
CloseFile( hid_t file_id )
{
    // herr_t herr = H5Fclose( file_id );
    // if( herr<0 ) {
    //   string msg = "Failed to close Velodyne plot file: \"";
    //   msg += filename_; msg+="\"\n";
    //   throw std::runtime_error(msg);
    // }
}

void PVLD_Reader::
CloseFile()
{
    if( file_id_<=0 ) return;
    herr_t herr = H5Fclose( file_id_ );
    if( herr<0 )
    {
        string msg = "Failed to close Velodyne plot file: \"";
        msg += filename_;
        msg+="\"\n";
        throw std::runtime_error(msg);
    }
    file_id_=0;
}




hid_t PVLD_Reader::
OpenGroup( hid_t loc, const char* name )
{
    hid_t gid = H5Gopen1( loc, name );
    if( gid<0 )
    {
        string msg( "Failed to open Group: \"" );
        msg += name;
        //msg += "\" in Velodyne plot file \""; msg += filename_;
        msg += "\".\n";
        throw std::runtime_error(msg);
    }
    return gid;
}


void PVLD_Reader::
CloseGroup( hid_t gid )
{
    herr_t herr = H5Gclose(gid);
    if( herr<0 )
    {
        string msg( "Failed to close Group.\n" );
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
CollectGroupDataSets( hid_t gid, vector<string>& dsname, vector<vector<int> >& dsdims )
{
    hsize_t idx=0;
    herr_t herr = H5Literate( gid,
                              H5_INDEX_NAME, H5_ITER_NATIVE,
                              &idx,
                              GetDataSetName, &dsname );
    if( herr!=0 )
        throw std::runtime_error("Failed in iterating link names in PVLD_Reader::CollectGroupDataSets()");

    // std::for_each( dsname.begin(), dsname.end(),
    //          [&]( const string& name )
    //          {
    //            hid_t dsid = OpenDataSet( gid, name.c_str() );
    //            hid_t spid = H5Dget_space( dsid );
    //            hsize_t dims[1024];
    //            int ndim = H5Sget_simple_extent_dims( spid, dims, NULL );
    //            H5Sclose(spid);
    //            H5Dclose(dsid);

    //            vector<int> dd;
    //            for( int i=0; i<ndim; i++ )
    //              dd.push_back( dims[i] );
    //            dsdims.push_back( dd );
    //          }
    //          );
    for( vector<string>::const_iterator iter=dsname.begin(); iter!=dsname.end(); iter++ )
    {
        const string& name = *iter;
        hid_t dsid = OpenDataSet( gid, name.c_str() );
        hid_t spid = H5Dget_space( dsid );
        hsize_t dims[1024];
        int ndim = H5Sget_simple_extent_dims( spid, dims, NULL );
        H5Sclose(spid);
        H5Dclose(dsid);

        vector<int> dd;
        for( int i=0; i<ndim; i++ )
            dd.push_back( dims[i] );
        dsdims.push_back( dd );
    }
}




void PVLD_Reader::
CollectSubgroups( hid_t gid, vector<string>& sgname )
{
    hsize_t idx=0;
    herr_t herr = H5Literate( gid,
                              H5_INDEX_NAME, H5_ITER_NATIVE,
                              &idx,
                              GetSubgroupName, &sgname );
    if( herr!=0 )
    {
        throw std::runtime_error("Failed in iterating link names in PVLD_Reader::CollectSubgroups()");
    }
}





hid_t PVLD_Reader::
OpenAttribute( hid_t loc, const char* name )
{
    hid_t aid = H5Aopen_name( loc, name );
    if( aid<0 )
    {
        string msg("Failed to open attribute \"");
        msg += name;
        msg += "\".\n";
        throw std::runtime_error(msg);
    }
    return aid;
}


void PVLD_Reader::
CloseAttribute( hid_t aid)
{
    herr_t herr = H5Aclose(aid);
    if( herr<0 )
    {
        string msg( "Failed to close Attribute.\n" );
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
GetAttributeSize( hid_t aid, int& ndim, hsize_t* dims )
{
    hid_t sid = H5Aget_space(aid);
    if( sid<0 )
    {
        string msg("Failed to obtain space of attribute.\n");
        throw std::runtime_error(msg);
    }

    hsize_t dd[1024];
    int nd = H5Sget_simple_extent_dims( sid, dd, NULL );
    if( ndim<0 )
    {
        H5Sclose(sid);
        string msg("Failed to obtain dimensions of attribute.\n");
        throw std::runtime_error(msg);
    }

    if( dims != NULL )
    {
        for( int i=0; i<nd; i++ )
            if( i<=ndim ) dims[i] = dd[i];
    }
    ndim = nd;
    H5Sclose(sid);
}


void PVLD_Reader::
ReadAttribute( hid_t aid, hid_t type, void* buf )
{
    herr_t herr = H5Aread( aid, type, buf );
    if( herr<0 )
    {
        string msg("Failed to read attribute.\n");
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
ReadAttribute( hid_t loc, const char* name, hid_t type, void* buf )
{
    hid_t aid = OpenAttribute( loc, name );
    ReadAttribute( aid, type, buf );
    CloseAttribute( aid );
}



hid_t PVLD_Reader::
OpenDataSet( hid_t gid, const char* name )
{
    hid_t dsid = H5Dopen1( gid, name );
    if( dsid<0 )
    {
        string msg("Failed to open dataset \"");
        msg += name;
        msg += "\".\n";
        throw std::runtime_error(msg);
    }
    return dsid;
}


void PVLD_Reader::
CloseDataSet( hid_t dsid)
{
    herr_t herr = H5Dclose(dsid);
    if( herr<0 )
    {
        string msg( "Failed to close dataset.\n" );
        throw std::runtime_error(msg);
    }
}


void PVLD_Reader::
GetDataSetSize( hid_t dsid, int& ndim, hsize_t* dims )
{
    hid_t sid = H5Dget_space(dsid);
    if( sid<0 )
    {
        string msg("Failed to obtain space of dataset.\n");
        throw std::runtime_error(msg);
    }

    hsize_t dd[1024];
    int nd = H5Sget_simple_extent_dims( sid, dd, NULL );
    if( ndim<0 )
    {
        H5Sclose(sid);
        string msg("Failed to obtain dimensions of dataset.\n");
        throw std::runtime_error(msg);
    }

    if( dims != NULL )
    {
        for( int i=0; i<nd; i++ )
            if( i<=ndim ) dims[i] = dd[i];
    }
    ndim = nd;
    H5Sclose(sid);
}



void PVLD_Reader::
ReadDataSet( hid_t dsid, hid_t type, void* buf,
             int ndim, const hsize_t* sft, const hsize_t* cnt )
{
    herr_t herr;
// #ifdef PARALLEL
//   hid_t xftid = H5Pcreate( H5P_DATASET_XFER );
//   herr = H5Pset_dxpl_mpio( xftid, H5FD_MPIO_INDEPENDENT );
// #else
//   hid_t xftid = H5Pcopy( H5P_DEFAULT );
// #endif
    hid_t xftid = H5Pcopy( H5P_DEFAULT );

    if( ndim<=0 )
    {
        herr = H5Dread( dsid, type, H5S_ALL, H5S_ALL, xftid, buf );
        if( herr<0 )
        {
            H5Pclose( xftid );
            string msg("Failed to read whole dataset.\n");
            throw std::runtime_error(msg);
        }
    }
    else
    {
        hid_t memid = H5Screate_simple( ndim, cnt, NULL );

        hid_t spcid = H5Dget_space( dsid );
        herr = H5Sselect_hyperslab( spcid, H5S_SELECT_SET, sft, NULL, cnt, NULL );
        if( herr<0 )
        {
            H5Pclose( xftid );
            H5Sclose( memid );
            H5Sclose( spcid );
            string msg("Failed to select hyperslab when reading dataset.\n");
            throw std::runtime_error(msg);
        }

        herr = H5Dread( dsid, type, memid, spcid, xftid, buf );
        if( herr<0 )
        {
            H5Pclose( xftid );
            H5Sclose( memid );
            H5Sclose( spcid );
            string msg("Failed to read given section of dataset.\n");
            throw std::runtime_error(msg);
        }

        H5Sclose( memid );
        H5Sclose( spcid );
    }

    H5Pclose( xftid );
}



void PVLD_Reader::
ReadGroupDataSet( hid_t fid, const char* gname, const char* dsname,
                  hid_t type, void* buf,
                  int ndim, const hsize_t* sft, const hsize_t* cnt )
{
    try
    {
        hid_t gid = OpenGroup( fid, gname );
        hid_t did = OpenDataSet( gid, dsname );
        ReadDataSet( did, type, buf, ndim, sft, cnt );
        CloseDataSet(did);
        CloseGroup(gid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg += "Failure in PVLD_Reader::ReadGroupDataSet()\n";
        throw std::runtime_error(msg);
    }
}












void PVLD_Reader::
ReadIndexVariableInfo( hid_t file_id,
                       const char* mshname,
                       const char* ivname,
                       const vector<string>& dsnames,
                       IndexVariables& iv )
{
    try
    {
        hid_t gid = OpenGroup( file_id, mshname );

        vector<string> gnames;
        ReadStringArrayAttribute( gid, ivname, gnames );
        if( gnames.size()>0 )
        {
            vector<string> sgnames;
            CollectSubgroups( gid, sgnames );

            for( vector<string>::const_iterator iter=gnames.begin(); iter!=gnames.end(); iter++ )
            {
                //debug1 << "gnames: " << *iter << "\n";
                if( std::find( sgnames.begin(), sgnames.end(),
                               *iter ) != sgnames.end() )
                {
                    string ivn = *iter + "Index";
                    if( std::find( dsnames.begin(), dsnames.end(),
                                   ivn ) != dsnames.end() )
                    {
                        //debug1 << "\t find index : " << ivn << "\n";
                        vector<string> dsn;
                        vector<vector<int> > dsd;
                        vector<vector<double> > dst;

                        hid_t sgid = OpenGroup( gid, (*iter).c_str() );
                        CollectGroupDataSets( sgid, dsn, dsd );
                        CloseGroup(sgid);
                        //debug1 << "\t n_index_name : " << dsn.size() << "\n";
                        //debug1 << "\t n_index_dims : " << dsd.size() << "\n";

                        vector<double> dt;
                        for( vector<string>::const_iterator i=dsn.begin(); i!=dsn.end(); i++ )
                            dst.push_back( dt );

                        iv.names_.push_back( *iter );
                        iv.dsnames_.push_back( dsn );
                        iv.dsdims_.push_back( dsd );
                        iv.dsdata_.push_back( dst );
                    }
                }
            }
        }
        CloseGroup(gid);
    }
    catch( std::exception& e )
    {
        string msg = e.what();
        msg+="Failure in PVLD_IndexReader::ReadIndexVariableInfo(";
        msg+=mshname;
        msg+=", ";
        msg+=ivname;
        msg+=").\n";
        throw std::runtime_error(msg);
    }
}




void PVLD_Reader::
ReadStringArrayAttribute( hid_t loc, const char* att_name,  vector<string>& str_array )
{
    if( H5Aexists( loc, att_name ) <= 0 )  return;

    hid_t aid = OpenAttribute( loc, att_name );

    hid_t sid = H5Aget_space( aid );
    int ndim = H5Sget_simple_extent_ndims( sid );
    vector<hsize_t> dims( ndim );
    H5Sget_simple_extent_dims( sid, &dims[0], NULL );
    H5Sclose(sid);
    if( ndim != 1 )
    {
        string msg = "Unexpected number of dimensions in attribute \"";
        msg+=att_name;
        msg+="\" in PVLD_Reader::ReadStringArrayAttribute().\n";
        throw std::runtime_error( msg );
    }

    hid_t type = H5Tcopy( H5T_C_S1 );
    H5Tset_size( type, H5T_VARIABLE );

    char** names = new char* [ dims[0] ];
    herr_t herr = H5Aread( aid, type, names ); (void) herr;

    for( hsize_t i=0; i<dims[0]; i++ )
    {
        str_array.push_back( names[i] );
        free( names[i] );
    }
    delete [] names;

    CloseAttribute( aid );
}



void PVLD_Reader::
PickIndexVariable( const vector<int>&    dims,
                   const vector<double>& smp,
                   const vector<float>&  idx,
                   vector<float>&  dat )
{
    int esz=1;
    for( size_t i=1; i<dims.size(); i++ )
        esz*=dims[i];

    int mx = smp.size()/esz;
    dat.resize( esz*idx.size() );
    std::fill( dat.begin(), dat.end(), 0. );

    for( size_t i=0; i<idx.size(); i++ )
    {
        int id = int(idx[i])-1;
        if( id>=0 && id<mx )
        {
            int s1 = i*esz;
            int s2 = id*esz;
            for( int j=0; j<esz; j++ )
                dat[s1+j] = smp[s2+j];
        }
    }
}



void PVLD_Reader::
ReadSolidBlockIndexVariable( const string& idxname,
                             const string& varname,
                             int domain,
                             vector<int>&   dims,
                             vector<float>& data )
{
    const string& mn = solid_name;
    vector<int>& pt = solid_part_; (void) pt;
    IndexVariables& iv = solid_idxvar_;

    try
    {
        size_t iind=0;
        for( iind=0; iind<iv.names_.size(); iind++ )
            if( iv.names_[iind] == idxname ) break;
        if( iind >= iv.names_.size() ) return;

        vector<string>& dsname = iv.dsnames_[iind];
        size_t vind=0;
        for( vind=0; vind<dsname.size(); vind++ )
            if( dsname[vind] == varname ) break;
        if( vind >= dsname.size() ) return;

        const vector<int>& sdim = iv.dsdims_[iind][vind];
        vector<double>& smpl = iv.dsdata_[iind][vind];

        size_t dsz=1;
        for( size_t i=0; i<sdim.size(); i++ )
            dsz *= sdim[i];

        if( smpl.size() != dsz )
        {
            smpl.resize(dsz);
#     ifdef PARALLEL
            if( PAR_Rank()==0 )
            {
#     endif
                hid_t fid = OpenFile();
                hid_t gid = OpenGroup( fid, mn.c_str() );
                ReadGroupDataSet( gid,
                                  idxname.c_str(),
                                  varname.c_str(),
                                  H5T_NATIVE_DOUBLE,
                                  &smpl[0] );
                CloseGroup( gid );
                CloseFile( fid );
#     ifdef PARALLEL
            }
            BroadcastDoubleArray( &smpl[0], dsz );
#     endif
        }

        string ivn = idxname + "Index";
        vector<int>   idims;
        vector<float> idata;
        ReadSolidBlockData( ivn.c_str(), domain, idims, idata );

        dims = sdim;
        dims[0] = idims[0];
        PickIndexVariable( sdim, smpl, idata, data );
    }
    catch( std::exception& e )
    {
        string msg = "Exception caught in PVLD_Reader::ReadSolidBlockIndexVariable(";
        msg+=idxname;
        msg+=", ";
        msg+=varname;
        msg+=").\n";
        msg += e.what();
        throw std::runtime_error(msg);
    }
}




void PVLD_Reader::
ReadShellBlockIndexVariable( const string& idxname,
                             const string& varname,
                             int domain,
                             vector<int>&   dims,
                             vector<float>& data )
{
    const string& mn = shell_name;
    vector<int>& pt = shell_part_; (void) pt;
    IndexVariables& iv = shell_idxvar_;

    try
    {
        size_t iind=0;
        size_t vind=0;
        for( iind=0; iind<iv.names_.size(); iind++ )
            if( iv.names_[iind] == idxname ) break;
        if( iind >= iv.names_.size() ) return;

        const vector<string>& dsname = iv.dsnames_[iind];
        for( vind=0; vind<dsname.size(); vind++ )
            if( dsname[vind] == varname ) break;
        if( vind >= dsname.size() ) return;

        const vector<int>& sdim = iv.dsdims_[iind][vind];
        vector<double>& smpl = iv.dsdata_[iind][vind];

        size_t dsz=1;
        for( size_t i=0; i<sdim.size(); i++ )
            dsz *= sdim[i];

        if( smpl.size() != dsz )
        {
            smpl.resize(dsz);
#     ifdef PARALLEL
            if( PAR_Rank()==0 )
            {
#     endif
                hid_t fid = OpenFile();
                hid_t gid = OpenGroup( fid, mn.c_str() );
                ReadGroupDataSet( gid,
                                  idxname.c_str(),
                                  varname.c_str(),
                                  H5T_NATIVE_DOUBLE,
                                  &smpl[0] );
                CloseGroup( gid );
                CloseFile( fid );
#     ifdef PARALLEL
            }
            BroadcastDoubleArray( &smpl[0], dsz );
#     endif
        }

        string ivn = idxname + "Index";
        vector<int>   idims;
        vector<float> idata;
        ReadShellBlockData( ivn.c_str(), domain, idims, idata );

        dims = sdim;
        dims[0] = idims[0];
        PickIndexVariable( sdim, smpl, idata, data );
    }
    catch( std::exception& e )
    {
        string msg = "Exception caught in PVLD_IndexReader::ReadShellBlockIndexVariable(";
        msg+=idxname;
        msg+=", ";
        msg+=varname;
        msg+=").\n";
        msg += e.what();
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
CheckNumberOfEngines( int npart )
{
    int nengines = 1;
#ifdef PARALLEL
    nengines = PAR_Size();
#endif

    if( npart < nengines )
    {
        char buf[1024];
        sprintf( buf, "NUMBER OF ENGINES MUST NOT BE GREATER THAN %d ", npart );

        string msg="\n\n";
        msg+=buf;
        msg += "FOR DATAFILE \"";
        msg+=filename_;
        msg+="\"!!!\n";
        sprintf( buf, "CURRENTLY YOU ARE USING %d ENGINES.\n", nengines );
        msg+=buf;
        msg += "YOU CAN EITHER USE FEWER ENGINES OR SELECT ELEMENT-BASED PARTITION METHOD TO AVOID THIS PROBLEM.\n";
        msg += "\n\n";
        throw std::runtime_error(msg);
    }
}



void PVLD_Reader::
GenDefaultSphVariables( vector<string>& dsname, vector<vector<int> >& dsdims )
{
    vector<int> dims;

    dims.resize(1);
    dims[0]=1;

    dsname.push_back( "Index" );
    dsdims.push_back( dims );

    dsname.push_back( "Material" );
    dsdims.push_back( dims );


    dims.resize(2);
    dims[0]=1;
    dims[1]=3;

    dsname.push_back( "Coordinate" );
    dsdims.push_back( dims );

    dsname.push_back( "Velocity" );
    dsdims.push_back( dims );
}

