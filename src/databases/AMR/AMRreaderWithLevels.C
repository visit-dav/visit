#include <AMRreaderWithLevels.h>

#include <algorithm>
#include <math.h>

AMRreaderWithLevels::AMRreaderWithLevels() : AMRreaderInterface(), patchesBuilt(false), patches(), blkkey2bid()
{
    reader = InstantiateAMRreader();
}

AMRreaderWithLevels::~AMRreaderWithLevels()
{
    delete reader;
}

int
AMRreaderWithLevels::freedata()
{
    patchesBuilt = false;
    patches.clear();
    blkkey2bid.clear();
    return reader->freedata();
}

int
AMRreaderWithLevels::getInfo( const char* filename )
{
    return reader->getInfo(filename);
}

void
AMRreaderWithLevels::BuildMetaData()
{
    // Build an octree
    if(!patchesBuilt)
    {
        MakeOctTree();
        patchesBuilt = true;
    }
}

// ****************************************************************************
// Method: AMRreaderWithLevels::FindBlock
//
// Purpose:
// Scans through the list of blocks from the file for a particular block key and
// returns the bid for the block with that key in the file.
//
// Arguments:
//   key : The block key.
//
// Returns:    The index of the block having the specified key or -1 if it could
//             not be found.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:11:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AMRreaderWithLevels::FindBlock(OctKey key)
{
    int nb = reader->GetNumberOfBlocks();
    for(int i = 0; i < nb; ++i)
    {
        if(OctKey_Equal(reader->GetBlockKey(i), key))
            return i;
    }
    return -1;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::GetNumberOfLevels
//
// Purpose:
//   Returns the number of levels in the AMR tree.
//
// Arguments:
//
//
// Returns:    The number of levels.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:12:17 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AMRreaderWithLevels::GetNumberOfLevels()
{
    int nb, level = 0;
    if((nb = reader->GetNumberOfBlocks()) > 0)
    {
        level = OctKey_NumLevels(reader->GetBlockKey(0));
        for(int bid = 1; bid < nb; ++bid)
        {
            int thisLevel = OctKey_NumLevels(reader->GetBlockKey(bid));
            level = std::max(level, thisLevel);
        }
    }
    return level;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::MakeOctTree
//
// Purpose:
//   Entry function for building an octree with intermediate patches that we
//   need for AMR with VisIt. The leaves correspond to blocks from the file.
//
// Arguments:
//
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:12:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
AMRreaderWithLevels::MakeOctTree()
{
    // Get the size of the entire box using the first and last blocks from the file.
    int dim[3];
    float xs[3], dx[3];
    float xMin,  yMin, zMin, xMax, yMax, zMax;
    reader->GetBlockMesh(0, xs, dx);
    xMin = xs[0];
    yMin = xs[1];
    zMin = xs[2];

    int lastbid = reader->GetNumberOfBlocks()-1;
    reader->GetBlockMesh(lastbid, xs, dx);
    reader->GetBlockDimensions(lastbid, dim);
    xMax = xs[0] + dim[0] * dx[0];
    yMax = xs[1] + dim[1] * dx[1];
    zMax = xs[2] + dim[2] * dx[2];

    // Make the root
    Patch p;
    p.level = 0;
    p.ijk_start[0] = 0;
    p.ijk_start[1] = 0;
    p.ijk_start[2] = 0;
    p.xs[0] = xMin;
    p.xs[1] = yMin;
    p.xs[2] = zMin;
    p.xe[0] = xMax;
    p.xe[1] = yMax;
    p.xe[2] = zMax;
    p.key = OctKey_Root();
    p.fileBID = -1;
    patches.push_back(p);

    // Determine the maximum number of levels that we need to make.
    int maxLevels = GetNumberOfLevels() - 1;

    // Subdivide
    first_subdivide8(xMin, yMin, zMin, xMax, yMax, zMax, p.key, maxLevels);

    // Make a map of bid to key.
    for(int i = 0; i < (int) patches.size(); ++i)
        blkkey2bid[patches[i].key] = i;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::first_subdivide8
//
// Purpose:
//   This is convenience function to subdivide the first level of the octree
//   but we don't save patches for it.
//
// Arguments:
//
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:13:41 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
AMRreaderWithLevels::first_subdivide8(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax, OctKey current, int maxLevels)
{
    float halfX = (xMax - xMin) * 0.5f;
    float halfY = (yMax - yMin) * 0.5f;
    float halfZ = (zMax - zMin) * 0.5f;

    int dims[3];
    GetBlockDefaultDimensions(dims);

    int cell = 0;
    for(int k = 0; k < 2; ++k)
    {
        for(int j = 0; j < 2; ++j)
        {
            for(int i = 0; i < 2; ++i)
            {
                Patch p;
                p.xs[0] = xMin + float(i) * halfX;
                p.xs[1] = yMin + float(j) * halfY;
                p.xs[2] = zMin + float(k) * halfZ;
                p.xe[0] = xMin + float(i+1) * halfX;
                p.xe[1] = yMin + float(j+1) * halfY;
                p.xe[2] = zMin + float(k+1) * halfZ;
                p.key = OctKey_AddLevel(current, cell);
                cell++;

                // The start/end window for this area in level 1.
                int ijk_start[3], ijk_end[3];
                ijk_start[0] = i * dims[0]*2;
                ijk_start[1] = j * dims[1]*2;
                ijk_start[2] = k * dims[2]*2;
                ijk_end[0]   = (i+1) * dims[0]*2 - 1;
                ijk_end[1]   = (j+1) * dims[1]*2 - 1;
                ijk_end[2]   = (k+1) * dims[2]*2 - 1;

                subdivide8(ijk_start, ijk_end, p.xs[0], p.xs[1], p.xs[2], p.xe[0], p.xe[1], p.xe[2], p.key, 1, maxLevels);
            }
        }
    }
}

// ****************************************************************************
// Method: AMRreaderWithLevels::subdivide8
//
// Purpose:
//   Subdivides the current octree cell recursively and adds patches for all
//   of the levels until the subdivision terminates at leaf blocks from the
//   reader.
//
// Arguments:
//   ijk_start : The logical start indices for the block at the current level.
//   ijk_end   : The logical end indices for the block at the current level.
//   xMin      : The min X of the block.
//   yMin      : The min Y of the block.
//   zMin      : The min Z of the block.
//   xMax      : The max X of the block.
//   yMax      : The max Y of the block.
//   zMax      : The max Z of the block.
//   current   : The octree key of the block.
//   thisLevel : The level of the block in the octree.
//   maxLevels : The maximum recursion level.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:14:26 PDT 2014
//
// Modifications:
//   Brad Whitlock, Fri Jun  6 14:17:41 PDT 2014
//   I added code to handle the case where a child cell0 covers the whole
//   cell so we don't have to refine (the siblings were combined).
//
// ****************************************************************************

void
AMRreaderWithLevels::subdivide8(int *ijk_start, int *ijk_end,
                                float xMin, float yMin, float zMin, float xMax, float yMax, float zMax,
                                OctKey current, int thisLevel, int maxLevels)
{
    float halfX = (xMax - xMin) * 0.5f;
    float halfY = (yMax - yMin) * 0.5f;
    float halfZ = (zMax - zMin) * 0.5f;

    int nx_octcell = ijk_end[0] - ijk_start[0] + 1;
    int ny_octcell = ijk_end[1] - ijk_start[1] + 1;
    int nz_octcell = ijk_end[2] - ijk_start[2] + 1;

    int cell = 0;
    for(int k = 0; k < 2; ++k)
    {
        for(int j = 0; j < 2; ++j)
        {
            for(int i = 0; i < 2; ++i)
            {
                Patch p;
                p.level = thisLevel;
                p.ijk_start[0] = ijk_start[0] + i * nx_octcell / 2;
                p.ijk_start[1] = ijk_start[1] + j * ny_octcell / 2;
                p.ijk_start[2] = ijk_start[2] + k * nz_octcell / 2;

                p.xs[0] = xMin + float(i) * halfX;
                p.xs[1] = yMin + float(j) * halfY;
                p.xs[2] = zMin + float(k) * halfZ;
                p.xe[0] = xMin + float(i+1) * halfX;
                p.xe[1] = yMin + float(j+1) * halfY;
                p.xe[2] = zMin + float(k+1) * halfZ;
                p.key = OctKey_AddLevel(current, cell);
                p.fileBID = FindBlock(p.key);
                patches.push_back(p);

// NOTE: might need to split into 2 phases to ensure that patches within this octree cell are adjacent in the patch list that we build.

                if(thisLevel+1 <= maxLevels)
                {
                    // Test the patches in the file to see if any of them inherit
                    // from the current patch. If so, we need to refine.
// NOTE: maybe we can just check the next 8 patches from the file, depending on
// how the patches are arranged.
                    bool needRefinement = false;
                    int nb = reader->GetNumberOfBlocks();
                    for(int fbid = 0; fbid < nb; ++fbid)
                    {
                        OctKey fkey = reader->GetBlockKey(fbid);
                        if(OctKey_Inherits(fkey, p.key))
                        {
                            needRefinement = true;
                            break;
                        }
                    }

                    if(needRefinement)
                    {
                        // Some of the AMR sub-readers combine octree cells in the
                        // same level. If that has happened then they combine the
                        // cells into cell 0 at that level of the octree. If we find
                        // that cell 0 has block dimensions that are larger than normal
                        // then let's assume that cells in the finer level were combined
                        // and we don't need to refine more.

                        OctKey cell0 = OctKey_AddLevel(p.key, 0);
                        int cell0_bid = FindBlock(cell0);
                        if(cell0_bid != -1)
                        {
                            int defaultDims[3], dims[3];
                            reader->GetBlockDefaultDimensions(defaultDims);
                            reader->GetBlockDimensions(cell0_bid, dims);

                            if(dims[0] > defaultDims[0] ||
                                    dims[1] > defaultDims[1] ||
                                    dims[2] > defaultDims[2])
                            {
                                needRefinement = false;

                                // We're not subdividing but we should add that
                                // cell0 patch. That subpatch spans the space of
                                // the patch we're in too but it's finer res.
                                p.level = thisLevel + 1;
                                p.ijk_start[0] *= 2;
                                p.ijk_start[1] *= 2;
                                p.ijk_start[2] *= 2;
                                p.key = cell0;
                                p.fileBID = cell0_bid;
                                patches.push_back(p);
                            }
                        }
                    }

                    // Refine the patch into 8 more.
                    if(needRefinement)
                    {
                        int ijk_start_next_level[3], ijk_end_next_level[3];
                        ijk_start_next_level[0] = p.ijk_start[0] * 2;
                        ijk_start_next_level[1] = p.ijk_start[1] * 2;
                        ijk_start_next_level[2] = p.ijk_start[2] * 2;

                        ijk_end_next_level[0] = p.ijk_start[0] * 2 + nx_octcell - 1;
                        ijk_end_next_level[1] = p.ijk_start[1] * 2 + ny_octcell - 1;
                        ijk_end_next_level[2] = p.ijk_start[2] * 2 + nz_octcell - 1;

                        subdivide8(ijk_start_next_level, ijk_end_next_level,
                                   p.xs[0], p.xs[1], p.xs[2], p.xe[0], p.xe[1], p.xe[2], p.key, thisLevel+1, maxLevels);
                    }
                }

                cell++;
            }
        }
    }
}

// ****************************************************************************
// Method: AMRreaderWithLevels::GetBlockHierarchicalIndices
//
// Purpose:
//   Get the logical indices for the block in the AMR hierarchy.
//
// Arguments:
//   bid : The block id.
//   level : Return  the level for the block.
//   ijk_start : Return the IJK start for the block.
//   ijk_end   : Return the IJK end for the block.
//
// Returns:    0 on success; -1 on failure.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:18:29 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AMRreaderWithLevels::GetBlockHierarchicalIndices(int bid, int *level, int *ijk_start, int *ijk_end)
{
    int retval = -1;
    if(bid >= 0 && bid < GetNumberOfBlocks())
    {
        int dim[3];
        retval = GetBlockDimensions(bid, dim);

        *level = patches[bid].level;
        ijk_start[0] = patches[bid].ijk_start[0];
        ijk_start[1] = patches[bid].ijk_start[1];
        ijk_start[2] = patches[bid].ijk_start[2];
        ijk_end[0]   = patches[bid].ijk_start[0] + dim[0] - 1;
        ijk_end[1]   = patches[bid].ijk_start[1] + dim[1] - 1;
        ijk_end[2]   = patches[bid].ijk_start[2] + dim[2] - 1;
    }

    return retval;
}

int
AMRreaderWithLevels::GetNumCycles() const
{
    return reader->GetNumCycles();
}

double
AMRreaderWithLevels::GetSimuTime() const
{
    return reader->GetSimuTime();
}

int
AMRreaderWithLevels::GetNumberOfBlocks() const
{
    return (int)patches.size();
}

int
AMRreaderWithLevels::GetBlockDimensions(int bid, int *dim) const
{
    int retval = -1;
    if(bid >= 0 && bid < GetNumberOfBlocks())
    {
        // If we have a block that came from the file then use that size.
        // Otherwise, assume the default block dimensions.
        if(patches[bid].fileBID != -1)
        {
            retval = reader->GetBlockDimensions(patches[bid].fileBID, dim);
        }
        else
        {
            retval = reader->GetBlockDefaultDimensions(dim);

            // The root domain will have 2x the default dimensions.
            if(bid == 0)
            {
                dim[0] *= 2;
                dim[1] *= 2;
                dim[2] *= 2;
            }
        }
    }

    return retval;
}

int
AMRreaderWithLevels::GetBlockDefaultDimensions(int *dim) const
{
    return reader->GetBlockDefaultDimensions(dim);
}

int
AMRreaderWithLevels::GetBlockSize(int bid) const
{
    int retval = 0;
    if(bid >= 0 && bid < GetNumberOfBlocks())
    {
        // If we have a block that came from the file then use its size.
        // Otherwise, assume the default block dimensions.
        if(patches[bid].fileBID != -1)
        {
            retval = reader->GetBlockSize(patches[bid].fileBID);
        }
        else
        {
            int dims[3];
            GetBlockDimensions(bid, dims);
            retval = dims[0] * dims[1] * dims[2];
        }
    }

    return retval;
}

int
AMRreaderWithLevels::GetBlockMesh(int bid, float *xs, float *dx)
{
    int retval = -1;
    if(bid >= 0 && bid < GetNumberOfBlocks())
    {
        // If we have a block that came from the file then use its size.
        // Otherwise, use the block that we computed with the octree.
        if(patches[bid].fileBID != -1)
        {
            retval = reader->GetBlockMesh(patches[bid].fileBID, xs, dx);
        }
        else
        {
            xs[0] = patches[bid].xs[0];
            xs[1] = patches[bid].xs[1];
            xs[2] = patches[bid].xs[2];

            int dim[3];
            GetBlockDimensions(bid, dim);
            dx[0] = (patches[bid].xe[0] - patches[bid].xs[0]) / float(dim[0]);
            dx[1] = (patches[bid].xe[1] - patches[bid].xs[1]) / float(dim[1]);
            dx[2] = (patches[bid].xe[2] - patches[bid].xs[2]) / float(dim[2]);

            retval = 0;
        }
    }
    return retval;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::AssembleBlockVariable
//
// Purpose:
//   Obtains data for the specified block, creating it from finer resolutions
//   if needed.
//
// Arguments:
//   bid : The block id.
//   vid : The variable id.
//   dat : The buffer that will contain the data.
//   dims : The dimensions of the buffer.
//
// Returns:    0 on success. -1 otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:06:10 PDT 2014
//
// Modifications:
//
// ****************************************************************************

inline bool CloseEnough(float a, float b)
{
    return fabs(a-b) < 1.e-6;
}

int
AMRreaderWithLevels::AssembleBlockVariable(int bid, int vid, float *dat, const int *dims)
{
    int retval = -1;

    // If the bid has data in the file, return it.
    if(patches[bid].fileBID != -1)
    {
        retval = reader->GetBlockVariable(patches[bid].fileBID, vid, dat);
    }
    else
    {
        // We must make data from subcells.
        int halfdims[3];
        halfdims[0] = dims[0] / 2, halfdims[1] = dims[1] / 2, halfdims[2] = dims[2] / 2;

        int celldataSize = 0;
        float *celldata = NULL;

        // We must assemble data. Iterate over each subcell and put some samples
        // from its data into the "dat" array.
        for(int cell = 0; cell < 8; ++cell)
        {
            OctKey cellkey = OctKey_AddLevel(patches[bid].key, cell);
            int i_off = kid2i[cell] * halfdims[0];
            int j_off = kid2j[cell] * halfdims[1];
            int k_off = kid2k[cell] * halfdims[2];

            int cellbid = BlockKeyToBID(cellkey);
            bool cellCoversEntirely = false;
            if(cellbid != -1)
            {
                // Make sure that we have enough storage to hold the data
                // for the cell.
                int celldims[3];
                GetBlockDimensions(cellbid, celldims);
                int cellsz = celldims[0] * celldims[1] * celldims[2];
                if(cellsz != celldataSize)
                {
                    delete [] celldata;
                    celldata = new float[cellsz];
                    celldataSize = cellsz;
                }

                // See if any cell combining has happened in the child cell 0.
                if(cell == 0)
                {
                    float xs[3], dx[3], xs0[3], dx0[3];
                    GetBlockMesh(bid, xs, dx);
                    GetBlockMesh(cellbid, xs0, dx0);

                    cellCoversEntirely = CloseEnough(xs[0], xs0[0]) &&
                                         CloseEnough(xs[1], xs0[1]) &&
                                         CloseEnough(xs[2], xs0[2]) &&
                                         CloseEnough(xs[0]+dims[0]*dx[0], xs0[0]+celldims[0]*dx0[0]) &&
                                         CloseEnough(xs[1]+dims[1]*dx[1], xs0[1]+celldims[1]*dx0[1]) &&
                                         CloseEnough(xs[2]+dims[2]*dx[2], xs0[2]+celldims[2]*dx0[2]);
                }
                int windowSize[3];
                if(cellCoversEntirely)
                {
                    // The cell covers all octants so we want to sample over the
                    // entire
                    windowSize[0] = dims[0];
                    windowSize[1] = dims[1];
                    windowSize[2] = dims[2];
                }
                else
                {
                    // Normal case is that we're filling in an octant of the cell.
                    windowSize[0] = halfdims[0];
                    windowSize[1] = halfdims[1];
                    windowSize[2] = halfdims[2];
                }

                // Get the data for the cell.
                if(AssembleBlockVariable(cellbid, vid, celldata, celldims) != -1)
                {
                    // Sample data from the celldata into this oct cell.
                    for(int k = 0; k < windowSize[2]; ++k)
                    {
                        float fk = float(k) / float(windowSize[2]-1);
                        int ck = int(fk * float(celldims[2]-1));

                        for(int j = 0; j < windowSize[1]; ++j)
                        {
                            float fj = float(j) / float(windowSize[1]-1);
                            int cj = int(fj * float(celldims[1]-1));

                            for(int i = 0; i < windowSize[0]; ++i)
                            {
                                float fi = float(i) / float(windowSize[0]-1);
                                int ci = int(fi * float(celldims[0]-1));

                                int cell_idx = ck * celldims[1]*celldims[0] +
                                               cj * celldims[0] +
                                               ci;

                                int dest_idx = (k_off + k) * dims[1]*dims[0] +
                                               (j_off + j) * dims[0] +
                                               (i_off + i);

                                dat[dest_idx] = celldata[cell_idx];
                            }
                        }
                    }
                }
                else
                {
                    // Indicate that we need to fill with zeroes.
                    cellbid = -1;
                }
            }

            if(cellbid == -1)
            {
                // poke zeroes in for the cell data. This likely will not
                // happen because at the finest level, all of space is covered
                // in patches.
//                cout << "zero filling"<< endl;
                for(int k = 0; k < halfdims[2]; ++k)
                {
                    for(int j = 0; j < halfdims[1]; ++j)
                    {
                        for(int i = 0; i < halfdims[0]; ++i)
                        {
                            int dest_idx = (k_off + k) * dims[1]*dims[0] +
                                           (j_off + j) * dims[0] +
                                           (i_off + i);

                            dat[dest_idx] = 0;
                        }
                    }
                }
            }

            // We don't need to continue in this case.
            if(cellCoversEntirely)
                break;
        }

        delete [] celldata;
        retval = 0;
    }

    return retval;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::BlockKeyToBID
//
// Purpose:
//   Returns the block id of the block having the specified block key.
//
// Arguments:
//   k : The block key that we're looking for.
//
// Returns:    The block id of the block with the specified key.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:07:41 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AMRreaderWithLevels::BlockKeyToBID(const OctKey &k) const
{
    int retval = -1;
    std::map<OctKey,int>::const_iterator pos = blkkey2bid.find(k);
    if(pos != blkkey2bid.end())
        retval = pos->second;
    return retval;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::GetBlockVariable
//
// Purpose:
//   Get the block variable for the specified block.
//
// Arguments:
//   bid : The block id.
//   vid : The varible id.
//   dat : The buffer that will contain the data.
//
// Returns:    0 on success; -1 on failure.
//
// Note:       Block 0 is special since it contains reduced data from all of the
//             octree level 2 blocks.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:08:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AMRreaderWithLevels::GetBlockVariable(int bid, int vid, float *dat)
{
    int retval = -1;
    if(bid >= 0 && bid < GetNumberOfBlocks())
    {
        if(bid == 0)
        {
            // Get the dimensions of block 0
            int dims[3];
            GetBlockDimensions(bid, dims);

            // Get the dimensions of the default block we'll use for level 2 blocks.
            int celldims[3];
            GetBlockDefaultDimensions(celldims);
            int sz = celldims[0]*celldims[1]*celldims[2];
            float *celldata = new float[sz];

            // The level 0 mesh is made from all of the level 2 blocks.
            OctKey root = OctKey_Root();
            for(int l0 = 0; l0 < 8; ++l0)
            {
                OctKey k0 = OctKey_AddLevel(root, l0);
                for(int l1 = 0; l1 < 8; ++l1)
                {
                    OctKey key = OctKey_AddLevel(k0, l1);
                    int cellbid = BlockKeyToBID(key);

                    // Get the data for the block identified by key.
                    retval = AssembleBlockVariable(cellbid, vid, celldata, celldims);

                    // Get the patch for the key and use the logical extents
                    // to compute the destination within the level 0 patch.
                    int i_dest = patches[cellbid].ijk_start[0] / 2;
                    int j_dest = patches[cellbid].ijk_start[1] / 2;
                    int k_dest = patches[cellbid].ijk_start[2] / 2;
                    for(int k = 0, kk = 0; k < celldims[2]; k += 2, kk++)
                    {
                        for(int j = 0, jj = 0; j < celldims[1]; j += 2, jj++)
                        {
                            for(int i = 0, ii = 0; i < celldims[0]; i += 2, ii++)
                            {
                                // Index within the src patch.
                                int src_idx = k * celldims[0]*celldims[1] + j*celldims[0] + i;

                                // Index within block 0.
                                int dest_idx = (kk + k_dest) * dims[1]*dims[0] +
                                               (jj + j_dest) * dims[0] +
                                               (ii + i_dest);

                                dat[dest_idx] = celldata[src_idx];
                            }
                        }
                    }
                }
            }

            delete [] celldata;
        }
        else
        {
            // The block dimensions and the size of the dat array should
            // match already.
            int dims[3];
            GetBlockDimensions(bid, dims);
            retval = AssembleBlockVariable(bid, vid, dat, dims);
        }
    }

    return retval;
}

// ****************************************************************************
// Method: AMRreaderWithLevels::GetBlockKey
//
// Purpose:
//   Get the block key for the specified block.
//
// Arguments:
//   bid : The block id.
//
// Returns:    The block key.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  6 14:10:14 PDT 2014
//
// Modifications:
//
// ****************************************************************************

OctKey
AMRreaderWithLevels::GetBlockKey(int bid)
{
    OctKey key = OctKey_new();
    if(bid >= 0 && bid < GetNumberOfBlocks())
    {
        key = patches[bid].key;
    }
    return key;
}

void
AMRreaderWithLevels::GetInterfaceSizes( int* np, int* ne ) const
{
    return reader->GetInterfaceSizes(np, ne);
}

int
AMRreaderWithLevels::GetInterfaceVariable( int vid, void* dat )
{
    return reader->GetInterfaceVariable(vid, dat);
}

bool
AMRreaderWithLevels::HasTag() const
{
    return reader->HasTag();
}
