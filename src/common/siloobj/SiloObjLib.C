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
//                               SiloObjLib.C                                //
// ************************************************************************* //

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SiloObjLib.h>
#include <Utility.h>


// ****************************************************************************
//  Function: CondenseStringArray
//
//  Purpose:
//      This routine will take an array of character strings and condense it
//      to one array.  This is used for writing an array of strings to a SILO
//      file with one DBWrite.
//
//  Arguments:
//     INPUT:   sA       -  The array of strings.
//              size     -  The number of strings in sA.
//     OUTPUT:  outSize  -  The length of the string returned.
//
//  Returns:    A pointer to the array.  The array is located on the heap, so
//              this is safe.
//
//  Note:       The return value must be freed after it is used.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

char *
CondenseStringArray(char **sA, int size, int *outSize)
{
    int   i, j;

    // 
    // NULL input is valid, so handle gracefully.
    //
    if (sA == NULL)
    {
        char *rv = new char[1];
        strcpy(rv, "\0");
        *outSize = 1;
        return rv;
    }
      
    *outSize = 0;
    for (i = 0 ; i < size ; i++)
    {
        (*outSize)++;  // This is for the '\0'
        for (j = 0 ; sA[i][j] != '\0' ; j++)
        {
            (*outSize)++;
        }
    }

    char *rv = new char[*outSize];

    char *p = rv;
    for (i = 0 ; i < size ; i++)
    {
        strcpy(p, sA[i]);
        p += strlen(sA[i]) + 1;  // +1 is for '\0'
    }

    /* // DEBUG
    cerr << "Condensed string is:" << endl;
    for (i = 0 ; i < *outSize ; i++)
        if (isalnum(rv[i]))
        {
            cerr << rv[i] << (i%25 == 24 ? '\n' : ' ');
        }
        else
        {
            cerr << (int) rv[i] << (i%25 == 24 ? '\n' : ' ');
        }
    cerr << endl;
    */

    return rv;
}


// ****************************************************************************
//  Function:  Construct1DArray
//
//  Purpose:
//      Takes a two dimensional array and compresses it into a one dimensional
//      array so that it can be easily written into a SILO file.
//
//  Arguments:
//      INPUT:
//          nArrays       -  The number of arrays in the 2D array.
//          arraySize     -  The size of each array in the 2D array.  This is
//                           an array of size nArrays.
//          arrays        -  the 2D array.  This is an array of size 
//                           nArrays x arraySize[i].
//      OUTPUT:
//          outputArray      -  The array to write into.
//          outputArraySize  -  The size of the array outputted.
//
//  Note:       The outputArray must be freed by the calling function.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Construct1DArray(int nArrays, int *arraySize, int **arrays, int **outputArray,
                 int *outputArraySize)
{
    int   i, j;

    //
    // Figure out what outputArraySize will be.
    //
    *outputArraySize = 0;
    for (i = 0 ; i < nArrays ; i++)
    {
        *outputArraySize += arraySize[i];
    }

    //
    // Set up the 1D array to write into.
    //
    *outputArray = new int[*outputArraySize];

    int   count = 0;
    for (i = 0 ; i < nArrays ; i++)
    {
        if (arrays[i] != NULL)
        {
            for (j = 0 ; j < arraySize[i] ; j++)
            {
                (*outputArray)[count] = arrays[i][j];
                count++;
            }
        }
        else
        {
            //
            // We handle this case for parallel processing reasons.
            //
            for (j = 0 ; j < arraySize[i] ; j++)
            {
                (*outputArray)[count] = 0;
                count++;
            }
        }
    }
}


// ****************************************************************************
//  Function: UncondenseStringArray
//
//  Purpose:
//      Takes a string that contains multiple strings all delimited by \0's,
//      and copies in into a 2D char array.
//
//  Input Arguments:
//      s     -  The string that contains \0-delimited substrings.
//      n     -  The number of strings condensed into s.
//  Input/Output Arguments:
//      outA  -  The 2D array to copy each of the substrings into.  This array
//               should be of size n.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
UncondenseStringArray(char **outA, int n, char *s)
{
    char  *p = s;
    for (int i = 0 ; i < n ; i++)
    {
        outA[i] = CXX_strdup(p);
        p += strlen(p)+1;
    }
}


// ****************************************************************************
//  Function: InsertionSort
//
//  Purpose:
//      Sorts a list using insertion sort, which is linear for sorted lists.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2000
//
// ****************************************************************************

void
InsertionSort(int *list, int listN)
{
    //
    // At each iteration i, the first i elements of the list are sorted.  
    //
    for (int i = 0 ; i < listN ; i++)
    {
        for (int j = i ; j > 0 ; j--)
        {
            if (list[j] > list[j-1])
            {
                int temp  = list[j];
                list[j]   = list[j-1];
                list[j-1] = temp;
            }
            else
            {
                //
                // Since the first i elements are sorted, all of the elements
                // before this one must be smaller, so we don't need to check.
                //
                break;
            }
        }
    }
}


// ****************************************************************************
//  Function: MakeVisitFileList
//
//  Purpose:
//      Takes the names of the mesh files and state files and translates them
//      into a NULL-delimited string.
//      The string is of the form:
//        <number of entries>
//        <number of mesh files> {<mesh files names>}
//        <number of states>
//        {<number of states files> {<state files names>}}
//
//  Arguments:
//      nMesh          The number of mesh files.
//      mesh           An array of mesh file names.
//      nStates        The number of states.
//      nStateFiles    The number of files for each state.
//      states         The name of each state file for each state.
//      length         The number of characters in the output string.
//
//  Returns:   A NULL-delimited string that contains all of the information
//             about the number of names of files.
//
//  Note:       The return value must be freed.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2000
//
// ****************************************************************************

char *
MakeVisitFileList(int nMesh, char **mesh, int nStates, int *nStateFiles,
                   char ***states, int *length)
{
    int    i, j;
    int    numStrings = 0;

    numStrings += 1;  // Total number of strings.
    numStrings += 1;  // Number of files for the mesh.
    numStrings += nMesh;

    numStrings += 1;  // Number of states
    for (i = 0 ; i < nStates ; i++)
    {
        numStrings += 1;  // Number of files for state i.
        numStrings += nStateFiles[i];
    }

    char  **list  = new char*[numStrings];
    int     listN = 0;

    char   numStringsString[32];
    sprintf(numStringsString, "%d", numStrings);
    list[listN++] = numStringsString;

    char   nMeshString[32];
    sprintf(nMeshString, "%d", nMesh);
    list[listN++] = nMeshString;

    for (i = 0 ; i < nMesh ; i++)
    {
        list[listN++] = mesh[i];
    }

    char numStatesString[32];
    sprintf(numStatesString, "%d", nStates);
    list[listN++] = numStatesString;

    char  **nums = new char*[nStates];
    for (i = 0 ; i < nStates ; i++)
    {
        nums[i] = new char[32];
        sprintf(nums[i], "%d", nStateFiles[i]);
        list[listN++] = nums[i];
        for (j = 0 ; j < nStateFiles[i] ; j++)
        {
            list[listN++] = states[i][j];
        }
    }

    char *rv = CondenseStringArray(list, listN, length);

    delete [] list;
    for (i = 0 ; i < nStates ; i++)
    {
        delete [] nums[i];
    }
    delete [] nums;

    return rv;
} 


// ****************************************************************************
//  Function: ParseVisitFileList
//
//  Purpose:
//      Parses out the file list from the .visit file from the long, NULL-
//      delimited string generated by MakeVisitFileList.
//
//  Arguments:
//      list      The visit file list.
//      nMesh     The number of meshes.
//      meshes    The mesh files.
//      nStates   The number of states.
//      nFiles    The number of files in each state.
//      states    The files for each state.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2000
//
// ****************************************************************************

void
ParseVisitFileList(char *list, int &nMesh, char **&meshes, int &nStates, 
                  int *&nFiles, char ***&states)
{
    int  i, j;

    //
    // The first element of list is the total number of entries.
    //
    int  num = atoi(list);
    if (num <= 0)
    {
        cerr << "Unable to parse visit file list." << endl;
        return;
    }

    char *p = list;
    for (i = 1 ; i < num ; i++)
    {
        p += strlen(p)+1;
    }

    
    char **clist = new char*[num];
    UncondenseStringArray(clist, num, list);

    int  listN = 1;  // Element 0 was used to calculate num.

    nMesh  = atoi(clist[listN++]);
    meshes = new char*[nMesh];
    for (i = 0 ; i < nMesh ; i++)
    {
        meshes[i] = clist[listN++];
    }

    nStates = atoi(clist[listN++]);
    nFiles  = new int[nStates];
    states  = new char**[nStates];
    for (i = 0 ; i < nStates ; i++)
    {
        nFiles[i] = atoi(clist[listN++]);
        states[i] = new char*[nFiles[i]];
        for (j = 0 ; j < nFiles[i] ; j++)
        {
            states[i][j] = clist[listN++];
        }
    }
}


