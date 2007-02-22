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
//                             avtLocateQuery.h                              //
// ************************************************************************* //

#ifndef AVT_LOCATE_QUERY_H
#define AVT_LOCATE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>

class vtkDataSet;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtLocateQuery
//
//  Purpose:
//      This query locates a cell or node and domain given a world-coordinate 
//      point or ray. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004
//    Added VerifyInput.
//
//    Kathleen Bonnell, Wed Oct  6 10:48:23 PDT 2004 
//    Added RayIntersectsDataSet.
//
//    Kathleen Bonnell, Mon Jun 27 15:46:29 PDT 2005
//    Removed ijk args from RGridIsect, made it return int instead of bool. 
//
//    Kathleen Bonnell, Wed Jun 14 16:41:03 PDT 2006
//    Add ApplyFilters.
//
// ****************************************************************************

class QUERY_API avtLocateQuery : public avtDatasetQuery
{
  public:
                                    avtLocateQuery();
    virtual                        ~avtLocateQuery();

    void                            SetPickAtts(const PickAttributes *);
    const PickAttributes           *GetPickAtts(void);


  protected:
    PickAttributes                  pickAtts;
    int                             foundDomain;
    int                             foundElement;
    double                          minDist;

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual void                    VerifyInput(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
    bool                            RayIntersectsDataSet(vtkDataSet *);
    int                             RGridIsect(vtkRectilinearGrid *, 
                                               double &dist, 
                                               double isect[3]);
                                               
    int                             LocatorFindCell(vtkDataSet *ds, 
                                                    double &dist, 
                                                    double *isect);
};


#endif


