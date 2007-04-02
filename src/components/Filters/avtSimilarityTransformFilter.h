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
//  File: avtSimilarityTransformFilter.h
// ************************************************************************* //

#ifndef AVT_SimilarityTransform_FILTER_H
#define AVT_SimilarityTransform_FILTER_H
#include <filters_exports.h>

#include <avtPluginStreamer.h>
#include <avtTransform.h>
#include <SimilarityTransformAttributes.h>
#include <vtkMatrix4x4.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtSimilarityTransformFilter
//
//  Purpose:
//      A filter that performs transformations where the matrix is a 
//      similarity matrix.  This allows for rotations, translation, and
//      scaling.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Nov 28 16:59:53 PST 2001
//    Added RefashionDataObjectInfo, PerformRestriction.
//
//    Hank Childs, Tue Mar  5 16:05:25 PST 2002
//    Removed RefashionDataObjectInfo, since what is was doing is also
//    applicable to the base class, so I pushed the logic into it.
//
//    Kathleen Bonnell, Thu Apr 10 11:07:48 PDT 2003 
//    Store inverse matrix for possible use later in the pipeline. 
//    Added PostExecute method.
//    
//    Hank Childs, Mon Jun 30 15:03:32 PDT 2003
//    Made this filter no longer be a streamer.  Renamed with Similarity.
//    Also moved to components directory.
//
// ****************************************************************************

class AVTFILTERS_API avtSimilarityTransformFilter : public avtTransform
{
  public:
                         avtSimilarityTransformFilter();
    virtual             ~avtSimilarityTransformFilter();

    virtual const char  *GetType(void)  
                             { return "avtSimilarityTransformFilter"; };
    virtual const char  *GetDescription(void) { return "Transforming"; };

    virtual void         SetAtts(const AttributeGroup*);

  protected:
    SimilarityTransformAttributes   atts;
    vtkMatrix4x4         *M;
    vtkMatrix4x4         *invM;

    void                  SetupMatrix();
    virtual vtkMatrix4x4 *GetTransform() { SetupMatrix(); return M; };
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
    virtual void          RefashionDataObjectInfo(void);
    virtual void          PostExecute(void);
};


#endif
