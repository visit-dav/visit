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
//                            avtFacelistFilter.h                            //
// ************************************************************************* //

#ifndef AVT_FACELIST_FILTER_H
#define AVT_FACELIST_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeStreamer.h>


class   vtkRectilinearGridFacelistFilter;
class   vtkStructuredGridFacelistFilter;
class   vtkUnstructuredGridFacelistFilter;
class   vtkPolyData;

class   avtMultiFacelist;


// ****************************************************************************
//  Class: avtFacelistFilter
//
//  Purpose:
//      A filter that takes determines the facelist and outputs it as polydata.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Apr 12 10:49:10 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Thu Sep  6 11:14:38 PDT 2001
//    Removed logic for preventing dynamic load balancing.
//
//    Eric Brugger, Wed Jan 23 15:18:39 PST 2002
//    I modified the class to use vtkUnstructuredGridFacelistFilter instead
//    of vtkGeometryFilter for unstructured grids.
//
//    Jeremy Meredith, Tue Jul  9 14:00:32 PDT 2002
//    Added the "create3DCellNumbers" flag.
//
//    Hank Childs, Sun Aug 18 11:18:20 PDT 2002
//    Make special accomodations for meshes that are made up of disjoint
//    elements.
//
//    Hank Childs, Tue Sep 10 12:36:42 PDT 2002
//    Redefine ReleaseData.
//
//    Hank Childs, Wed Oct  2 16:59:10 PDT 2002
//    Removed unused data member f2d.
//
//    Hank Childs, Wed Aug 11 09:46:53 PDT 2004
//    Added PerformRestriction.
//
//    Kathleen Bonnell, Fri Feb 18 11:13:16 PST 2005 
//    Added ConvertToPolys. 
//
//    Hank Childs, Fri Mar 11 08:07:26 PST 2005
//    Remove data member filters.  Also remove ReleaseData.
//
//    Hank Childs, Fri Sep 23 10:48:36 PDT 2005
//    Add a flag that will cause edge list generation for 2D datasets.
//
//    Hank Childs, Wed Dec 20 09:25:42 PST 2006
//    Add a flag that forces poly data construction.
//
//    Hank Childs, Thu Dec 28 09:08:34 PST 2006
//    Re-inherit from data tree streamer (a single input may now produce
//    multiple outputs ... 3D structured grid gives 6 2D structured grids).
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
// ****************************************************************************

class AVTFILTERS_API avtFacelistFilter : public avtDataTreeStreamer
{
  public:
                                         avtFacelistFilter();
    virtual                             ~avtFacelistFilter();

    virtual const char                  *GetType(void)
                                               { return "avtFacelistFilter"; };
    virtual const char                  *GetDescription(void)
                                     { return "Calculating external faces"; };

    void                                 SetCreate3DCellNumbers(bool);
    void                                 SetForceFaceConsolidation(bool);
    void                                 SetCreateEdgeListFor2DDatasets(bool);
  
    void                                 MustCreatePolyData(void)
                                              { mustCreatePolyData = true; };

  protected:
    bool                                 useFacelists;
    bool                                 create3DCellNumbers;
    bool                                 createEdgeListFor2DDatasets;
    bool                                 mustCreatePolyData;
    int                                  forceFaceConsolidation;

    virtual avtDataTree_p                ExecuteDataTree(vtkDataSet *, int,
                                                     std::string);
    vtkDataSet                          *Take2DFaces(vtkDataSet *);
    vtkDataSet                          *FindEdges(vtkDataSet *);
    avtDataTree_p                        Take3DFaces(vtkDataSet *, int,
                                                     std::string);
    vtkDataSet                          *ConvertToPolys(vtkDataSet *, int);

    virtual void                         InitializeFilter(void);

    virtual void                         RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p   PerformRestriction(
                                                   avtPipelineSpecification_p);
    virtual bool                        FilterUnderstandsTransformedRectMesh();
};


#endif


