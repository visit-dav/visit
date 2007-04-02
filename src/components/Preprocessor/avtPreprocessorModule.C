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
//                            avtPreprocessorModule.C                        //
// ************************************************************************* //

#include <avtPreprocessorModule.h>

#include <stdio.h>

#include <vtkDataSet.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtPreprocessorModule constructor
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

avtPreprocessorModule::avtPreprocessorModule()
{
    stem = NULL;
}


// ****************************************************************************
//  Method: avtPreprocessorModule destructor
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

avtPreprocessorModule::~avtPreprocessorModule()
{
   if (stem != NULL)
   {
       delete [] stem;
       stem = NULL;
   }
}


// ****************************************************************************
//  Method: avtPreprocessorModule::ProcessCommandLine
//
//  Purpose:
//      Gives the preprocessor module a chance to takes its arguments off the
//      command line.  This implementation does nothing, but is defined so 
//      derived types can accept command line arguments if appropriate.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

void
avtPreprocessorModule::ProcessCommandLine(char **&, int &)
{
    ;
}


// ****************************************************************************
//  Method: avtPreprocessorModule::SetStem
//
//  Purpose:
//      Sets the stem that is appropriate for output file names.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

void
avtPreprocessorModule::SetStem(const char *s)
{
    stem = new char[strlen(s)+1];
    strcpy(stem, s);
}


// ****************************************************************************
//  Method: avtPreprocessorModule::InputIsReady
//
//  Purpose:
//      This is what is called by an originating sink when the Update/Execute
//      cycle is complete.  Call Preprocess so that our derived types can
//      do their preprocessing.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtPreprocessorModule::InputIsReady(void)
{
    debug1 << "Starting preprocessing for " << GetType() << endl;
    Preprocess();
    debug1 << "Done preprocessing for " << GetType() << endl;
}


