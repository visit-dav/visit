// ************************************************************************* //
//                              Value_VTK.C                                  //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <TableOfContents.h>
#include <Value_VTK.h>


// ****************************************************************************
//  Method: Value_VTK constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

Value_VTK::Value_VTK()
{
    toc   = NULL;
    cache = NULL;
}


// ****************************************************************************
//  Method: Value_VTK destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

Value_VTK::~Value_VTK()
{
    if (cache != NULL)
    {
        for (int i = 0 ; i < nDomains ; i++)
        {
            if (cache[i] != NULL)
            {
                delete [] cache[i];
            }
        }
        delete [] cache;
    }

    //
    // Don't delete reference to toc.
    //
}


// ****************************************************************************
//  Method: Value_VTK::GetDomains
// 
//  Purpose:
//      Reads in from a file the arrays of values corresponding to the 
//      domain list specified.
//
//  Arguments:
//      domainList     A list of domains to read in.
//      listN          The size of the domain list.
//      vals           An address of a pointer which will point to the values.
//      sizes          An address of a pointer which will point to an array
//                     containing the sizes of the values from each domain.
//
//  Note:       The vals and sizes array should both be freed by the calling
//              routine.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
Value_VTK::GetDomains(const int *domainList, int listN, float ***vals, 
                      int **sizes)
{
    int   i, j;

    //
    // Create the arrays to be returned.
    //
    *sizes = new int[listN*nVals];
    *vals  = new float *[listN*nVals];

    //
    // Go through each domain on the list and see how big the array is and
    // whether or not is in the cache.
    //
    for (i = 0 ; i < listN ; i++)
    {
        int  sizeOfCurrentArray = 0;
        int  currentDomain      = domainList[i];
        for (j = 0 ; j < nVals ; j++)
        {
            sizeOfCurrentArray += lengths[currentDomain*nVals + j];
            (*sizes)[i*nVals + j] = lengths[currentDomain*nVals + j];
        }
        if (cache[currentDomain] != NULL)
        {
            //
            // We have already read in this domain and can steal it
            // from cache.
            //
            (*vals)[i] = cache[currentDomain];
        }
        else
        {
            //
            // We need to read in this domain, so allocate some memory
            // for the domain right now.
            //
            (*vals)[i] = new float[sizeOfCurrentArray];
        }
    }

    //
    // Determine the name of the data set to read from.
    //
    char  arrayName[LONG_STRING];
    sprintf(arrayName, "%s%s", name, ARRAY_STRING);

    //
    // Read in the arrays that have not already been cached.
    //
    for (i = 0 ; i < listN ; i++)
    {
        int  domain = domainList[i];

        if ((*vals)[i] == cache[domain])
        {
            //
            // We already read in this domain and stole it from cache.
            //
            cerr << "Stole domain " << domain << " from cache, not reading "
                 << "in again." << endl;
            continue;
        }

        //
        // Get the SILO file.  The toc saves the last one we have used, so
        // this is not as bad as it may appear.
        //
        DBfile *dbfile;
        if (strstr(entryNames[domain], ":") != NULL)
        {
            //
            // Create the filename by 'nulling out' the colon that separates
            // the filename and domain specifier.
            //
            char filename[LONG_STRING];
            strcpy(filename, entryNames[domain]);
            char *p = strstr(filename, ":");
            filename[p-filename] = '\0';
            cerr << "Getting " << domain << " from file " << filename << endl;
            dbfile = toc->GetSILOFile(filename);
        }
        else
        {
            dbfile = toc->GetSILOFile(NULL);
        }

        //
        // We will read in all dimensions at one time.
        //
        int  amountToRead = 0;
        for (j = 0 ; j < nVals ; j++)
        {
            amountToRead += (*sizes)[i*nVals + j];
        }
        int  offset = offsets[domain*nVals];
        int  stride = 1;
        int  ndims  = 1;
        if (DBReadVarSlice(dbfile, arrayName, &offset, &amountToRead, &stride, 
                       ndims, (*vals)[i]) < 0)
        {
            cerr << "Failed read for array " << arrayName << endl;
            // throw
            return;
        }
        
        //
        // Cache this domain.
        //
        cache[domain] = (*vals)[i];

        //
        // Do not need to close the file since the toc will manage it.
        //
    }  

}


// ****************************************************************************
//  Method: Value_VTK::UpdateReferences
//
//  Purpose:
//      Updates the table of contents reference.  Value_VTK is not a container
//      class, so no data members need to be updated.  This is a good place to
//      create the cache for arrays read in, though.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
Value_VTK::UpdateReferences(TableOfContents *t)
{
    toc = t;
    CreateCache();
}


// ****************************************************************************
//  Method: Value_VTK::CreateCache
//
//  Purpose:
//      Creates the cache used to put all of the arrays read in.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

void
Value_VTK::CreateCache(void)
{
    cache = new float*[nDomains];
    for (int i = 0 ; i < nDomains ; i++)
    {
        cache[i] = NULL;
    }
}


