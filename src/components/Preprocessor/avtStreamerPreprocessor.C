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
//                           avtStreamerPreprocessor.C                       //
// ************************************************************************* //

#include <avtStreamerPreprocessor.h>

#include <vtkDataSet.h>

#include <avtDataTree.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtStreamerPreprocessor constructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtStreamerPreprocessor::avtStreamerPreprocessor()
{
    ;
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor destructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtStreamerPreprocessor::~avtStreamerPreprocessor()
{
    ;
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::Preprocess
//
//  Purpose:
//      Does the actual preprocessing work.  Since most derived types do not
//      want to code up how to unwrap a data tree, this does it for them.  It
//      then calls ProcessDomain for each data tree, which the derived types
//      must define.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::Preprocess(void)
{
    avtDataTree_p tree = GetInputDataTree();
    int totalNodes = tree->GetNumberOfLeaves();

    debug3 << "Preprocessing with " << totalNodes << " nodes." << endl;
    Initialize(totalNodes);

    debug3 << "Preprocessing the top level tree" << endl;
    PreprocessTree(tree);

    debug3 << "Allowing preprocessing module to finalize." << endl;
    Finalize();
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::PreprocessTree
//
//  Purpose:
//      Unwraps a data tree and calls ProcessDomain on each leaf. 
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 15:15:24 PDT 2006
//    Remove call to SetSource(NULL) as it now removes information necessary
//    for the dataset.
//
// ****************************************************************************

void
avtStreamerPreprocessor::PreprocessTree(avtDataTree_p tree)
{
    if (*tree == NULL)
    {
        return;
    }

    int numChildren = tree->GetNChildren();

    if ( (numChildren <= 0) && (!(tree->HasData())) )
    {
        return;
    }

    if (numChildren == 0)
    {
        //
        // There is only one dataset to process (the leaf).
        //
        vtkDataSet *in_ds = tree->GetDataRepresentation().GetDataVTK();
        int dom = tree->GetDataRepresentation().GetDomain();

        //
        // Ensure that there is no funny business when we do an Update.
        //
        // NO LONGER A GOOD IDEA
        //in_ds->SetSource(NULL);

        ProcessDomain(in_ds, dom);
    }
    else
    {
        for (int i = 0 ; i < numChildren ; i++)
        {
            if (tree->ChildIsPresent(i))
            {
                PreprocessTree(tree->GetChild(i));
            }
        }
    }
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::Initialize
//
//  Purpose:
//      Gives the preprocessor module a chance to initialize itself.  This
//      implementation does nothing, but is defined so derived types can
//      initalize themselves if appropriate.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::Initialize(int)
{
    ;
}


// ****************************************************************************
//  Method: avtStreamerPreprocessor::Finalize
//
//  Purpose:
//      Gives the preprocessor module a chance to finalize itself.  This
//      implementation does nothing, but is defined so derived types can
//      finalize themselves if appropriate.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtStreamerPreprocessor::Finalize(void)
{
    ;
}


