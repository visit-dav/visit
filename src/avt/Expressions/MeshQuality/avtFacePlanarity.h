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
//                            avtFacePlanarity.h                             //
// ************************************************************************* //

#ifndef AVT_FACE_PLANARITY_H
#define AVT_FACE_PLANARITY_H

#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtFacePlanarity
//
//  Purpose: Calculates either relative or absolute face planarity for a cell.
//  Face planarity is defined ONLY for cells that have at least one face that
//  is non-simplex (e.g. non-triangle). That means wedge, pyramid and hex
//  cells. For a given face, a face planarity measure of zero means all the
//  points on the face lie on the same plane. This is true by definition for
//  triangular faces. For quadrilateral faces, the distance between the 4th
//  node and the plane defined by the first 3 nodes represents the face
//  planarity measure. If this distance is divided by the average of the 
//  lengths of the 4 edges, it is a relative planarity measure for the face. 
//  Finally, the face planarity measure for a whole cell consisting
//  of potentially several non-triangular faces is taken to be the maximum
//  over its faces.
//
//  Programmer: Mark C. Miller
//  Created:    Wed Jun  3 12:34:41 PDT 2009
//
// ****************************************************************************

class EXPRESSION_API avtFacePlanarity : public avtSingleInputExpressionFilter
{
  public:
                                avtFacePlanarity();

    virtual const char         *GetType(void) { return "avtFacePlanarity"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating face planarities"; };

    void                        SetTakeRelative(bool tr) { takeRel = tr; };
    
  protected:
    bool                        takeRel;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *);
    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };
};


#endif


