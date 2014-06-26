
#include <algorithm>

#include <Partition.h>
#include <DebugStream.h>


class Mat
{
public:
    int ind,wei;
    Mat() : ind(0),wei(0) {}
};

class MatCompare
{
public:
    bool operator()( const Mat& m1, const Mat& m2 )
    {
        return m1.wei >= m2.wei;
    }
};

class Prt
{
public:
    int wei;
    vector<int> clt;
    Prt() : wei(0), clt() {}
};

class PrtCompare
{
public:
    bool operator()( const Prt& m1, const Prt& m2 )
    {
        return m1.wei < m2.wei;
    }
};



void PartitionWeights( const vector<int>& wlst,
                       int npart, vector< vector<int> >& part )
{
    vector<Mat> mlst;
    int cnt=0;
    for( vector<int>::const_iterator iter=wlst.begin(); iter!=wlst.end(); iter++ )
    {
        Mat smp;
        smp.ind = cnt++;
        smp.wei = *iter;
        mlst.push_back( smp );
    }
    //std::sort( mlst.begin(), mlst.end(), MatCompare() );
    std::stable_sort( mlst.begin(), mlst.end(), MatCompare() );

    // greedy algorithm for partition
    vector<Prt> plst( npart );
    vector<Mat>::const_iterator mloc = mlst.begin();
    for( vector<Prt>::iterator iter=plst.begin(); iter!=plst.end(); ++iter )
        if( mloc != mlst.end() )
        {
            iter->wei += mloc->wei;
            iter->clt.push_back( mloc->ind );
            ++mloc;
        }
    while( mloc != mlst.end() )
    {
        vector<Prt>::iterator ploc = std::min_element( plst.begin(), plst.end(), PrtCompare() );
        ploc->wei += mloc->wei;
        ploc->clt.push_back( mloc->ind );
        ++mloc;
    }

    part.clear();
    for( vector<Prt>::const_iterator iter=plst.begin(); iter!=plst.end(); ++iter )
        if( !(iter->clt.empty()) )
            part.push_back( iter->clt );
}


