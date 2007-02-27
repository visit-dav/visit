/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                           TimeSequence_Prep.C                             //
// ************************************************************************* //

#include <visitstream.h>
#include <silo.h>
#include <stdlib.h>
#include <string.h>

#include <IntervalTree.h>
#include <SiloObjLib.h>
#include <TimeSequence_Prep.h>


// 
// Initialization of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const TimeSequence_Prep::SILO_LOCATION              = "/";
char * const TimeSequence_Prep::SILO_IN_CYCLE_NAME         = "cycle";
char * const TimeSequence_Prep::SILO_IN_TIME_NAME          = "dtime";
char * const TimeSequence_Prep::SILO_IN_TIME_NAME_ALT      = "time";


// ****************************************************************************
//  Method: TimeSequence_Prep constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

TimeSequence_Prep::TimeSequence_Prep()
{
    iStates            = 0;
    readInFields       = false;
    readInDimension    = NULL;
}


// ****************************************************************************
//  Method: TimeSequence_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

TimeSequence_Prep::~TimeSequence_Prep()
{
    if (readInDimension != NULL)
    {
        delete [] readInDimension;
    }
}


// ****************************************************************************
//  Method: TimeSequence_Prep::Write
//
//  Purpose:
//      Writes the time sequence as a SILO object to the file specified.
//
//  Arguments:
//      db_file  -  This is a pointer to the `visit' DBfile.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
  extern int my_rank;
  if (my_rank != 0)
  {
      return;
  }
#endif

    //
    // Set the directory in the file to be correct.
    //
    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to " 
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We start off reading in all of the Fields, regardless of command line
    // specifications.  Eliminate the Fields that were read in, but never
    // populated because we aren't supposed to process them.
    //
    EliminateUnusedFields();

    TimeSequence::Write(dbfile);
}


// ****************************************************************************
//  Method: TimeSequence_Prep::EliminateUnusedFields
//
//  Purpose:
//      We read in all of the Fields at the beginning, but only wanted some of
//      them, so we will eliminate them now.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
TimeSequence_Prep::EliminateUnusedFields(void)
{
    int   last = 0;
    for (int i = 0 ; i < nFields ; i++)
    {
        if (readInDimension[i] == false)
        {
            //
            // First free any information associated with this Field.
            //
            if (fieldNames[i] != NULL)
            {
                delete [] fieldNames[i];
                fieldNames[i] = NULL;
            }
            if (fieldIntervalTree[i] != NULL)
            {
                delete [] fieldIntervalTree[i];
                fieldIntervalTree[i] = NULL;
            }
        }
        else
        {
            //
            // This is a valid Field, so swap it to the front of the list to
            // replace an invalid one.  This does not cause memory leaks, but
            // can appear to be a little fishy.  Essentially, its ok because
            // nFields is altered at the end.
            //
            readInDimension[last]   = readInDimension[i];
            fieldNames[last]        = fieldNames[i];
            fieldTypes[last]        = fieldTypes[i];
            fieldDimension[last]    = fieldDimension[i];
            fieldIntervalTree[last] = fieldIntervalTree[i];
            last++;
        }
    }
    
    //
    // The new number of fields is the valid entries we found.
    //
    nFields = last;
}


// ****************************************************************************
//  Method: TimeSequence_Prep::SetFields
//
//  Purpose:
//      Sets the number of fields.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::SetFields(int N)
{
    nFields           = N;
    fieldNames        = new char *[nFields];
    fieldTypes        = new int   [nFields];
    fieldDimension    = new int   [nFields];
    fieldIntervalTree = new char *[nFields];
    readInDimension   = new bool  [nFields];
    for (int i = 0 ; i < N ; i++)
    {
        fieldNames[i]        = NULL;
        fieldTypes[i]        = -1;
        fieldDimension[i]    = -1;
        fieldIntervalTree[i] = NULL;
        readInDimension[i]   = false;
    }
}


// ****************************************************************************
//  Method: TimeSequence_Prep::SetStates
//
//  Purpose: 
//      Sets the number of states.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Modifications:
//
//      Hank Childs, Thu Jun 15 14:01:28 PDT 2000
//      Got the state name instead of the state pattern.
//
// ****************************************************************************

void
TimeSequence_Prep::SetStates(NamingConvention *code)
{
    nStates = code->GetStatesN();
    iStates = 0;
    cycles  = new int[nStates];
    times   = new double[nStates];

    states  = new char*[nStates];
    for (int i = 0 ; i < nStates ; i++)
    {
        char  name[LONG_STRING];
        code->GetState(name, i);
        states[i] = CXX_strdup(name);
    }
}


// ****************************************************************************
//  Method: TimeSequence_Prep::AddState
//
//  Purpose:
//      Adds the information for a given state.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
// ****************************************************************************

void 
TimeSequence_Prep::AddState(int c, double t)
{
    if (iStates >= nStates)
    {
        cerr << "iStates exceeded nStates, bailing out..." << endl;
        exit(EXIT_FAILURE);
    }
    cycles[iStates] = c;
    times[iStates]  = t;
    iStates++;
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadFromRootFile
//
//  Purpose:
//      Gets the information contained in a root file for a time sequence 
//      object.  This includes the cycles, times, field names, and field
//      types.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::ReadFromRootFile(DBfile *dbfile, DBtoc *toc)
{
    ReadCycleAndTime(dbfile);
    if (readInFields == false)
    {
        ReadFields(dbfile, toc);
    }
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadVar (meshvar)
//
//  Purpose:
//      Gets the information out of the variables related to reading in 
//      a TimeSequence_Prep object.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::ReadVar(char *name, DBmeshvar *mv)
{
    ReadDimension(name, mv->nvals);
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadVar (quadvar)
//
//  Purpose:
//      Gets the information out of the variables related to reading in 
//      a TimeSequence_Prep object.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::ReadVar(char *name, DBquadvar *qv)
{
    ReadDimension(name, qv->nvals);
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadVar (ucdvar)
//
//  Purpose:
//      Gets the information out of the variables related to reading in 
//      a TimeSequence_Prep object.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::ReadVar(char *name, DBucdvar *uv)
{
    ReadDimension(name, uv->nvals);
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadDimension 
//
//  Purpose:
//      Figures out which variable you are reading the dimension for, and 
//      assigns it.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::ReadDimension(char *name, int dim)
{
    int   i;
    for (i = 0 ; i < nFields ; i++)
    {
        if (strcmp(name, fieldNames[i]) == 0)
        {
            // Found a match
            break;
        }
    }
  
    if (i == nFields)
    {
        // No match -- output a warning
        cerr << "Could not match \"" << name << "\" to a dimension." << endl;
        return;
    }

    if (readInDimension[i] == false)
    {
        fieldDimension[i]  = dim;
        readInDimension[i] = true;
    }
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadFields
//
//  Purpose:
//      Reads nFields, fieldNames, fieldTypes, and fieldDimension.
//      Also generates fieldIntervalTree.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
// ****************************************************************************

void
TimeSequence_Prep::ReadFields(DBfile *, DBtoc *toc)
{
    int    i, iField = 0;

    //
    // The number of fields is the sum of all of the variable types counters.
    // All of the numbers but one should be zero.
    //
    int n = toc->nmultivar + toc->nqvar + toc->nucdvar + toc->nptvar;
    SetFields(n);

    //
    // Read the multivar fields if there are any.
    //
    for (i = 0 ; i < toc->nmultivar ; i++)
    {
        fieldTypes[iField] = DB_MULTIVAR;
        fieldNames[iField] = CXX_strdup(toc->multivar_names[i]);

        iField++;
    }  

    //
    // Read the UCD variables if there are any.
    //
    for (i = 0 ; i < toc->nucdvar ; i++)
    {
        fieldTypes[iField] = DB_UCDVAR;
        fieldNames[iField] = CXX_strdup(toc->ucdvar_names[i]);

        iField++;
    }

    //
    // Read the Quad vars if there are any.
    //
    for (i = 0 ; i < toc->nqvar ; i++)
    {
        fieldTypes[iField] = DB_QUADVAR;
        fieldNames[iField] = CXX_strdup(toc->qvar_names[i]);

        iField++;
    }

    //
    // Read the Point vars if there are any.
    //
    for (i = 0 ; i < toc->nptvar ; i++)
    {
        fieldTypes[iField] = DB_POINTVAR;
        fieldNames[iField] = CXX_strdup(toc->ptvar_names[i]);

        iField++;
    }

    //
    // Sanity check
    //
    if (iField != nFields)
    {
        cerr << "iField != nFields, internal error" << endl;
        exit(EXIT_FAILURE);
    }
 
    //
    // Generate the FieldIntervalTree.
    //
    for (int j = 0 ; j < nFields ; j++)
    {
        char  s[LONG_STRING];
        sprintf(s, "%s%s", fieldNames[j], IntervalTree::NAME);
        fieldIntervalTree[j] = CXX_strdup(s);
    }

    readInFields = true;
}


// ****************************************************************************
//  Method: TimeSequence_Prep::ReadCycleAndTime
//
//  Purpose:
//      Reads the cycle and time for a given state out the DBfile passed in
//      as an argument.
//  
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Modifications:
//
//    Hank Childs, Wed Aug 30 13:20:22 PDT 2000
//    Allow for alternate time name.
//
// ****************************************************************************

void
TimeSequence_Prep::ReadCycleAndTime(DBfile *dbfile)
{
    int     c;
    int     varType = DBGetVarType(dbfile, SILO_IN_CYCLE_NAME);
    switch (varType)
    {
        case -1:
            cerr << "Cannot locate variable cycle." << endl;
            exit(EXIT_FAILURE);
            break;
        case DB_INT:
        {
            DBReadVar(dbfile, SILO_IN_CYCLE_NAME, &c);
            break;
        }
        default:
            cerr << "Do not support type " << varType << " for cycle." << endl;
            exit(EXIT_FAILURE);
            /* NOTREACHED */ break;
    }

    double t;
    char  *time_str = NULL;
    if (DBInqVarExists(dbfile, SILO_IN_TIME_NAME) != 0)
    {
        time_str = SILO_IN_TIME_NAME;
    }
    else if (DBInqVarExists(dbfile, SILO_IN_TIME_NAME_ALT) != 0)
    {
        time_str = SILO_IN_TIME_NAME_ALT;
    }
    
    if (time_str == NULL)
    {
        cerr << "Unable to locate variable time, using 0." << endl;
        t = 0.;
    }
    else
    {
        varType = DBGetVarType(dbfile, time_str);
        switch (varType)
        {
            case -1:
                cerr << "Cannot locate variable time." << endl;
                t = -1.;
                break;
            case DB_FLOAT:
            {
                float  f;
                DBReadVar(dbfile, time_str, &f);
                t = static_cast< double >(f);
                break;
            }
            case DB_DOUBLE:
            {
                DBReadVar(dbfile, time_str, &t);
                break;
            }
            default:
                cerr << "Do not support type " << varType << " for time." 
                     << endl;
                exit(EXIT_FAILURE);
                /* NOTREACHED */ break;
        }
    }

    AddState(c, t);
}


