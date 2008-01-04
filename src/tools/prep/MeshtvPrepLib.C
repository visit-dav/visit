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
//                             MeshtvPrepLib.C                               //
// ************************************************************************* //

#include <ctype.h>
#include <visitstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <MeshtvPrepLib.h>

// 
// File scoped variables.  
//

static int   nFiles = 0;
static char  OutputDirectory[LONG_STRING];
static bool  OutputDirectorySet = false;

#ifdef PARALLEL

#include "MPIResourceManager.h"
    //
    // Controls resource contention.
    //
    static MPIResourceManager  *outFileManager = NULL;
#endif


//
// Static prototypes.
//

static void     Partition(int, int, int, int *, int *);


// ****************************************************************************
//  Function:  NumToString
//
//  Purpose:
//      Converts a number to a string and also adds prepended 0's if necessary.
//
//  Input Arguments:
//      num            -  The number that is going to be put in the string.
//      totalNum       -  The total number in the series.  This is needed
//                        to see if the defaultDigits should be overrided.
//      defaultDigits  -  the number of digits that the string should have
//                        provided the totalNum has this many or less digits.
//  Output Arguments:
//      buff           -  The string to write into.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
// ****************************************************************************

void
NumToString(char *buff, int num, int totalNum, int defaultDigits)
{
    //
    // Figure out how many digits are required.
    //
    int totalDigits = 0;
    while (totalNum > 0)
    {
        totalNum /= 10;
        totalDigits++;
    }
    if (totalDigits < defaultDigits)
    {
        totalDigits = defaultDigits;
    }

    //
    // Figure out how many prepended '0's are needed.
    //
    int numDigits = 0;
    int tempNum   = num;
    while (tempNum > 0)
    {
        tempNum /= 10;
        numDigits++;
    }
    
    //
    // Treat 0 as a 1-digit number.
    //
    if (num == 0)
    {
        numDigits++;
    }

    //
    // Put in the prepended 0's
    //
    int  offset = 0;
    for (int i = 0 ; i < totalDigits - numDigits ; i++)
    {
        buff[offset] = '0';
        offset++;
    }
 
    //
    // Copy in the number.  Sprintf will NULL terminate the string for us.
    //
    sprintf(buff+offset, "%d", num);
}


// ****************************************************************************
//  Function:  GetSiloOutputFile
//
//  Purpose:
//      Tries to open a SILO file.  Kind of outdated since it used to also
//      create files so it needed some logic for that.  
//      Easier to leave this in.
//
//  Arguments:
//      filename   The name of the output file to open.
//
//  Returns:    The handle to the SILO file.
//
//  Programmer: Hank Childs
//  Creation:   December 19, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Sep 11 16:21:28 PDT 2000
//    Use the OutputDirectory if it is set.
//
// ****************************************************************************

DBfile *
GetSiloOutputFile(char *filename)
{
    DBfile *dbfile;

    char  qualified_filename[LONG_STRING];
    if (OutputDirectorySet)
    {
        sprintf(qualified_filename, "%s%s", OutputDirectory, filename);
    }
    else
    {
        strcpy(qualified_filename, filename);
    }

    dbfile = DBOpen(qualified_filename, DB_UNKNOWN, DB_APPEND);
    if (dbfile != NULL)
    {
        //
        // File already exists.  We opened it in append mode so that we can
        // write more to it.
        //
        return dbfile;
    }

    cerr << "Unable to open \"" << filename << "\" for writing."
         << endl;
    exit(EXIT_FAILURE);
 
    /* NOTREACHED */ return NULL;
}


// ****************************************************************************
//  Method: CreateSiloOutputFile
//
//  Purpose:
//      Creates a SILO output file and closes it.  Only prints an output 
//      statement if unsuccessful.  It will clobber pre-existing files, but
//      print out that it is doing so.
//
//  Arguments:
//      filename   The name of the output file to open.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Jun 21 13:53:55 PDT 2000
//    Corrected logic for trying to create a file that already exists.
//
//    Hank Childs, Fri Sep  8 15:12:53 PDT 2000
//    Add variable `_filetype' to indicate this is a siloobj file.
//
//    Hank Childs, Mon Sep 11 16:21:28 PDT 2000
//    Use the OutputDirectory if it is set.
//
// ****************************************************************************

void
CreateSiloOutputFile(char *filename)
{
    char qual_filename[LONG_STRING];
    if (OutputDirectorySet)
    {
        sprintf(qual_filename, "%s%s", OutputDirectory, filename);
    }
    else
    {
        strcpy(qual_filename, filename);
    }

    DBfile *dbfile = NULL;
    if (DBInqFile(qual_filename) == 0)
    {
        //
        // The file does not exist, so create it.
        //
        dbfile = DBCreate(qual_filename, DB_NOCLOBBER, DB_LOCAL, NULL, DB_PDB);
    }
    else
    {
        //
        // We are overwriting a file, so print that out and then clobber it.
        //
        cerr << "Clobbering pre-existing file \"" << filename << "\"..."
             << endl;
        dbfile = DBCreate(qual_filename, DB_CLOBBER, DB_LOCAL, NULL, DB_PDB);
    }

    int siloobj_length = 7;
    DBWrite(dbfile, "_filetype", "siloobj", &siloobj_length, 1, DB_CHAR);

    if (dbfile != NULL)
    {
        DBClose(dbfile);
    }
    else
    {
        cerr << "Could not create file \"" << filename << "\"..." << endl;
        exit(EXIT_FAILURE);
    }
}


// ****************************************************************************
//  Method: SetNFiles
//
//  Purpose:
//      Construct the MPIResourceManager.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 15:36:43 PDT 2000
//      Added the resource number to the constructor call of the resource
//      manager.
//
// ****************************************************************************

void
SetNFiles(int n)
{
    nFiles = n;
    if (nFiles < 1)
    {
        cerr << "You must have at least one output file." << endl;
        exit(EXIT_FAILURE);
    }
   
#ifdef PARALLEL
    outFileManager = new MPIResourceManager(nFiles, MAX_RESOURCES);
#endif
}


// ****************************************************************************
//  Method: ObtainResource
//
//  Purpose:
//      Obtains sole possession of the resource.  The resource is arbitrary,
//      so it must be agreed upon beforehand.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

/* ARGSUSED */
void
ObtainResource(RESOURCE_e resource)
{
#ifdef PARALLEL
    outFileManager->ObtainResource(resource);
#else
    //
    // Do nothing - there is only one processor, so every resource 
    // automatically belongs to proc. 1.
    //
#endif
}


// ****************************************************************************
//  Method: RelinquishResource
//
//  Purpose:
//      Relinquish the resource so that other procs can use it.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

/* ARGSUSED */
void
RelinquishResource(RESOURCE_e resource)
{
#ifdef PARALLEL
    outFileManager->RelinquishResource(resource);
#else
    // 
    // Do nothing, see note from ObtainResource
    //
#endif
}


// ****************************************************************************
//  Method: GetCommunicator
//
//  Purpose:
//      Retrieve the communicator associated with this ring.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

#ifdef PARALLEL
MPI_Comm &
GetCommunicator(RESOURCE_e)
{
    return outFileManager->GetCommunicator();
}
#endif


// ****************************************************************************
//  Method: RootWithinGroup
//
//  Purpose:
//      Checks if your processor is a root within its group for the resource
//      specified.
//
//  Returns:    true if the group rank is 0, false otherwise.
//
//  Note:       The lint specifier ARGSUSED is required because the argument
//              is used only if PARALLEL is defined.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
// ****************************************************************************

/* ARGSUSED */
bool
RootWithinGroup(RESOURCE_e)
{
#ifdef PARALLEL
    if (outFileManager->GetGroupRank() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return true;
#endif
}


// ****************************************************************************
//  Function: SortListByNumber
//
//  Purpose:
//      To take a list and rearrange it so that strings that are appended with 
//      numbers will be sorted in the order of that number (as opposed to
//      lexicographical order).
//  
//  Input/Output Arguments:
//      list  -  A list of strings.  The strings will not change, but their
//               ordering in the list will.
//  Input Arguments:
//      size  -  The number of strings in list.
//
//  Example Input (lexicographics order):   XXX0, XXX10, XXX2
//  Returned order:                         XXX0, XXX2,  XXX10
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

void
SortListByNumber(char **list, int size)
{
    int   i, j;

/*
    cerr << "List before sort: " << endl;
    for (i = 0 ; i < size ; i++)
    {
        cerr << list[i];
        cerr << (i % 5 == 4 ? "\n" : ", ");
    }
*/

    if (size <= 0)
    {
        //
        // Degenerate case.
        //
        return;
    }

    //
    // Put the list in lexicographical order so that we will have all of the
    // strings that have the same base name and appended numbers next to each
    // other.
    //
    for (i = 0 ; i < size ; i++)
    {
        for (j = i+1 ; j < size ; j++)
        {
            if (strcmp(list[j], list[i]) < 0)
            {
                char  *temp = list[j];
                list[j] = list[i];
                list[i] = temp;
            }
        }
    }

    for (i = 0 ; i < size ; i++)
    {
        //
        // Check to see if the current string (list[i]) is in the format
        // we are interested in (string appended with a number).
        //
        int  length = strlen(list[i]);
        for (j = 0 ; j < length ; j++)
        {
            if (isdigit(list[i][j]))
            {
                break;
            }
        }
        if (j == length)
        {
            //
            // There are no digits in the string, so this is not in the format
            // we are interested in.
            //
            continue;
        }

        int   baseNameLength = j;
        for (j = i+1 ; j < size ; j++)
        {
            if (strncmp(list[i], list[j], baseNameLength) != 0)
            {
                //
                // This is a string that does not match the base name of
                // the current string.  Furthermore, there will be no more 
                // matches in the rest of the list since it is sorted in
                // lexicographical order.
                //
                break;
            }
            if (!isdigit(list[j][baseNameLength]))
            {
                //
                // This is a string that has the same base name as the 
                // current string, but does not have a number right after it,
                // so it is not a match.  Furthermore, there will be no matches
                // in the rest of the list since it is sorted in 
                // lexicographical order.
                //
                break;
            }
        }   //  End 'for' searching for all strings that match the basename
        
        if (j == i+1)
        {
            // 
            // There were no strings in the list that could possibly be
            // misordered with the current one.
            //
            continue;
        }
 
        //
        // We have a span in the list from i to j of strings that share a 
        // base name and are followed by digits.  Sort them according to
        // number
        //
        for (int k = i ; k < j ; k++)
        {
            int   num1 = atoi(list[k]+baseNameLength);
            for (int l = k+1 ; l < j ; l++)
            {
                int  num2 = atoi(list[l]+baseNameLength);
                if (num2 < num1)
                {
                    char  *temp = list[k];
                    list[k] = list[l];
                    list[l] = temp;
                }
            }
        }

    }  // End 'for' over all the strings in the list

/*
    cerr << "List after sort: " << endl;
    for (i = 0 ; i < size ; i++)
    {
        cerr << list[i];
        cerr << (i % 5 == 4 ? "\n" : ", ");
    }
*/
}


// ****************************************************************************
//  Function: GetNFileRange
//
//  Purpose:
//      Determines which output files correspond to this processor.
//
//  Arguments:
//      Input/Output:
//          minNFile - the minimum index for a file that corresponds to this 
//                     processor.
//          maxNFile - the first index of a file that does not correspond to
//                     this processor.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2000
//
// ****************************************************************************

void
GetNFileRange(int *minNFile, int *maxNFile)
{
    if (nFiles <= 0)
    {
        //
        // nFiles has not been set yet, so report and return.
        //
        cerr << "GetNFileRange cannot be used before nFiles has been set." 
             << endl;
        exit(EXIT_FAILURE);
    }

    SplitAmongProcessors(nFiles, minNFile, maxNFile);
}


// ****************************************************************************
//  Function: GetDomainRange
//
//  Purpose:
//      Determines which domains correspond to this processor.
//
//  Arguments:
//      Input:
//          nDomains  -  The number of domains.
//      Input/Output:
//          minDomain -  The minimum domain that corresponds to this processor.
//          maxDomain -  The first index that does not correspond to this
//                       processor after minDomain.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2000
//
// ****************************************************************************

void
GetDomainRange(int nDomains, int *minDomain, int *maxDomain)
{
    SplitAmongProcessors(nDomains, minDomain, maxDomain);
}


// ****************************************************************************
//  Function: SplitAmongProcessors
//
//  Purpose:
//      Decides what the minimum and maximum index should be for a group (may
//      be domains or files) based on the processors rank and the number of
//      processors.
//
//  Arguments:
//      Input/Output:
//          min  -  The minimum domain number to consider.
//          max  -  The first domain not to consider (ie '<' not '<=').
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2000
//
// ****************************************************************************

void
SplitAmongProcessors(int nGroup, int *min, int *max)
{
#ifndef PARALLEL
    *min = 0;
    *max = nGroup;
#else
    extern int my_rank, num_processors;

    if (nGroup >= num_processors)
    {
        Partition(nGroup, num_processors, my_rank, min, max);
    }
    else
    {
        //
        // Partition is not really set up for this, but I don't want to write
        // a new function in case the Partition function changes and they 
        // won't be in sync.  
        //
        // This case happens if the number of processors is greater than the 
        // number of output files and they need to share a file.  
        //
        // Partition the processors among each element in the group.  When
        // the element in the group that contains the processor is returned,
        // use that.  This is wasteful, but probably nothing compared to I/O.
        // 
        for (int i = 0 ; i < nGroup ; i++)
        {
            Partition(num_processors, nGroup, i, min, max);
            if (my_rank >= *min && my_rank < *max)
            {
                //
                // The processor belongs to this element.
                //
                *min = i;
                *max = i+1;
                break;
            }
        }
    }
#endif
}

// ****************************************************************************
//  Function: Partition
//
//  Purpose:
//      Partitions a set with in a specified cardinality into the specified
//      number of subsets and calculates which elements will fall into the
//      specified subset.
//
//  Arguments:
//      Input:
//          cardinality   -  The size of the set to be partitioned.
//          num_subsets   -  The number of sets to partition the larger set 
//                           into.
//          my_subset     -  The subset to report about with min and max.
//      Input/Output:
//          min           -  The first element put into my_subset.
//          max           -  The first element not put into my_subset after 
//                           min.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2000
//
// ****************************************************************************

static void
Partition(int cardinality, int num_subsets, int my_subset, int *min, int *max)
{
    //
    // If the group does not divide evenly, assign the extras to the lower
    // numbered processors.
    // 
    // This is done so that resource allocation can pass blindly from 
    // processor i to i+1 and the leftovers get taken care of seamlessly.
    //
    int  myAmount = cardinality / num_subsets;

    int  numLeftOver = cardinality % num_subsets;

    //
    // Have the lowered numbered processors take over the "off-by-one" files.
    //
    if (my_subset < numLeftOver)
    {
        myAmount++;
    }

    int  myStart  = myAmount * my_subset;
    if (my_subset >= numLeftOver)
    {
        //
        // The subsets that came before this processor had some extra 
        // elements (1 each for the first numLeftOver).
        // Add those in now.
        //
        myStart += numLeftOver;
    }

    *min = myStart;
    *max = myStart + myAmount;
}


// ****************************************************************************
//  Function: GetOutputFileNum
//
//  Purpose:
//      Determines which output file the domain should go into.
//
//  Note:       Although it can lead to some stupid results, the domains are
//              assigned to processors and then the domains for a processor
//              are assigned to a file.  This is to prevent some stupid 
//              situations (ie nprocs=2, nfiles=3) where processors have to 
//              share files and own files.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2000
//
// ****************************************************************************

int
GetOutputFileNum(int domain, int nDomains)
{
    int minFile, maxFile;
    GetNFileRange(&minFile, &maxFile);

    int minDomain, maxDomain;
    GetDomainRange(nDomains, &minDomain, &maxDomain);

    int  numFiles   = maxFile - minFile;
    int  numDomains = maxDomain - minDomain;
    int  domainsPerFile = numDomains / numFiles;
    if (numDomains % numFiles != 0)
    {
        domainsPerFile++;
    }

    if (domainsPerFile <= 0)
    {
        //
        // Happens when nFiles > nDomains
        //
        domainsPerFile = 1;
    }

    int  relativeFile = (domain - minDomain) / domainsPerFile;
    return  relativeFile + minFile;
}


// ****************************************************************************
//  Function: ForceControlToRoot
//
//  Purpose:
//      Return the control of the output files to the root manager.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2000
//
// ****************************************************************************

void
ForceControlToRoot(void)
{
#ifdef PARALLEL
    outFileManager->ForceControlToRoot();
#endif
}


// ****************************************************************************
//  Function: SetOutputDirectory
//
//  Purpose:
//      Sets the output directory for all of the Silo files.
//
//  Arguments:
//      dir     The directory.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2000
//
// ****************************************************************************

void
SetOutputDirectory(char *dir)
{
    strcpy(OutputDirectory, dir);
    OutputDirectorySet = true;
}


