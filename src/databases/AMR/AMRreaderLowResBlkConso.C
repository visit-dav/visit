
#include <stdlib.h>
#include <DebugStream.h>
#include <AMRreaderLowResBlkConso.h>


void AMRreaderLowResBlkConso::
init()
{
    AMRreaderLowRes::init();
    nbc_=0;
    bcsft_=NULL;
    ldata_=0;
}


int AMRreaderLowResBlkConso::
freedata()
{
    AMRreaderLowRes::freedata();
    if( bcsft_!=NULL ) free( bcsft_ );
    nbc_=0;
    bcsft_=NULL;
    ldata_=0;
    return 0;
}


int AMRreaderLowResBlkConso::
Consolidate()
{
    bcsft_ = (int*)malloc( (nlows_+1)*sizeof(int) );
    if( bcsft_==NULL )
    {
        debug1 << "Failed to allocate lowbid_ in AMRreaderLowResBlkConso::Consolidate().\n";
        return -1;
    }

    int ierr = ConsolidateBlocks( nlows_, blkdim_, blkxs_, blkdx_, &nbc_, bcsft_ );
    if( ierr!=0 )
    {
        debug1 << "Failed in ConsolidateBlocks() in AMRreaderLowResBlkConso::Consolidate().\n";
        return -1;
    }

    bcsft_ = (int*)realloc( bcsft_, (nbc_+1)*sizeof(int) );
    return 0;
}


int AMRreaderLowResBlkConso::
getInfo( const char* filename )
{
    int ierr = AMRreaderLowRes::getInfo( filename );
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes::getInfo().\n";
        return ierr;
    }

    ierr = RestrictMesh();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes::RestrictMesh().\n";
        return ierr;
    }
    lmesh_=1;

    ierr = Consolidate();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowResBlkConso::Consolidate().\n";
        return ierr;
    }

    debug5 << "Number of blocks (AMRreaderLowResBlkConso) = " << nbc_ << "\n";
    return 0;
}


int AMRreaderLowResBlkConso::
GetBlockDimensions( int bid, int* dim ) const
{
    dim[0] = blkdim_[0];
    dim[1] = blkdim_[1];
    dim[2] = blkdim_[2];
    if( bcsft_[bid]+8==bcsft_[bid+1] )
    {
        dim[0] *= 2;
        dim[1] *= 2;
        dim[2] *= 2;
    }
    return 0;
}


int AMRreaderLowResBlkConso::
GetBlockSize( int bid ) const
{
    int sz = blksz_;
    if( bcsft_[bid]+8 == bcsft_[bid+1] )  sz*=8;
    return sz;
}


int AMRreaderLowResBlkConso::
GetBlockMesh( int bid, float* xs, float* dx )
{
    int sft=3*bcsft_[bid];

    xs[0] = blkxs_[sft];
    xs[1] = blkxs_[sft+1];
    xs[2] = blkxs_[sft+2];

    dx[0] = blkdx_[sft];
    dx[1] = blkdx_[sft+1];
    dx[2] = blkdx_[sft+2];
    return 0;
}



int AMRreaderLowResBlkConso::
GetBlockVariable( int bid, int vid, float* dat )
{
    int ierr = PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed to PreprocessData() in GetBlockVariable()\n";
        return ierr;
    }

    int sz  = blksz_;
    int tid = bcsft_[bid];
    if( tid+8 == bcsft_[bid+1] ) sz*=8;

    ierr = compvar( vid, datbuf_+tid*5*blksz_, dat, sz );
    if( ierr!=0 )
    {
        debug1 << "Failed to compute requested variable: " << vid << " .\n";
        return ierr;
    }
    return 0;
}


int AMRreaderLowResBlkConso::
PreprocessData()
{
    if( ldata_==1 ) return 0;

    int ierr = AMRreaderLowRes::PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes::PreprocessData()\n";
        return ierr;
    }

    if( CombineData() != 0 )
    {
        debug1 << "Failed in CombineData().\n";
        return -2;
    }
    ldata_=1;
    return 0;
}




int AMRreaderLowResBlkConso::
CombineBlockData( int nv, float* bd[8], float* src )
{
    int lsft=0;
    for( int k=0; k<8; k++ )
        for( int j=0; j<nv*blksz_; j++ )
            bd[k][j] = src[lsft++];

    for( int k=0; k<8; k++ )
    {
        int is = kid2i[k]*blkdim_[0];
        int js = kid2j[k]*blkdim_[1];
        int ks = kid2k[k]*blkdim_[2];

        int ksft=0;
        for( int n=0; n<blkdim_[2]; n++ )
        {
            int kk = (ks+n) * 4 * blkdim_[1] * blkdim_[0];
            for( int m=0; m<blkdim_[1]; m++ )
            {
                int jj = (js+m) * 2 * blkdim_[0];
                for( int l=0; l<blkdim_[0]; l++ )
                {
                    int ii = is+l;
                    lsft = nv*( ii + jj + kk );

                    for( int v=0; v<nv; v++ )
                        src[lsft++] = bd[k][ksft++];
                }
            }
        }
    }
    return 0;
}



int AMRreaderLowResBlkConso::
CombineScalarArray( float*bd[8], float* scl )
{
    for( int i=0; i<nbc_; i++ )
    {
        int bid = bcsft_[i];
        float* src = scl + bid*blksz_;

        if( bid+8 == bcsft_[i+1] )
        {
            CombineBlockData( 1, bd, src );
        }
    }
    return 0;
}




int AMRreaderLowResBlkConso::
CombineData()
{
    float* bd[8];
    for( int k=0; k<8; k++ )
    {
        bd[k] = new float[5*blksz_];
        if( bd[k]==NULL )
        {
            debug1 << "Failed to allocate memory for temperary kids node data.\n";
            return -1;
        }
    }

    for( int i=0; i<nbc_; i++ )
    {
        int bid = bcsft_[i];
        float* src = datbuf_ + bid*5*blksz_;
        if( bid+8 == bcsft_[i+1] )
        {
            CombineBlockData( 5, bd, src );
        }
    }

    if( eos_->EOStype()==SesameEOS_type )
    {
        CombineScalarArray( bd, prebuf_ );
        CombineScalarArray( bd, sndbuf_ );
        CombineScalarArray( bd, tmpbuf_ );
    }

    for( int k=0; k<8; k++ )
        delete [] bd[k];
    return 0;
}






void AMRreaderLowRes2BlkConso::
init()
{
    AMRreaderLowRes2::init();
    nbc_=0;
    bcsft_=NULL;
    ldata_=0;
}


int AMRreaderLowRes2BlkConso::
freedata()
{
    AMRreaderLowRes2::freedata();
    if( bcsft_!=NULL ) free( bcsft_ );
    nbc_=0;
    bcsft_=NULL;
    ldata_=0;
    return 0;
}


int AMRreaderLowRes2BlkConso::
Consolidate()
{
    bcsft_ = (int*)malloc( (nlows_+1)*sizeof(int) );
    if( bcsft_==NULL )
    {
        debug1 << "Failed to allocate lowbid_ in AMRreaderLowRes2BlkConso::Consolidate().\n";
        return -1;
    }

    int ierr = ConsolidateBlocks( nlows_, blkdim_, blkxs_, blkdx_, &nbc_, bcsft_ );
    if( ierr!=0 )
    {
        debug1 << "Failed in ConsolidateBlocks() in AMRreaderLowRes2BlkConso::Consolidate().\n";
        return -1;
    }

    bcsft_ = (int*)realloc( bcsft_, (nbc_+1)*sizeof(int) );
    return 0;
}


int AMRreaderLowRes2BlkConso::
getInfo( const char* filename )
{
    int ierr = AMRreaderLowRes2::getInfo( filename );
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes2::getInfo().\n";
        return ierr;
    }

    ierr = RestrictMesh();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes2::RestrictMesh().\n";
        return ierr;
    }
    lmesh_=1;

    ierr = Consolidate();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes2BlkConso::Consolidate().\n";
        return ierr;
    }

    debug5 << "Number of blocks (AMRreaderLowRes2BlkConso) = " << nbc_ << "\n";
    return 0;
}


int AMRreaderLowRes2BlkConso::
GetBlockDimensions( int bid, int* dim ) const
{
    dim[0] = blkdim_[0];
    dim[1] = blkdim_[1];
    dim[2] = blkdim_[2];
    if( bcsft_[bid]+8==bcsft_[bid+1] )
    {
        dim[0] *= 2;
        dim[1] *= 2;
        dim[2] *= 2;
    }
    return 0;
}


int AMRreaderLowRes2BlkConso::
GetBlockSize( int bid ) const
{
    int sz = blksz_;
    if( bcsft_[bid]+8 == bcsft_[bid+1] )  sz*=8;
    return sz;
}


int AMRreaderLowRes2BlkConso::
GetBlockMesh( int bid, float* xs, float* dx )
{
    int sft=3*bcsft_[bid];

    xs[0] = blkxs_[sft];
    xs[1] = blkxs_[sft+1];
    xs[2] = blkxs_[sft+2];

    dx[0] = blkdx_[sft];
    dx[1] = blkdx_[sft+1];
    dx[2] = blkdx_[sft+2];
    return 0;
}



int AMRreaderLowRes2BlkConso::
GetBlockVariable( int bid, int vid, float* dat )
{
    int ierr = PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed to PreprocessData() in GetBlockVariable()\n";
        return ierr;
    }

    int sz  = blksz_;
    int tid = bcsft_[bid];
    if( tid+8 == bcsft_[bid+1] ) sz*=8;

    ierr = compvar( vid, datbuf_+tid*5*blksz_, dat, sz );
    if( ierr!=0 )
    {
        debug1 << "Failed to compute requested variable: " << vid << " .\n";
        return ierr;
    }
    return 0;
}


int AMRreaderLowRes2BlkConso::
PreprocessData()
{
    if( ldata_==1 ) return 0;

    int ierr = AMRreaderLowRes2::PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes2::PreprocessData()\n";
        return ierr;
    }

    if( CombineData() != 0 )
    {
        debug1 << "Failed in CombineData().\n";
        return -2;
    }
    ldata_=1;
    return 0;
}




int AMRreaderLowRes2BlkConso::
CombineBlockData( int nv, float* bd[8], float* src )
{
    int lsft=0;
    for( int k=0; k<8; k++ )
        for( int j=0; j<nv*blksz_; j++ )
            bd[k][j] = src[lsft++];

    for( int k=0; k<8; k++ )
    {
        int is = kid2i[k]*blkdim_[0];
        int js = kid2j[k]*blkdim_[1];
        int ks = kid2k[k]*blkdim_[2];

        int ksft=0;
        for( int n=0; n<blkdim_[2]; n++ )
        {
            int kk = (ks+n) * 4 * blkdim_[1] * blkdim_[0];
            for( int m=0; m<blkdim_[1]; m++ )
            {
                int jj = (js+m) * 2 * blkdim_[0];
                for( int l=0; l<blkdim_[0]; l++ )
                {
                    int ii = is+l;
                    lsft = nv*( ii + jj + kk );

                    for( int v=0; v<nv; v++ )
                        src[lsft++] = bd[k][ksft++];
                }
            }
        }
    }
    return 0;
}



int AMRreaderLowRes2BlkConso::
CombineScalarArray( float*bd[8], float* scl )
{
    for( int i=0; i<nbc_; i++ )
    {
        int bid = bcsft_[i];
        float* src = scl + bid*blksz_;

        if( bid+8 == bcsft_[i+1] )
        {
            CombineBlockData( 1, bd, src );
        }
    }
    return 0;
}




int AMRreaderLowRes2BlkConso::
CombineData()
{
    float* bd[8];
    for( int k=0; k<8; k++ )
    {
        bd[k] = new float[5*blksz_];
        if( bd[k]==NULL )
        {
            debug1 << "Failed to allocate memory for temperary kids node data.\n";
            return -1;
        }
    }

    for( int i=0; i<nbc_; i++ )
    {
        int bid = bcsft_[i];
        float* src = datbuf_ + bid*5*blksz_;
        if( bid+8 == bcsft_[i+1] )
        {
            CombineBlockData( 5, bd, src );
        }
    }

    if( eos_->EOStype()==SesameEOS_type )
    {
        CombineScalarArray( bd, prebuf_ );
        CombineScalarArray( bd, sndbuf_ );
        CombineScalarArray( bd, tmpbuf_ );
    }

    for( int k=0; k<8; k++ )
        delete [] bd[k];
    return 0;
}

