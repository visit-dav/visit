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


