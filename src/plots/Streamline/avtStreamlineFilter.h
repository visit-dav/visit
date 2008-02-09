/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              avtStreamlineFilter.h                        //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_FILTER_H
#define AVT_STREAMLINE_FILTER_H
#include <avtStreamer.h>

class vtkVisItStreamLine;
class vtkTubeFilter;
class vtkPolyData;
class vtkRibbonFilter;
class vtkAppendPolyData;

#define STREAMLINE_SOURCE_POINT      0
#define STREAMLINE_SOURCE_LINE       1
#define STREAMLINE_SOURCE_PLANE      2
#define STREAMLINE_SOURCE_SPHERE     3
#define STREAMLINE_SOURCE_BOX        4

#define STREAMLINE_COLOR_SOLID       0
#define STREAMLINE_COLOR_SPEED       1
#define STREAMLINE_COLOR_VORTICITY   2

#define STREAMLINE_DISPLAY_LINES     0
#define STREAMLINE_DISPLAY_TUBES     1
#define STREAMLINE_DISPLAY_RIBBONS   2

// ****************************************************************************
// Class: avtStreamlineFilter
//
// Purpose:
//    This operator is the implied operator associated with a Streamline plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 4 15:32:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 12:45:29 PDT 2004
//   Added ability to color by vorticity and the ability to display as ribbons.
//
//   Brad Whitlock, Mon Jan 3 10:56:47 PDT 2005
//   Added SetZToZero to ensure that we can set restrict streamline sources
//   to 2D when the input data is 2D.
//
//   Brad Whitlock, Tue Jan 4 10:56:47 PDT 2005
//   Removed the integrator member since it was not being used.
//
//   Hank Childs, Sat Mar  3 09:43:57 PST 2007
//   Add new data member: useWholeBox.  Also added method PreExecute to 
//   support it.  Added PostExecute method to get the extents right.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add support for streamline direction option.
//
// ****************************************************************************

class avtStreamlineFilter : public avtStreamer
{
  public:
                              avtStreamlineFilter();
    virtual                  ~avtStreamlineFilter();

    virtual const char       *GetType(void)   { return "avtStreamlineFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating streamlines"; };

    virtual void              ReleaseData(void);
//    bool                      Equivalent(bool, int);

    // Methods to set the filter's attributes.
    void                      SetSourceType(int sourceType);
    void                      SetStepLength(double len);
    void                      SetMaxTime(double mt);

    void                      SetPointSource(double pt[3]);
    void                      SetLineSource(double pt[3], double pt2[3]);
    void                      SetPlaneSource(double O[3], double N[3],
                                             double U[3], double R);
    void                      SetSphereSource(double O[3], double R);
    void                      SetBoxSource(double E[6]);
    void                      SetUseWholeBox(bool b)
                                      { useWholeBox = b; };

    void                      SetDisplayMethod(int d);
    void                      SetShowStart(bool);
    void                      SetRadius(double rad);
    void                      SetPointDensity(int den);
    void                      SetStreamlineDirection(int dir);
    void                      SetColoringMethod(int);

  protected:
    int    sourceType;   
    double stepLength;
    double maxTime;
    double radius;
    int    displayMethod;
    bool   showStart;
    int    pointDensity;
    int    streamlineDirection;
    int    coloringMethod;

    // Various starting locations for streamlines.
    double pointSource[3];
    double lineStart[3], lineEnd[3];
    double planeOrigin[3], planeNormal[3], planeUpAxis[3], planeRadius;
    double sphereOrigin[3], sphereRadius;
    double boxExtents[6];
    bool   useWholeBox;

    // Internal filters.
    vtkVisItStreamLine       *streamline;
    vtkTubeFilter            *tubes;
    vtkRibbonFilter          *ribbons;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              UpdateDataObjectInfo(void);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);

    vtkPolyData              *AddStartSphere(vtkPolyData *, float val,
                                             double pt[3]);

    void                      SetZToZero(vtkPolyData *) const;
};


#endif
