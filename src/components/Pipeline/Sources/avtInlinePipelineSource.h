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
//                         avtInlinePipelineSource.h                         //
// ************************************************************************* //

#ifndef AVT_INLINE_PIPELINE_SOURCE_H
#define AVT_INLINE_PIPELINE_SOURCE_H

#include <pipeline_exports.h>

#include <avtTerminatingSource.h>


// ****************************************************************************
//  Class: avtInlinePipelineSource
//
//  Purpose:
//      There are occasions where a pipeline is "inlined" inside another
//      pipeline.  When that occurs, the there needs to be a new terminating
//      source -- avtInlinePipelineSource.  That source should be smart enough
//      to act like its own terminating source in some instances and to bypass
//      to the pipeline's real terminating source in others.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  9 09:14:57 PDT 2004
//    Added species aux data.
//
//    Hank Childs, Sat Mar  5 18:36:02 PST 2005
//    Turn off load balancing for inline pipeline sources.
//
// ****************************************************************************

class PIPELINE_API avtInlinePipelineSource : virtual public avtTerminatingSource
{
  public:
                           avtInlinePipelineSource(avtDataObject_p);
    virtual               ~avtInlinePipelineSource();

    virtual avtDataSpecification_p 
                           GetFullDataSpecification(void);

  protected:
    avtTerminatingSource  *realPipelineSource;

    virtual void           FetchMeshAuxiliaryData(const char *type, void *args,
                               avtDataSpecification_p, VoidRefList &);
    virtual void           FetchVariableAuxiliaryData(const char *type,
                               void *args, avtDataSpecification_p,
                               VoidRefList &);
    virtual void           FetchMaterialAuxiliaryData(const char *type,
                               void *args, avtDataSpecification_p,
                               VoidRefList &);
    virtual void           FetchSpeciesAuxiliaryData(const char *type,
                               void *args, avtDataSpecification_p,
                               VoidRefList &);

    virtual bool           ArtificialPipeline(void)  { return true; };
    virtual bool           UseLoadBalancer(void) { return false; };
};


#endif


