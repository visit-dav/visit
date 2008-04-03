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
//                                  avtprep.C                                //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>

#include <vector>

#include <Init.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtExtentsPreprocessor.h>
#include <avtResamplePreprocessor.h>
#include <avtOriginatingSource.h>


using std::vector;


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      This is the main routine for a preprocessor that is based totally on
//      AVT modules.  The modules it currently uses are for determining the
//      extents of a dataset and for resampling onto a rectilinear mesh.  Only
//      one variable at a time works and for one timestep.  All parallel issues
//      are currently ignored.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Sep 19 15:15:33 PDT 2001
//    Make main return an int.
//
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
// ****************************************************************************

int
main(int argc, char *argv[])
{
    Init::SetComponentName("avtprep");
    Init::Initialize(argc, argv, 0, 1, true);

    if (argc < 3)
    {
        cerr << "Usage: " << argv[0] << " <filename> <variable> "
             << "<prep-module-arguments>" << endl;
        exit(EXIT_FAILURE);
    }

    avtDatabase *db = NULL;
    if (strstr(argv[1], ".visit") != NULL)
    {
        db = avtDatabaseFactory::VisitFile(argv[1]);
    }
    else
    {
        db = avtDatabaseFactory::FileList(argv + 1, 1);
    }

    avtDataObject_p dob = db->GetOutput(argv[2], 0);

    avtResamplePreprocessor *res = new avtResamplePreprocessor;
    avtExtentsPreprocessor  *exts = new avtExtentsPreprocessor;
   
    vector<avtPreprocessorModule *> preps;
    preps.push_back(res);
    preps.push_back(exts);

    int   i;
    for (i = 0 ; i < preps.size() ; i++)
    {
        preps[i]->SetStem(argv[1]);
        preps[i]->ProcessCommandLine(argv, argc);
    }

    avtOriginatingSource  *src = dob->GetOriginatingSource();
    avtDataRequest_p ds  = src->GetFullDataRequest();
    for (i = 0 ; i < preps.size() ; i++)
    {
        //
        // Give each pipeline its own index in case we put in the parallel
        // hooks one day.
        //
        preps[i]->SetPipelineIndex(i+1);

        //
        // This Update will actually spawn off the preprocessing.
        //
        preps[i]->SetInput(dob);
        preps[i]->Execute(ds);
    }

    return 0;
}


