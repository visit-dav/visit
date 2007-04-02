/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                             avtMeshFilter.h                               //
// ************************************************************************* //

#ifndef AVT_MESH_FILTER_H
#define AVT_MESH_FILTER_H


#include <avtDataTreeStreamer.h>
#include <MeshAttributes.h>

class vtkExtractEdges;
class vtkGeometryFilter;
class vtkLinesFromOriginalCells;
class vtkDataSet;


// ****************************************************************************
//  Class: avtMeshFilter
//
//  Purpose:
//      A filter that extracts the mesh edges of an avtDataSet.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 8, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Added string argument to Execute method.
//
//    Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002 
//    Added PerformRestriction method.
//
//    Kathleen Bonnell, Thu Feb  5 10:34:16 PST 2004 
//    Added vtkExtractEdges, removed vtkUniqueFeatureEdges.
//
//    Kathleen Bonnell, Tue Nov  2 10:41:33 PST 2004 
//    Added keepNodeZone. 
//
//    Hank Childs, Thu Mar 10 09:13:03 PST 2005
//    Removed data member filters to simplify memory management.  Also removed
//    ReleaseData.
//
// ****************************************************************************

class avtMeshFilter : public avtDataTreeStreamer
{
  public:
                               avtMeshFilter(const MeshAttributes &);
    virtual                   ~avtMeshFilter();

    virtual const char        *GetType(void)  { return "avtMeshFilter"; };
    virtual const char        *GetDescription(void)  
                                   { return "Constructing mesh"; };

  protected:
    MeshAttributes             atts;
    bool                       keepNodeZone;

    virtual avtDataTree_p      ExecuteDataTree(vtkDataSet *, int, string);
    virtual void               RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p     
                               PerformRestriction(avtPipelineSpecification_p spec);
};


#endif


