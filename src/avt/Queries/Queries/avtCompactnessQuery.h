/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                           avtCompactnessQuery.h                           //
// ************************************************************************* //

#ifndef AVT_COMPACTNESS_QUERY_H
#define AVT_COMPACTNESS_QUERY_H
#include <query_exports.h>

#include <avtTwoPassDatasetQuery.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

// ****************************************************************************
//  Class: avtCompactnessQuery
//
//  Purpose:
//      This query sums all of the values for a variable.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:49:37 PDT 2003
//    Added additional queries which use a density variable.
//    Made it inherit from the (new) two-pass query.
//
//    Jeremy Meredith, Wed Jul 23 13:29:57 PDT 2003
//    Turned xBound and yBound into class data members, and made them
//    STL vectors, and made it collect the boundary points
//    across all domains and processors.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004 
//    Added VerifyInput. 
//
// ****************************************************************************

class QUERY_API avtCompactnessQuery : public avtTwoPassDatasetQuery
{
  public:
                                    avtCompactnessQuery();
    virtual                        ~avtCompactnessQuery();

    virtual const char             *GetType(void)
                                             { return "avtCompactnessQuery"; };
    virtual const char             *GetDescription(void)
                                             { return descriptionBuffer; };

  protected:
    int                             numDomains;

    std::vector<float>              xBound;
    std::vector<float>              yBound;

    double                          totalXSectArea;
    double                          totalRotVolume;
    double                          distBound_da_xsa;
    double                          distBound_da_vol;
    double                          distBound_dv_xsa;
    double                          distBound_dv_vol;
    double                          distOrigin_da;

    bool                            densityValid;
    double                          totalRotMass;
    double                          centMassX;
    double                          centMassY;
    double                          distBound_dv_den_vol;
    double                          distCMass_dv_den_vol;

    char                            descriptionBuffer[1024];

    virtual void                    Execute1(vtkDataSet *, const int);
    virtual void                    Execute2(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    MidExecute();
    virtual void                    PostExecute(void);
    virtual void                    VerifyInput(void);

    static void                     Get2DCellCentroid(vtkCell*,float&,float&);
    static float                    Get2DCellArea(vtkCell*);
    static float                    Get2DTriangleArea(double*,double*,double*);

};


#endif


