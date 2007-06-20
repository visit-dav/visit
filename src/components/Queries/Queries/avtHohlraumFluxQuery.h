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
//                             avtHohlraumFluxQuery.h                        //
// ************************************************************************* //

#ifndef AVT_HOHLRAUM_FLUX_QUERY_H
#define AVT_HOHLRAUM_FLUX_QUERY_H

#include <query_exports.h>
#include <avtLineScanQuery.h>

// ****************************************************************************
//  Class: avtDistanceFromBoundaryQuery
//
//  Purpose:
//    This query calculates flux from a hohlraum capsule by integrating 
//    random rays, accumulating radiation bin by bin.  It may be extended
//    to be applied to one or more response functions, to match detectors
//    installed in NIF.  
//
//  Programmer: David Bremer
//  Creation:   November 17, 2006
//
// ****************************************************************************

class QUERY_API avtHohlraumFluxQuery : public avtLineScanQuery
{
  public:
                              avtHohlraumFluxQuery();
    virtual                  ~avtHohlraumFluxQuery();

    virtual const char       *GetType(void) 
                                 { return "avtHohlraumFluxQuery"; }
    virtual const char       *GetDescription(void)
                                 { return "Calculating Flux For a Hohlraum"; }

    void                      SetVariableNames(const stringVector &names);
    void                      SetRayCenter(float x, float y, float z);
    void                      SetRadius(float r);
    void                      SetThetaPhi(float thetaInDegrees, float phiInDegrees);

  protected:
    float                     rayCenter[3];
    float                     radius;
    float                     theta, phi;

    std::string               absVarName;  //e.g. "absorbtivity"
    std::string               emisVarName; //e.g. "emissivity"
    double *                  radBins;
    std::vector<double>       binWidths;

    //FYI:  Inherits from avtLineScanQuery
    //int                       numBins;    //Used for radiation bins.
                                            //Number is obtained from the mesh, not set by the user
    //int                       numLines;   //Used just as in avtLineScanQuery
    //double                    minLength;  //Unused
    //double                    maxLength;  //Unused
    //int                       numLinesPerIteration;  //Used just as in avtLineScanQuery
    //std::string               varname;    //Unused
    virtual void              IntegrateLine(int oneSide, int otherSide,
                                            vtkPolyData *output,
                                            vtkIntArray *lineids,
                                            int lineid, double dir[3],
                                            vtkDataArray *absorbtivityBins,
                                            vtkDataArray *emissivityBins,
                                            double *tmpBins);
    virtual avtLineScanFilter *CreateLineScanFilter();
    virtual void              GetSecondaryVars( std::vector<std::string> &outVars );

  private:
    virtual void              ExecuteLineScan(vtkPolyData *pd);
    virtual void              PostExecute(void);
};


#endif
