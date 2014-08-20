
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

#include <Partition.h>
#include <pvldPartReader.h>

using std::vector;
using std::string;

// defined in pvldRead.C
void EquallyPartition( int npart, int total, int* sft );


void ShowVector( const char* str, const vector<int>& v )
{
    debug1 << str << " = [ ";
    for( vector<int>::const_iterator i=v.begin(); i!=v.end(); i++ )
        debug1 << *i << ", ";
    debug1 << " ]\n";
}



void PVLD_Part_Reader::
FreeResource()
{
    debug1 << "PVLD_Part_Reader::FreeResource( \"" << filename_ << "\" ) is called.\n";

    PVLD_Reader::FreeResource();

    solid_matidx_.clear();
    FreeVector( solid_matidx_ ); //.shrink_to_fit();
    solid_elmdef_.clear();
    FreeVector( solid_elmdef_ );
    solid_elmvar_.clear();
    FreeVector( solid_elmvar_ );
    solid_hvesft_.clear();
    FreeVector( solid_hvesft_ );
    solid_varnam_="";

    shell_matidx_.clear();
    FreeVector( shell_matidx_ );
    shell_elmdef_.clear();
    FreeVector( shell_elmdef_ );
    shell_elmvar_.clear();
    FreeVector( shell_elmvar_ );
    shell_hvesft_.clear();
    FreeVector( shell_hvesft_ );
    shell_varnam_="";

    surf_matidx_.clear();
    FreeVector( surf_matidx_ );
    surf_elmdef_.clear();
    FreeVector( surf_elmdef_ );
    surf_elmvar_.clear();
    FreeVector( surf_elmvar_ );
    surf_varnam_="";
}



void PVLD_Part_Reader::
ReadTOC()
{
    H5open();
    try
    {
        PVLD_Reader::ReadTOC();

        hid_t file_id = OpenFile();

        if( nsolid_>0 )
        {
            int npart = solid_part_.size()-1;
            solid_part_.clear();
            ReadMaterialInfo( file_id, solid_name.c_str(), solid_dsname_,
                              npart, nsolid_,
                              solid_part_,
                              solid_prtsft_, solid_matid_,
                              solid_matsft_, solid_matidx_ );
        }

        if( nshell_>0 )
        {
            int npart = shell_part_.size()-1;
            shell_part_.clear();
            ReadMaterialInfo( file_id, shell_name.c_str(), shell_dsname_,
                              npart, nshell_,
                              shell_part_,
                              shell_prtsft_, shell_matid_,
                              shell_matsft_, shell_matidx_ );
        }

        if( nsurf_>0 )
        {
            int npart = surf_part_.size()-1;
            surf_part_.clear();
            ReadMaterialInfo( file_id, surface_name.c_str(), surf_dsname_,
                              npart, nsurf_,
                              surf_part_,
                              surf_prtsft_, surf_matid_,
                              surf_matsft_, surf_matidx_ );
        }

        CloseFile( file_id );
    }
    catch( std::exception& e )
    {
        string msg("Exception caught in PVLD_Part_Reader::ReadTOC() : \n\t");
        msg += e.what();
        throw std::runtime_error(msg);
    }
}




void PVLD_Part_Reader::
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
        if( sph_lhv_ )
        {
            PVLD_Reader::ReadHistoryDataInfo( file_id, sph_name.c_str(), sph_part_,
                                              sph_mxhv_, sph_hvpart_ );
            int nblks = GetNumOfSphBlocks();
            sph_hvdisp_.resize( nblks );
        }
        CloseFile( file_id );
    }
    catch( std::exception& e )
    {
        string msg("Exception caught in PVLD_Part_Reader::ReadHistoryTOC() : \n\t");
        msg += e.what();
        throw std::runtime_error(msg);
    }
}







void PVLD_Part_Reader::
ReadSolidBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( solid_matidx_.size()==0 )
        ReadElementIndex( fid, solid_name.c_str(),
                          nsolid_, solid_matid_, solid_matsft_,
                          solid_matidx_ );

    if( solid_elmdef_.size()==0 )
    {
        size_t ind;
        for( ind=0; ind<solid_dsname_.size(); ind++ )
            if( solid_dsname_[ind] == "Nodes" ) break;
        if( ind>=solid_dsname_.size() )
            throw std::runtime_error( "No element definition is found in PVLD_Part_Reader::ReadSolidBlockMesh()\n");
        ReadRawData( fid, solid_name.c_str(),
                     "Nodes", solid_dsdims_[ind],
                     solid_matidx_, solid_elmdef_ );
    }

    int sft = 8*solid_part_.at(nb);
    int len = 8*solid_part_.at(nb+1) - sft;
    elmt.resize( len );
    for( int i=0; i<len; i++ )
        elmt[i] = solid_elmdef_[ sft+i ];

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



void PVLD_Part_Reader::
ReadSolidBlockData( hid_t fid, int varInd, int blkInd,
                    vector<int>& dims, vector<float>& edat )
{
    if( solid_matidx_.size()==0 )
        ReadElementIndex( fid, solid_name.c_str(),
                          nsolid_, solid_matid_, solid_matsft_,
                          solid_matidx_ );

    if( solid_varnam_ != solid_dsname_[varInd] )
    {
        ReadRawData( fid, solid_name.c_str(),
                     solid_dsname_[varInd].c_str(), solid_dsdims_[varInd],
                     solid_matidx_, solid_elmvar_ );
        solid_varnam_ = solid_dsname_[varInd];
    }

    int sft  = solid_part_.at(blkInd);
    int len  = solid_part_.at(blkInd+1) - sft;

    const vector<int>& dd = solid_dsdims_[varInd];
    int ndim = dd.size();
    dims.resize( ndim );
    dims[0] = len;
    int esz = 1;
    for( int d=1; d<ndim; d++ )
    {
        esz *= dd[d];
        dims[d] = dd[d];
    }

    sft *= esz;
    len *= esz;
    edat.resize( len );
    for( int i=0; i<len; i++ )
        edat[i]= solid_elmvar_[ sft+i ];
}




void PVLD_Part_Reader::
ReadSolidBlockHistoryData( hid_t fid, int blkInd, int ind,
                           vector<int>& dims, vector<float>& data )
{
    if( solid_matidx_.size()==0 )
        ReadElementIndex( fid, solid_name.c_str(),
                          nsolid_, solid_matid_, solid_matsft_,
                          solid_matidx_ );

    if( solid_varnam_ != history_name )
    {
        ReadRawHistoryData( fid, solid_name.c_str(),
                            solid_matidx_,
                            solid_hvesft_,
                            solid_elmvar_ );
        solid_varnam_ = history_name;
    }

    int sft  = solid_part_.at(blkInd);
    int len  = solid_part_.at(blkInd+1) - sft;

    dims.resize(1);
    dims[0]=len;

    data.resize(len);
    std::fill( data.begin(), data.end(), 1.23456e0 );

    for( int i=0; i<len; i++ )
    {
        int j = solid_hvesft_[sft+i] + ind;
        if( j < solid_hvesft_[ sft+i+1 ] )
            data[i] = solid_elmvar_[j];
    }
}


void PVLD_Part_Reader::
ReadSolidBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfSolidBlocks();
    if( blkInd >= nblks ) return;

    int estr = solid_part_.at(blkInd);
    int estp = solid_part_.at(blkInd+1);

    int pstr = solid_prtsft_.at( blkInd );
    int pstp = solid_prtsft_.at( blkInd + 1 );

    int mstr = solid_matsft_.at( pstr );
    int mstp = solid_matsft_.at( pstp );
    // debug1 << "elmt range: [" << estr << " : " << estp << " )\n";
    // debug1 << "part range: [" << pstr << " : " << pstp << " )\n";
    // debug1 << "memb range: [" << mstr << " : " << mstp << " )\n";
    if( mstr != estr || mstp != estp )
    {
        string msg="Inconsistence in number of elements selection in ReadSolidBlockMaterial()\n";
        throw std::runtime_error(msg);
    }

    dims.resize(1);
    dims[0] = estp - estr;

    data.resize( dims[0] );
    int loc=0;
    for( int p=pstr; p<pstp; p++ )
    {
        int mid = solid_matid_[p];
        int len = solid_matsft_[p+1] - solid_matsft_[p];
        for( int j=0; j<len; j++ )
            data[loc++] = mid;
    }
}




void PVLD_Part_Reader::
ReadShellBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( shell_matidx_.size()==0 )
        ReadElementIndex( fid, shell_name.c_str(),
                          nshell_, shell_matid_, shell_matsft_,
                          shell_matidx_ );

    if( shell_elmdef_.size()==0 )
    {
        size_t ind;
        for( ind=0; ind<shell_dsname_.size(); ind++ )
            if( shell_dsname_[ind] == "Nodes" ) break;
        if( ind>=shell_dsname_.size() )
            throw std::runtime_error( "No element definition is found in PVLD_Part_Reader::ReadShellBlockMesh()\n");
        ReadRawData( fid, shell_name.c_str(),
                     "Nodes", shell_dsdims_[ind],
                     shell_matidx_, shell_elmdef_ );
    }

    int sft = 4*shell_part_.at(nb);
    int len = 4*shell_part_.at(nb+1) - sft;
    elmt.resize( len );
    for( int i=0; i<len; i++ )
        elmt[i] = shell_elmdef_[ sft+i ];

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


void PVLD_Part_Reader::
ReadShellBlockData( hid_t fid, int varInd, int blkInd,
                    vector<int>& dims, vector<float>& edat )
{
    if( shell_matidx_.size()==0 )
        ReadElementIndex( fid, shell_name.c_str(),
                          nshell_, shell_matid_, shell_matsft_,
                          shell_matidx_ );

    if( shell_varnam_ != shell_dsname_[varInd] )
    {
        ReadRawData( fid, shell_name.c_str(),
                     shell_dsname_[varInd].c_str(), shell_dsdims_[varInd],
                     shell_matidx_, shell_elmvar_ );
        shell_varnam_ = shell_dsname_[varInd];
    }

    int sft  = shell_part_.at(blkInd);
    int len  = shell_part_.at(blkInd+1) - sft;

    const vector<int>& dd = shell_dsdims_[varInd];
    int ndim = dd.size();
    dims.resize( ndim );
    dims[0] = len;
    int esz = 1;
    for( int d=1; d<ndim; d++ )
    {
        esz *= dd[d];
        dims[d] = dd[d];
    }

    sft *= esz;
    len *= esz;
    edat.resize( len );
    for( int i=0; i<len; i++ )
        edat[i]= shell_elmvar_[ sft+i ];
}



void PVLD_Part_Reader::
ReadShellBlockHistoryData( hid_t fid, int blkInd, int ind,
                           vector<int>& dims, vector<float>& data )
{
    if( shell_matidx_.size()==0 )
        ReadElementIndex( fid, shell_name.c_str(),
                          nshell_, shell_matid_, shell_matsft_,
                          shell_matidx_ );

    if( shell_varnam_ != history_name )
    {
        ReadRawHistoryData( fid, shell_name.c_str(),
                            shell_matidx_,
                            shell_hvesft_,
                            shell_elmvar_ );
        shell_varnam_ = history_name;
    }

    int sft  = shell_part_.at(blkInd);
    int len  = shell_part_.at(blkInd+1) - sft;

    dims.resize(1);
    dims[0]=len;

    data.resize(len);
    std::fill( data.begin(), data.end(), 1.23456e0 );

    for( int i=0; i<len; i++ )
    {
        int j = shell_hvesft_[sft+i] + ind;
        if( j < shell_hvesft_[ sft+i+1 ] )
            data[i] = shell_elmvar_[j];
    }
}



void PVLD_Part_Reader::
ReadShellBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfShellBlocks();
    if( blkInd >= nblks ) return;

    int estr = shell_part_.at(blkInd);
    int estp = shell_part_.at(blkInd+1);

    int pstr = shell_prtsft_.at( blkInd );
    int pstp = shell_prtsft_.at( blkInd + 1 );

    int mstr = shell_matsft_.at( pstr );
    int mstp = shell_matsft_.at( pstp );
    // debug1 << "elmt range: [" << estr << " : " << estp << " )\n";
    // debug1 << "part range: [" << pstr << " : " << pstp << " )\n";
    // debug1 << "memb range: [" << mstr << " : " << mstp << " )\n";
    if( mstr != estr || mstp != estp )
    {
        string msg="Inconsistence in number of elements selection in ReadShellBlockMaterial()\n";
        throw std::runtime_error(msg);
    }

    dims.resize(1);
    dims[0] = estp - estr;

    data.resize( dims[0] );
    int loc=0;
    for( int p=pstr; p<pstp; p++ )
    {
        int mid = shell_matid_[p];
        int len = shell_matsft_[p+1] - shell_matsft_[p];
        for( int j=0; j<len; j++ )
            data[loc++] = mid;
    }
}



void PVLD_Part_Reader::
ReadSurfaceBlockMesh( hid_t fid, int nb, vector<int>& vmap, vector<float>& vcrd, vector<int>& elmt )
{
    if( surf_matidx_.size()==0 )
        ReadElementIndex( fid, surface_name.c_str(),
                          nsurf_, surf_matid_, surf_matsft_,
                          surf_matidx_ );

    if( surf_elmdef_.size()==0 )
    {
        size_t ind;
        for( ind=0; ind<surf_dsname_.size(); ind++ )
            if( surf_dsname_[ind] == "Nodes" ) break;
        if( ind>=surf_dsname_.size() )
            throw std::runtime_error( "No element definition is found in PVLD_Part_Reader::ReadSurfaceBlockMesh()\n");
        ReadRawData( fid, surface_name.c_str(),
                     "Nodes", surf_dsdims_[ind],
                     surf_matidx_, surf_elmdef_ );
    }

    int sft = 4*surf_part_.at(nb);
    int len = 4*surf_part_.at(nb+1) - sft;
    elmt.resize( len );
    for( int i=0; i<len; i++ )
        elmt[i] = surf_elmdef_[ sft+i ];

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



void PVLD_Part_Reader::
ReadSurfaceBlockData( hid_t fid, int varInd, int blkInd,
                      vector<int>& dims, vector<float>& edat )
{
    if( surf_matidx_.size()==0 )
        ReadElementIndex( fid, surface_name.c_str(),
                          nsurf_, surf_matid_, surf_matsft_,
                          surf_matidx_ );

    if( surf_varnam_ != surf_dsname_[varInd] )
    {
        ReadRawData( fid, surface_name.c_str(),
                     surf_dsname_[varInd].c_str(), surf_dsdims_[varInd],
                     surf_matidx_, surf_elmvar_ );
        surf_varnam_ = surf_dsname_[varInd];
    }

    int sft  = surf_part_.at(blkInd);
    int len  = surf_part_.at(blkInd+1) - sft;

    const vector<int>& dd = surf_dsdims_[varInd];
    int ndim = dd.size();
    dims.resize( ndim );
    dims[0] = len;
    int esz = 1;
    for( int d=1; d<ndim; d++ )
    {
        esz *= dd[d];
        dims[d] = dd[d];
    }

    sft *= esz;
    len *= esz;
    edat.resize( len );
    for( int i=0; i<len; i++ )
        edat[i]= surf_elmvar_[ sft+i ];
}



void PVLD_Part_Reader::
ReadSurfaceBlockMaterial( int blkInd, vector<int>& dims, vector<float>& data )
{
    int nblks = GetNumOfSurfaceBlocks();
    if( blkInd >= nblks ) return;

    int estr = surf_part_.at(blkInd);
    int estp = surf_part_.at(blkInd+1);

    int pstr = surf_prtsft_.at( blkInd );
    int pstp = surf_prtsft_.at( blkInd + 1 );

    int mstr = surf_matsft_.at( pstr );
    int mstp = surf_matsft_.at( pstp );

    if( mstr != estr || mstp != estp )
    {
        string msg="Inconsistence in number of elements selection in PVLD_Part_Reader::ReadSurfaceBlockMaterial()\n";
        throw std::runtime_error(msg);
    }

    dims.resize(1);
    dims[0] = estp - estr;

    data.resize( dims[0] );
    int loc=0;
    for( int p=pstr; p<pstp; p++ )
    {
        int mid = surf_matid_[p];
        int len = surf_matsft_[p+1] - surf_matsft_[p];
        for( int j=0; j<len; j++ )
            data[loc++] = mid;
    }
}



void PVLD_Part_Reader::
ReadMaterialInfo( hid_t fid, const char* grpname,
                  const vector<string>& dsname,
                  int npart, int ne,
                  vector<int>& part,
                  vector<int>& prtsft, vector<int>& mat,
                  vector<int>& matsft, vector<int>& matidx )
{
    if( std::find( dsname.begin(),
                   dsname.end(),
                   "Material" )
            == dsname.end() )
    {
        string msg="No material data is found in .";
        msg+=grpname;
        msg+=" in ReadMaterialInfo().\n";
        throw std::runtime_error(msg);
    }

    part.clear();
    prtsft.clear();
    mat.clear();
    matsft.clear();
    matidx.clear();

#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        vector<int> mdat( ne );
        ReadGroupDataSet( fid, grpname, "Material",
                          H5T_NATIVE_INT, &mdat[0] );

        vector<int> mcnt( nmmat_ );
        std::fill( mcnt.begin(), mcnt.end(), 0 );
        for( vector<int>::iterator iter=mdat.begin(); iter!=mdat.end(); iter++ )
        {
            int mid = *iter;
            if( mid>nmmat_ || mid<=0 )
            {
                string msg="Unexpected Material id is found in .";
                msg+=grpname;
                msg+=" in ReadMaterialInfo().\n";
                throw std::runtime_error(msg);
            }
            ++(mcnt[mid-1]);
        }

        vector<int> mmap,wlst;
        int cnt=0;
        int last=0;
        for( int m=0; m<nmmat_; m++ )
            if( mcnt[m]>0 )
            {
                last += mcnt[m];
                wlst.push_back( mcnt[m] );
                mmap.push_back( m+1 );
            }
        if( last != ne )
        {
            string msg="Unmatched number of elements is found in .";
            msg += grpname;
            msg+=" in ReadMaterialInfo().\n";
            throw std::runtime_error(msg);
        }
        //ShowVector( "wlst", wlst );

        vector< vector<int> > matprt;
        PartitionWeights( wlst, npart, matprt );
        //debug1 << "matprt.size()= " << matprt.size() << "\n";

        cnt=0;
        last=0;
        prtsft.push_back( cnt );
        matsft.push_back( last );
        for( vector< vector<int> >::const_iterator ip=matprt.begin(); ip!=matprt.end(); ++ip )
        {
            //ShowVector( "matprt cmp", *ip );
            for( vector<int>::const_iterator im=ip->begin(); im!=ip->end(); ++im )
            {
                int ind = *im;
                int mid = mmap[ind];
                last += wlst[ind];
                mat.push_back( mid );
                matsft.push_back( last );
                ++cnt;
            }
            prtsft.push_back( cnt );
        }
        // ShowVector( "mat", mat );
        // ShowVector( "matsft", matsft );
        // ShowVector( "prtsft", prtsft );

        mmap.resize( nmmat_ );
        std::fill( mmap.begin(), mmap.end(), 0 );
        for( size_t i=0; i<mat.size(); i++ )
            mmap.at( mat[i]-1 ) = i+1;

        matidx.resize( ne );
        std::fill( wlst.begin(), wlst.end(), 0 );
        for( int i=0; i<ne; i++ )
        {
            int mid = mdat[i];
            int ind = mmap.at( mid-1 ) - 1;
            int loc = matsft.at( ind ) + wlst[ind]++;
            matidx.at(loc) = i;
        }
        //ShowVector( "aft idx wlst", wlst );
        //ShowVector( "aft idx matsft", matsft );

        for( vector<int>::iterator ip=prtsft.begin(); ip!=prtsft.end(); ++ip )
        {
            int ind = *ip;
            int loc = matsft.at( ind );
            part.push_back( loc );
        }
        //ShowVector( "part", part );
    }
#ifdef PARALLEL
    {
        int len;

        len = part.size();
        BroadcastInt( len );
        part.resize(len);
        BroadcastIntArray( &part[0], len );

        len = prtsft.size();
        BroadcastInt( len );
        prtsft.resize(len);
        BroadcastIntArray( &prtsft[0], len );

        len = mat.size();
        BroadcastInt( len );
        mat.resize(len);
        BroadcastIntArray( &mat[0], len );

        len = matsft.size();
        BroadcastInt( len );
        matsft.resize(len);
        BroadcastIntArray( &matsft[0], len );

        len = matidx.size();
        BroadcastInt( len );
        matidx.resize(len);
        BroadcastIntArray( &matidx[0], len );
    }
#endif
}


void PVLD_Part_Reader::
ReadHistoryDataInfo( hid_t fid, const char* meshname, const vector<int>& part, int& mxnb, vector<int>& hvpart )
{
#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        int np = part.size()-1;
        int ne = part[np];
        vector<int> cnt(ne);
        ReadGroupDataSet( fid, meshname, number_of_history_name.c_str(),
                          H5T_NATIVE_INT, &cnt[0] );
        mxnb = *(std::max_element( cnt.begin(), cnt.end() ));
    }
#ifdef PARALLEL
    {
        BroadcastInt( mxnb );
    }
#endif
}


void PVLD_Part_Reader::
ReadRawData( hid_t fid,
             const char* grpname,
             const char* varname,
             const vector<int>& dims,
             const vector<int>& idx,
             vector<int>& data )
{
    int nd = dims.size();
    int ne = dims[0];
    debug1 << "dims[0]= " << ne << ", dims[1]= " << dims[1] << "\n";
    debug1 << "idx.size()= " << idx.size() << "\n";
    if( (size_t)ne != idx.size() )
    {
        string msg="Unmatched number of elements in ReadRawData(int)( ";
        msg+=grpname;
        msg+=", ";
        msg+=varname;
        msg+=",...).\n";
        throw std::runtime_error(msg);
    }

    int sz = 1;
    for( int i=1; i<nd; i++ )
        sz *= dims[i];

    data.resize( ne*sz );
#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        vector<int> tem( ne*sz );
        ReadGroupDataSet( fid, grpname, varname, H5T_NATIVE_INT, &tem[0] );

        for( int i=0; i<ne; i++ )
        {
            int src = sz*i;
            int tgt = sz*idx[i];
            for( int j=0; j<sz; j++ )
                data[src+j] = tem[tgt+j];
        }
    }
#ifdef PARALLEL
    {
        BroadcastIntArray( &data[0], ne*sz );
    }
#endif
}


void PVLD_Part_Reader::
ReadRawData( hid_t fid,
             const char* grpname,
             const char* varname,
             const vector<int>& dims,
             const vector<int>& idx,
             vector<float>& data )
{
    int nd = dims.size();
    int ne = dims[0];
    if( (size_t)ne != idx.size() )
    {
        string msg="Unmatched number of elements in ReadRawData(float)( ";
        msg+=grpname;
        msg+=", ";
        msg+=varname;
        msg+=",...).\n";
        throw std::runtime_error(msg);
    }

    int sz = 1;
    for( int i=1; i<nd; i++ )
        sz *= dims[i];

    data.resize( ne*sz );
    vector<double> tem( ne*sz );
#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        ReadGroupDataSet( fid, grpname, varname, H5T_NATIVE_DOUBLE, &tem[0] );
    }
#ifdef PARALLEL
    {
        BroadcastDoubleArray( &tem[0], ne*sz );
    }
#endif
    for( int i=0; i<ne; i++ )
    {
        int src = sz*i;
        int tgt = sz*idx[i];
        for( int j=0; j<sz; j++ )
            data[src+j] = tem[tgt+j];
    }
}





void PVLD_Part_Reader::
ReadRawHistoryData( hid_t fid,
                    const char* grpname,
                    const vector<int>& idx,
                    vector<int>&   sft,
                    vector<float>& dat )
{
    vector<double> tgt;
    int ne = idx.size();

#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        vector<int> cnt(ne);
        ReadGroupDataSet( fid, grpname, number_of_history_name.c_str(),
                          H5T_NATIVE_INT, &cnt[0] );

        vector<int> dsp(ne+1);
        dsp[0]=0;
        for( int i=0; i<ne; i++ )
            dsp[i+1] = dsp[i] + cnt[i];

        vector<double> tem( dsp[ne] );
        ReadGroupDataSet( fid, grpname, history_name.c_str(), H5T_NATIVE_DOUBLE, &tem[0] );

        tgt.resize( dsp[ne] );
        sft.resize(ne+1);
        sft[0]=0;
        for( int i=0; i<ne; i++ )
        {
            int j = idx[i];
            int k = dsp[j];
            int s = cnt[j];
            sft[i+1] = sft[i] + s;
            for( int c=0; c<s; c++ )
                tgt[ sft[i] + c ] = tem[ k+c ];
        }
    }
#ifdef PARALLEL
    {
        int len;
        len = sft.size();
        BroadcastInt( len );
        sft.resize(len);
        BroadcastIntArray( &sft[0], len );
        len = tgt.size();
        BroadcastInt( len );
        tgt.resize(len);
        BroadcastDoubleArray( &tgt[0], len );
    }
#endif

    dat.resize( tgt.size() );
    std::copy( tgt.begin(), tgt.end(), dat.begin() );
}




void PVLD_Part_Reader::
ReadElementIndex( hid_t fid,
                  const char* grpname,
                  const int ne,
                  const vector<int>& mat,
                  const vector<int>& matsft,
                  vector<int>& matidx )
{
    matidx.resize( ne );
#ifdef PARALLEL
    if( PAR_Rank()==0 )
#endif
    {
        vector<int> mdat( ne );
        ReadGroupDataSet( fid, grpname, "Material",
                          H5T_NATIVE_INT, &mdat[0] );

        vector<int> mmap( nmmat_ );
        std::fill( mmap.begin(), mmap.end(), 0 );
        for( size_t i=0; i<mat.size(); i++ )
            mmap.at( mat[i]-1 ) = i+1;

        vector<int> wlst( nmmat_ );
        std::fill( wlst.begin(), wlst.end(), 0 );

        for( int i=0; i<ne; i++ )
        {
            int mid = mdat[i];
            int ind = mmap.at( mid-1 ) - 1;
            int loc = matsft.at( ind ) + wlst[ind]++;
            matidx.at(loc) = i;
        }
    }
#ifdef PARALLEL
    {
        BroadcastIntArray( &matidx[0], ne );
    }
#endif
}
