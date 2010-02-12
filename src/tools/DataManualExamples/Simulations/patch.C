#include <map>
#include "patch.h"
#include <cstring>

// ****************************************************************************
// Functions: Functions related to the patch object
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 25 14:05:02 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
patch_print(FILE *f, patch_t *patch)
{
    fprintf(f, "patch {\n");
    fprintf(f, "    id=%d\n", patch->id);
    fprintf(f, "    level=%d\n", patch->level);
    fprintf(f, "    logical_extents={x=%d %d  y=%d %d}\n",
        patch->logical_extents[0],
        patch->logical_extents[1],
        patch->logical_extents[2],
        patch->logical_extents[3]);
    fprintf(f, "    window={x=%g %g  y=%g %g}\n",
        patch->window[0],
        patch->window[1],
        patch->window[2],
        patch->window[3]);
    fprintf(f, "    nx=%d\n", patch->nx);
    fprintf(f, "    ny=%d\n", patch->ny);
    fprintf(f, "    nsubpatches=%d\n", patch->nsubpatches);
    fprintf(f, "    subpatches={");
    for(int i = 0; i < patch->nsubpatches; ++i)
        fprintf(f, "%d ", patch->subpatches[i].id);
    fprintf(f, "}\n");
    fprintf(f, "}\n");
} 

void
patch_ctor(patch_t *patch)
{
    memset(patch, 0, sizeof(patch_t));
}

void
patch_dtor(patch_t *patch)
{
    int i;
    FREE(patch->data);

    for(i = 0; i < patch->nsubpatches; ++i)
        patch_dtor(&patch->subpatches[i]);
    FREE(patch->subpatches);
}

void
patch_alloc_data(patch_t *patch, int nx, int ny)
{
    patch->nx = nx;
    patch->ny = ny;
    patch->data = ALLOC(nx*ny, unsigned char);
}

patch_t *
patch_add_subpatch(patch_t *patch)
{
    if(patch->nsubpatches == 0)
    {
        patch->subpatches = ALLOC(1, patch_t);
        patch->nsubpatches = 1;
    }
    else
    {
        ++patch->nsubpatches;
        patch->subpatches = REALLOC(patch->subpatches, patch->nsubpatches, patch_t);
    }
    patch_ctor(&patch->subpatches[patch->nsubpatches-1]);
    return &patch->subpatches[patch->nsubpatches-1];
}

static void
patch_num_patches_helper(patch_t *patch, int *sum)
{
    *sum = *sum + 1;
    for(int i = 0; i < patch->nsubpatches; ++i)
        patch_num_patches_helper(&patch->subpatches[i], sum);
}

int
patch_num_patches(patch_t *patch)
{
    int sum = 0;
    patch_num_patches_helper(patch, &sum);
    return sum;
}

patch_t *
patch_get_patch(patch_t *patch, int id)
{
    if(patch->id == id)
        return patch;

    for(int i = 0; i < patch->nsubpatches; ++i)
    {
        patch_t *p = patch_get_patch(&patch->subpatches[i], id);
        if(p != NULL)
            return p;
    }

    return NULL;
}

static void
patch_num_levels_helper(patch_t *patch, int *maxlevel)
{
    if(patch->level > *maxlevel)
        *maxlevel = patch->level;
    for(int i = 0; i < patch->nsubpatches; ++i)
        patch_num_levels_helper(&patch->subpatches[i], maxlevel);
}

int
patch_num_levels(patch_t *patch)
{
    int maxlevel = 0;
    patch_num_levels_helper(patch, &maxlevel);
    return maxlevel+1;
}

static void
patch_flat_array_helper(patch_t *patch, patch_t **arr, int *index)
{
    arr[*index] = patch;
    *index = *index + 1;
    for(int i = 0; i < patch->nsubpatches; ++i)
        patch_flat_array_helper(&patch->subpatches[i], arr, index);
}

patch_t **
patch_flat_array(patch_t *patch)
{
    int np = patch_num_patches(patch);
    patch_t **patches = ALLOC(np, patch_t*);
    int index = 0;
    patch_flat_array_helper(patch, patches, &index);
    return patches;
}

/******************************************************************************/

typedef struct
{
    int *data;
    int nx;
    int ny;
} score_t;

static void
score_fill_box(score_t *score, int indices[4], int value)
{
    for(int j = indices[2]; j < indices[3]; ++j)
    {
        for(int i = indices[0]; i < indices[1]; ++i)
        {
            int thispix = score->nx * j + i;
            score->data[thispix] = value;
        }
    }
}

static void
score_get_partition(int type, int indices[4], int *partition0, int *partition1)
{
    int dx = indices[1] - indices[0];
    int dy = indices[3] - indices[2];

    // vertical breakup
    if(type == 0)
    {
        partition0[0] = indices[0];
        partition0[1] = indices[0] + dx/2;
        partition0[2] = indices[2];
        partition0[3] = indices[3];

        partition1[0] = indices[0] + dx/2;
        partition1[1] = indices[1];
        partition1[2] = indices[2];
        partition1[3] = indices[3];
    }
    else
    {
        partition0[0] = indices[0];
        partition0[1] = indices[1];
        partition0[2] = indices[2];
        partition0[3] = indices[2] + dy/2;

        partition1[0] = indices[0];
        partition1[1] = indices[1];
        partition1[2] = indices[2] + dy/2;
        partition1[3] = indices[3];
    }
}

// ****************************************************************************
// Function: score_image_helper
//
// Purpose: 
//   Helper function that recurses on a region of the mask image and determines
//   whether a new patch can be created for the region.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 25 14:00:09 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

#define SCORE_PIXELS_DIFFERENT 0
#define SCORE_PIXELS_ALL_SAME  1

static int
score_image_helper(image_t *mask, score_t *score, int level, int indices[4],
    int *boxid)
{
    int retval = 0;
    int dx = indices[1] - indices[0];
    int dy = indices[3] - indices[2];

    const int min_patch_size = 8;
    const int patch_divisor = 16;
    int minx = ((mask->nx / patch_divisor) < min_patch_size) ? min_patch_size : (mask->nx / patch_divisor);
    int miny = ((mask->ny / patch_divisor) < min_patch_size) ? min_patch_size : (mask->ny / patch_divisor);

    bool verticalDivideOkay = (dx > minx);
    bool horizontalDivideOkay = (dy > miny);
    if(!verticalDivideOkay && !horizontalDivideOkay)
    {
        /* Check all of the pixels in the indices and see if they are
           the same.
         */
        int pix0 = mask->nx * indices[2] + indices[0];
        for(int j = indices[2]; j < indices[3]; ++j)
        {
            for(int i = indices[0]; i < indices[1]; ++i)
            {
                int thispix = mask->nx * j + i;
                if(mask->data[pix0] != mask->data[thispix])
                {
                    score_fill_box(score, indices, *boxid);
                    *boxid = *boxid + 1;
                    return SCORE_PIXELS_DIFFERENT;
                }
            }
        }
        retval = SCORE_PIXELS_ALL_SAME;
    }
    else
    {
        int partition0[4], partition1[4];
        bool doVertical = verticalDivideOkay && ((level & 1) == 0);
        if(doVertical)
            score_get_partition(0, indices, partition0, partition1);
        else
            score_get_partition(1, indices, partition0, partition1);

        int s0 = score_image_helper(mask, score, level+1, partition0, boxid);
        int s1 = score_image_helper(mask, score, level+1, partition1, boxid);

        if(s0 == SCORE_PIXELS_DIFFERENT && s1 == SCORE_PIXELS_DIFFERENT)
        {
            // Fill in the score with the same id.
            score_fill_box(score, indices, *boxid);
            *boxid = *boxid + 1;
            retval = SCORE_PIXELS_DIFFERENT;
        }
        else
            retval = SCORE_PIXELS_ALL_SAME;
    }
    return retval;
}

// ****************************************************************************
// Function: score_image
//
// Purpose: 
//   Creates a score image from a mask image. The score image is an image 
//   that contains a bunch of "colored" rectangles that correspond to patches
//   that we'll create.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 25 14:00:09 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static void
score_image(image_t *mask, score_t *score)
{
    int indices[4];
    int boxid = 0;

    indices[0] = 0;
    indices[1] = mask->nx;
    indices[2] = 0;
    indices[3] = mask->ny;
    score_image_helper(mask, score, 0, indices, &boxid);
}

struct score_patch_info
{
    int startx, endx;
    int starty, endy;
};

// ****************************************************************************
// Function: patches_abut
//
// Purpose: 
//   Returns true if the 2 patches abut in a way that would allow them to
//   be combined.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 27 08:49:10 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static bool
patches_abut(const score_patch_info &p0, const score_patch_info &p1)
{
    if(p0.startx == p1.startx && p0.endx == p1.endx)
    {
        // top-bottom alignment
        if(p0.endy+1 == p1.starty)
           return true;
        else if(p0.starty-1 == p1.endy)
           return true;
    }
    else if(p0.starty == p1.starty && p0.endy == p1.endy)
    {
        // left-right alignment
        if(p0.endx+1 == p1.startx)
           return true;
        else if(p0.startx-1 == p1.endx)
           return true;
    }
    return false;
}

// ****************************************************************************
// Function: patch_add
//
// Purpose: 
//   Adds 2 patches together.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 27 08:49:50 PDT 2009
//
// Modifications:
//   
// ****************************************************************************
#define MIN(A,B) (((A)<(B)) ? (A) : (B))
#define MAX(A,B) (((A)>(B)) ? (A) : (B))

static score_patch_info
patch_add(const score_patch_info &p0, const score_patch_info &p1)
{
    score_patch_info s;

    s.startx = MIN(p0.startx, p1.startx);
    s.endx = MAX(p0.endx, p1.endx);
    s.starty = MIN(p0.starty, p1.starty);
    s.endy = MAX(p0.endy, p1.endy);

    return s;
}

// ****************************************************************************
// Method: combine_patches
//
// Purpose: 
//   Iterates through the patch map and combines compatible patches to 
//   reduce the patch count.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 27 08:50:24 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
combine_patches(std::map<int, score_patch_info> &patchmap)
{
    std::map<int, score_patch_info>::iterator p0, p1;
    bool found_matches;

    do
    {
        found_matches = false;
        for(p0 = patchmap.begin(); p0 != patchmap.end(); ++p0)
        {
            std::map<int, score_patch_info> matches;
            for(p1 = patchmap.begin(); p1 != patchmap.end(); ++p1)
            {
                if(p0->first == p1->first)
                    continue;

                if(patches_abut(p0->second, p1->second))
                    matches[p1->first] = p1->second;
            }

            // Now that we have a list of matches for p0, let's choose the
            // one that gives us the most area.
            int max_area = -1;
            int best_match = -1;
            for(p1 = matches.begin(); p1 != matches.end(); ++p1)
            {
                int nx = p1->second.endx - p1->second.startx + 1;
                int ny = p1->second.endy - p1->second.starty + 1;
                if(nx*ny > max_area)
                {
                    max_area = nx*ny;
                    best_match = p1->first;
                }
            }

            p1 = patchmap.find(best_match);
            if(p1 != patchmap.end())
            {
                // Let's add best_match to p0 and remove p1 from the map.
                p0->second = patch_add(p0->second, p1->second);
                patchmap.erase(p1);
                found_matches = true;
                break;
            }
        }
    } while(found_matches);
}

// ****************************************************************************
// Function: patch_refine
//
// Purpose: 
//   This method examines the patch's data using the provided maskcb function
//   and refines according to the mask that the callback created. New patches
//   get created as children of the input patch and we can further calculate
//   on them.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 25 14:01:33 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
patch_refine(patch_t *patch, int refinement_ratio, 
    void (*maskcb)(patch_t *, image_t *))
{
    int i, j, index;

    /* Call the maskcb callback to flag the cells in the patch that need
     * to be refined.
     */
    image_t mask;
    mask.nx = patch->nx;
    mask.ny = patch->ny;
    mask.data = ALLOC(mask.nx*mask.ny, unsigned char);
    (*maskcb)(patch, &mask);

    /* Use the mask image to create a set of patches that fit well and 
     * cover the values that are on.
     */
    score_t score;
    score.nx = patch->nx;
    score.ny = patch->ny;
    score.data = ALLOC(score.nx*score.ny, int);
    score_image(&mask, &score);

    /* Now that we have a score image, which breaks up the input patch
       into subpatches, let's scan through the image and get the extents
       for the patches so we can create subpatches.
     */
    std::map<int, score_patch_info> patchmap;
    int current_patch = -1;
    for(j = 0; j < patch->ny; ++j)
        for(i = 0; i < patch->nx; ++i)
        {
            if(score.data[j*score.nx+i] > 0)
            {
                // see if we need to add a new patch to the map
                current_patch = score.data[j*score.nx+i];
                std::map<int, score_patch_info>::iterator it = patchmap.find(current_patch);
                if(it == patchmap.end())
                {
                    // add a new patch
                    patchmap[current_patch].starty = j;
                    patchmap[current_patch].startx = i;
                    patchmap[current_patch].endy = j;
                    patchmap[current_patch].endx = i;
                }
            }
        }

    // Iterate through the patch list and the score image to determine
    // each patch's end indices.
    for(std::map<int, score_patch_info>::iterator it = patchmap.begin();
        it != patchmap.end(); ++it)
    {
        for(j = it->second.starty+1; j < patch->ny; ++j)
        {
            if(score.data[j*score.nx + it->second.startx] == it->first)
                it->second.endy++;
            else 
                break;
        }
        for(i = it->second.startx+1; i < patch->nx; ++i)
        {
            if(score.data[it->second.starty*score.nx + i] == it->first)
                it->second.endx++;
            else 
                break;
        }
    }
    FREE(score.data);
    FREE(mask.data);

    // Combine compatible patches
    combine_patches(patchmap);    

    // Now that we have a list of patches, create subpatches based on them.
    for(std::map<int, score_patch_info>::iterator it = patchmap.begin();
        it != patchmap.end(); ++it)
    {
        // now that we know the patch's indices, let's create a subpatch
        patch_t *newpatch = patch_add_subpatch(patch);
        int nx = it->second.endx - it->second.startx + 1;
        int ny = it->second.endy - it->second.starty + 1;
        float cellWidth = (patch->window[1] - patch->window[0]) / ((float)patch->nx);
        float cellHeight = (patch->window[3] - patch->window[2]) / ((float)patch->ny);
        newpatch->window[0] = patch->window[0] + it->second.startx * cellWidth;
        newpatch->window[1] = newpatch->window[0] + ((float)nx) * cellWidth;
        newpatch->window[2] = patch->window[2] + it->second.starty * cellHeight;
        newpatch->window[3] = newpatch->window[2] + ((float)ny) * cellHeight;

        int startI = patch->logical_extents[0] + it->second.startx;
        int endI = patch->logical_extents[0] + it->second.endx;
        int startJ = patch->logical_extents[2] + it->second.starty;
        int endJ = patch->logical_extents[2] + it->second.endy;
        newpatch->logical_extents[0] = startI*refinement_ratio;
        newpatch->logical_extents[1] = (endI+1)*refinement_ratio-1;
        newpatch->logical_extents[2] = startJ*refinement_ratio;
        newpatch->logical_extents[3] = (endJ+1)*refinement_ratio-1;

        patch_alloc_data(newpatch, nx*refinement_ratio, ny*refinement_ratio);
    }
}
