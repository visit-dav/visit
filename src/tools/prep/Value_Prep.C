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
//                               Value_Prep.C                                //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#ifdef PARALLEL
    #include <mpi.h>
#endif

#include <NamingConvention.h>
#include <MeshtvPrepLib.h>
#include <SiloObjLib.h>
#include <Value_Prep.h>


//
// Initialization of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const Value_Prep::SILO_LOCATION       = "/";

char *       Value_Prep::fileBase            = NULL;
int          Value_Prep::fileNum             = 0;
int          Value_Prep::nFiles              = 0;


// ****************************************************************************
//  Method: Value_Prep constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Value_Prep::Value_Prep()
{
    files           = NULL;
    setNames        = false;
    setNVals        = false;
    totalSize       = 0;
    sizeForFile     = new int[nFiles];
    for (int i = 0 ; i < nFiles ; i++)
    {
        sizeForFile[i] = 0;
    }
    thisProcsOffset = 0;
}


// ****************************************************************************
//  Method: Value_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Value_Prep::~Value_Prep()
{
    if (files != NULL)
    {
        delete [] files;
    }
    if (sizeForFile != NULL)
    {
        delete [] sizeForFile;
    }
}


// ****************************************************************************
//  Method: Value_Prep::SetFileBase
//
//  Purpose:
//      Allows the base string for output file names to be set one time for
//      the class.
//
//  Programmer: Hank Childs
//  Creation:   January 4, 2000
//
// ****************************************************************************

void
Value_Prep::SetFileBase(char *p)
{
    fileBase = p;
}


// ****************************************************************************
//  Method: Value_Prep::SetNFiles
//
//  Purpose:
//      Allows for the total number of files to be entered one time.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2000
//
// ****************************************************************************

void
Value_Prep::SetNFiles(int n)
{
    nFiles = n;
}


// ****************************************************************************
//  Method: Value_Prep::SetFileNum
//
//  Purpose:
//      Sets the file number for this file.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2000
//
// ****************************************************************************

void
Value_Prep::SetFileNum(void)
{
    int minFile, maxFile;
    GetNFileRange(&minFile, &maxFile);
    if (minFile+1 == maxFile)
    {
        Value_Prep::fileNum = minFile;
    }
}



// ****************************************************************************
//  Method: Value_Prep::SetNDomains
// 
//  Purpose:
//      To set the number of domains and initialize the data members that 
//      are dependent on that value.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
// ****************************************************************************

void
Value_Prep::SetNDomains(int d)
{
    nDomains     = d;
    entryNames   = new char*[nDomains];
    files        = new int[nDomains];

    for (int i = 0 ; i < nDomains ; i++)
    {
        entryNames[i] = NULL;
        files[i]      = 0;
    }
}


// ****************************************************************************
//  Method: Value_Prep::SetNVals
//
//  Purpose:
//      Sets the rank of the vector variable.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

void
Value_Prep::SetNVals(int v)
{
    if (setNVals == true)
    {
        //
        // We have already initialized the arrays.
        //
        return;
    }

    nVals        = v;
    totalEntries = nDomains * nVals;
    offsets      = new int[totalEntries];
    lengths      = new int[totalEntries];
    for (int i = 0 ; i < totalEntries ; i++)
    {
        offsets[i] = 0;
        lengths[i] = 0;
    }
    setNVals = true;
}


// ****************************************************************************
//  Method: Value_Prep::SetFieldName
//
//  Purpose:
//      Set the name data member of a Value_Prep object.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
// ****************************************************************************

void
Value_Prep::SetFieldName(char *f)
{
    char  s[LONG_STRING];
    sprintf(s, "%s%s", f, Value_Prep::NAME);
    name = CXX_strdup(s);
}


// ****************************************************************************
//  Method: Value_Prep::SetNames
//
//  Purpose:
//      Sets the 'entryNames' data member to have the correct values.
//
//  Note:       This should be run after the location of each domain is known.
//              The best time to do this is in WrapUp.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
// ****************************************************************************

void
Value_Prep::SetNames()
{
    char  s[LONG_STRING];

    for (int i = 0 ; i < nDomains ; i++)
    {
        if (fileBase == NULL || strcmp(fileBase, "") == 0)
        {
            sprintf(s, "%s%d", name, i);
            entryNames[i] = CXX_strdup(s);
        }
        else
        { 
            //
            // Resolve the correct state extension (ie .00 or .01, etc).
            //
            char  fNum[LONG_STRING];
            NumToString(fNum, files[i], nFiles, 2);

            //
            // Resolve the file type (ie .mesh or .XXXX).
            //
            char  *resourceStr = NULL;
            char   stateStr[LONG_STRING];
            if (resource == STATE_FILE)
            {
                extern NamingConvention *stateNameMaker;
                extern int               currentState;
                stateNameMaker->GetState(stateStr, currentState);
                resourceStr = stateStr;
            }
            else
            {
                resourceStr = ".mesh";
            }
                
            sprintf(s, "%s%s.%s:%s%d", fileBase, resourceStr, fNum, name, i);
            entryNames[i] = CXX_strdup(s);
        }
    }
    setNames = true;
}


// ****************************************************************************
//  Method:  Value_Prep::Write
//
//  Purpose:
//      Writes the Value_Prep object to a SILO file.  
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Value_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
  extern int my_rank;
  if (my_rank != 0)
  {
      return;
  }
#endif

    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to " 
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    // 
    // Call the Write method for the base type.
    //
    Value::Write(dbfile);
}


// ****************************************************************************
//  Method: Value_Prep::Consolidate
//
//  Purpose:
//      Now that the initial pass is completed, we can calculate the offsets
//      so that the arrays can be written to the correct location when
//      the preprocessing begins in earnest.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Value_Prep::Consolidate(void)
{
    if (nVals < 0)
    {
        // 
        // This is an unused Value object and treating its data members
        // as valid is dangerous.
        //
        return;
    }

#ifdef PARALLEL
    extern int num_processors;
    if (nFiles >= num_processors)
    {
        //
        // Communicator groups were not set up, so do not reference them.
        //
        thisProcsOffset = 0;
    }
    else
    {
        //
        // nFiles < num_processors, so this processor is writing to one
        // file.  Combine size information with the rest of the processors
        // that write to this file.
        //
        int  arraySizeForThisProc = sizeForFile[fileNum];
        MPI_Allreduce(&arraySizeForThisProc, &(sizeForFile[fileNum]), 1, 
                      MPI_INT, MPI_SUM, GetCommunicator(STATE_FILE));

        //
        // Now determine the offset for this processor into that array.  ie, if
        // proc 3, 4, and 5 form an output group, and size 3 = 1000, 
        // size 4 = 500, and size 5 = 700, then offset 3 = 0, offset 4 = 1000, 
        // offset 5 = 1500.
        // Calculate these offsets.
        //
        MPI_Scan(&arraySizeForThisProc, &thisProcsOffset, 1, MPI_INT, MPI_SUM,
                 GetCommunicator(STATE_FILE));

        //
        // The offset number includes this processors size, so subtract it out.
        //
        thisProcsOffset -= arraySizeForThisProc;
    }

    //
    // Calculate the total size across all processors.
    //
    int  tempSize;
    MPI_Allreduce(&totalSize, &tempSize, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    totalSize = tempSize;
#else
    thisProcsOffset = 0;
#endif

    //
    // Calculate the offsets into the array.  If the length of the array for
    // a particular domain is 0, make the offset -1 as a concession to the 
    // parallel case which will later be used in some MPI calls.
    //
    int  currentOutputFile = 0;
    int  currentOffset;
    if (currentOutputFile == fileNum)
    {
        currentOffset = thisProcsOffset;
    }
    else
    {
        currentOffset = 0;
    }
    for (int i = 0 ; i < totalEntries ; i++)
    {
        int domain = i / nVals;
        if (files[domain] != currentOutputFile)
        {
            //
            // We are now working on a new file, so reset the offset.
            //
            currentOutputFile = files[domain];
            if (currentOutputFile == fileNum)
            {
                currentOffset = thisProcsOffset;
            }
            else
            {
                currentOffset = 0;
            }
        }

        if (lengths[i] > 0)
        {
            offsets[i] = currentOffset;
            currentOffset += lengths[i];
        }
        else
        {
            offsets[i] = -1;
        }
    }
}


// ****************************************************************************
//  Method: Value_Prep::WrapUp
//
//  Purpose:
//      Concludes the data input for a Value_Prep object.  Now that all of the 
//      domains have been read in, the lengths can be passed to processor 0
//      (parallel only), and the offsets can be calculated.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
// ****************************************************************************

void
Value_Prep::WrapUp(void)
{
    if (nVals < 0)
    {
        // 
        // This is an unused Value object and treating its data members
        // as valid is dangerous.
        //
        return;
    }

#ifdef PARALLEL
    // 
    // Make the lengths array correct by communicating with the other
    // processors.  Do this by sending the whole array and taking the
    // max of each element.  This assumes that lengths are positive and
    // that the array has been initialized.
    //
    // The MPI_IN_PLACE flag does not appear to be valid for this 
    // implementation of MPI, so create a receiving buffer.
    //
    extern   int num_processors;
    extern   int my_rank;
    if (num_processors > 1)
    {
        int  *outL = new int[totalEntries];
        int   i;

        MPI_Allreduce(offsets, outL, totalEntries, MPI_INT, MPI_MAX,
                      MPI_COMM_WORLD);
        for (i = 0 ; i < totalEntries ; i++)
        {
            offsets[i] = outL[i];
        }

        MPI_Allreduce(lengths, outL, totalEntries, MPI_INT, MPI_MAX, 
                      MPI_COMM_WORLD);
        for (i = 0 ; i < totalEntries ; i++)
        {
            lengths[i] = outL[i];
        }

        //
        // Get the group number for each domain.  This is used for figuring
        // out which file the domain is in.  We could calculate it, but then
        // we are exposed if the partitioning method is changed.
        //
        MPI_Allreduce(files, outL, nDomains, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        for (i = 0 ; i < nDomains ; i++)
        {
            files[i] = outL[i];
        }

        int  s;
        MPI_Allreduce(&totalSize, &s, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        totalSize = s;

        delete [] outL;
    }
#endif

    //
    // Now that we know which domains are going to which files, we can set the 
    // entryNames.
    //
    SetNames();
}
  

// ****************************************************************************
//  Method: Value_Prep::Index
//
//  Purpose:
//      An inlined function that calculates the proper index for a given
//      domain and the variable within the vector.
//      Currently, the variables within a vector are grouped together to
//      help with caching issues.
//
//  Arguments:
//      val     -  The index into the vector variable.
//      domain  -  The domain for this variable.
//
//  Returns:    The index into the array of size totalElements.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

int
Value_Prep::Index(int val, int domain)
{
    return domain*nVals + val;
}


// ****************************************************************************
//  Method:  Value_Prep::AddDomain
//
//  Purpose:
//      Save the length of this domain.
//
//  Programmer: Hank Childs
//  Creation:   December 17, 1999
//
// ****************************************************************************

void
Value_Prep::AddDomain(int *n, int domain)
{
    files[domain] = GetOutputFileNum(domain, nDomains);
    for (int i = 0 ; i < nVals ; i++)
    {
        lengths[Index(i, domain)] = n[i];
        sizeForFile[files[domain]] += n[i];
        totalSize += n[i];
    }
}


// ****************************************************************************
//  Method: Value_Prep::UpdateLength
//
//  Purpose:
//      Because some sizes (ie nshapes from zonelists) change when they are
//      read in, it is necessary to estimate the size of some domains for
//      some Value_Preps.  When the true size is obtained, this function allows
//      it to be updated.  The array that is written out the file will still
//      be oversized, but when writing the slab for this domain, the write
//      functions will not walk over the end of the array.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Value_Prep::UpdateLength(int domain, int *newLengths)
{
    for (int i = 0 ; i < nVals ; i++)
    {
        lengths[Index(i, domain)] = newLengths[i];
    }
}


// ****************************************************************************
//  Method: Value_Prep::WriteArray
//
//  Purpose:
//      Writes out a slab of an array for a Value_Prep object. 
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
Value_Prep::WriteArray(DBfile *dbfile, void **vals, int datatype, int domain) 
{
    if (vals == NULL)
    {
        //
        // Some arrays in SILO files (ie mixvals) are NULL.
        //
        return;
    }

    char newName[LONG_STRING];
    sprintf(newName, "%s%s", name, ARRAY_STRING);

    for (int i = 0 ; i < nVals ; i++)
    {
        int   offset = offsets[Index(i, domain)];
        int   length = lengths[Index(i, domain)];

        #ifdef DEBUG
        cerr << "Decided to write " << name << ", domain=" << domain
             << " at offset " << offset << " for length " << length << endl;
        #endif

        if (length == 0)
        {
            //
            // Some domains have no values (like mixvals).
            //
            continue;
        }

        int  size   = sizeForFile[files[domain]];
        int  stride = 1;
        if (datatype == DB_DOUBLE)
        {
            datatype = DB_FLOAT;
        }

        //
        // Sanity Check
        //
        if ((offset + length) > size)
        {
            cerr << "Writing " << name << " would cause an error." 
                 << endl;
            cerr << "offset(" << offset << ") + length(" << length 
                 << ") > size(" << size << ")" << endl;
            cerr << "\tfor domain " << domain << " in file " << files[domain]
                 << endl;
            continue;
        }

        DBWriteSlice(dbfile, newName, vals[i], datatype, &offset, &length, 
                     &stride, &size, 1);
    }
}


// ****************************************************************************
//  Method: Value_Prep::SetResource
//
//  Purpose:
//      Sets the resource type for the Value_Prep object.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2000
//
// ****************************************************************************

void
Value_Prep::SetResource(RESOURCE_e r)
{
    resource = r;
}


