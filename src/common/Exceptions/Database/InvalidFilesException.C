// ************************************************************************* //
//                            InvalidFilesException.C                        //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <InvalidFilesException.h>

// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      numfiles   The number of files that were too big for the database.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(int numfiles)
{
    char str[1024];
    sprintf(str, "This database cannot handle more than %d files.", numfiles);
    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Feb 7 11:45:26 PDT 2002
//    Added a little to the error message.
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const char *filename)
{
    char str[1024];
    sprintf(str, "The database had an error opening %s. It may be an "
            "invalid file.", filename);

    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//      message     A more informative message than the default one.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
//  Modifications:
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const std::string &filename,
                                             const std::string &message)
{
    char str[2048];
    sprintf(str, "The database had an error opening %s: %s", filename.c_str(),
            message.c_str());

    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      list     A list of filenames.
//      listN    The number of files in list.
//
//  Programmer:  Hank Childs
//  Creation:    September 13, 2000
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const char * const * list, 
                                             int listN)
{
    msg = "The database had an error opening one of the following files:\n";
    for (int i = 0 ; i < listN ; i++)
    {
        msg += "\t";
        msg += list[i];
    }
}


