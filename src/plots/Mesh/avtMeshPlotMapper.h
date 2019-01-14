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
//                            avtMeshPlotMapper.h                            //
// ************************************************************************* //

#ifndef AVT_MESHPLOTMAPPER_H
#define AVT_MESHPLOTMAPPER_H

#include <avtMapper.h>


// ****************************************************************************
//  Class:  avtMeshPlotMapper
//
//  Purpose:
//      Mesh plot specific mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 29, 2016
//
//  Modifications:
//
// ****************************************************************************

class avtMeshPlotMapper : public avtMapper
{
  public:
                               avtMeshPlotMapper();
    virtual                   ~avtMeshPlotMapper();

    //virtual void               SetSurfaceRepresentation(int rep);
    virtual bool               GetLighting(void) { return false; }
    virtual bool               ActorIsShiftable(int);


    void                       SetMeshColor(double rgb[3]);
    void                       SetSurfaceColor(double rgb[3]);

    void                       SetOpacity(double val);
    void                       SetLineWidth(int lw);
    void                       SetSurfaceVisibility(bool);


    void                       InvalidateTransparencyCache(void);

  protected:
    virtual void               CustomizeMappers(void);
    virtual void               SetLabels(std::vector<std::string> &, bool);

  private:

    bool surfaceVis;
    int lineWidth;
    double opacity;
    double linesColor[3];
    double polysColor[3];
    std::vector<std::string>   labels;

    void                       NotifyTransparencyActor(void);

};


#endif


