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
//                            avtDataSetReader.C                             //
// ************************************************************************* //

#include <avtDataSetReader.h>

#include <string>
#include <vector>

#include <vtkDataSetReader.h>

#include <avtDataset.h>
#include <avtCommonDataFunctions.h>

#include <DebugStream.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDataSetReader constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Apr 11 08:01:46 PDT 2001
//    Removed initialization of numDomains, no longer a member. 
//
// ****************************************************************************

avtDataSetReader::avtDataSetReader()
{
    dataTree  = NULL;
    haveFetchedThisData = false;
}


// ****************************************************************************
//  Method: avtDataSetReader destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataSetReader::~avtDataSetReader()
{
    ;
}


// ****************************************************************************
//  Method: avtDataSetReader::Read
//
//  Arguments:
//      length  the string to read
//      csr     reference to a character string -- the original input.
//
//  Returns:    The length of the dataset in bytes.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 28, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 09:27:41 PDT 2001
//    Blew away old comments.  Removed issues with knowing the number of
//    children for a data tree.
//
//    Hank Childs, Mon Sep 17 11:00:23 PDT 2001
//    Added argument csr.
//
// ****************************************************************************

int 
avtDataSetReader::Read(char *input, CharStrRef &csr)
{
    int  size = 0;

    dataTree = ReadDataTree(input, size, csr);
    GetDataTree() = dataTree;
    haveFetchedThisData = true;

    return size;
}


// ****************************************************************************
//  Method: avtDataSetReader::ReadDataTree
//
//  Arguments:
//      input   the string to read
//      size    the length of the domain tree in bytes 
//      csr     reference to a character string -- the original input.
//
//  Returns:    The domain  tree.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Apr 11 08:01:46 PDT 2001
//    Change method name to reflect avtDomainTree now called avtDataTree.
//
//    Hank Childs, Mon Jun 18 09:35:29 PDT 2001
//    Read chunk numbers.
//
//    Hank Childs, Tue Jun 19 16:37:57 PDT 2001
//    Handle bad data trees better.
//
//    Hank Childs, Mon Sep 17 11:00:23 PDT 2001
//    Added argument csr.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Read label associated with dataset. 
//
//    Hank Childs, Thu Sep 27 17:18:01 PDT 2001
//    Added support for the dataset's type.
//
//    Hank Childs, Mon Jan  7 16:47:28 PST 2002
//    Fix memory leak.
//
// ****************************************************************************

avtDataTree_p
avtDataSetReader::ReadDataTree(char * &input, int &size, CharStrRef &csr)
{
    avtDataTree_p result;

    // read number of children
    int nc;
    memcpy(&nc, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    if (nc < 0)
    {
        //
        // We have an invalid data tree.
        //
        return NULL;
    }

    if (nc > 0)
    {
        avtDataTree_p *children = new avtDataTree_p [nc];
        for (int i = 0; i < nc; ++i)
        {
            int IsPresent;
            memcpy(&IsPresent, input, sizeof(int));
            input += sizeof(int), size += sizeof(int);
            if (IsPresent == 1)
            {
                children[i] = ReadDataTree(input, size, csr);
            }
            else
            {
                children[i] = NULL; 
            }
        }
        result = new avtDataTree(nc, children);        
        delete [] children;
    }
    else
    {
        int len;
        memcpy(&len, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        int chunk;
        memcpy(&chunk, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);

        DataSetType dst;
        memcpy(&dst, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);

        int labelSize;
        memcpy(&labelSize, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        string label;
        if (labelSize > 0)
        {
            string l(input, labelSize);
            input += labelSize; size += labelSize;
            label = l;
        }

        avtDataRepresentation child(input, len, chunk, label, csr, dst);
        debug5 << "Chunk " << chunk << " contributed " << len
               << " bytes to the serialized string." << endl;
        input += len; size += len;

        result = new avtDataTree(child);
    }
    return result;
}


// ****************************************************************************
//  Method: avtDataSetReader::FetchDataset
//
//  Purpose:
//      Gets the dataset specified from the input data tree that was read in
//      from a string.
//
//  Arguments:
//      spec    Data specification of which data we want.
//      output  A place to put the data tree.
//
//  Returns:    Returns true if the data has changed, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep 19 12:41:15 PDT 2001
//    Restructured the method to call PruneTree method if we 
//    don't want the entire tree.
//
// ****************************************************************************

bool
avtDataSetReader::FetchDataset(avtDataSpecification_p spec,
                               avtDataTree_p &output)
{
    vector<int> chunks;
    spec->GetSIL().GetDomainList(chunks);
    
    if (chunks.empty() || (chunks.size() == 1 && chunks[0] == -1))
    {
        // 
        // We want everything in the original tree.
        // 
        output = dataTree;
    }
    else
    {
        // 
        // Prune the tree to the specified SIL.
        // 
        output = dataTree->PruneTree(chunks);
    }
    bool rv = !haveFetchedThisData;
    haveFetchedThisData = true;

    return rv;
}

