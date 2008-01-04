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
//                             avtSideVolume.h                               //
// ************************************************************************* //

#ifndef AVT_SIDE_VOLUME_H
#define AVT_SIDE_VOLUME_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtSideVolume
//
//  Purpose:
//      Calculates all of the side volumes of the sides of a cell and then
//      assigns the side volume for the cell to be smallest.
//
//      A side volume is defined as the following:
//      A zone Z is comprised of edges, faces, and nodes.
//      Each face F has a center, C, where C is the linear average
//      of the nodes on F.
//      The center of Z is the linear average of the center of its
//      faces.
//      
//      Then there are two sides for each edge, E.
//      If edge E is made up of points V1 and V2 and E is incident
//      to faces F1 and F2, then one side is a tet consisting of
//      V1, V2, center(F1) and center(Z), where the other tet
//      consists of V1, V2, center(F2) and center(Z).
//      
//      The side volume expression returns either the smallest volume or the 
//      largest volume of the sides of a zone.  This is a little bit of 
//      laziness on our part, since the right thing to do would probably be to 
//      subdivide the mesh into tets and go from there.  But then there would 
//      be additional issues, and all of the customers agree this is a 
//      reasonable thing to do.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Sep 22 15:39:11 PDT 2005
//    Add min and max variants.
//
// ****************************************************************************

class EXPRESSION_API avtSideVolume : public avtSingleInputExpressionFilter
{
  public:
                                avtSideVolume();

    virtual const char         *GetType(void) { return "avtSideVolume"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating side volume"; };

    void                        SetTakeMin(bool tm) { takeMin = tm; };
    
  protected:
    bool                        haveIssuedWarning;
    bool                        takeMin;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *);
    virtual void                PreExecute(void);

    virtual bool                IsPointVariable(void)  { return false; };

    double                      GetZoneVolume(vtkCell *);
};


#endif


