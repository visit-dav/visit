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
//                                avtDrawer.h                                //
// ************************************************************************* //

#ifndef AVT_DRAWER_H
#define AVT_DRAWER_H
#include <plotter_exports.h>


#include <avtDatasetToImageFilter.h>
#include <avtMapper.h>
#include <avtViewInfo.h>

class vtkCamera;


// ****************************************************************************
//  Class: avtDrawer
//
//  Purpose:
//    Creates an avtImage from an avtGeometryDrawable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 04, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jan  8 11:48:07 PST 2001
//    Inherit from avtDatasetToImageFilter. 
//
// ****************************************************************************

class PLOTTER_API avtDrawer : public avtDatasetToImageFilter
{
  public:
                          avtDrawer(avtMapper *);
    virtual              ~avtDrawer();

    virtual const char   *GetType(void) { return "avtDrawer"; };

    void                  SetWindowSize(const int size[2]);
    void                  SetWindowSize(const int, const int);

    void                  SetViewInfo(const avtViewInfo &);
    void                  SetViewInfo(vtkCamera *);

  protected:
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
    virtual void          Execute(void);

    avtMapper            *mapper;
    avtViewInfo           viewInfo;
    int                   windowSize[2];
};

#endif


