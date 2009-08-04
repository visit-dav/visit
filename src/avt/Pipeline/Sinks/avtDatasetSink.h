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
//                               avtDatasetSink.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_SINK_H
#define AVT_DATASET_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtDataset.h>


// ****************************************************************************
//  Class: avtDatasetSink
//
//  Purpose:
//      This is a data object sink whose input is a dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//    Hank Childs, Sun May 26 18:55:01 PDT 2002
//    Make GetInput a public method like it is in the base type.
//
//    Brad Whitlock, Thu Oct 9 15:05:27 PST 2003
//    Fixed the friend declaration so it builds with newer g++ versions.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Tom Fogal, Tue Jun 23 19:57:34 MDT 2009
//    Added const edition of GetInput.
//
// ****************************************************************************

class PIPELINE_API avtDatasetSink : virtual public avtDataObjectSink
{
    friend class                  avtMultipleInputSink;

  public:
                                  avtDatasetSink();
    virtual                      ~avtDatasetSink();

    virtual avtDataObject_p       GetInput(void);
    virtual const avtDataObject_p GetInput(void) const;

  protected:
    avtDataTree_p                 GetInputDataTree();
    avtDataset_p                  GetTypedInput(void) { return input; };

    virtual void                  SetTypedInput(avtDataObject_p);

  private:
    avtDataset_p              input;
};


#endif


