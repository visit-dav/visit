// ************************************************************************* //
//                              TableOfContents.C                            //
// ************************************************************************* //

#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <TableOfContents.h>


// ****************************************************************************
//  Method: TableOfContents constructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

TableOfContents::TableOfContents()
{
    nFiles       = 0;
    files        = NULL;
    openFile     = NULL;
    openFileName = NULL;
}


// ****************************************************************************
//  Method: TableOfContents destructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

TableOfContents::~TableOfContents()
{
    if (files != NULL)
    {
        for (int i = 0 ; i < nFiles ; i++)
        {
            if (files[i] != NULL)
            {
                delete [] files[i];
            }
        }
        delete [] files;
    }
    if (openFile != NULL)
    {
        DBClose(openFile);
    }
    if (openFileName != NULL)
    {
        delete [] openFileName;
    }
}


// ****************************************************************************
//  Method: TableOfContents::Read
//
//  Purpose:
//      Reads in all of the SILO objects from the files given.
//
//  Arguments:
//      list    A list of files.
//      size    The number of files in list.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
TableOfContents::Read(int size, const char * const *list)
{
    int  i;

    files = new char*[size];
    for (i = 0 ; i < size ; i++)
    {
        files[i] = CXX_strdup(list[i]);
        nFiles++;
    }

    if (nFiles == 1)
    {
        //
        // Need this for '.visit'.
        //
        ReadFile(files[0]);
    }
    else
    {
        for (i = 0 ; i < nFiles ; i++)
        {
            if (strstr(files[i], ".00") != NULL)
            {
                //
                // This is the file that contains all of the object 
                // definitions.
                //
                ReadFile(files[i]);
            }
        }
    }
 
    UpdateReferences();
}


// ****************************************************************************
//  Method: TableOfContents::CloseSILOFile
//
//  Purpose:
//      Closes a SILO file.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2000
//
// ****************************************************************************

void
TableOfContents::CloseSILOFile(void)
{
    if (openFile != NULL)
    {
        DBClose(openFile);
        openFile = NULL;
    }
    if (openFileName != NULL)
    {
        delete [] openFileName;
        openFileName = NULL;
    }
}


// ****************************************************************************
//  Method: TableOfContents::GetSILOFile
//
//  Purpose:
//      Gets a handle to the SILO file.
//
//  Arguments:
//      filename   The name of the file to open.
//
//  Returns:    A handle to the SILO file.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

DBfile *
TableOfContents::GetSILOFile(const char *filename)
{
    if (filename == NULL)
    {
        //
        // This comes from a Value object that only has knowledge of one
        // file, so ASS U ME it is files[0].
        //
        return GetSILOFile(files[0]);
    }

    if (openFile != NULL)
    {
        //
        // We already had a file saved, check to see if they are the same
        // by comparing the file names.
        //
        if (strcmp(openFileName, filename) == 0)
        {
            //
            // This is the file we have been holding a reference too.
            //
            return openFile;
        }
        else
        {
            //
            // We have to close the file before we open another.
            //
            CloseSILOFile();
        }
    }

    openFileName = CXX_strdup(filename);

    //
    // We have the filename, but not the path to it.  Look through our list
    // of files and find the file with its path.  
    //
    bool  foundPath = false;
    int i;
    for (i = 0 ; i < nFiles ; i++)
    {
        if (strstr(files[i], filename) != NULL)
        { 
            foundPath = true;
            break;
        }
    }

    if (foundPath == false)
    {
        cerr << "Not able to locate path of \"" << filename << "\"." << endl;
        // throw
        return NULL;
    }

    openFile = DBOpen(files[i], DB_UNKNOWN, DB_READ);

    if (openFile == NULL)
    {
        cerr << "Unable to open file \"" << filename << "\"." << endl;
        // throw
        return NULL;
    }
  
    return openFile;
}


// ****************************************************************************
//  Method: TableOfContents::PrintSelf
//
//  Purpose:
//      Prints out the Table of Contents object.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
TableOfContents::PrintSelf()
{
    int   i;

    cerr << "Files: " << endl;
    for (i = 0 ; i < nFiles ; i++)
    {
        cerr << "\t" << files[i] << endl;
    }
}


