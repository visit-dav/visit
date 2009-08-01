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
//  File: avtTransformFilter.h
// ************************************************************************* //

#ifndef AVT_Transform_FILTER_H
#define AVT_Transform_FILTER_H


#include <avtPluginFilter.h>
#include <avtSingleFilterFacade.h>
#include <TransformAttributes.h>

class avtLinearTransformFilter;
class avtSimilarityTransformFilter;
class avtCoordSystemConvert;


// ****************************************************************************
//  Class: avtTransformFilter
//
//  Purpose:
//      A plugin operator for Transform.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Nov 28 16:59:53 PST 2001
//    Added UpdateDataObjectInfo, ModifyContract.
//
//    Hank Childs, Tue Mar  5 16:05:25 PST 2002
//    Removed UpdateDataObjectInfo, since what is was doing is also
//    applicable to the base class, so I pushed the logic into it.
//
//    Kathleen Bonnell, Thu Apr 10 11:07:48 PDT 2003 
//    Store inverse matrix for possible use later in the pipeline. 
//    Added PostExecute method.
//    
//    Hank Childs, Tue Jul  1 08:59:08 PDT 2003
//    Moved original avtTransformFilter to libpipeline with the name
//    avtSimilarityTransformFilter.  Re-worked this filter to be a single
//    filter facade.
//
//    Hank Childs, Tue Feb  1 16:37:56 PST 2005
//    Allow for coordinate system transformations as well.
//
//    Jeremy Meredith, Tue Apr 15 13:17:24 EDT 2008
//    Added linear transform.
//
//    Tom Fogal, Tue Jun 23 20:53:28 MDT 2009
//    Added const version of GetFacadedFilter.
//
// ****************************************************************************

class avtTransformFilter : public virtual avtPluginFilter,
                           public virtual avtSingleFilterFacade
{
  public:
                         avtTransformFilter();
    virtual             ~avtTransformFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtTransformFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Transforming"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    TransformAttributes            atts;
    avtLinearTransformFilter      *ltf;
    avtSimilarityTransformFilter  *stf;
    avtCoordSystemConvert         *csc;

    virtual avtFilter             *GetFacadedFilter(void);
    virtual const avtFilter       *GetFacadedFilter(void) const;
};

#endif
