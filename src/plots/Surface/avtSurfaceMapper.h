/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                           avtSurfaceMapper.h                              //
// ************************************************************************* //

#ifndef AVT_SURFACEMAPPER_H
#define AVT_SURFACEMAPPER_H

#include <avtMapper.h>
#include <string>
#include <vector>

class vtkLookupTable;

// ****************************************************************************
//  Class:  avtSurfaceMapper
//
//  Purpose:
//      Surface plot specific mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//
// ****************************************************************************

class avtSurfaceMapper : public avtMapper
{
  public:
                               avtSurfaceMapper();
    virtual                   ~avtSurfaceMapper();

    // these are called from avtMapper
    virtual void               SetSurfaceRepresentation(int rep);
    virtual bool               GetLighting(void) { return !ignoreLighting; }


    // these are called from the plot

    void                       SetEdgeVisibility(bool);
    void                       SetEdgeColor(double rgb[3]);
    void                       SetLineWidth(int lw);
    void                       SetSurfaceColor(double rgb[3]);


    void                       CanApplyGlobalRepresentation(bool);
    void                       SetRepresentation(bool);
    void                       SetIgnoreLighting(bool);
    void                       SetLookupTable(vtkLookupTable *);

    void                       SetScalarVisibility(bool);
    void                       SetScalarRange(double, double);
    bool                       GetDataRange(double &rmin, double &rmax);
    bool                       GetCurrentDataRange(double &rmin, double &rmax);

  protected:
    // these are called from avtMapper
    virtual void               CustomizeMappers(void);

  private:

    bool            edgeVis;
    bool            scalarVis;
    int             lineWidth;
    double          edgeColor[3];
    double          surfaceColor[3];
    bool            canApplyGlobalRep;
    bool            ignoreLighting;
    bool            wireMode;
    vtkLookupTable *lut;
    double          scalarRange[2];

    void                       NotifyTransparencyActor(void);


};


#endif


