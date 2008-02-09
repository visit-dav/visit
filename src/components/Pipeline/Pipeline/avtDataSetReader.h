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
//                              avtDataSetReader.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_READER_H
#define AVT_DATASET_READER_H
#include <pipeline_exports.h>

#include <array_ref_ptr.h>
#include <ref_ptr.h>

#include <avtOriginatingDatasetSource.h>


// ****************************************************************************
//  Class: avtDataSetReader
//
//  Purpose:
//      A class which takes as input an avtDataSet and can serialize it.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 09:07:37 PDT 2001
//    Blew away old comments.  Re-wrote for new inheritance structure.
//
//    Hank Childs, Mon Sep 17 10:58:02 PDT 2001
//    Made read also accept a character string reference to prevent unneeded
//    copying.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataSetReader : public avtOriginatingDatasetSource
{
  public:
                           avtDataSetReader();
    virtual               ~avtDataSetReader();

    virtual bool           ArtificialPipeline(void) { return true; };
    int                    Read(char *input, CharStrRef &);
    avtDataTree_p          ReadDataTree(char *&, int &, CharStrRef &);

    virtual bool           UseLoadBalancer(void) { return false; };

  protected:
    avtDataTree_p          dataTree;
    bool                   haveFetchedThisData;

    virtual bool           FetchDataset(avtDataRequest_p,avtDataTree_p&);
};


typedef ref_ptr<avtDataSetReader>  avtDataSetReader_p;

#endif


