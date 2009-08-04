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
//                           avtWireframeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_WIREFRAME_FILTER_H
#define AVT_WIREFRAME_FILTER_H


#include <avtDataTreeIterator.h>
#include <SurfaceAttributes.h>


class vtkDataSet;
class vtkAppendPolyData;
class vtkUniqueFeatureEdges;
class vtkGeometryFilter;


// ****************************************************************************
//  Class: avtWireframeFilter
//
//  Purpose:
//    A filter that turns a 2d dataset into a 3d dataset based upon
//    scaled point or cell data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 24, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jul 30 12:09:16 PDT 2004
//    Moved PostExecute to avtSurfaceFilter.
//
// ****************************************************************************

class avtWireframeFilter : public avtDataTreeIterator
{
  public:
                            avtWireframeFilter(const AttributeGroup*);
    virtual                ~avtWireframeFilter();

    static avtFilter       *Create(const AttributeGroup*);

    virtual const char     *GetType(void)  { return "avtWireframeFilter"; };
    virtual const char     *GetDescription(void)
                            { return "Creating wireframe of surface."; };

    virtual void            ReleaseData(void);
    virtual bool            Equivalent(const AttributeGroup*);

  protected:
    SurfaceAttributes       atts;
    vtkGeometryFilter      *geoFilter;
    vtkAppendPolyData      *appendFilter;
    vtkUniqueFeatureEdges  *edgesFilter;

    virtual vtkDataSet     *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


