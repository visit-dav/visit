
#include <stdlib.h>
#include <DebugStream.h>
#include <AMRreaderLowRes.h>


void AMRreaderLowRes::
init()
{
    AMRreader::init();
    nlows_=0;
    lowbid_=NULL;
    lmesh_=ldata_=0;
}


int AMRreaderLowRes::
freedata()
{
    AMRreader::freedata();
    if( lowbid_!=NULL ) free( lowbid_ );
    nlows_=0;
    lowbid_=NULL;
    lmesh_=ldata_=0;
    return 0;
}



int AMRreaderLowRes::
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
        debug1 << "Failed in AMRreaderLowRes::FlagLowBid().\n";
        return ierr;
    }

    debug5 << "Number of blocks (AMRreaderLowRes) = " << nlows_ << "\n";
    return 0;
}



int AMRreaderLowRes::
FlagLowBid()
{
    lowbid_ = (int*)malloc( (nblks_+1)*sizeof(int) );
    if( lowbid_==NULL )
    {
        debug1 << "Failed to allocate lowbid_ in AMRreaderLowRes::FlagLowBid().\n";
        return -1;
    }

    int ierr = ConsolidateBlocks( nblks_, blkdim_, blkxs_, blkdx_, &nlows_, lowbid_ );
    if( ierr!=0 )
    {
        debug1 << "Failed in ConsolidateBlocks() in AMRreaderLowRes::FlagLowBid().\n";
        return -1;
    }

    lowbid_ = (int*)realloc( lowbid_, (nlows_+1)*sizeof(int) );
    return 0;
}


int AMRreaderLowRes::
RestrictBlockData( int nv, float* src, float* trg )
{
    for( int k=0; k<8; k++ )
    {
        float* bd = src + k*nv*blksz_;

        int is = kid2i[k]*(blkdim_[0]/2);
        int js = kid2j[k]*(blkdim_[1]/2);
        int ks = kid2k[k]*(blkdim_[2]/2);

        for( int n=0; n<blkdim_[2]/2; n++ )
            for( int m=0; m<blkdim_[1]/2; m++ )
                for( int l=0; l<blkdim_[0]/2; l++ )
                {
                    int lsft = nv*( is+l + blkdim_[0]*( js+m + blkdim_[1]* (ks+n) ) );

                    for( int v=0; v<nv; v++ )
                    {
                        trg[lsft++] = ( bd[ v + nv*( 2*l   + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n  )))] +
                                        bd[ v + nv*( 2*l+1 + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n  )))] +
                                        bd[ v + nv*( 2*l   + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n  )))] +
                                        bd[ v + nv*( 2*l+1 + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n  )))] +
                                        bd[ v + nv*( 2*l   + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n+1)))] +
                                        bd[ v + nv*( 2*l+1 + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n+1)))] +
                                        bd[ v + nv*( 2*l   + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n+1)))] +
                                        bd[ v + nv*( 2*l+1 + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n+1)))] )
                                      *0.125;
                    }
                }
    }
    return 0;
}



int AMRreaderLowRes::
RestrictScalarArray( float** scl )
{
    for( int i=0; i<nlows_; i++ )
    {
        int bid = lowbid_[i];
        float* trg = *scl +   i*5*blksz_;
        float* src = *scl + bid*5*blksz_;

        if( bid+8 == lowbid_[i+1] )
        {
            RestrictBlockData( 1, src, trg  );
        }
        else
        {
            if( i!=bid )
            {
                for( int j=0; j<blksz_; j++ )
                    trg[j] = src[j];
            }
        }
    }
    *scl = (float*)realloc( *scl, nlows_*blksz_*sizeof(float) );
    return 0;
}



int AMRreaderLowRes::
RestrictMesh()
{
    for( int i=0; i<nlows_; i++ )
    {
        int bid = lowbid_[i];
        int sft = 3*bid;
        int tft = 3*i;

        if( i!=bid )
        {
            blkxs_[tft]  = blkxs_[sft];
            blkxs_[tft+1]= blkxs_[sft+1];
            blkxs_[tft+2]= blkxs_[sft+2];

            blkdx_[tft]  = blkdx_[sft];
            blkdx_[tft+1]= blkdx_[sft+1];
            blkdx_[tft+2]= blkdx_[sft+2];
        }
        if( bid+8 == lowbid_[i+1] )
        {
            blkdx_[tft]  *= 2.;
            blkdx_[tft+1]*= 2.;
            blkdx_[tft+2]*= 2.;
        }
    }

    blkxs_ = (float*)realloc( blkxs_, 3*nlows_*sizeof(float) );
    blkdx_ = (float*)realloc( blkdx_, 3*nlows_*sizeof(float) );
    return 0;
}




int AMRreaderLowRes::
RestrictData()
{
    for( int i=0; i<nlows_; i++ )
    {
        int bid = lowbid_[i];
        float* trg = datbuf_ +   i*5*blksz_;
        float* src = datbuf_ + bid*5*blksz_;

        if( bid+8 == lowbid_[i+1] )
        {
            RestrictBlockData( 5, src, trg );
        }
        else
        {
            if( i!=bid )
            {
                for( int j=0; j<5*blksz_; j++ )
                    trg[j] = src[j];
            }
        }
    }
    datbuf_ = (float*)realloc( datbuf_, nlows_*5*blksz_*sizeof(float) );

    if( eos_->EOStype()==SesameEOS_type )
    {
        RestrictScalarArray( &prebuf_ );
        RestrictScalarArray( &sndbuf_ );
        RestrictScalarArray( &tmpbuf_ );
    }
    return 0;
}






int AMRreaderLowRes::
GetBlockMesh( int bid, float* xs, float* dx )
{
    if( lmesh_==0 )
    {
        if( RestrictMesh() !=0 )
        {
            debug1 << "Failed in RestrictMesh().\n";
            return -1;
        }
        lmesh_=1;
    }

    int sft=3*bid;

    xs[0] = blkxs_[sft];
    xs[1] = blkxs_[sft+1];
    xs[2] = blkxs_[sft+2];

    dx[0] = blkdx_[sft];
    dx[1] = blkdx_[sft+1];
    dx[2] = blkdx_[sft+2];
    return 0;
}



int AMRreaderLowRes::
GetBlockVariable( int bid, int vid, float* dat )
{
    int ierr = PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed to PreprocessData() in GetBlockVariable()\n";
        return ierr;
    }

    ierr = compvar( vid, datbuf_+(5*blksz_*bid), dat, blksz_ );
    if( ierr!=0 )
    {
        debug1 << "Failed to compute requested variable: " << vid << " .\n";
        return ierr;
    }
    return 0;
}


int AMRreaderLowRes::
PreprocessData()
{
    if( ldata_==1 ) return 0;

    int ierr = AMRreader::PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreader::PreprocessData()\n";
        return ierr;
    }

    if( RestrictData() != 0 )
    {
        debug1 << "Failed in RestricData() in PreprocessData().\n";
        return -2;
    }
    ldata_=1;
    return 0;
}





void AMRreaderLowRes2::
init()
{
    AMRreaderLowRes::init();
    nlows_=0;
    lowbid_=NULL;
    lmesh_=ldata_=0;
}


int AMRreaderLowRes2::
freedata()
{
    AMRreaderLowRes::freedata();
    if( lowbid_!=NULL ) free( lowbid_ );
    nlows_=0;
    lowbid_=NULL;
    lmesh_=ldata_=0;
    return 0;
}



int AMRreaderLowRes2::
getInfo( const char* filename )
{
    int ierr = AMRreaderLowRes::getInfo( filename );
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes2::getInfo().\n";
        return ierr;
    }

    ierr = AMRreaderLowRes::RestrictMesh();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes::RestrictMesh()\n";
        return -2;
    }
    AMRreaderLowRes::lmesh_=1;

    ierr = FlagLowBid();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes2::FlagLowBid().\n";
        return ierr;
    }

    debug5 << "Number of blocks (AMRreaderLowRes2) = " << nlows_ << "\n";
    return 0;
}



int AMRreaderLowRes2::
FlagLowBid()
{
    lowbid_ = (int*)malloc( (AMRreaderLowRes::nlows_+1)*sizeof(int) );
    if( lowbid_==NULL )
    {
        debug1 << "Failed to allocate lowbid_ in AMRreaderLowRes2::FlagLowBid().\n";
        return -1;
    }

    int ierr = ConsolidateBlocks( AMRreaderLowRes::nlows_, blkdim_, blkxs_, blkdx_, &nlows_, lowbid_ );
    if( ierr!=0 )
    {
        debug1 << "Failed in ConsolidateBlocks() in AMRreaderLowRes2::FlagLowBid().\n";
        return -3;
    }

    lowbid_ = (int*)realloc( lowbid_, (nlows_+1)*sizeof(int) );
    return 0;
}



int AMRreaderLowRes2::
RestrictMesh()
{
    for( int i=0; i<nlows_; i++ )
    {
        int bid = lowbid_[i];
        int sft = 3*bid;
        int tft = 3*i;

        if( i!=bid )
        {
            blkxs_[tft]  = blkxs_[sft];
            blkxs_[tft+1]= blkxs_[sft+1];
            blkxs_[tft+2]= blkxs_[sft+2];

            blkdx_[tft]  = blkdx_[sft];
            blkdx_[tft+1]= blkdx_[sft+1];
            blkdx_[tft+2]= blkdx_[sft+2];
        }
        if( bid+8 == lowbid_[i+1] )
        {
            blkdx_[tft]  *= 2.;
            blkdx_[tft+1]*= 2.;
            blkdx_[tft+2]*= 2.;
        }
    }

    blkxs_ = (float*)realloc( blkxs_, 3*nlows_*sizeof(float) );
    blkdx_ = (float*)realloc( blkdx_, 3*nlows_*sizeof(float) );
    return 0;
}



int AMRreaderLowRes2::
RestrictData()
{
    for( int i=0; i<nlows_; i++ )
    {
        int bid = lowbid_[i];
        float* trg = datbuf_ +   i*5*blksz_;
        float* src = datbuf_ + bid*5*blksz_;

        if( bid+8 == lowbid_[i+1] )
        {
            RestrictBlockData( 5, src, trg );
        }
        else
        {
            if( i!=bid )
            {
                for( int j=0; j<5*blksz_; j++ )
                    trg[j] = src[j];
            }
        }
    }
    datbuf_ = (float*)realloc( datbuf_, nlows_*5*blksz_*sizeof(float) );

    if( eos_->EOStype()==SesameEOS_type )
    {
        RestrictScalarArray( &prebuf_ );
        RestrictScalarArray( &sndbuf_ );
        RestrictScalarArray( &tmpbuf_ );
    }
    return 0;
}



int AMRreaderLowRes2::
GetBlockMesh( int bid, float* xs, float* dx )
{
    if( lmesh_==0 )
    {
        if( RestrictMesh() !=0 )
        {
            debug1 << "Failed in RestrictMesh().\n";
            return -1;
        }
        lmesh_=1;
    }

    int sft=3*bid;

    xs[0] = blkxs_[sft];
    xs[1] = blkxs_[sft+1];
    xs[2] = blkxs_[sft+2];

    dx[0] = blkdx_[sft];
    dx[1] = blkdx_[sft+1];
    dx[2] = blkdx_[sft+2];
    return 0;
}



int AMRreaderLowRes2::
PreprocessData()
{
    if( ldata_==1 ) return 0;

    int ierr = AMRreaderLowRes::PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed in AMRreaderLowRes::PreprocessData()\n";
        return ierr;
    }

    if( RestrictData() != 0 )
    {
        debug1 << "Failed in RestricData() in PreprocessData().\n";
        return -2;
    }
    ldata_=1;
    return 0;
}


int AMRreaderLowRes2::
GetBlockVariable( int bid, int vid, float* dat )
{
    int ierr = PreprocessData();
    if( ierr!=0 )
    {
        debug1 << "Failed to PreprocessData() in GetBlockVariable()\n";
        return ierr;
    }

    ierr = compvar( vid, datbuf_+(5*blksz_*bid), dat, blksz_ );
    if( ierr!=0 )
    {
        debug1 << "Failed to compute requested variable: " << vid << " .\n";
        return ierr;
    }
    return 0;
}
