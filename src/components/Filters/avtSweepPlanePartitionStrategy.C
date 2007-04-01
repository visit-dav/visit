// ************************************************************************* //
//                       avtSweepPlanePartitionStrategy.C                    //
// ************************************************************************* //

#include <avtSweepPlanePartitionStrategy.h>


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy constructor
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

avtSweepPlanePartitionStrategy::avtSweepPlanePartitionStrategy()
{
    dist = NULL;
    jDist = NULL;
    haveDistance = NULL;
    isAvailable = NULL;
    proposedGridIndex = NULL;
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy destructor
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

avtSweepPlanePartitionStrategy::~avtSweepPlanePartitionStrategy()
{
    if (dist != NULL)
        delete [] dist;
    if (jDist != NULL)
        delete [] jDist;
    if (haveDistance != NULL)
        delete [] haveDistance;
    if (isAvailable != NULL)
        delete [] isAvailable;
    if (proposedGridIndex != NULL)
        delete [] proposedGridIndex;
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::ConstructPartition
//
//  Purpose:
//      Constructs a partition for the structured mesh chunker, using a 
//      sweep plane strategy.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2005
//
// ****************************************************************************

void
avtSweepPlanePartitionStrategy::ConstructPartition(const int *cell_dims ,
                             avtStructuredMeshChunker::ZoneDesignation *d_plus,
                             std::vector<int> &boxes)
{
    boxes.clear();

    // Only deal with real 3D structured meshes.
    if (cell_dims[0] <= 1 || cell_dims[1] <= 1 || cell_dims[2] <= 1)
        return;

    Initialize(cell_dims, d_plus);

    bool foundGrid = true;
    while (foundGrid)
    {
        foundGrid = GreedyFindGrids(boxes);
        for (int i = 0 ; i < plane_size ; i++)
            dist[i] = 0;
    }
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::Initialize
//
//  Purpose:
//      Initializes data members for sweeping planes.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2005
//
// ****************************************************************************

void
avtSweepPlanePartitionStrategy::Initialize(const int *d, 
                                 avtStructuredMeshChunker::ZoneDesignation *zd)
{
    int  i;

    if (dist != NULL)
        delete [] dist;
    if (jDist != NULL)
        delete [] jDist;
    if (haveDistance != NULL)
        delete [] haveDistance;
    if (isAvailable != NULL)
        delete [] isAvailable;
    if (proposedGridIndex != NULL)
        delete [] proposedGridIndex;

    dims[0] = d[0];
    dims[1] = d[1];
    dims[2] = d[2];

    plane_size = dims[0]*dims[1];
    line_size = dims[0];
    line_depth = dims[1];
    sweep_depth = dims[2];

    dist = new int[plane_size];
    for (i = 0 ; i < plane_size ; i++)
        dist[i] = 0;

    jDist = new int[line_size];
    for (i = 0 ; i < line_size ; i++)
        jDist[i] = 0;

    haveDistance = new bool[sweep_depth+1];
    for (i = 0 ; i < sweep_depth+1 ; i++)
        haveDistance[i] = false;

    int num_entries = dims[0]*dims[1]*dims[2];
    isAvailable = new bool[num_entries];
    proposedGridIndex = new int[num_entries];
    for (i = 0 ; i < num_entries ; i++)
    {
        isAvailable[i] = (zd[i] == avtStructuredMeshChunker::RETAIN 
                           ? true : false);
        proposedGridIndex[i] = -1;
    }
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::GreedyFindGrids
//
//  Purpose:
//      Employs a greedy algorithm to find boxes.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2005
//
// ****************************************************************************

bool
avtSweepPlanePartitionStrategy::GreedyFindGrids(std::vector<int> &boxes)
{
    int   k, k2;

    //
    // Sweep K-planes through the data set.
    //
    for (k = 0 ; k < sweep_depth ; k++)
    {
        for (k2 = 0 ; k2 < sweep_depth ; k2++)
            haveDistance[k] = false;

        //
        // We want to calculate "dist".  For some (i,j), which gets mapped
        // index idx, dist[idx] = the number of zones we can walk past the
        // sweep plane and still see "retained" zones.  So if dist[idx] = 5,
        // then we know that isAvailable[i][j][k] is true, 
        // isAvailable[i][j][k+1] is true, and so on through 
        // isAvailable[i][j][k+4].
        //
        bool layoutHasChanged = CalculateRunLength(k);
        if (!layoutHasChanged)
            continue;

        FindBiggestGrids(k);
    }

    bool keepGoing = CommitProposedGrids(boxes);
    return keepGoing;
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::FindBiggestGrids
//
//  Purpose:
//      Finds the biggest grids starting at sweep-plane K=k.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2005
//
// ****************************************************************************

void
avtSweepPlanePartitionStrategy::FindBiggestGrids(int k)
{
    // For each "distance", determine the biggest box.
    for (int d = sweep_depth ; d > 0 ; d--)
    {
        if (!haveDistance[d])
            continue;
        FindBiggestGridWithDistance(d, k);
    }
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::FindBiggestGridWithDistance
//
//  Purpose:
//      Finds the biggest grids starting at sweep-plane K=k and having distance
//      d.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2005
//
// ****************************************************************************

void
avtSweepPlanePartitionStrategy::FindBiggestGridWithDistance(int d, int k)
{
    int i, j, j2;

    for (i = 0 ; i < line_size ; i++)
        jDist[i] = 0;

    // Now we have to sweep a line to find the biggest box of all
    // distance d or greater.
    bool *haveDistance2 = new bool[line_depth+1];
    for (j = 0 ; j < line_depth ; j++)
    {
        for (j2 = 0 ; j2 < line_depth+1 ; j2++)
            haveDistance2[j2] = false;
        for (i = 0 ; i < line_size ; i++)
        {
            if (jDist[i] > 0)
            {
                jDist[i]--;
            }
            else
            {
                bool allValid = true;
                jDist[i] = 0;
                for (int d2 = j ; d2 < line_depth && allValid ; d2++)
                {
                    int idx = d2*dims[0] + i;
                    if (dist[idx] >= d)
                        jDist[i]++;
                    else
                        allValid = false;
                }
            }
            haveDistance2[jDist[i]] = true;
        }

        for (j2 = line_depth ; j2 > 0  ; j2--)
        {
            if (!haveDistance2[j2])
                continue;
            int runStart = -1;
            bool inRun = false;
            for (i = 0 ; i < line_size ; i++)
            {
                if (jDist[i] >= j2)
                {
                    if (!inRun)
                    {
                        inRun = true;
                        runStart = i;
                    }
                }
                else
                {
                    if (inRun)
                    {
                        inRun = false;
                        ProposeGrid(runStart, i-1, j, j+(j2-1), k, k+d-1);
                    }
                }
            }
            if (inRun)
            {
                inRun = false;
                ProposeGrid(runStart, line_size-1, j, j+(j2-1), k, k+d-1);
            }
        }
    }
    delete [] haveDistance2;
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::ProposeGrid
//
//  Purpose:
//      Suggest a possible grid.
//
//  Programmer: Hank Childs
//  Creation:   March 20, 2005
//
// ****************************************************************************

void
avtSweepPlanePartitionStrategy::ProposeGrid(int iStart, int iEnd, int jStart,
                                             int jEnd, int kStart, int kEnd)
{
    int gridSize = (iEnd-iStart+1)*(jEnd-jStart+1)*(kEnd-kStart+1);
    if (gridSize < minimumSize)
        return;

    //
    // Determine if this grid is in conflict with another grid.  If so,
    // then determine which is bigger.  If there is a bigger pre-existing
    // grid, give up on this candidate.  If this candidate is bigger than
    // all previously proposed grids, then make this be a candidate and 
    // remove all the previous candidates.
    //
    std::vector<bool> removeGrid(proposedGrid.size(), false);
    for (int i = iStart ; i <= iEnd ; i++)
        for (int j = jStart ; j <= jEnd ; j++)
            for (int k = kStart ; k <= kEnd ; k++)
            {
                int idx = k*plane_size + j*dims[0] + i;
                int gridIndex = proposedGridIndex[idx];
                if (gridIndex >= 0)
                {
                    GridCandidate &gc = proposedGrid[gridIndex];
                    if (gc.gridSize >= gridSize)
                        // The existing grid is bigger than us.
                        return;
                    else
                        removeGrid[gridIndex] = true;
                }
            }

    //
    // Remove all the grids that have conflict with our grid.
    //
    for (int g = 0 ; g < removeGrid.size() ; g++)
        if (removeGrid[g])
        {
            GridCandidate &gc = proposedGrid[g];
            gc.stillCandidate = false;
            for (int i = gc.iStart ; i <= gc.iEnd ; i++)
                for (int j = gc.jStart ; j <= gc.jEnd ; j++)
                    for (int k = gc.kStart ; k <= gc.kEnd ; k++)
                    {
                        int idx = k*plane_size + j*dims[0] + i;
                        proposedGridIndex[idx] = -1;
                    }
        }

    //
    // Now make the new grid an official candidate.
    //
    GridCandidate gc;
    gc.iStart = iStart;
    gc.iEnd   = iEnd;
    gc.jStart = jStart;
    gc.jEnd   = jEnd;
    gc.kStart = kStart;
    gc.kEnd   = kEnd;
    gc.gridSize = gridSize;
    gc.stillCandidate = true;
    proposedGrid.push_back(gc);
    int gridIndex = proposedGrid.size() - 1;
    for (int i = iStart ; i <= iEnd ; i++)
        for (int j = jStart ; j <= jEnd ; j++)
            for (int k = kStart ; k <= kEnd ; k++)
            {
                int idx = k*plane_size + j*dims[0] + i;
                proposedGridIndex[idx] = gridIndex;
            }
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::CommitProposedGrids
//
//  Purpose:
//      Take all active proposed candidates and commit them.
//
//  Programmer: Hank Childs
//  Creation:   March 20, 2005
//
// ****************************************************************************

bool
avtSweepPlanePartitionStrategy::CommitProposedGrids(std::vector<int> &boxes)
{
    bool haveGrids = false;
    for (int g = 0 ; g < proposedGrid.size() ; g++)
    {
        GridCandidate &gc = proposedGrid[g];
        if (!gc.stillCandidate)
            continue;
        haveGrids = true;
        boxes.push_back(gc.iStart);
        boxes.push_back(gc.iEnd);
        boxes.push_back(gc.jStart);
        boxes.push_back(gc.jEnd);
        boxes.push_back(gc.kStart);
        boxes.push_back(gc.kEnd);
        for (int i = gc.iStart ; i <= gc.iEnd ; i++)
            for (int j = gc.jStart ; j <= gc.jEnd ; j++)
                for (int k = gc.kStart ; k <= gc.kEnd ; k++)
                {
                    int idx = k*plane_size + j*dims[0] + i;
                    proposedGridIndex[idx] = -1;
                    isAvailable[idx] = false;
                }
    }
    proposedGrid.clear();

    return haveGrids;
}


// ****************************************************************************
//  Method: avtSweepPlanePartitionStrategy::CalculateRunLength
//
//  Purpose:
//      We want to calculate "dist".  For some (i,j), which gets mapped
//      index idx, dist[idx] = the number of zones we can walk past the
//      sweep plane and still see "retained" zones.  So if dist[idx] = 5,
//      then we know that isAvailable[i][j][k] is true, 
//      isAvailable[i][j][k+1] is true, and so on through 
//      isAvailable[i][j][k+4].
//
//  Note:  Although one call of this method can take O(n), the amortized
//         cost for all sweep_depth planes is also O(n).
//
//  Programmer: Hank Childs
//  Creation:   March 20, 2005
//
// ****************************************************************************

bool
avtSweepPlanePartitionStrategy::CalculateRunLength(int k)
{
    bool newLayout = false;

    for (int k2 = 0 ; k2 < sweep_depth+1 ; k2++)
        haveDistance[k2] = false;

    for (int p = 0 ; p < plane_size ; p++)
    {
        //
        // If we can, just use last iterations distance.  We moved one
        // unit in K, so the distance is one less.
        //
        if (dist[p] > 0)
            dist[p]--;
        else
        {
            bool allValid = true;
            dist[p] = 0;
            for (int d = k ; d < sweep_depth && allValid ; d++)
            {
                int idx = d*plane_size + p;
                if (isAvailable[idx])
                    dist[p]++;
                else
                    allValid = false;
            }
            if (dist[p] != 0)
                newLayout = true;
        }
        haveDistance[dist[p]] = true;
    }

    return newLayout;
}


