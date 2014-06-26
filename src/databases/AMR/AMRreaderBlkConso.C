#include <stdlib.h>
#include <DebugStream.h>
#include <AMRreaderBlkConso.h>

void AMRreaderBlkConso::
init()
{
    AMRreader::init();
    nbc_=0;
    bcsft_=NULL;
    ldata_=0;
}


int AMRreaderBlkConso::
freedata()
{
    AMRreader::freedata();
    if( bcsft_!=NULL ) free( bcsft_ );
    nbc_=0;
    bcsft_=NULL;
    ldata_=0;
    return 0;
}


int AMRreaderBlkConso::
FlagLowBid()
{
    bcsft_ = (int*)malloc( (nblks_+1)*sizeof(int) );
    if( bcsft_==NULL )
    {
        debug1 << "Failed to allocate lowbid_ in AMRreaderBlkConso::FlagLowBid().\n";
        return -1;
    }

    int ierr = ConsolidateBlocks( nblks_, blkdim_, blkxs_, blkdx_, &nbc_, bcsft_ );
    if( ierr!=0 )
    {
        debug1 << "Failed in ConsolidateBlocks() in AMRreaderBlkConso::FlagLowBid().\n";
        return -1;
    }

    bcsft_ = (int*)realloc( bcsft_, (nbc_+1)*sizeof(int) );
    return 0;
}


int AMRreaderBlkConso::
getInfo( const char* filename )
{
    int ierr = AMRreader::getInfo( filename );
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreader::getInfo().\n";
        return ierr;
    }

    ierr = readAMRmesh();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreader::readAMRmesh().\n";
        return ierr;
    }

    ierr = FlagLowBid();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderBlkConso::FlagLowBid().\n";
        return ierr;
    }

    debug5 << "Number of blocks (AMRreaderBlkConso) = " << nbc_ << "\n";
    return 0;
}



int AMRreaderBlkConso::
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


int AMRreaderBlkConso::
GetBlockSize( int bid ) const
{
    int sz = blksz_;
    if( bcsft_[bid]+8 == bcsft_[bid+1] )  sz*=8;
    return sz;
}



int AMRreaderBlkConso::
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

OctKey
AMRreaderBlkConso::GetBlockKey(int bid)
{
    OctKey k(blkkey_[bcsft_[bid]]);

    // If the data were combined then we shift the key right 3 to remove the
    // octree level that was combined.
//    if( bcsft_[bid]+8 == bcsft_[bid+1] )
//        k = OctKey(k.key.eb >> 3);

    return k;
}


int AMRreaderBlkConso::
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
//#define RADIAL_TEST_PATTERN
#ifdef RADIAL_TEST_PATTERN
    // Compute distance from 0,0,0 to cell centers so we return data
    // that makes sense for testing.
    int dims[3];
    GetBlockDimensions(bid, dims);

    float xs[3], dx[3];
    GetBlockMesh(bid, xs, dx);
    float *ptr = dat;
    for(int k = 0; k < dims[2]; ++k)
        for(int j = 0; j < dims[1]; ++j)
            for(int i = 0; i < dims[0]; ++i)
            {
                float x = xs[0] + (float(i) + 0.5) * dx[0];
                float y = xs[1] + (float(j) + 0.5) * dx[1];
                float z = xs[2] + (float(k) + 0.5) * dx[2];

                *ptr++ = sqrt(x*x + y*y + z*z);
            }
    return 0;
#endif
    ierr = compvar( vid, datbuf_+(tid*5*blksz_), dat, sz );
    if( ierr!=0 )
    {
        debug1 << "Failed to compute requested variable: " << vid << " .\n";
        return ierr;
    }
    return 0;
}


int AMRreaderBlkConso::
PreprocessData()
{
    if( ldata_==1 ) return 0;

    int ierr = AMRreader::PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreader::PreprocessData()\n";
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



int AMRreaderBlkConso::
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



int AMRreaderBlkConso::
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




int AMRreaderBlkConso::
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

    if( blktag_==1 )
    {
        CombineScalarArray( bd, tagbuf_ );
    }

    for( int k=0; k<8; k++ )
        delete [] bd[k];
    return 0;
}

