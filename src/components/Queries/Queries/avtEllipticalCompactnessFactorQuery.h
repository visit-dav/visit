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
//                   avtEllipticalCompactnessFactorQuery.h                   //
// ************************************************************************* //

#ifndef AVT_ELLIPTICAL_COMPACTNESS_FACTOR_QUERY_H
#define AVT_ELLIPTICAL_COMPACTNESS_FACTOR_QUERY_H

#include <avtTwoPassDatasetQuery.h>

#include <query_exports.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

class avtBinaryMultiplyFilter;
class avtVMetricVolume;
class avtRevolvedVolume;


// ****************************************************************************
//  Class: avtEllipticalCompactnessFactorQuery
//
//  Purpose:
//      Calculates the elliptical compactness factor of a data set.  This will
//      calculate the volume of the data set.  It will then propose many
//      possible ellipses that have the same volume.  Then it
//      will calculate what percentage of the data set is within each of
//      the ellipses.  The ECF is the highest percentage.
//
//  Programmer: Hank Childs
//  Creation:   May 16, 2006
//
// ****************************************************************************

class QUERY_API avtEllipticalCompactnessFactorQuery 
    : public avtTwoPassDatasetQuery
{
  public:
                                    avtEllipticalCompactnessFactorQuery();
    virtual                        ~avtEllipticalCompactnessFactorQuery();

    virtual const char             *GetType(void)
                         {return "avtEllipticalCompactnessFactorQuery";};
    virtual const char             *GetDescription(void)
                         {return "Calculating Elliptical Compactness Factor";};

  protected:
    double                          centroid[3];
    double                          ellipse_center[3];
    // NOTE: numGuesses should have an integer sqrt.
    static const int                numGuesses = 289;
    double                          x_radius[numGuesses];
    double                          y_radius[numGuesses];
    double                          z_radius[numGuesses];
    double                          bounds[6];
    double                          total_volume;
    double                          volume_inside[numGuesses];
    bool                            is2D;
    avtRevolvedVolume              *rev_volume;
    avtVMetricVolume               *volume;

    virtual void                    Execute1(vtkDataSet *, const int);
    virtual void                    Execute2(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    MidExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
};


#endif


