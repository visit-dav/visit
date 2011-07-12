/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

    void                            SetImageProperties(float *pos,
                                                       float  theta,
                                                       float  phi,
                                                       float  dx,
                                                       float  dy,
                                                       int    nx,
                                                       int    ny);
    void                            SetDivideEmisByAbsorb(bool);

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

    int                             numPixels;
    int                             numPixelsPerIteration;

    double                         *radBins;

    int                             numBins;    //Used for radiation bins.
                                                //Number is obtained from the
                                                //mesh, not set by the user

    int                             iFragment;
    int                             nImageFragments;
    int                            *imageFragmentSizes;
    float                         **imageFragments;

    int                             actualPixelsPerIteration;
    int                             pixelsForFirstPass;
    int                             pixelsForLastPass;
    int                             pixelsForFirstPassFirstProc;
    int                             pixelsForFirstPassLastProc;
    int                             pixelsForLastPassFirstProc;
    int                             pixelsForLastPassLastProc;

    virtual void                    Execute(void);

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);

    virtual void                    CartesianExecute(vtkDataSet *, int &,
                                        std::vector<double>&, std::vector<int>&,
                                        float **&);
    virtual void                    CylindricalExecute(vtkDataSet *, int &,
                                        std::vector<double>&, std::vector<int>&,
                                        float **&);

  private:
    void                            ImageStripExecute(int, vtkDataSet **,
                                        int &, int *&, double *&, float **&);

    void                            RedistributeLines(int, int *,
                                        std::vector<double> *, std::vector<int> *,
                                        int,  float ***, int &, int *&,
                                        double *&, float **&);

    void                            CalculateLines(void);

    void                            CheckDataSets(int, vtkDataSet **);

    void                            IntegrateLines(int, int, int *, double *,
                                        float *, float *);
    float                          *CollectImages(int, int, int*, float **);

};


#endif


