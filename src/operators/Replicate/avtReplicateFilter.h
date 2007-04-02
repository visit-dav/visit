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
//  File: avtReplicateFilter.h
// ************************************************************************* //

#ifndef AVT_Replicate_FILTER_H
#define AVT_Replicate_FILTER_H


#include <avtDataTreeStreamer.h>
#include <avtPluginFilter.h>
#include <ReplicateAttributes.h>


class vtkDataArray;
class vtkDataSet;
class vtkPointSet;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtReplicateFilter
//
//  Purpose:
//      A plugin operator for Replicate.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 23 12:27:29 EST 2007
//    Added support for transformed rectilinear grids.  Also fixed a typo.
//
// ****************************************************************************

class avtReplicateFilter : public virtual avtDataTreeStreamer,
                         public virtual avtPluginFilter
{
  public:
                         avtReplicateFilter();
    virtual             ~avtReplicateFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtReplicateFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Replicating the data"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    ReplicateAttributes   atts;

    virtual void          PostExecute(void);
    virtual avtDataTree_p ExecuteDataTree(vtkDataSet *, int, std::string);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
    virtual void          RefashionDataObjectInfo(void);

    vtkDataSet           *Replicate(vtkDataSet *, double[3]);
    vtkDataSet           *ReplicateRectilinear(vtkRectilinearGrid *, double[3]);
    vtkDataSet           *ReplicatePointSet(vtkPointSet *, double[3]);
    vtkDataArray         *OffsetDataArray(vtkDataArray *, double);

    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif
