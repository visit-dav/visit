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
//                               avtMetaData.h                               //
// ************************************************************************* //

#ifndef AVT_META_DATA_H
#define AVT_META_DATA_H

#include <pipeline_exports.h>

#include <avtPipelineSpecification.h>

class     avtFacelist;
class     avtIntervalTree;
class     avtMaterial;
class     avtMixedVariable;
class     avtSpecies;
class     avtTerminatingSource;

// ****************************************************************************
//  Class: avtMetaData
//
//  Purpose:
//      This object is responsible for holding and possibly calculating meta-
//      data.  All of the meta-data is calculated in a lazy evaluation style -
//      only meta-data that is requested is calculated.  In addition, the
//      source is given an opportunity to give its copy of meta-data, in case
//      it is a source from a database and can go fetch some preprocessed 
//      meta-data.  The meta-data object currently only holds spatial and data
//      extents.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 08:05:29 PDT 2001
//    Changed interface to allow for a data specification to be considered
//    when getting materials or facelists.  Blew away outdated comments.
//
//    Jeremy Meredith, Thu Dec 13 16:00:29 PST 2001
//    Removed GetMaterial since it was never used.
//
//    Hank Childs, Tue Aug 12 10:27:25 PDT 2003
//    Added GetMaterial for matvf expressions.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Jeremy Meredith, Wed Jun  9 09:12:09 PDT 2004
//    Added GetSpecies.
//
//    Kathleen Bonnell, Mon Jun 28 08:05:38 PDT 2004 
//    Added optional int arg (for time) to GetMaterial, GetSpecies. 
//
//    Kathleen Bonnell, Thu Jul  1 16:41:57 PDT 2004 
//    Added GetMixedVar.
//
//    Mark C. Miller, Mon Oct 18 13:02:37 PDT 2004
//    Added optional var args to GetDataExtents/GetSpatialExtents
//
// ****************************************************************************

class PIPELINE_API avtMetaData
{
  public:
                                 avtMetaData(avtTerminatingSource *);
    virtual                     ~avtMetaData();

    avtIntervalTree             *GetDataExtents(const char *var = NULL);
    avtIntervalTree             *GetSpatialExtents(const char *var = NULL);

    avtFacelist                 *GetExternalFacelist(int);
    avtMaterial                 *GetMaterial(int, int = -1);
    avtSpecies                  *GetSpecies(int, int = -1);
    avtMixedVariable            *GetMixedVar(int, int = -1);

  protected:
    avtTerminatingSource        *source;

    avtPipelineSpecification_p   GetPipelineSpecification(void);
    avtPipelineSpecification_p   GetPipelineSpecification(int);
};

#endif


