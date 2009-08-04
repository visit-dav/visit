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
//                            avtDataSetWriter.h                             //
// ************************************************************************* //

#ifndef AVT_DATASET_WRITER_H
#define AVT_DATASET_WRITER_H

#include <pipeline_exports.h>

#include <avtTerminatingDatasetSink.h>
#include <avtDataObjectWriter.h>


class     avtDataObjectString;
class     avtDataTree;


// ****************************************************************************
//  Class: avtDataSetWriter
//
//  Purpose:
//      A class which takes as input an avtDataSet and can serialize it.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Brad Whitlock, Mon Oct 23 10:58:35 PDT 2000
//    I renamed the type reps and added a private method to write an integer.
//
//    Hank Childs, Thu Dec 28 16:27:33 PST 2000
//    Stripped out a lot of functionality to go into avtDataObjectWriter.
//
//    Kathleen Bonnell, Thu Mar  1 10:01:44 PST 2001
//    Added method WriteDomainTree.
//
//    Kathleen Bonnell, Wed Apr 11 08:01:46 PDT 2001 
//    Reflect that avtDomainTree now called avtDataTree. 
// 
//    Hank Childs, Mon Oct  1 08:47:41 PDT 2001
//    Re-worked inheritance hierarchy.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataSetWriter : virtual public avtTerminatingDatasetSink, 
                         virtual public avtDataObjectWriter
{
  public:
                    avtDataSetWriter();
    virtual        ~avtDataSetWriter();

    virtual bool    MustMergeParallelStreams(void) { return true; };

  protected:
    virtual void    DataObjectWrite(avtDataObjectString &);
    void            WriteDataTree(avtDataTree_p, avtDataObjectString &);
};


#endif


