// ************************************************************************* //
//                                  avtprep.C                                //
// ************************************************************************* //

#include <iostream.h>
#include <stdlib.h>

#include <vector>

#include <Init.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtExtentsPreprocessor.h>
#include <avtResamplePreprocessor.h>
#include <avtTerminatingSource.h>


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
// ****************************************************************************

int
main(int argc, char *argv[])
{
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

    avtTerminatingSource  *src = dob->GetTerminatingSource();
    avtDataSpecification_p ds  = src->GetFullDataSpecification();
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


