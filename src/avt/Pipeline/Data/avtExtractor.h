// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Hank Childs, Sat Oct  6 14:02:06 PDT 2007
//    Check in Timo Bremer's changes.  New virtual method StoreRay.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Thu Dec 20 17:02:48 PST 2007
//    Change signature of InterpolateToPlane to avoid confusing the compiler.
//
//    Hank Childs, Fri Jan  9 14:03:43 PST 2009
//    Add a data member for jittering.
//
// ****************************************************************************

class PIPELINE_API avtExtractor
{
  public:
                           avtExtractor(int, int, int, avtVolume *, 
                                        avtCellList *);
    virtual               ~avtExtractor();

    void                   SendCellsMode(bool);
    void                   SetJittering(bool j) { jitter = j; };
    void                   Restrict(int, int, int, int);

  protected:
    double                 x_step, y_step, z_step;
    int                    width, height, depth;
    int                    restrictedMinWidth, restrictedMaxWidth;
    int                    restrictedMinHeight, restrictedMaxHeight;
    avtVolume             *volume;
    avtCellList           *celllist;
    double                (*tmpSampleList)[AVT_VARIABLE_LIMIT];

    int                    minx, maxx, miny, maxy, minz, maxz;
    bool                   sendCellsMode;
    bool                   jitter;

    void                   ExtractTriangle(int,const double [3],
                                      const double [3],
                                      const double[3][AVT_VARIABLE_LIMIT], int);
    void                   ExtractLine(int, int, double, double, 
                                       double[AVT_VARIABLE_LIMIT],
                                       double[AVT_VARIABLE_LIMIT], int);
    virtual void           StoreRay(int, int, int, int,
                                    const double (*)[AVT_VARIABLE_LIMIT]);
    

    void                   OrientTriangle(double [3], double [3],
                                          double [3][AVT_VARIABLE_LIMIT], int);
    inline void            InterpolateToPlane(const double[3], const double[3],
                                              const double *, const double *,
                                              const double &, double &, double &,
                                              double *, int);

    int                    ConstructBounds(const double (*)[3], int);
    void                   ContributeSmallCell(const double (*)[3],
                                     const double (*)[AVT_VARIABLE_LIMIT], int);
    int                    IndexToTriangulationTable(const double(*)[3], int,
                                                     double);
    void                   AddCell(char *);

    static const double     FRUSTUM_MIN_X;
    static const double     FRUSTUM_MAX_X;
    static const double     FRUSTUM_MIN_Y;
    static const double     FRUSTUM_MAX_Y;
    static const double     FRUSTUM_MIN_Z;
    static const double     FRUSTUM_MAX_Z;

    inline int             SnapXRight(double);
    inline int             SnapXLeft(double);
    inline int             SnapYTop(double);
    inline int             SnapYBottom(double);
    inline int             SnapZFront(double);
    inline int             SnapZBack(double);

    inline double           XFromIndex(int);
    inline double           YFromIndex(int);
    inline double           ZFromIndex(int);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtExtractor(const avtExtractor &) {;};
    avtExtractor        &operator=(const avtExtractor &) { return *this; };
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
//    Hank Childs, Sat Jan 29 18:49:00 PST 2005
//    Add checks for degenerate volumes.
//
// ****************************************************************************

int 
avtExtractor::SnapXRight(double x)
{
    if (x_step == 0.)
        return restrictedMinWidth;

    double close_to_index = (x - FRUSTUM_MIN_X) / x_step;
    int   index = (int)ceil((double)close_to_index);

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
avtExtractor::SnapXLeft(double x)
{
    if (x_step == 0.)
        return restrictedMinWidth;

    double close_to_index = (x - FRUSTUM_MIN_X) / x_step;
    int   index = (int)floor((double)close_to_index);

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
avtExtractor::SnapYTop(double y)
{
    if (y_step == 0.)
        return restrictedMinHeight;

    double close_to_index = (y - FRUSTUM_MIN_Y) / y_step;
    int   index = (int)ceil((double)close_to_index);

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
avtExtractor::SnapYBottom(double y)
{
    if (y_step == 0.)
        return restrictedMinHeight;

    double close_to_index = (y - FRUSTUM_MIN_Y) / y_step;
    int   index = (int)floor((double)close_to_index);

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
avtExtractor::SnapZBack(double z)
{
    if (z_step == 0.)
        return 0;

    double close_to_index = (z - FRUSTUM_MIN_Z) / z_step;
    int   index = (int)ceil((double)close_to_index);

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
avtExtractor::SnapZFront(double z)
{
    if (z_step == 0.)
        return 0;

    double close_to_index = (z - FRUSTUM_MIN_Z) / z_step;
    int   index = (int)floor((double)close_to_index);

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

double
avtExtractor::XFromIndex(int xi)
{
    return FRUSTUM_MIN_X + (xi*x_step);
}


double
avtExtractor::YFromIndex(int yi)
{
    return FRUSTUM_MIN_Y + (yi*y_step);
}


double
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
//    Hank Childs, Thu Dec 20 17:02:48 PST 2007
//    Change signature to avoid confusing the compiler.
//
// ****************************************************************************

void
avtExtractor::InterpolateToPlane(const double v1[3], const double v2[3],
                                 const double *val1, const double *val2,
                                 const double &x, double &y, double &z, 
                                 double *v, int nVars)
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
    double t = (x - v1[0]) / (v2[0] - v1[0]);

    y = t*(v2[1] - v1[1]) + v1[1];
    z = t*(v2[2] - v1[2]) + v1[2];
    for (int i = 0 ; i < nVars ; i++)
    {
        v[i] = t*(val2[i] - val1[i]) + val1[i];
    }
}


#endif


