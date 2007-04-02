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
//                           avtLocateCellQuery.h                            //
// ************************************************************************* //

#ifndef AVT_LOCATE_CELL_QUERY_H
#define AVT_LOCATE_CELL_QUERY_H
#include <query_exports.h>

#include <avtLocateQuery.h>

class vtkDataSet;

// ****************************************************************************
//  Class: avtLocateCellQuery
//
//  Purpose:
//      This query locates a cell and domain given a world-coordinate point. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    Added data member 'minDist'.
//
//    Kathleen Bonnell, Mon Apr 14 09:43:11 PDT 2003
//    Added member invTransform.
//
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003   
//    Removed member invTransform.
//
//    Kathleen Bonnell, Wed May  7 13:24:37 PDT 2003 
//    Add methods 'RGridFindCell' and 'LocatorFindCell'.
//
//    Kathleen Bonnell, Tue Jun  3 15:20:35 PDT 2003 
//    Removed 'tolerance' parameter from FindCell methods.
//
//    Kathleen Bonnell, Fri Oct 10 11:45:24 PDT 2003
//    Added DeterminePickedNode. 
//
//    Kathleen Bonnell, Tue Nov  4 08:18:54 PST 2003 
//    Added  SetPickAtts, GetPickAtts, and PickAttributes data member.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Tue May 18 13:12:09 PDT 2004
//    Inherit from avtLocateQuery.  Moved Node specific code to 
//    avtLocateNodeQuery. 
//
//    Kathleen Bonnell, Wed Jul  7 14:48:44 PDT 2004 
//    Added FindClosestCell, for use with line plots. 
//
// ****************************************************************************

class QUERY_API avtLocateCellQuery : public avtLocateQuery
{
  public:
                                    avtLocateCellQuery();
    virtual                        ~avtLocateCellQuery();

    virtual const char             *GetType(void)
                                             { return "avtLocateCellQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Locating cell."; };

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
    int                             RGridFindCell(vtkDataSet *, 
                                                    double &, double*);
    int                             FindClosestCell(vtkDataSet *ds, 
                                                    double &minDist, 
                                                    double isect[3]);
};


#endif


