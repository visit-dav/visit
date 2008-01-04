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
//                          avtViscousStressFilter.h                         //
// ************************************************************************* //

#ifndef AVT_VISCOUS_STRESS_FILTER_H
#define AVT_VISCOUS_STRESS_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtViscousStressFilter
//
//  Purpose:
//      Constructs a viscous stress tensor given a velocity vector.
//
//  Programmer: Cyrus Harrison
//  Creation:   June  5, 2007
//
//  Modifications:
//   Cyrus Harrison, Sun Jun 10 14:18:08 PDT 2007
//   Added CalculateVStress2D helper function.
//
// ****************************************************************************

class EXPRESSION_API avtViscousStressFilter
    : public avtSingleInputExpressionFilter
{
  public:
                             avtViscousStressFilter();
    virtual                 ~avtViscousStressFilter();

    virtual const char      *GetType(void)
                             { return "avtViscousStressFilter"; };
    virtual const char      *GetDescription(void)
                             { return "Calculating viscous stress";};

  protected:
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *);
    virtual avtVarType       GetVariableType(void) { return AVT_TENSOR_VAR; };
    virtual bool             IsPointVariable(void) { return false; };
    virtual void             CalculateVStress2D(vtkDataSet *ds,
                                                vtkDataArray *vel,
                                                int idx,
                                                bool rz_mesh,
                                                double *vstress);


};

#endif
