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
//                             avtSurfaceFilter.h                            //
// ************************************************************************* //

#ifndef AVT_SURFACE_FILTER_H
#define AVT_SURFACE_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

#include <SurfaceFilterAttributes.h>


class vtkDataSet;
class vtkSurfaceFilter;


// ****************************************************************************
//  Class: avtSurfaceFilter
//
//  Purpose:
//    A filter that turns a 2d dataset into a 3d dataset based upon
//    scaled point or cell data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Mar 29 13:54:27 PST 2001 
//    Added method 'SkewTheValue'.
//
//    Kathleen Bonnell, Tue Apr 10 11:46:11 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Tue Jun 12 13:52:27 PDT 2001
//    Renamed several methods to reflect changes in the base class.
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Added filters to extract the edges, for wireframe rendering. 
//    Added PreExecute and ModifyContract.  Added members min, max,
//    Ms, Bs.
//
//    Kathleen Bonnell, Fri Oct 10 10:48:24 PDT 2003
//    Added PostExecute. 
//
//    Mark C. Miller, Sun Feb 29 18:08:26 PST 2004
//    Added zValMin, zValMax data members
//
//    Mark C. Miller, Tue Mar  2 17:55:30 PST 2004
//    Removed zValMin, zValMax
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Moved geofilter, appendFilter and edgesFilter to avtWireframeFilter. 
//    Removed PostExecute. 
//
//    Hank Childs, Fri Jul 30 12:09:16 PDT 2004
//    Added PostExecute.
//
//    Hank Childs, Sun Jan 30 13:48:54 PST 2005
//    Changed attributes and more to support stand-alone filter so Elevate
//    operator can also use.
//
//    Hank Childs, Fri Mar  4 08:47:07 PST 2005
//    Removed cd2pd.
//
// ****************************************************************************

class AVTFILTERS_API avtSurfaceFilter : public avtDataTreeIterator
{
  public:
                            avtSurfaceFilter(const AttributeGroup*);
    virtual                ~avtSurfaceFilter();

    static avtFilter       *Create(const AttributeGroup*);

    virtual const char     *GetType(void)  { return "avtSurfaceFilter"; };
    virtual const char     *GetDescription(void)
                            { return "Elevating 2D data by variable values"; };

    virtual void            ReleaseData(void);
    virtual bool            Equivalent(const AttributeGroup*);

  protected:
    SurfaceFilterAttributes    atts;
    vtkSurfaceFilter          *filter;
    double                     min;
    double                     max;
    double                     Ms;
    double                     Bs;
    double                     zValMin, zValMax;
    bool                       stillNeedExtents;
    bool                       haveIssuedWarning;

    double                  SkewTheValue(const double);
    void                    CalculateScaleValues(double *, double*);

    virtual vtkDataSet     *ExecuteData(vtkDataSet *, int, std::string);
    virtual void            UpdateDataObjectInfo(void);
    virtual void            VerifyInput(void);
    virtual void            PreExecute(void);
    virtual void            PostExecute(void);
    virtual avtContract_p
                            ModifyContract(avtContract_p);
};


#endif


