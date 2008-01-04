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
//                            avtLineScanFilter.h                            //
// ************************************************************************* //

#ifndef AVT_LINE_SCAN_FILTER_H
#define AVT_LINE_SCAN_FILTER_H


#include <avtStreamer.h>
#include <avtVector.h>
#include <filters_exports.h>
#include <string>


// ****************************************************************************
//  Class: avtLineScanFilter
//
//  Purpose:
//      This should really be a query, not a filter.  It sums all of the values
//      for a variable.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Jul 28 09:44:24 PDT 2006
//    Added CylindricalExecute.
//
//    David Bremer, Tue Nov 21 17:35:17 PST 2006
//    Added cylindrical ray distribution for hohlraum flux queries.
//    In this case, generate a cylindrical column of rays, all with 
//    the same direction and evenly distributed in space.  In 2D,
//    use a rectangular column of rays.  Rays may not cover the 
//    entire mesh.  FYI, this is unrelated to "CylindricalExecute"
// ****************************************************************************

class AVTFILTERS_API avtLineScanFilter : public avtStreamer
{
  public:
                                    avtLineScanFilter();
    virtual                        ~avtLineScanFilter();

    void                            SetNumberOfLines(int);

    virtual const char             *GetType(void)
                                             { return "avtLineScanFilter"; };
    virtual const char             *GetDescription(void)
                                             { return "Scanning lines"; };
    virtual void                    RefashionDataObjectInfo(void);
    virtual void                    SetRandomSeed(int s) { seed = s; };

    virtual const double           *GetLines() const { return lines; }

    enum LineDistributionType {
        UNIFORM_RANDOM_DISTRIB,     //Uniform position and direction within
                                    //a circle or sphere enclosing the data
        CYLINDER_DISTRIB            //Uniform spatial distribution along the 
                                    //length of a user-defined cylinder in 3D,
                                    //or a rectangle in 2D.
    };

    virtual void                   SetUniformRandomDistrib();
    virtual void                   SetCylinderDistrib(float *pos_, 
                                                      float  theta_,
                                                      float  phi_, 
                                                      float  radius_);

  protected:
    int                             nLines;
    int                             seed;
    double                         *lines;

    LineDistributionType            distribType;
    avtVector                       pos;
    float                           theta, phi, radius;

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);

    virtual vtkDataSet             *ExecuteData(vtkDataSet *, int,std::string);
    virtual vtkDataSet             *CartesianExecute(vtkDataSet *);
    virtual vtkDataSet             *CylindricalExecute(vtkDataSet *);
};


#endif


