// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtXRayFilter.h                             //
// ************************************************************************* //

#ifndef AVT_XRAY_FILTER_H
#define AVT_XRAY_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtVector.h>

#include <vtkPolyData.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtXRayFilter
//
//  Purpose:
//      It creates an x ray image using an absorbtivity and emissivity.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Jul 16 12:22:55 PDT 2010
//    I modified the filter to handle the case where some of the processors
//    didn't have any data sets when executing in parallel.
//
//    Eric Brugger, Mon Dec  6 12:33:59 PST 2010
//    I modified the view information stored internally to correspond more
//    closely to an avtView3D structure instead of having it match the
//    parameters to SetImageProperty.
//
//    Eric Brugger, Tue Dec 28 14:18:09 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Eric Brugger, Fri Mar 18 14:10:42 PDT 2011
//    I corrected a bug where the filter would crash when running in parallel
//    and the number of pixels processed in a pass was divisible by the
//    number of processors.
//
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Templatized some methods, for double-precision support.
//
//    Kathleen Biagas, Wed Oct 17 14:36:30 PDT 2012
//    Added 'up' argument to SetImageProperties.
//
//    Eric Brugger, Mon Dec  3 13:36:08 PST 2012
//    I added the ability to output the cells intersected by a specified
//    ray to a vtk file.
//
//    Gunther H. Weber, Wed Jan 23 15:23:55 PST 2013
//    Added support for specifying background intensity entering the volume 
//
//    Eric Brugger, Wed Nov 19 15:46:09 PST 2014
//    I modified SetImageProperties so that its arguments map one for one
//    with the actual image properties stored in the class.
//
//    Eric Brugger, Mon Dec  3 13:41:10 PST 2012
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Thu May 21 12:20:04 PDT 2015
//    I added support for debugging a ray.
//
//    Eric Brugger, Wed May 27 10:04:57 PDT 2015
//    I modified the filter to also output the path length field.
//
//    Eric Brugger, Thu Jun  4 15:56:25 PDT 2015
//    I added an option to enable outputting the ray bounds to a vtk file.
// 
//    Justin Privitera, Mon Dec 12 13:28:55 PST 2022
//    SetImageProperties now takes avtVectors for a few arguments.
//
// ****************************************************************************

class AVTFILTERS_API avtXRayFilter : public avtDatasetToDatasetFilter
{
  public:
                                    avtXRayFilter();
    virtual                        ~avtXRayFilter();

    virtual const char             *GetType(void)
                                       { return "avtXRayFilter"; };
    virtual const char             *GetDescription(void)
                                       { return "Performing x ray filter"; };
    virtual void                    UpdateDataObjectInfo(void);

    void                            SetVariableNames(std::string abs,
                                                     std::string emis)
                                       { absVarName = abs;
                                         emisVarName = emis; };

    void                            SetImageProperties(avtVector _normal,
                                                       avtVector _focus,
                                                       avtVector _viewUp,
                                                       double    _viewAngle,
                                                       double    _parallelScale,
                                                       double    _nearPlane,
                                                       double    _farPlane,
                                                       double   *_imagePan,
                                                       double    _imageZoom,
                                                       bool      _perspective,
                                                       int      *_imageSize);
    void                            SetDivideEmisByAbsorb(bool);
    void                            SetBackgroundIntensity(double);
    void                            SetBackgroundIntensities(double *, int);
    void                            SetDebugRay(int);
    void                            SetOutputRayBounds(bool);

  protected:
    std::string                     absVarName;
    std::string                     emisVarName;

    int                             linesForThisPass;
    int                             linesForThisPassFirstProc;
    int                             lineOffset;
    double                         *lines;

    int                             currentNode; //Index of current vtk dataset
    int                             totalNodes; //Total number of vtk datasets

    double                          normal[3], focus[3], viewUp[3];
    double                          viewAngle;
    double                          parallelScale;
    double                          nearPlane, farPlane;
    double                          imagePan[2], imageZoom;
    bool                            perspective;
    int                             imageSize[2];

    bool                            divideEmisByAbsorb;
    double                          backgroundIntensity;
    double                         *backgroundIntensities;
    int                             nBackgroundIntensities;

    int                             numPixels;
    int                             numPixelsPerIteration;

    double                         *intensityBins;
    double                         *pathBins;

    int                             numBins;    //Used for radiation bins.
                                                //Number is obtained from the
                                                //mesh, not set by the user

    int                             iFragment;
    int                             nImageFragments;
    int                            *imageFragmentSizes;
    vtkDataArray                  **intensityFragments;
    vtkDataArray                  **pathLengthFragments;

    int                             iPass;
    int                             numPasses;

    int                             actualPixelsPerIteration;
    int                             pixelsForFirstPass;
    int                             pixelsForLastPass;
    int                             pixelsForFirstPassFirstProc;
    int                             pixelsForFirstPassLastProc;
    int                             pixelsForLastPassFirstProc;
    int                             pixelsForLastPassLastProc;

    int                             debugRay;
    bool                            outputRayBounds;

    virtual void                    Execute(void);

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);

  private:
    template <typename T>
    void                            ImageStripExecute(int, vtkDataSet **);
    template <typename T>
    void                            CartesianExecute(vtkDataSet *, int &,
                                        std::vector<double>&, std::vector<int>&,
                                        T **&);
    template <typename T>
    void                            CylindricalExecute(vtkDataSet *, int &,
                                        std::vector<double>&, std::vector<int>&,
                                        T **&);


    template <typename T>
    void                            RedistributeLines(int, int *,
                                        std::vector<double> *, 
                                        std::vector<int> *,
                                        int,  T ***, int &, int *&,
                                        double *&, T **&);

    void                            CalculateLines(void);

    void                            CheckDataSets(int, vtkDataSet **);

    template <typename T>
    void                            IntegrateLines(int, int, int *, double *,
                                        T *, T *);
    template <typename T>
    void                            CollectFragments(int, int, int*,
                                        vtkDataArray **, vtkDataArray *&);
    void                            MergeFragments(int iBin, vtkDataArray **,
                                        vtkDataArray *&imageArray);

    void                            DumpRayHexIntersections(int, int,
                                        std::vector<int> &, std::vector<int> &,
                                        vtkDataSet *, vtkDataArray **);

    // how cell data will be stored for use internally
    int                             cellDataType;
};


#endif


