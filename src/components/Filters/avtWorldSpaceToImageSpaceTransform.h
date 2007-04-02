/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                    avtWorldSpaceToImageSpaceTransform.h                   //
// ************************************************************************* //

#ifndef AVT_WORLD_SPACE_TO_IMAGE_SPACE_TRANSFORM_H
#define AVT_WORLD_SPACE_TO_IMAGE_SPACE_TRANSFORM_H
#include <filters_exports.h>


#include <vector>

#include <avtTransform.h>
#include <avtViewInfo.h>


class   avtIntervalTree;


// ****************************************************************************
//  Class: avtWorldSpaceToImageSpaceTransform
//
//  Purpose:
//      Transforms an avtDataset by a view matrix.
//
//  Note:     This class should probably redefine the CalcDomainList method and
//            have it cull away unused domains using a spatial extents interval
//            tree.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 18:33:16 PST 2001
//    Add support for aspect ratios.
//
//    Hank Childs, Fri Nov 19 13:38:21 PST 2004
//    Define ExecuteData so we can pass rectilinear grids through if specified.
//
//    Jeremy Meredith, Wed Jan 17 11:39:01 EST 2007
//    Added support for transformed rectilinear grids.
//
// ****************************************************************************

class AVTFILTERS_API avtWorldSpaceToImageSpaceTransform : public avtTransform
{
  public:
                       avtWorldSpaceToImageSpaceTransform(const avtViewInfo &,
                                                          double);
                       avtWorldSpaceToImageSpaceTransform(const avtViewInfo &,
                                                          const double *);
    virtual           ~avtWorldSpaceToImageSpaceTransform();

    virtual const char  *GetType(void)
                              { return "avtWorldSpaceToImageSpaceTransform"; };
    virtual const char  *GetDescription(void)
                              { return "Transforming data to image cube"; };

    static void        GetDomainsList(const avtViewInfo &, std::vector<int> &,
                                      const avtIntervalTree *);

    void               TightenClippingPlanes(bool t)
                              { tightenClippingPlanes = t; };
    void               SetPassThruRectilinearGrids(bool t)
                              { passThruRectilinear = t; };

  protected:
    vtkMatrix4x4           *transform;
    avtViewInfo             view;
    double                  scale[3];
    double                  aspect;
    bool                    tightenClippingPlanes;
    bool                    passThruRectilinear;

    virtual vtkMatrix4x4   *GetTransform(void);
    virtual vtkDataSet     *ExecuteData(vtkDataSet *, int, std::string);

    static void             CalculatePerspectiveTransform(const avtViewInfo &,
                                                          vtkMatrix4x4 *);
    static void             CalculateOrthographicTransform(const avtViewInfo &,
                                                           vtkMatrix4x4 *);
    static void             CalculateTransform(const avtViewInfo &,
                                       vtkMatrix4x4 *, const double *, double);
    virtual avtPipelineSpecification_p
                            PerformRestriction(avtPipelineSpecification_p);

    virtual void            RefashionDataObjectInfo(void);
    virtual void            PreExecute(void);
    virtual bool            FilterUnderstandsTransformedRectMesh();
};


#endif


