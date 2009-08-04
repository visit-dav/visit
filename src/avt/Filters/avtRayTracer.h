/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                                 avtRayTracer.h                            //
// ************************************************************************* //

#ifndef AVT_RAY_TRACER_H
#define AVT_RAY_TRACER_H

#include <filters_exports.h>

#include <avtDatasetToImageFilter.h>
#include <avtViewInfo.h>

class   avtRayFunction;
class   vtkMatrix4x4;


// ****************************************************************************
//  Class: avtRayTracer
//
//  Purpose:
//      Performs ray tracing, taking in a dataset as a source and has an
//      image as an output.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 16:52:26 PST 2001
//    Added "Get" functions.
//
//    Hank Childs, Sat Feb  3 20:37:01 PST 2001
//    Removed pixelizer and added mechanism to change background color.
//
//    Hank Childs, Tue Feb 13 15:15:50 PST 2001
//    Added ability to insert an opaque image into the rendering.
//
//    Brad Whitlock, Wed Dec 5 11:13:18 PDT 2001
//    Added gradient backgrounds.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Sun Dec  4 18:00:55 PST 2005
//    Add method that estimates number of stages.
//
//    Hank Childs, Mon Jan 16 11:11:47 PST 2006
//    Add support for kernel based sampling.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Wed Dec 24 14:17:03 PST 2008
//    Add method TightenClippingPlanes.
//
// ****************************************************************************

class AVTFILTERS_API avtRayTracer : public avtDatasetToImageFilter
{
  public:
                          avtRayTracer();
    virtual              ~avtRayTracer();

    virtual const char   *GetType(void) { return "avtRayTracer"; };
    virtual const char   *GetDescription(void) { return "Ray tracing"; };
    virtual void          ReleaseData(void);

    void                  SetView(const avtViewInfo &);

    static int            GetNumberOfStages(int, int, int);

    void                  InsertOpaqueImage(avtImage_p);

    void                  SetRayFunction(avtRayFunction *);
    void                  SetScreen(int, int);
    void                  SetSamplesPerRay(int);
    void                  SetBackgroundColor(const unsigned char [3]);
    void                  SetBackgroundMode(int mode);
    void                  SetGradientBackgroundColors(const double [3],
                                                      const double [3]);
    int                   GetSamplesPerRay(void)  { return samplesPerRay; };
    const int            *GetScreen(void)         { return screen; };

    void                  SetKernelBasedSampling(bool v)
                                    { kernelBasedSampling = v; };

  protected:
    avtViewInfo           view;

    int                   screen[2];
    int                   samplesPerRay;
    bool                  kernelBasedSampling;
    int                   backgroundMode;
    unsigned char         background[3];
    double                gradBG1[3];
    double                gradBG2[3];
    avtRayFunction       *rayfoo;
    
    avtImage_p            opaqueImage;

    virtual void          Execute(void);
    virtual avtContract_p ModifyContract(avtContract_p);
    static int            GetNumberOfDivisions(int, int, int);
    virtual bool          FilterUnderstandsTransformedRectMesh();
    void                  TightenClippingPlanes(const avtViewInfo &view,
                                                vtkMatrix4x4 *,
                                                double &, double &);
};


#endif


