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
//                            avtDataObjectWriter.h                          //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_WRITER_H
#define AVT_DATA_OBJECT_WRITER_H

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <TypeRepresentation.h>

#include <avtDataset.h>
#include <avtImage.h>
#include <avtOriginatingSink.h>

class     avtDataObjectInformation;
class     avtDataObjectString;
class     AttributeGroup;


// ****************************************************************************
//  Class: avtDataObjectWriter
//
//  Purpose:
//      A module that you can hand a data object (meaning dataset or image to)
//      and it will set up the correct writer.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Feb 12 08:38:18 PST 2001
//    Added methods to get the input out for the ui process in parallel.
//
//    Jeremy Meredith, Thu Jul 26 12:36:30 PDT 2001
//    Made it derive from avtOriginatingSink so we could do dynamic
//    load balancing.
//
//    Hank Childs, Mon Oct  1 08:56:17 PDT 2001
//    Made this be an inheritance layer rather than a container for different
//    types of data objects.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Mark C. Miller, Mon Oct 31 18:12:49 PST 2005
//    Added code to support compression of data object string
//
//    Kathleen Bonnell, Mon Nov 27 12:32:24 PST 2006
//    Added WriteAtts method. 
//
// ****************************************************************************

class PIPELINE_API avtDataObjectWriter : virtual public avtOriginatingSink
{
  public:
                               avtDataObjectWriter();
    virtual                   ~avtDataObjectWriter();

    void                       Write(avtDataObjectString &);

    void                       SetDestinationFormat(const TypeRepresentation&);
    void                       SetUseCompression(bool val) { useCompression = val; };

    void                       WriteInt(avtDataObjectString &, int) const;
    void                       WriteFloat(avtDataObjectString &, float) const;
    void                       WriteDouble(avtDataObjectString &,double) const;
    void                       WriteInt(avtDataObjectString &, const int *,
                                        int) const;
    void                       WriteFloat(avtDataObjectString &, const float *,
                                          int) const;
    void                       WriteDouble(avtDataObjectString &,
                                           const double *, int) const;
    void                       WriteAtts(avtDataObjectString &,
                                         AttributeGroup *) const;

    virtual bool               MustMergeParallelStreams(void) { return false;};

  protected:
    TypeRepresentation         sourceFormat;
    TypeRepresentation         destinationFormat;
    bool                       useCompression;

    virtual void               DataObjectWrite(avtDataObjectString &) = 0;
};


typedef  ref_ptr<avtDataObjectWriter>  avtDataObjectWriter_p;


#endif


