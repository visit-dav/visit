
#include <stdlib.h>
#include <AMRreaderCoarse.h>


const int kid2i[8] = {0,1,0,1,0,1,0,1};
const int kid2j[8] = {0,0,1,1,0,0,1,1};
const int kid2k[8] = {0,0,0,0,1,1,1,1};


int AMRreaderCoarse::
genCoarseMesh()
{
    for( int i=0; i<nagg_; i++ )
    {
        int bid = aggbid_[i];

        if( bid<0 )
        {
            bid = -(bid+1);

            blkxs_[3*i]   = blkxs_[3*bid];
            blkxs_[3*i+1] = blkxs_[3*bid+1];
            blkxs_[3*i+2] = blkxs_[3*bid+2];

            blkdx_[3*i]   = 2.*blkdx_[3*bid];
            blkdx_[3*i+1] = 2.*blkdx_[3*bid+1];
            blkdx_[3*i+2] = 2.*blkdx_[3*bid+2];
        }
        else
        {
            blkxs_[3*i]   = blkxs_[3*bid];
            blkxs_[3*i+1] = blkxs_[3*bid+1];
            blkxs_[3*i+2] = blkxs_[3*bid+2];

            blkdx_[3*i]   = blkdx_[3*bid];
            blkdx_[3*i+1] = blkdx_[3*bid+1];
            blkdx_[3*i+2] = blkdx_[3*bid+2];
        }
    }

    blkxs_  = (float*)realloc( blkxs_, (size_t)3*(size_t)nagg_*sizeof(float) );
    blkdx_  = (float*)realloc( blkdx_, (size_t)3*(size_t)nagg_*sizeof(float) );
    return 0;
}



int AMRreaderCoarse::
genCoarseData()
{
    float* bd[8];
    for( int k=0; k<8; k++ )
    {
        bd[k] = new float[bvsz_];
        if( bd[k]==NULL )
        {
            debug1 << "Failed to allocate memory for kids node data.\n";
            return -1;
        }
    }

    long sft;
    int bid,lsft;
    int is,js,ks;
    float* dbf;
    float* tbf;

    for( int i=0; i<nagg_; i++ )
    {
        bid = aggbid_[i];
        sft = aggsft_[i];
        tbf = datbuf_ + (long)i*(long)bvsz_;
        dbf = datbuf_ + sft;

        if( bid<0 )
        {
            bid = -(bid+1);

            lsft=0;
            for( int k=0; k<8; k++ )
            {
                for( int j=0; j<bvsz_; j++ )
                    bd[k][j] = dbf[lsft++];
            }

            for( int k=0; k<8; k++ )
            {
                is = kid2i[k]*(blkdim_[0]/2);
                js = kid2j[k]*(blkdim_[1]/2);
                ks = kid2k[k]*(blkdim_[2]/2);

                for( int n=0; n<blkdim_[2]/2; n++ )
                {
                    for( int m=0; m<blkdim_[1]/2; m++ )
                    {
                        for( int l=0; l<blkdim_[0]/2; l++ )
                        {
                            lsft = 5*( is+l + blkdim_[0]*( js+m + blkdim_[1]* (ks+n) ) );

                            for( int v=0; v<5; v++ )
                            {
                                tbf[lsft++] = ( bd[k][ v + 5*( 2*l   + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n  )))] +
                                                bd[k][ v + 5*( 2*l+1 + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n  )))] +
                                                bd[k][ v + 5*( 2*l   + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n  )))] +
                                                bd[k][ v + 5*( 2*l+1 + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n  )))] +
                                                bd[k][ v + 5*( 2*l   + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n+1)))] +
                                                bd[k][ v + 5*( 2*l+1 + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n+1)))] +
                                                bd[k][ v + 5*( 2*l   + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n+1)))] +
                                                bd[k][ v + 5*( 2*l+1 + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n+1)))] )
                                              *0.125;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if( !(tbf==dbf) )
            {
                for( int j=0; j<bvsz_; j++ )
                    tbf[j] = dbf[j];
            }
        }
    }

    datbuf_ = (float*)realloc( datbuf_, (size_t)bvsz_*(size_t)nagg_*sizeof(float) );

    for( int k=0; k<8; k++ )
        delete [] bd[k];

    aggdat_=1;
    return 0;
}



int AMRreaderCoarse::
genCoarseScalar( float** pdat )
{
    float* bd[8];
    for( int k=0; k<8; k++ )
    {
        bd[k] = new float[blksz_];
        if( bd[k]==NULL )
        {
            debug1 << "Failed to allocate memory for kids node data.\n";
            return -1;
        }
    }

    long sft;
    int bid,lsft;
    int is,js,ks;
    float* dbf;
    float* tbf;
    float* dat= *pdat;

    for( int i=0; i<nagg_; i++ )
    {
        bid = aggbid_[i];
        sft = aggsft_[i];
        tbf = dat + (long)i*(long)blksz_;
        dbf = dat + sft/5;

        if( bid<0 )
        {
            bid = -(bid+1);

            lsft=0;
            for( int k=0; k<8; k++ )
            {
                for( int j=0; j<blksz_; j++ )
                    bd[k][j] = dbf[lsft++];
            }

            for( int k=0; k<8; k++ )
            {
                is = kid2i[k]*(blkdim_[0]/2);
                js = kid2j[k]*(blkdim_[1]/2);
                ks = kid2k[k]*(blkdim_[2]/2);

                for( int n=0; n<blkdim_[2]/2; n++ )
                {
                    for( int m=0; m<blkdim_[1]/2; m++ )
                    {
                        for( int l=0; l<blkdim_[0]/2; l++ )
                        {
                            lsft = is+l + blkdim_[0]*( js+m + blkdim_[1]* (ks+n) );
                            tbf[lsft] = ( bd[k][ 2*l   + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n  ))] +
                                          bd[k][ 2*l+1 + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n  ))] +
                                          bd[k][ 2*l   + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n  ))] +
                                          bd[k][ 2*l+1 + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n  ))] +
                                          bd[k][ 2*l   + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n+1))] +
                                          bd[k][ 2*l+1 + blkdim_[0]*( 2*m   + blkdim_[1]*(2*n+1))] +
                                          bd[k][ 2*l   + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n+1))] +
                                          bd[k][ 2*l+1 + blkdim_[0]*( 2*m+1 + blkdim_[1]*(2*n+1))] )
                                        *0.125;
                        }
                    }
                }
            }
        }
        else
        {
            if( !(tbf==dbf) )
            {
                for( int j=0; j<blksz_; j++ )
                    tbf[j] = dbf[j];
            }
        }
    }

    *pdat = (float*)realloc( dat, (size_t)bvsz_*(size_t)nagg_*sizeof(float) );

    for( int k=0; k<8; k++ )
        delete [] bd[k];

    return 0;
}



int AMRreaderCoarse::
genCoarseAdditionData()
{
    if( genCoarseScalar( &prebuf_ )!=0 )
    {
        debug1 << "Failed to aggregate pressure variable\n";
        return -1;
    }
    if( genCoarseScalar( &sndbuf_ )!=0 )
    {
        debug1 << "Failed to aggregate soundspeed variable\n";
        return -2;
    }
    if( genCoarseScalar( &tmpbuf_ )!=0 )
    {
        debug1 << "Failed to aggregate temperature variable\n";
        return -3;
    }
    return 0;
}




int AMRreaderCoarse::
GetBlockMesh( int bid, float* xs, float* dx )
{
    if( blkxs_==NULL )
    {
        if( readAMRmesh()!=0 )
        {
            debug1 << "Failed to read AMR mesh.\n";
            return -1;
        }
        genCoarseMesh();
    }

    // int tid = aggbid_[bid];
    // if( tid<0 ) { tid = -(tid+1); }

    // int sft=3*tid;
    // xs[0] = blkxs_[sft];
    // xs[1] = blkxs_[sft+1];
    // xs[2] = blkxs_[sft+2];

    // dx[0] = blkdx_[sft];
    // dx[1] = blkdx_[sft+1];
    // dx[2] = blkdx_[sft+2];

    // if( aggbid_[bid]<0 ) {
    //   dx[0]*=2.;
    //   dx[1]*=2.;
    //   dx[2]*=2.;
    // }
    // return 0;
    return AMRreader::GetBlockMesh( bid, xs, dx );
}




int AMRreaderCoarse::
GetBlockVariable( int bid, int vid, float* dat )
{
    //debug5 << "AMRreaderAgg::GetBlockVariable( " << bid << ", " << vid << " )" << endl;
    if( datbuf_==NULL )
    {
        if( readAMRdata()!=0 )
        {
            debug1 << "Failed to read in AMR data.\n";
            return -1;
        }
        if( eos_->EOStype()==SesameEOS_type )
        {
            if( readAMRadditionData()!=0 )
            {
                debug1 << "Failed to read in AMR additional data.\n";
                return -1;
            }
        }
    }

    //debug5 << "AMRreaderAgg::genAggAdditionData()\n";
    if( aggdat_==0 )
    {
        if( genCoarseData()!=0 )
        {
            debug1 << "Failed to aggregate blocks in AMR data.\n";
            return -2;
        }
        if( eos_->EOStype()==SesameEOS_type )
        {
            if( genCoarseAdditionData()!=0 )
            {
                debug1 << "Failed to aggregate blocks in additional AMR data.\n";
                return -2;
            }
        }
    }

    return AMRreader::GetBlockVariable( bid, vid, dat );
}
