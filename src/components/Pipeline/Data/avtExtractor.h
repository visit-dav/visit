// ************************************************************************* //
//                               avtExtractor.h                              //
// ************************************************************************* //

#ifndef AVT_EXTRACTOR_H
#define AVT_EXTRACTOR_H
#include <pipeline_exports.h>


#include <math.h>

#include <avtCellTypes.h>
#include <avtSamplePoints.h>


// ****************************************************************************
//  Class: avtExtractor
//
//  Purpose:
//      The base type for all of the extractors, this contains routines for
//      doing the extractions once the cell has been cut by a plane.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Jan 27 20:55:11 PST 2001
//    Accounted for deferring of sampling of cells.
//
//    Hank Childs, Sat Feb  3 18:52:30 PST 2001
//    Changed orientation of slicing to start with x instead of z to take
//    advantage of cache coherence.
//
//    Hank Childs, Wed Jan 23 11:03:08 PST 2002
//    Add support for cells that do not intersect sample points.
//
// ****************************************************************************

class PIPELINE_API avtExtractor
{
  public:
                           avtExtractor(int, int, int, avtVolume *, 
                                        avtCellList *);
    virtual               ~avtExtractor();

    void                   SendCellsMode(bool);
    void                   Restrict(int, int, int, int);

  protected:
    float                  x_step, y_step, z_step;
    int                    width, height, depth;
    int                    restrictedMinWidth, restrictedMaxWidth;
    int                    restrictedMinHeight, restrictedMaxHeight;
    avtVolume             *volume;
    avtCellList           *celllist;
    float                (*tmpSampleList)[AVT_VARIABLE_LIMIT];

    int                    minx, maxx, miny, maxy, minz, maxz;
    bool                   sendCellsMode;

    void                   ExtractTriangle(int,const float [3],const float [3],
                                      const float[3][AVT_VARIABLE_LIMIT], int);
    void                   ExtractLine(int, int, float, float, 
                                       float[AVT_VARIABLE_LIMIT],
                                       float[AVT_VARIABLE_LIMIT], int);

    void                   OrientTriangle(float [3], float [3],
                                          float [3][AVT_VARIABLE_LIMIT], int);
    inline void            InterpolateToPlane(const float[3], const float[3],
                                              const float [AVT_VARIABLE_LIMIT],
                                              const float [AVT_VARIABLE_LIMIT],
                                              const float &, float &, float &,
                                              float [AVT_VARIABLE_LIMIT], int);

    int                    ConstructBounds(const float (*)[3], int);
    void                   ContributeSmallCell(const float (*)[3],
                                     const float (*)[AVT_VARIABLE_LIMIT], int);
    int                    IndexToTriangulationTable(const float(*)[3], int,
                                                     float);
    void                   AddCell(char *);

    static const float     FRUSTUM_MIN_X;
    static const float     FRUSTUM_MAX_X;
    static const float     FRUSTUM_MIN_Y;
    static const float     FRUSTUM_MAX_Y;
    static const float     FRUSTUM_MIN_Z;
    static const float     FRUSTUM_MAX_Z;

    inline int             SnapXRight(float);
    inline int             SnapXLeft(float);
    inline int             SnapYTop(float);
    inline int             SnapYBottom(float);
    inline int             SnapZFront(float);
    inline int             SnapZBack(float);

    inline float           XFromIndex(int);
    inline float           YFromIndex(int);
    inline float           ZFromIndex(int);
};


// ****************************************************************************
//  Methods:  Snap[Coordinate][Direction]
//
//  Purpose:
//      Finds the appropriate index for a location along that axis.  Round off
//      occurs in the direction specified.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jan 29 21:02:56 PST 2001
//    Accounted for screen being restricted.
//
//    Hank Childs, Fri Apr  6 17:24:13 PDT 2001
//    Put in a tolerance for numerical (im)precision.
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Cast to get rid of compiler warning.
//
// ****************************************************************************

int 
avtExtractor::SnapXRight(float x)
{
    float close_to_index = (x - FRUSTUM_MIN_X) / x_step;
    int   index = (int)ceil(close_to_index);

    //
    // Have to correct for floating point imprecision.  Net result is
    // potentially redundant work.
    //
    if ((index - close_to_index) > 0.999)
    {
        index--;
    }

    if (index < restrictedMinWidth)
    {
        return restrictedMinWidth;
    }
    if (index >= restrictedMaxWidth)
    {
        return restrictedMaxWidth;
    }
    
    return index;
}

int 
avtExtractor::SnapXLeft(float x)
{
    float close_to_index = (x - FRUSTUM_MIN_X) / x_step;
    int   index = (int)floor(close_to_index);

    //
    // Have to correct for floating point imprecision.  Net result is
    // potentially redundant work.
    //
    if ((close_to_index - index) > 0.999)
    {
        index++;
    }

    if (index <= restrictedMinWidth)
    {
        return restrictedMinWidth;
    }
    if (index >= restrictedMaxWidth)
    {
        return restrictedMaxWidth;
    }
    
    return index;
}

int 
avtExtractor::SnapYTop(float y)
{
    float close_to_index = (y - FRUSTUM_MIN_Y) / y_step;
    int   index = (int)ceil(close_to_index);

    //
    // Have to correct for floating point imprecision.  Net result is
    // potentially redundant work.
    //
    if ((index - close_to_index) > 0.999)
    {
        index--;
    }

    if (index <= restrictedMinHeight)
    {
        return restrictedMinHeight;
    }
    if (index >= restrictedMaxHeight)
    {
        return restrictedMaxHeight;
    }
    
    return index;
}

int 
avtExtractor::SnapYBottom(float y)
{
    float close_to_index = (y - FRUSTUM_MIN_Y) / y_step;
    int   index = (int)floor(close_to_index);

    //
    // Have to correct for floating point imprecision.  Net result is
    // potentially redundant work.
    //
    if ((close_to_index - index) > 0.999)
    {
        index++;
    }

    if (index < restrictedMinHeight)
    {
        return restrictedMinHeight;
    }
    if (index >= restrictedMaxHeight)
    {
        return restrictedMaxHeight;
    }
    
    return index;
}

int 
avtExtractor::SnapZBack(float z)
{
    float close_to_index = (z - FRUSTUM_MIN_Z) / z_step;
    int   index = (int)ceil(close_to_index);

    //
    // Have to correct for floating point imprecision.  Net result is
    // potentially redundant work.
    //
    if ((index - close_to_index) > 0.999)
    {
        index--;
    }

    if (index < 0)
    {
        return 0;
    }
    if (index >= depth-1)
    {
        return depth-1;
    }
    
    return index;
}

int 
avtExtractor::SnapZFront(float z)
{
    float close_to_index = (z - FRUSTUM_MIN_Z) / z_step;
    int   index = (int)floor(close_to_index);

    //
    // Have to correct for floating point imprecision.  Net result is
    // potentially redundant work.
    //
    if ((close_to_index - index) > 0.999)
    {
        index++;
    }

    if (index < 0)
    {
        return 0;
    }
    if (index >= depth-1)
    {
        return depth-1;
    }
    
    return index;
}


// ****************************************************************************
//  Methods:  [Coordinate]FromIndex
//
//  Purpose:
//      Determine what the coordinate value is at a sample point from the
//      index.
//
//  Arguments:
//      ind        An index for x, y, or z.
//
//  Returns:       The true x, y, or z location
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2000
//
// ****************************************************************************

float
avtExtractor::XFromIndex(int xi)
{
    return FRUSTUM_MIN_X + (xi*x_step);
}


float
avtExtractor::YFromIndex(int yi)
{
    return FRUSTUM_MIN_Y + (yi*y_step);
}


float
avtExtractor::ZFromIndex(int zi)
{
    return FRUSTUM_MIN_Z + (zi*z_step);
}


// ****************************************************************************
//  Method: avtExtractor::InterpolateToPlane
//
//  Purpose:
//      Takes two vertices and interpolates those vertices along the line 
//      connecting them to a specific x (the plane to interpolate to).  Returns
//      the (y, z) of that line and what the variable is.
//
//  Arguments:
//      v1     The first vertex.
//      v2     The second vertex.
//      val1   The value at v1.
//      val2   The value at v2.
//      x      The x-value to interpolate to.
//      y      The resulting y-value.
//      z      The resulting z-value.
//      v      The resulting variable value.
//      nVars  The number of variables.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 15:56:21 PST 2001
//    Interpolate to x-plane instead of z-plane.
//
//    Hank Childs, Tue Nov 13 16:52:15 PST 2001
//    Allow for multiple variables.
//
// ****************************************************************************

void
avtExtractor::InterpolateToPlane(const float v1[3], const float v2[3],
                                 const float val1[AVT_VARIABLE_LIMIT],
                                 const float val2[AVT_VARIABLE_LIMIT],
                                 const float &x, float &y, float &z, 
                                 float v[AVT_VARIABLE_LIMIT], int nVars)
{
    //
    // Make sure this is not from a degenerate cell.
    //
    if (v1[0] == v2[0])
    {
        return;
    }

    //
    // t is the distance we have to go along the line connecting v1 and v2 to
    // get to a point that has its x-coordinate equal to x.
    //
    float t = (x - v1[0]) / (v2[0] - v1[0]);

    y = t*(v2[1] - v1[1]) + v1[1];
    z = t*(v2[2] - v1[2]) + v1[2];
    for (int i = 0 ; i < nVars ; i++)
    {
        v[i] = t*(val2[i] - val1[i]) + val1[i];
    }
}


#endif


