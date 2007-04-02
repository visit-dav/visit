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
//                             avtVolumeFilter.h                             //
// ************************************************************************* //

#ifndef AVT_VOLUME_FILTER_H
#define AVT_VOLUME_FILTER_H

#include <avtDatasetToDatasetFilter.h>

#include <VolumeAttributes.h>

#include <avtImage.h>

class     WindowAttributes;


// ****************************************************************************
//  Class: avtVolumeFilter
//
//  Purpose:
//      Decides if a volume renderer should output a software rendered image
//      or if it should output a resampled volume plot.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 24 16:21:39 PST 2004
//    Changed inheritance hierarchy.  This filter now simply does software
//    volume rendering and is used by the volume plot.  It is the interface
//    from the volume plot to the ray tracer.  Also removed many support 
//    methods that are no longer necessary since this filter doesn't switch
//    between multiple modes.
//
// ****************************************************************************

class avtVolumeFilter : public avtDatasetToDatasetFilter
{
  public:
                             avtVolumeFilter();
    virtual                 ~avtVolumeFilter();

    void                     SetAttributes(const VolumeAttributes &);
    virtual const char      *GetType(void) { return "avtVolumeFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Volume rendering"; };

    avtImage_p               RenderImage(avtImage_p, const WindowAttributes &);
    int                      GetNumberOfStages(const WindowAttributes &);

  protected:
    VolumeAttributes         atts;
    char                    *primaryVariable;

    virtual void             Execute(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual void             VerifyInput(void);
};


#endif


