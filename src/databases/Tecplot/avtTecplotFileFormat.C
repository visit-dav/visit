/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtTecplotFileFormat.C                         //
// ************************************************************************* //

#include <avtTecplotFileFormat.h>

#include <stdlib.h>
#include <string>

#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include <avtDatabaseMetaData.h>
#include <vtkPolyData.h>
#include <vtkVisItUtility.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <DebugStream.h>

using std::string;
using std::vector;
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif


#if defined(_MSC_VER) || !defined(HAVE_STRTOF) || !defined(HAVE_STRTOF_PROTOTYPE)
#ifndef strtof
#define strtof(f1,f2) ((float)strtod(f1,f2))
#endif
#endif


// ****************************************************************************
//  Method:  GetCoord/GuessCoord
//
//  Purpose:
//    Turns a variable name into an axis number (x=0,y=1,z=2, none=-1).
//    GetCoord only returns exact matches.
//    GuessCoord will also try non-exact maches (e.g. "X [m]" includes units).
//
//  Arguments:
//    tok        the token to convert into an axis number
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  7, 2008
//
// ****************************************************************************
static int GetCoord(const string &tok)
{
    if (tok=="X" || tok=="x" || tok=="I")
        return 0;
    if (tok=="Y" || tok=="y" || tok=="J")
        return 1;
    if (tok=="Z" || tok=="z" || tok=="K")
        return 2;
    return -1;
}

static int GuessCoord(const string &tok)
{
    int guessed = GetCoord(tok);

    if (tok.length() >= 3)
    {
        // do match: "x[m]" or "x (m)", etc.
        // don't match: "x velocity"
        if ((!isspace(tok[1]) && !isalnum(tok[1])) ||
            (isspace(tok[1] && !isalnum(tok[2]))))
        {
            guessed = GetCoord(tok.substr(0,1));
        }
    }

    return guessed;
}

// ****************************************************************************
//  Method:  SimplifyWhitespace
//
//  Purpose:
//    Remove leading and trailing tabs and spaces from a string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 18, 2007
//
// ****************************************************************************
static string SimplifyWhitespace(const std::string &s)
{
    int first = 0;
    int last = s.length()-1;
    while (first < last && (s[first] == ' ' || s[first] == '\t'))
        ++first;
    while (last > first && (s[last] == ' ' || s[last] == '\t'))
        --last;
    return s.substr(first, last-first+1);
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::PushBackToken
//
//  Purpose:
//    Needed for faking one-token lookahead.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecplotFileFormat::PushBackToken(const string &tok)
{
    saved_token = tok;
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::GetNextToken
//
//  Purpose:
//    The scanner.  It probably makes some serious assumptions about tecplot.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu May 22 10:25:12 EDT 2008
//    Support DOS format text files.
//
//    Jeremy Meredith, Tue Oct 26 17:23:53 EDT 2010
//    Change the scanner to allow any of []()= as single-character
//    tokens.  This allows us to parse some of the more complex patterns
//    such as FOO=([4-6,8]=A,[5,12]=B).
//
// ****************************************************************************
string
avtTecplotFileFormat::GetNextToken()
{
    // this is where we fake a one-token lookahead
    if (!saved_token.empty())
    {
        string retval = saved_token;
        saved_token = "";
        return retval;
    }

    // oops!  we hit EOF and someone still wanted more.
    if (next_char_eof)
        return "";

    next_char_eol = false;
    token_was_string = false;

    string retval = "";
    if (!next_char_valid)
    {
        next_char = file.get();
        next_char_valid = true;
        if (!file)
        {
            next_char_eof = true;
        }
    }

    // skip inter-token whitespace
    while (!next_char_eof   &&
           (next_char == ' '  ||
            next_char == '\n' ||
            next_char == '\r' ||
            next_char == '\t' ||
            next_char == ','))
    {
        if (next_char == '\n' || next_char == '\r')
        {
            next_char_eol = true;
        }

        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        

        // Ignore blank lines since they don't return a token
        if (next_char_eol)
            return GetNextToken();
    }
     
    if (next_char == '\"')
    {
        token_was_string = true;
        // do strings
        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        
        while (!next_char_eof && next_char != '\"')
        {
            retval += next_char;
            next_char = file.get();
            if (!file)
            {
                next_char_eof = true;
            }
        }

        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }
    }
    else if (next_char == '='  ||
             next_char == '('  ||
             next_char == ')'  ||
             next_char == '['  ||
             next_char == ']')
    {
        // simple one-character tokens
        retval += next_char;
        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        
    }
    else
    {
        // do a normal token
        while (!next_char_eof   &&
               (next_char != ' '  &&
                next_char != '\n' &&
                next_char != '\r' &&
                next_char != '\t' &&
                next_char != '='  &&
                next_char != '('  &&
                next_char != ')'  &&
                next_char != '['  &&
                next_char != ']'  &&
                next_char != ','))
        {
            if (next_char >= 'a' && next_char <= 'z')
                next_char += (int('A')-int('a'));
            retval += next_char;
            next_char = file.get();
            if (!file)
            {
                next_char_eof = true;
            }        
        }
    }

    // skip whitespace to EOL
    while (!next_char_eof   &&
           (next_char == ' '  ||
            next_char == '\n' ||
            next_char == '\r' ||
            next_char == '\t' ||
            next_char == ','))
    {
        if (next_char == '\n' || next_char == '\r')
        {
            next_char_eol = true;
        }

        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        

        if (next_char_eol)
            break;
    }
     
    return retval;
}

// ****************************************************************************
//  Method:  ParseArraysPoint
//
//  Purpose:
//    Reads the coordinates and/or nodal data for POINT and FEPOINT formats.
//
//  Arguments:
//    numNodes   the number of nodes
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Jul  7 14:09:28 EDT 2008
//    Allow X/Y/Z coordinate arrays to show up as normal variables.  This is
//    because we now have improved axis-variable guessing, and so if we guess
//    wrong we want to avoid hiding what might be a normal scalar variable.
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
//    Jeremy Meredith, Tue Oct 26 17:13:39 EDT 2010
//    Added support for comments.
//
//    Jeremy Meredith, Wed May 18 13:23:11 EDT 2011
//    Removed distinction between allVariableNames and variableNames, since
//    we treat even X/Y/Z coordinate arrays as normal variables, still.
//
//    Jeremy Meredith, Wed Jul 27 13:55:55 EDT 2011
//    Add support for 'nvals*value' repetition format for values.
//
//    Jeremy Meredith, Thu Oct 20 13:23:42 EDT 2011
//    Removed unused allScalars.
//    Support VARSHARELIST.
//    Minor performance improvements: it turns out the majority of the
//    time in this routine appears to be down in GetNextToken, not
//    any inefficiencies here, or even in the strtof call.
//
//    Jeremy Meredith, Mon Oct 24 12:06:45 EDT 2011
//    Add check for valid coord fields before assigned vals to them.
//
// ****************************************************************************
vtkPoints*
avtTecplotFileFormat::ParseArraysPoint(int numNodes, int numElements)
{
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(numNodes);
    float *pts = (float *)points->GetVoidPointer(0);
    int i;

    for (i=0; i<numNodes*3; i++)
    {
        pts[i] = 0;
    }

    vector<float*> allptr;

    for (int v=0; v<numTotalVars; v++)
    {
        if (variableShareMap[v] >= 0)
        {
            // just copy and add a reference to the array
            int dest = variableShareMap[v];
            vtkFloatArray *scalars = vars[variableNames[v]][dest];
            scalars->Register(NULL);
            vars[variableNames[v]].push_back(scalars);

            // still need the raw pointer so our indexing below is valid
            float *ptr = (float *) scalars->GetVoidPointer(0);
            allptr.push_back(ptr);
        }
        else
        {
            int numVals = (variableCellCentered[v] ? numElements : numNodes);

            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(numVals);
            float *ptr = (float *) scalars->GetVoidPointer(0);
            vars[variableNames[v]].push_back(scalars);

            allptr.push_back(ptr);
        }
    }

    int repeatCounter  = 0;
    float currentValue = 0;
    for (i=0; i<numNodes; i++)
    {
        bool doneWithCellCentered = (i>=numElements);
        for (int v = 0; v < numTotalVars; v++)
        {
            if (variableCellCentered[v] && doneWithCellCentered)
                continue;

            if (variableShareMap[v] >= 0)
            {
                // note: we assume repeat counter doesn't affect shared vars
                continue;
            }
            else
            {
                if (repeatCounter == 0)
                {
                    string tok = GetNextToken();
                    int toklen = tok.length();
                    if (toklen>0 && tok[0]=='#')
                    {
                        while (!next_char_eol)
                            tok = GetNextToken();
                        tok = GetNextToken();
                    }

                    char *endptr;
                    const char *cptr;
                    repeatCounter   = 1;
                    currentValue    = strtof((cptr=tok.c_str()), &endptr);
                    int   numparsed = endptr-cptr;
                    if (numparsed < toklen && tok[numparsed] == '*')
                    {
                        currentValue  = atof(tok.substr(numparsed+1).c_str());
                        repeatCounter = atoi(tok.substr(0,numparsed).c_str());
                    }
                }
                allptr[v][i] = currentValue;
                --repeatCounter;
            }
        }
    }

    for (i=0; i<numNodes; i++)
    {
        if (Xindex>=0)
            pts[3*i + 0] = allptr[Xindex][i];
        if (Yindex>=0)
            pts[3*i + 1] = allptr[Yindex][i];
        if (Zindex>=0)
            pts[3*i + 2] = allptr[Zindex][i];
    }

    return points;
}

// ****************************************************************************
//  Method:  ParseArraysBlock
//
//  Purpose:
//    Reads the coordinates and/or nodal data for BLOCK and FEBLOCK formats.
//
//  Arguments:
//    numNodes   the number of nodes
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Jul  7 14:09:28 EDT 2008
//    Allow X/Y/Z coordinate arrays to show up as normal variables.  This is
//    because we now have improved axis-variable guessing, and so if we guess
//    wrong we want to avoid hiding what might be a normal scalar variable.
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
//    Jeremy Meredith, Tue Oct 26 17:13:39 EDT 2010
//    Added support for comments.
//
//    Jeremy Meredith, Wed May 18 13:23:11 EDT 2011
//    Removed distinction between allVariableNames and variableNames, since
//    we treat even X/Y/Z coordinate arrays as normal variables, still.
//
//    Jeremy Meredith, Wed Jul 27 13:55:55 EDT 2011
//    Add support for 'nvals*value' repetition format for values.
//
//    Jeremy Meredith, Fri Oct 21 10:16:33 EDT 2011
//    Support VARSHARELIST.
//
// ****************************************************************************
vtkPoints*
avtTecplotFileFormat::ParseArraysBlock(int numNodes, int numElements)
{
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(numNodes);
    float *pts = (float *)points->GetVoidPointer(0);

    for (int i=0; i<numNodes*3; i++)
    {
        pts[i] = 0;
    }

    int repeatCounter  = 0;
    float currentValue = 0;
    for (int v = 0; v < numTotalVars; v++)
    {
        int numVals = (variableCellCentered[v] ? numElements : numNodes);
        float *ptr = NULL;

        // check if we're sharing with other variables first
        if (variableShareMap[v] >= 0)
        {
            int dest = variableShareMap[v];
            vtkFloatArray *scalars = vars[variableNames[v]][dest];
            scalars->Register(NULL);
            ptr = (float *) scalars->GetVoidPointer(0);
            vars[variableNames[v]].push_back(scalars);
        }
        else
        {
            // nope, okay; read it
            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(numVals);
            ptr = (float *) scalars->GetVoidPointer(0);
            for (int i=0; i<numVals; i++)
            {
                if (repeatCounter == 0)
                {
                    string tok = GetNextToken();
                    int   toklen = tok.length();
                    if (toklen>0 && tok[0]=='#')
                    {
                        while (!next_char_eol)
                            tok = GetNextToken();
                        tok = GetNextToken();
                    }

                    char *endptr;
                    const char *cptr;
                    repeatCounter   = 1;
                    currentValue    = strtof((cptr=tok.c_str()), &endptr);
                    int   numparsed = endptr-cptr;
                    if (numparsed < toklen && tok[numparsed] == '*')
                    {
                        currentValue  = atof(tok.substr(numparsed+1).c_str());
                        repeatCounter = atoi(tok.substr(0,numparsed).c_str());
                    }
                }

                ptr[i] = currentValue;

                --repeatCounter;
            }
            vars[variableNames[v]].push_back(scalars);
        }

        if (v==Xindex)
        {
            for (int i=0; i<numVals; i++)
                pts[3*i + 0] = ptr[i];
        }
        else if (v==Yindex)
        {
            for (int i=0; i<numVals; i++)
                pts[3*i + 1] = ptr[i];
        }
        else if (v==Zindex)
        {
            for (int i=0; i<numVals; i++)
                pts[3*i + 2] = ptr[i];
        }
    }

    return points;
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::ParseElements
//
//  Purpose:
//    Parses the element connectivity arrays.
//
//  Arguments:
//    numElements   the number of elements
//    elemType      the element type
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for 'POINT' element. This is not a keyword tecplot
//    understands but was necessary to enable support for point meshes.
//
//    Jeremy Meredith, Tue Jun 19 15:11:50 EDT 2007
//    Allowed an FE mesh with no ET (element type) field in the zone record
//    header to default to a point mesh.
//
//    Jeremy Meredith, Fri Oct  9 16:22:40 EDT 2009
//    Added new names for element types.
//
//    Jeremy Meredith, Tue Oct 26 11:03:35 EDT 2010
//    Added a couple "FE" variants that were missing.
//    Added support for comments.
//
//    Mark C. Miller, Thu Jun  4 11:47:02 PDT 2015
//    Added support for FELINESEG elem type
// ****************************************************************************
vtkUnstructuredGrid *
avtTecplotFileFormat::ParseElements(int numElements, const string &elemType)
{
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

    // construct the cell arrays
    int nelempts = -1;
    int idtype = -1;
    if (elemType == "BRICK" || elemType == "FEBRICK")
    {
        nelempts = 8;
        idtype = VTK_HEXAHEDRON;
        topologicalDimension = MAX(topologicalDimension, 3);
    }
    else if (elemType == "TRIANGLE" || elemType == "FETRIANGLE")
    {
        nelempts = 3;
        idtype = VTK_TRIANGLE;
        topologicalDimension = MAX(topologicalDimension, 2);
    }
    else if (elemType == "QUADRILATERAL" || elemType == "FEQUADRILATERAL")
    {
        nelempts = 4;
        idtype = VTK_QUAD;
        topologicalDimension = MAX(topologicalDimension, 2);
    }
    else if (elemType == "TETRAHEDRON" || elemType == "FETETRAHEDRON")
    {
        nelempts = 4;
        idtype = VTK_TETRA;
        topologicalDimension = MAX(topologicalDimension, 3);
    }
    else if (elemType == "LINESEG" || elemType == "FELINESEG")
    {
        nelempts = 2;
        idtype = VTK_LINE;
        topologicalDimension = MAX(topologicalDimension, 1);
    }
    else if (elemType == "POINT" || elemType == "FEPOINT" || elemType == "")
    {
        nelempts = 1;
        idtype = VTK_VERTEX;
        topologicalDimension = MAX(topologicalDimension, 0);
    }
    else
    {
        EXCEPTION2(InvalidFilesException, filename,
                   "Unknown element type for zone");
    }

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues((nelempts+1) * numElements);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(numElements);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(numElements);
    vtkIdType *cl = cellLocations->GetPointer(0);

    int offset = 0;
    for (int c=0; c<numElements; c++)
    {
        *ct++ = idtype;

        *nl++ = nelempts;
        // 1-origin connectivity array
        for (int j=0; j<nelempts; j++)
        {
            if (idtype == VTK_VERTEX)
                *nl++ = c;
            else
            {
                string tok = GetNextToken();
                if (tok.length()>0 && tok[0]=='#')
                {
                    while (!next_char_eol)
                        tok = GetNextToken();
                    tok = GetNextToken();
                }

                int val = atoi(tok.c_str());
                *nl++ = val - 1;
            }
        }
        
        *cl++ = offset;
        offset += nelempts+1;
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(numElements, nlist);
    nlist->Delete();

    ugrid->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    return ugrid;
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::ParseFEBLOCK
//
//  Purpose:
//    Parses an FEBLOCK zone
//
//  Arguments:
//    numNodes      the number of nodes
//    numElements   the number of elements
//    elemType      the element type
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed leak for 1D case
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for point meshes where topo dim is zero but spatial dim>1
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
//    Jeremy Meredith, Mon Apr 28 17:06:07 EDT 2014
//    Added support for copying connectivity from earlier ZONE instead
//    of having explicit arrays on disk.
//
//    Mark C. Miller, Thu Jun  4 11:47:43 PDT 2015
//    Adjusted logic for detecting curves defined on FELINESEG elem types
// ****************************************************************************
void
avtTecplotFileFormat::ParseFEBLOCK(int numNodes, int numElements,
                                   const string &elemType, int connectivitycopy)
{
    vtkPoints *points = ParseArraysBlock(numNodes,numElements);
    vtkUnstructuredGrid *ugrid = NULL;
    if (connectivitycopy >= 0)
    {
        vtkDataSet *from = meshes[connectivitycopy];
        vtkUnstructuredGrid *fromug = dynamic_cast<vtkUnstructuredGrid*>(from);
        if (from->GetDataObjectType() != VTK_UNSTRUCTURED_GRID ||
            fromug == NULL)
            EXCEPTION1(InvalidFilesException, "CONNECTIVITYCOPY refered to non-FE ZONE");
        ugrid = vtkUnstructuredGrid::New();
        ugrid->ShallowCopy(fromug);
    }
    else
    {
        ugrid = ParseElements(numElements, elemType);
    }
    ugrid->SetPoints(points);
    points->Delete();

    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension <= 1 && spatialDimension > 1))
    {
        meshes.push_back(ugrid);
    }
    else
    {
        ugrid->Delete();
    }
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::ParseFEPOINT
//
//  Purpose:
//    Parses an FEPOINT zone
//
//  Arguments:
//    numNodes      the number of nodes
//    numElements   the number of elements
//    elemType      the element type
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed leak for 1D case
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for point meshes where topo dim is zero but spatial dim>1
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
//    Jeremy Meredith, Mon Apr 28 17:06:07 EDT 2014
//    Added support for copying connectivity from earlier ZONE instead
//    of having explicit arrays on disk.
//
//    Mark C. Miller, Thu Jun  4 11:47:43 PDT 2015
//    Adjusted logic for detecting curves defined on FELINESEG elem types
// ****************************************************************************
void
avtTecplotFileFormat::ParseFEPOINT(int numNodes, int numElements,
                                   const string &elemType, int connectivitycopy)
{
    vtkPoints *points = ParseArraysPoint(numNodes,numElements);
    vtkUnstructuredGrid *ugrid = NULL;
    if (connectivitycopy >= 0)
    {
        vtkDataSet *from = meshes[connectivitycopy];
        vtkUnstructuredGrid *fromug = dynamic_cast<vtkUnstructuredGrid*>(from);
        if (from->GetDataObjectType() != VTK_UNSTRUCTURED_GRID ||
            fromug == NULL)
            EXCEPTION1(InvalidFilesException, "CONNECTIVITYCOPY refered to non-FE ZONE");
        ugrid = vtkUnstructuredGrid::New();
        ugrid->ShallowCopy(fromug);
    }
    else
    {
        ugrid = ParseElements(numElements, elemType);
    }
    ugrid->SetPoints(points);
    points->Delete();

    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension <= 1 && spatialDimension > 1))
    {
        meshes.push_back(ugrid);
    }
    else
    {
        ugrid->Delete();
    }
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::ParseBLOCK
//
//  Purpose:
//    Parses a structured BLOCK zone
//
//  Arguments:
//    numI/J/K   the number of nodes in each dimension
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed leak for 1D case
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for point meshes where topo dim is zero but spatial dim>1
//
//    Jeremy Meredith, Thu Jun 26 17:28:16 EDT 2008
//    Only assume it's a 1D mesh if *both* J and K dims are 1.
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
//    Mark C. Miller, Thu Jun  4 11:47:43 PDT 2015
//    Adjusted logic for detecting curves defined on FELINESEG elem types
// ****************************************************************************
void
avtTecplotFileFormat::ParseBLOCK(int numI, int numJ, int numK)
{
    int numNodes = numI * numJ * numK;

    if (numJ==1 && numK==1)
        topologicalDimension = MAX(topologicalDimension, 1);
    else if (numK==1)
        topologicalDimension = MAX(topologicalDimension, 2);
    else
        topologicalDimension = MAX(topologicalDimension, 3);

    int numElementsI = (numI <= 1) ? 1 : numI-1;
    int numElementsJ = (numJ <= 1) ? 1 : numJ-1;
    int numElementsK = (numK <= 1) ? 1 : numK-1;
    int numElements = numElementsI * numElementsJ * numElementsK;
    vtkPoints *points = ParseArraysBlock(numNodes, numElements);

    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    points->Delete();

    int dims[3] = {numI, numJ, numK};
    sgrid->SetDimensions(dims);

    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension <= 1 && spatialDimension > 1))
    {
        meshes.push_back(sgrid);
    }
    else
    {
        sgrid->Delete();
    }
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::ParsePOINT
//
//  Purpose:
//    Parses a structured POINT zone
//
//  Arguments:
//    numI/J/K   the number of nodes in each dimension
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed leak for 1D case
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for point meshes where topo dim is zero but spatial dim>1
//
//    Jeremy Meredith, Thu Jun 26 17:28:16 EDT 2008
//    Only assume it's a 1D mesh if *both* J and K dims are 1.
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
//    Mark C. Miller, Tue Sep  1 10:52:13 PDT 2009
//    Made it deal with POINT zones that contain more spatial dimensions
//    than topological ones.
//
//    Jeremy Meredith, Mon Nov  9 13:04:25 EST 2009
//    Don't force an M-topo-dimension zone in a N-spatial-dimension block
//    (where M<N) to become a point mesh.  These types of blocks are
//    perfectly valid, but extra support may need to be added to VisIt
//    proper to handle them correctly.
//
//    Mark C. Miller, Thu Jun  4 11:47:43 PDT 2015
//    Adjusted logic for detecting curves defined on FELINESEG elem types
// ****************************************************************************
void
avtTecplotFileFormat::ParsePOINT(int numI, int numJ, int numK)
{
    int numNodes = numI * numJ * numK;
    int topologicalDimensionOfZone;

    if (numJ==1 && numK==1)
        topologicalDimensionOfZone = 1;
    else if (numK==1)
        topologicalDimensionOfZone = 2;
    else
        topologicalDimensionOfZone = 3;

    topologicalDimension = MAX(topologicalDimension, topologicalDimensionOfZone);

    int numElementsI = (numI <= 1) ? 1 : numI-1;
    int numElementsJ = (numJ <= 1) ? 1 : numJ-1;
    int numElementsK = (numK <= 1) ? 1 : numK-1;
    int numElements = numElementsI * numElementsJ * numElementsK;
    vtkPoints *points = ParseArraysPoint(numNodes, numElements);

    if (topologicalDimensionOfZone == 0) 
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(points);
        points->Delete();
        meshes.push_back(ugrid);
    }
    else
    {
        vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
        sgrid->SetPoints(points);
        points->Delete();

        int dims[3] = {numI, numJ, numK};
        sgrid->SetDimensions(dims);

        if ((topologicalDimension == 2 || topologicalDimension == 3) ||
            (topologicalDimension <= 1 && spatialDimension > 1))
        {
            meshes.push_back(sgrid);
        }
        else
        {
            sgrid->Delete();
        }
    }
}

// ****************************************************************************
//  Method:  avtTecplotFileFormat::ReadFile
//
//  Purpose:
//    Top-level method to re-read the whole file.
//
//  Arguments:
//    none       (filename is stored as a class member already)
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
//  Modifications:
//    Hank Childs, Sat Mar  5 14:28:52 PST 2005
//    Add call to 'clear' so that the file can be re-used on subsequent calls.
//
//    Brad Whitlock, Tue Jul 26 14:09:45 PST 2005
//    I made it understand DATASETAUXDATA.
//
//    Jeremy Meredith, Mon Jun 18 15:17:45 EDT 2007
//    If no format is given, assume POINT.  Also, allow for whitespace in
//    the given names of the X/Y/Z variables; apparently this occurs in real
//    life due to having to play nice with FORTRAN.
//
//    Jeremy Meredith, Thu Jun 26 17:22:18 EDT 2008
//    Allow "DATAPACKING" as an alias for "F" in zone record header.
//    Add smarter X/Y/Z coordinate guessing.
//
//    Jeremy Meredith, Mon Jul  7 14:09:28 EDT 2008
//    Add two-stage guessing for axis variables, and make the guessing smarter.
//    Also, allow X/Y/Z coordinate arrays to show up as normal variables just
//    in case we still guessed wrong.
//    
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//
//    Jeremy Meredith, Fri Oct  9 16:21:53 EDT 2009
//    Add some support for new ways you can specify zone headers in
//    recent tecplot flavors, including new keywords.
//
//    Mark C. Miller, Tue Jan 12 17:36:23 PST 2010
//    Added logic to parse SOLUTIONTIME and set solTime.
//
//    Jeremy Meredith, Tue Jul 13 15:51:24 EDT 2010
//    Allow the "$" which seems to appear alone on the last line of some
//    ASCII tecplot files.
//
//    Jeremy Meredith, Mon Sep 27 16:03:56 EDT 2010
//    Accept "NODES" and "ELEMENTS" as aliases for "N" and "E" in ZONE records.
//
//    Jeremy Meredith, Tue Oct 26 17:13:39 EDT 2010
//    Don't be quite to restrictive about what constitutes an FE-style ZONE.
//
//    Jeremy Meredith, Tue Oct 26 17:26:00 EDT 2010
//    The parser now returns parens, brackets, and equals as tokens.
//    Added code to skip over these when needed.  Also added parsing
//    support for complex version of VARLOCATION parameter.
//
//    Jeremy Meredith, Wed May 18 13:24:06 EDT 2011
//    Removed unused numVars and eliminated distinction between variableNames
//    and allVariablesNames, since we now exposed even X/Y/Z coordinate
//    arrays as normal variables.
//    Allow multiple passes through VARIABLES records, as long as the number
//    of variables (and we assume their names, too, though don't yet check)
//    doesn't vary between passes.
//
//    Jeremy Meredith, Fri Oct 21 10:17:30 EDT 2011
//    Add support for VARSHARELIST.
//    Skip STRANDID for now -- read it, but continue treating each zone
//    as its own domain.
//
//    Jeremy Meredith, Tue Oct 25 12:37:42 EDT 2011
//    Allow user manual override of coordinate axis variables (via options).
//
//    Jeremy Meredith, Mon Apr 28 17:06:07 EDT 2014
//    Added support for CONNECTIVITYSHAREZONE.  Also ignore "C" zone token.
//
// ****************************************************************************

void
avtTecplotFileFormat::ReadFile()
{
    file.open(filename.c_str());
    string tok = GetNextToken();
    int currentZoneIndex = 0;
    bool got_next_token_already = false;
    bool first_token = true;

#define READING_UNTIL_END_OF_LINE !next_char_eof     &&\
                                  tok != "TITLE"     &&\
                                  tok != "VARIABLES" &&\
                                  tok != "ZONE"      &&\
                                  tok != "GEOMETRY"  &&\
                                  tok != "TEXT"      &&\
                                  tok != "DATASETAUXDATA"

    while (!next_char_eof)
    {
        got_next_token_already = false;
        if (tok == "")
        {
            // whitespace!  do nothing
        }
        else if (tok == "TITLE")
        {
            // it's a title
            GetNextToken(); // skip the equals sign
            title = GetNextToken();
            debug5 << "Tecplot: parsed title as: '"<<title<<"'\n";
        }
        else if (tok == "GEOMETRY")
        {
            // unsupported
            tok = GetNextToken();
            while (READING_UNTIL_END_OF_LINE)
            {
                // Skipping token
                tok = GetNextToken();
            }
            got_next_token_already = true;
            debug5 << "Tecplot: Skipped unsupported GEOMETRY token\n";
        }
        else if (tok == "TEXT")
        {
            // unsupported
            tok = GetNextToken();
            while (READING_UNTIL_END_OF_LINE)
            {
                // Skipping token
                tok = GetNextToken();
            }
            got_next_token_already = true;
            debug5 << "Tecplot: Skipped unsupported TEXT token\n";
        }
        else if (tok == "$")
        {
            // this seems to have appeared at the end
            // of some files; may simply be an eof
            // pre-indicator, but it seems to be safe
            // to skip it.....
            tok = GetNextToken();
            while (READING_UNTIL_END_OF_LINE)
            {
                // Skipping token
                tok = GetNextToken();
            }
            got_next_token_already = true;
        }
        else if (tok == "VARIABLES")
        {
            debug5 << "Tecplot: Parsing VARIABLES token\n";
            // keep track of values from previous pass through VARIABLES
            int old_numTotalVars = numTotalVars;
            numTotalVars = 0;
            variableNames.clear();

            int guessedXindex = -1;
            int guessedYindex = -1;
            int guessedZindex = -1;

            // variable lists
            GetNextToken(); // skip the equals sign
            tok = GetNextToken();
            while (token_was_string)
            {
                int l = tok.length();
                for (int i=0;i<l;i++)
                {
                    if (tok[i]=='(')
                        tok[i] = '[';
                    else if (tok[i]==')')
                        tok[i] = ']';
                    else if (tok[i]=='/')
                        tok[i] = '_';
                }

                string tok_nw = SimplifyWhitespace(tok);

                switch (GetCoord(tok_nw))
                {
                  case 0: Xindex = numTotalVars; break;
                  case 1: Yindex = numTotalVars; break;
                  case 2: Zindex = numTotalVars; break;
                  default: break;
                }
                switch (GuessCoord(tok_nw))
                {
                  case 0: guessedXindex = numTotalVars; break;
                  case 1: guessedYindex = numTotalVars; break;
                  case 2: guessedZindex = numTotalVars; break;
                  default: break;
                }

                variableNames.push_back(tok);
                debug5 << "Tecplot:    got quoted variable name " << tok << "\n";
                numTotalVars++;
                tok = GetNextToken();
            }
            if (numTotalVars==0)
            {
                debug5 << "Tecplot:    found no quoted variable names, assuming unquoted\n";
                // If we didn't get any quoted variables, then
                // it's probably just because they forgot to quote
                // them.  Assume all we get are variables till EOL.

                // FIND A WAY TO DISPLAY THIS TO THE USER AS A WARNING!
                //cerr << "Didn't get any quoted variables.  Assuming"
                //     << " they weren't quoted and continuing until "
                //     << "the end-of-line.\n";

                while (true)
                {
                    string tok_nw = SimplifyWhitespace(tok);

                    switch (GetCoord(tok_nw))
                    {
                      case 0: Xindex = numTotalVars; break;
                      case 1: Yindex = numTotalVars; break;
                      case 2: Zindex = numTotalVars; break;
                      default: break;
                    }
                    switch (GuessCoord(tok_nw))
                    {
                      case 0: guessedXindex = numTotalVars; break;
                      case 1: guessedYindex = numTotalVars; break;
                      case 2: guessedZindex = numTotalVars; break;
                      default: break;
                    }

                    variableNames.push_back(tok);
                    debug5 << "Tecplot:    got unquoted variable name " << tok << "\n";
                    numTotalVars++;
                    if (next_char_eol)
                    {
                        tok = GetNextToken();
                        break;
                    }
                    else
                        tok = GetNextToken();
                }
            }

            // Make sure if we've encountered a VARAIBLES record before, the
            // number of variables hasn't changed.  (We assume they are
            // consistent across all zones.)
            if (old_numTotalVars > 0 &&
                old_numTotalVars != numTotalVars)
            {
                EXCEPTION2(InvalidFilesException, filename,
                           "VARIABLES record differed among zones.");
            }

            // Default the centering to nodal
            variableCellCentered.clear();
            variableCellCentered.resize(numTotalVars, 0);
            // Default to no shared vars
            variableShareMap.clear();
            variableShareMap.resize(numTotalVars, -1);

            // If we didn't find an exact match for coordinate axis vars, guess
            if (Xindex < 0) Xindex = guessedXindex;
            if (Yindex < 0) Yindex = guessedYindex;
            if (Zindex < 0) Zindex = guessedZindex;

            // If the user specified, override any coordinate axis guessing
            if (userSpecifiedAxisVars)
            {
                if (userSpecifiedX >= numTotalVars)
                    Xindex = -1;
                else
                    Xindex = userSpecifiedX;

                if (userSpecifiedY >= numTotalVars)
                    Yindex = -1;
                else
                    Yindex = userSpecifiedY;

                if (userSpecifiedZ >= numTotalVars)
                    Zindex = -1;
                else
                    Zindex = userSpecifiedZ;
            }

            // Based on how many spatial coords we got, guess the spatial dim
            if (Xindex >= 0)
            {
                spatialDimension = 1;
                if (Yindex >= 0)
                {
                    spatialDimension = 2;
                    if (Zindex >= 0)
                    {
                        spatialDimension = 3;
                    }
                }
            }

            debug5 << "Tecplot:    based on axis variables, spatial dim is " << spatialDimension << "\n";

            got_next_token_already = true;
        }
        else if (tok == "ZONE")
        {
            debug5 << "Tecplot: starting ZONE\n";
            // Parse a zone!
            char untitledZoneTitle[40];
            sprintf(untitledZoneTitle, "zone%05d", currentZoneIndex);
            currentZoneIndex++;

            string zoneTitle = untitledZoneTitle;
            string elemType = "";
            string format = "";
            int numNodes = 0;
            int numElements = 0;
            int numI = 1, numJ = 1, numK = 1;
            int connectivitycopy = -1;

            tok = GetNextToken();
            while (!(tok != "T"  &&
                     tok != "I"  &&
                     tok != "J"  &&
                     tok != "K"  &&
                     tok != "N"  &&
                     tok != "NODES"  &&
                     tok != "E"  &&
                     tok != "ELEMENTS"  &&
                     tok != "ET" &&
                     tok != "F"  &&
                     tok != "ZONETYPE"  &&
                     tok != "DATAPACKING"  &&
                     tok != "SOLUTIONTIME"  &&
                     tok != "VARLOCATION"  &&
                     tok != "DT" &&
                     tok != "D" &&
                     tok != "C" &&
                     tok != "STRANDID" &&
                     tok != "VARSHARELIST" &&
                     tok != "CONNECTIVITYSHAREZONE"))
            {
                debug5 << "Tecplot:    got zone token " << tok << "\n";
                if (tok == "T")
                {
                    GetNextToken(); // skip the equals sign
                    zoneTitle = GetNextToken();
                    if (!token_was_string)
                    {
                        EXCEPTION2(InvalidFilesException, filename,
                                   "Zone titles MUST be quoted.");
                    }
                }
                else if (tok == "I")
                {
                    GetNextToken(); // skip the equals sign
                    numI = atoi(GetNextToken().c_str());
                }
                else if (tok == "J")
                {
                    GetNextToken(); // skip the equals sign
                    numJ = atoi(GetNextToken().c_str());
                }
                else if (tok == "K")
                {
                    GetNextToken(); // skip the equals sign
                    numK = atoi(GetNextToken().c_str());
                }
                else if (tok == "N" || tok == "NODES")
                {
                    GetNextToken(); // skip the equals sign
                    numNodes = atoi(GetNextToken().c_str());
                }
                else if (tok == "E" || tok == "ELEMENTS")
                {
                    GetNextToken(); // skip the equals sign
                    numElements = atoi(GetNextToken().c_str());
                }
                else if (tok == "ET" || tok == "ZONETYPE")
                {
                    GetNextToken(); // skip the equals sign
                    elemType = GetNextToken();
                }
                else if (tok == "F" || tok == "DATAPACKING")
                {
                    GetNextToken(); // skip the equals sign
                    format = GetNextToken();
                }
                else if (tok == "C")
                {
                    GetNextToken(); // skip the equals sign
                    GetNextToken(); // skip the color
                }
                else if (tok == "SOLUTIONTIME")
                {
                    GetNextToken(); // skip the equals sign
                    solTime = strtod(GetNextToken().c_str(),0);
                }
                else if (tok == "VARLOCATION")
                {
                    variableCellCentered.clear();
                    variableCellCentered.resize(numTotalVars, 0);

                    GetNextToken(); // skip the equals sign
                    GetNextToken(); // skip the open paren

                    string c;
                    c = GetNextToken();
                    if (c == "[")
                    {
                        // complex version
                        // e.g. VARLOCATION=([2-3,5]=CELLCENTERED,[4]=NODAL)
                        while (c != ")")
                        {
                            // c == "["
                            vector<int> varindices;
                            c = GetNextToken();
                            while (c != "]")
                            {
                                string::size_type pos = c.find("-");
                                if (pos != string::npos)
                                {
                                    // Okay, we got something like "4-6".
                                    // Note, this scans as a single token
                                    // because we haven't gone all-out on
                                    // a scanner rewrite.  So just separate
                                    // it into a "4 through 6" here.
                                    // ALSO NOTE: THIS WILL NOT WORK
                                    // WITH ANY WHITESPACE IN HERE.
                                    // The examples don't have any, but
                                    // the tecplot manual doesn't actually
                                    // specify what's really allowed, so
                                    // given past history, someone may
                                    // eventually do something like that....
                                    string first = c.substr(0,pos);
                                    string last  = c.substr(pos+1);
                                    int beg = atoi(first.c_str());
                                    int end = atoi(last.c_str());
                                    for (int ind=beg; ind<=end; ind++)
                                        varindices.push_back(ind);
                                }
                                else
                                {
                                    varindices.push_back(atoi(c.c_str()));
                                }
                                c = GetNextToken();
                            }
                            GetNextToken(); // skip the equals sign
                            c = GetNextToken(); // that's the centering keyword
                            if (c == "CELLCENTERED")
                            {
                                // remember given indices are 1-origin
                                for (size_t i=0; i<varindices.size(); i++)
                                    variableCellCentered[varindices[i]-1] = 1;
                            }
                            
                            // next....
                            c = GetNextToken();
                        }
                    }
                    else
                    {
                        // simple version
                        // e.g. VARLOCATION=(NODAL,CELLCENTERED,CELLCENTERED)
                        for (int i=0; i<numTotalVars; i++)
                        {
                            if (c == "CELLCENTERED")
                                variableCellCentered[i] = 1;
                            c = GetNextToken();
                            // after the last var this picks up the close paren
                        }
                    }
                }
                else if (tok == "DT")
                {
                    GetNextToken(); // skip the equals sign
                    GetNextToken(); // skip the open paren
                    for (int i=0; i<numTotalVars; i++)
                        GetNextToken();
                    GetNextToken(); // skip the close paren
                }
                else if (tok == "D")
                {
                    EXCEPTION2(InvalidFilesException, filename,
                               "Tecplot Zone record parameter 'D' is "
                               "currently unsupported.  Please contact a "
                               "VisIt developer if you need support for this "
                               "parameter.");
                }
                else if (tok == "STRANDID")
                {
                    // not supporting STRANDID for now; assume domains
                    GetNextToken(); // skip the equals sign
                    GetNextToken(); // skip the value
                }
                else if (tok == "VARSHARELIST")
                {
                    variableShareMap.clear();
                    variableShareMap.resize(numTotalVars, -1);

                    GetNextToken(); // skip the equals sign
                    GetNextToken(); // skip the open paren

                    string c;
                    c = GetNextToken();
                    // if (c == "[")
                    {
                        // Unlike VARLOCATION, this only has a complex version.
                        // Note that no "=" means implicitly use previous-zone.
                        // e.g. VARSHARELIST=([2-3,5]=1,[4])
                        while (c != ")")
                        {
                            // c == "["
                            vector<int> varindices;
                            c = GetNextToken();
                            while (c != "]")
                            {
                                string::size_type pos = c.find("-");
                                if (pos != string::npos)
                                {
                                    // Okay, we got something like "4-6".
                                    // Note, this scans as a single token
                                    // because we haven't gone all-out on
                                    // a scanner rewrite.  So just separate
                                    // it into a "4 through 6" here.
                                    // ALSO NOTE: THIS WILL NOT WORK
                                    // WITH ANY WHITESPACE IN HERE.
                                    // The examples don't have any, but
                                    // the tecplot manual doesn't actually
                                    // specify what's really allowed, so
                                    // given past history, someone may
                                    // eventually do something like that....
                                    string first = c.substr(0,pos);
                                    string last  = c.substr(pos+1);
                                    int beg = atoi(first.c_str());
                                    int end = atoi(last.c_str());
                                    for (int ind=beg; ind<=end; ind++)
                                        varindices.push_back(ind);
                                }
                                else
                                {
                                    varindices.push_back(atoi(c.c_str()));
                                }
                                c = GetNextToken();
                            }
                            c = GetNextToken(); 
                            int dest = currentZoneIndex-1;
                            if (c == "=")
                            {
                                c = GetNextToken();
                                dest = atoi(c.c_str());
                                // next....
                                c = GetNextToken();
                            }
                            for (size_t vi=0; vi<varindices.size(); vi++)
                            {
                                // remember given indices are 1-origin
                                variableShareMap[varindices[vi]-1] = dest-1;
                            }
                        }
                    }
                    //else
                    //{
                    //    unlike VARLOCATION, there is no simple version
                    //}
                    debug5 << "Tecplot:   varshare: ";
                    for (int i=0; i<numTotalVars; ++i)
                        debug5 << variableShareMap[i] << " ";
                    debug5 << endl;
                }
                else if (tok == "CONNECTIVITYSHAREZONE")
                {
                    GetNextToken(); // skip the equals sign
                    string fromzone = GetNextToken(); 
                    connectivitycopy = atoi(fromzone.c_str()) - 1; // change 1-origin to 0-origin
                }
                tok = GetNextToken();
                debug5 << "Tecplot:    NEXT TOKEN: " << tok << "\n";
            }
            PushBackToken(tok);

            // New flavors of the tecplot format let you specify
            // simply "point" or "block" for the format, and by
            // adding a zonetype which starts with FE
            // (e.g. "FETETRAHEDRON"), switch to an FE
            // parsing mode.   Ugh.

            // Let's make this simple.  Any element type which
            // is specified and is not "ORDERED" will be assumed
            // to make this a finite-element style zone.
            if (elemType.length() > 0 && elemType != "ORDERED")
            {
                // Below we use the format type keyword to determine
                // which style to use, so fix it up to conform to what
                // we were originally expecting.
                if (format == "POINT")
                    format = "FEPOINT";
                if (format == "BLOCK")
                    format = "FEBLOCK";
            }

            zoneTitles.push_back(zoneTitle);
            if (format=="FEBLOCK")
            {
                ParseFEBLOCK(numNodes, numElements, elemType, connectivitycopy);
            }
            else if (format=="FEPOINT")
            {
                ParseFEPOINT(numNodes, numElements, elemType, connectivitycopy);
            }
            else if (format=="BLOCK")
            {
                ParseBLOCK(numI,numJ,numK);
            }
            else if (format=="POINT")
            {
                ParsePOINT(numI,numJ,numK);
            }
            else if (format=="")
            {
                // No format given; we will assume we got a POINT format
                ParsePOINT(numI,numJ,numK);
            }
            else
            {
                // UNKNOWN FORMAT
                char msg[200];
                sprintf(msg, "The format '%s' found in the file was unknown.",
                        format.c_str());
                EXCEPTION2(InvalidFilesException, filename, msg);
            }
        }
        else if(tok == "DATASETAUXDATA")
        {
            int  tokIndex = 0;
            bool haveVectorExpr = false;
            tok = GetNextToken();

            while (READING_UNTIL_END_OF_LINE)
            {
                if(tokIndex == 0)
                {
                    haveVectorExpr = (tok == "VECTOR");
                }
                else if(tokIndex == 1)
                {
                    // skip the equals sign
                }
                else if(tokIndex == 2)
                {
                    if(haveVectorExpr)
                    {
                        // Remove spaces
                        std::string::size_type pos = tok.find(" ");
                        while(pos != std::string::npos)
                        {
                            tok.replace(pos, 1, "");
                            pos = tok.find(" ");
                        }

                        // Look for '('
                        pos = tok.find("(");
                        if(pos != std::string::npos)
                        {
                            std::string exprName(tok.substr(0, pos));
                            std::string exprDef(tok.substr(pos, tok.size()-pos));

                            exprDef.replace(0, 1, "{");

                            // Replace ')' with '}'
                            pos = exprDef.find(")");
                            if(pos != std::string::npos)
                            {
                                exprDef.replace(pos, 1, "}");
                                debug4 << "Expr name=" << exprName.c_str()
                                       << ", Expr def=" << exprDef.c_str()
                                       << endl;
                                Expression newE;
                                newE.SetName(exprName);
                                newE.SetDefinition(exprDef);
                                newE.SetType(Expression::VectorMeshVar);
                                expressions.AddExpressions(newE);
                            }
                        }
                    }
                }

                // Skipping token
                tok = GetNextToken();
                ++tokIndex;
            }

            got_next_token_already = true;
        }
        else if (first_token && token_was_string)
        {
            // Robust: assume it's a title
            title = tok;
        }
        else
        {
            debug5 << "Tecplot: unknown token " << tok << "\n";
            // UNKNOWN RECORD TYPE
            char msg[200];
            sprintf(msg, "The record type '%s' found in the file was unknown.",
                    tok.c_str());
            EXCEPTION2(InvalidFilesException, filename, msg);
        }

        first_token = false;
        if (!got_next_token_already)
        {
            tok = GetNextToken();
        }
    }
    file.close();
    file.clear();
    file_read = true;

    //
    // Fix up spatial and topological dimension.
    // It may be that a file reported that it had 2-d data
    // but we never found the "X" or "Y" variables, so we
    // must treat it as a curve.  See simpscat.tec as an example.
    //
    if (topologicalDimension > spatialDimension)
    {
        topologicalDimension = spatialDimension;
    }
}

// ****************************************************************************
//  Method: avtTecplot constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2004
//
//  Modifications:
//    Brad Whitlock, Tue Jul 26 14:59:48 PST 2005
//    Initialized expressions.
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Initialized topo dim to zero to allow for point meshes 
//
//    Mark C. Miller, Tue Jan 12 17:36:41 PST 2010
//    Initialize solTime.
//
//    Jeremy Meredith, Tue Oct 25 12:37:42 EDT 2011
//    Allow user manual override of coordinate axis variables (via options).
//
// ****************************************************************************

avtTecplotFileFormat::avtTecplotFileFormat(const char *fname,
                                           DBOptionsAttributes *readOpts)
    : avtSTMDFileFormat(&fname, 1), expressions()
{
    file_read = false;
    filename = fname;
    saved_token = "";
    next_char_valid = false;
    next_char_eof = false;
    next_char_eol = false;
    token_was_string = false;
    Xindex = -1;
    Yindex = -1;
    Zindex = -1;
    topologicalDimension = 0;
    spatialDimension = 1;
    numTotalVars = 0;
    solTime = avtFileFormat::INVALID_TIME;

    userSpecifiedAxisVars = false;
    userSpecifiedX = -1;
    userSpecifiedY = -1;
    userSpecifiedZ = -1;
    if (readOpts &&
        readOpts->FindIndex("Method to determine coordinate axes")>=0)
    {
        int index = readOpts->GetEnum("Method to determine coordinate axes");
        if (index==0) userSpecifiedAxisVars = false;
        if (index==1) userSpecifiedAxisVars = true;
    }

    if (readOpts &&
        readOpts->FindIndex("X axis variable index (or -1 for none)")>=0)
    {
        userSpecifiedX = readOpts->GetInt("X axis variable index (or -1 for none)");
    }
    if (readOpts &&
        readOpts->FindIndex("Y axis variable index (or -1 for none)")>=0)
    {
        userSpecifiedY = readOpts->GetInt("Y axis variable index (or -1 for none)");
    }
    if (readOpts &&
        readOpts->FindIndex("Z axis variable index (or -1 for none)")>=0)
    {
        userSpecifiedZ = readOpts->GetInt("Z axis variable index (or -1 for none)");
    }
}


// ****************************************************************************
//  Method: avtTecplot destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 17, 2005
//
// ****************************************************************************

avtTecplotFileFormat::~avtTecplotFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtTecplotFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Nov 5 15:44:16 PST 2004
//
//  Modifications:
//    Hank Childs, Sat Mar  5 10:03:47 PST 2005
//    Do not blow away file name, because we will need that to re-read the file
//    if asked.
//
//    Brad Whitlock, Tue Jul 26 14:59:03 PST 2005
//    Clear the expression list.
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//
//    Mark C. Miller, Mon Aug 31 19:51:39 PDT 2009
//    Set topologicalDimension to 0 to make consistent with constructor.
// ****************************************************************************

void
avtTecplotFileFormat::FreeUpResources(void)
{
    file_read = false;
    saved_token = "";
    next_char_valid = false;
    next_char_eof = false;
    next_char_eol = false;
    token_was_string = false;
    Xindex = -1;
    Yindex = -1;
    Zindex = -1;
    topologicalDimension = 0;
    spatialDimension = 1;
    numTotalVars = 0;

    unsigned int i,j;
    for (i=0; i<meshes.size(); i++)
    {
        meshes[i]->Delete();
    }
    meshes.clear();
    for (i=0; i<variableNames.size(); i++)
    {
        for (j=0; j<vars[variableNames[i]].size(); j++)
        {
            vars[variableNames[i]][j]->Delete();
        }
        vars[variableNames[i]].clear();
    }
    vars.clear();
    variableNames.clear();
    variableCellCentered.clear();
    curveNames.clear();
    curveFirstVar.clear();
    curveSecondVar.clear();
    zoneTitles.clear();
    expressions.ClearExpressions();
}


// ****************************************************************************
//  Method: avtTecplotFileFormat::DetermineAVTMeshType
//
//  Purpose: Walk through the mesh(s) and decide what the overall mesh type
//    should be.
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Sep  1 10:44:33 PDT 2009
//
// ****************************************************************************
avtMeshType avtTecplotFileFormat::DetermineAVTMeshType() const
{
    if (topologicalDimension == 0)
        return AVT_POINT_MESH;

    bool allStructuredGrid = true;
    for (size_t i = 0; i < meshes.size() && allStructuredGrid; i++)
        if (meshes[i]->GetDataObjectType() != VTK_STRUCTURED_GRID)
            allStructuredGrid = false;

    if (allStructuredGrid)
        return AVT_CURVILINEAR_MESH;

    return AVT_UNSTRUCTURED_MESH;
}

// ****************************************************************************
//  Method: avtTecplotFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Nov 5 15:44:16 PST 2004
//
//  Modifications:
//    Brad Whitlock, Tue Jul 26 15:00:41 PST 2005
//    I made it add expressions if there are any.
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for point meshes where topo dim is zero but spatial dim>1
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//
//    Mark C. Miller, Tue Sep  1 10:53:21 PDT 2009
//    Made it call DetermineAVTMeshType
//
//    Brad Whitlock, Wed Sep  2 14:15:37 PDT 2009
//    Set node origin to 1.
//
//    Jeremy Meredith, Wed May 18 13:26:05 EDT 2011
//    If we have at least two spatial dims, always expose a point
//    mesh, even if we have curves or a spatial mesh.  There are too
//    many conventions for doing point meshes to deal with.  It's
//    simplest to just expose the darn point mesh, and any
//    node-centered variables, and let the user choose it if that's
//    what they wanted.  Also, get rid of the "vs X" enforced
//    convention for curves if the file has X coordinates; it never worked.
//
//    Jeremy Meredith, Thu May 19 10:46:22 EDT 2011
//    Make point mesh variables on a separate namespace from the normal mesh.
//
//    Mark C. Miller, Thu Jun  4 11:47:43 PDT 2015
//    Adjusted logic for detecting curves defined on FELINESEG elem types
// ****************************************************************************

void
avtTecplotFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!file_read)
        ReadFile();

    // we always want a point mesh, whether or not we think they have
    // some sort of real grid or just curves
    if (spatialDimension > 1)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "points/mesh";
        mesh->topologicalDimension = 0;
        mesh->spatialDimension = spatialDimension;
        mesh->meshType = AVT_POINT_MESH;
        mesh->numBlocks = (int)zoneTitles.size();
        mesh->blockOrigin = 1;
        mesh->cellOrigin = 1;
        mesh->nodeOrigin = 1;
        mesh->blockTitle = "Zones";
        mesh->blockPieceName = "Zone";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        for (unsigned int i=0; i<variableNames.size(); i++)
        {
            if (variableCellCentered[i] == false)
            {
                string vn = string("points/")+variableNames[i];
                AddScalarVarToMetaData(md, vn,
                                       "points/mesh", AVT_NODECENT);
            }
        }
    }

    // and now do either curves or a real grid, depending....
    if ((topologicalDimension==2 || topologicalDimension==3) ||
        (topologicalDimension<=1 && spatialDimension > 1))
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh";
        mesh->topologicalDimension = topologicalDimension;
        mesh->spatialDimension = spatialDimension;
        mesh->meshType = DetermineAVTMeshType();
        mesh->numBlocks = (int)meshes.size();
        mesh->blockOrigin = 1;
        mesh->cellOrigin = 1;
        mesh->nodeOrigin = 1;
        mesh->blockTitle = "Zones";
        mesh->blockPieceName = "Zone";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        for (unsigned int i=0; i<variableNames.size(); i++)
        {
            AddScalarVarToMetaData(md, variableNames[i],
                                   "mesh", (variableCellCentered[i] ?
                                            AVT_ZONECENT : AVT_NODECENT));
        }
    }
    else
    {
        //
        // We're pretty sneaky about 1D plots -- we
        // actual populate every n^2 (roughly) pairing
        // of 1D values in the metadata but only 
        // construct the meshes upon request.
        //
        char s[200];
        for (unsigned int z = 0 ; z < zoneTitles.size(); z++)
        {
            for (unsigned int i=0; i<variableNames.size(); i++)
            {
                for (unsigned int j=0; j<variableNames.size(); j++)
                {
                    if (i==j) 
                        continue;
                    if (zoneTitles.size() > 1)
                    {
                        sprintf(s, "%s/%s vs/%s",
                                zoneTitles[z].c_str(),
                                variableNames[i].c_str(),
                                variableNames[j].c_str());
                    }
                    else
                    {
                        sprintf(s, "%s vs/%s",
                                variableNames[i].c_str(),
                                variableNames[j].c_str());
                    }
                    curveIndices[s] = (int)curveNames.size();
                    curveNames.push_back(s);
                    curveDomains.push_back(z);
                    curveFirstVar.push_back(i);
                    curveSecondVar.push_back(j);
                    avtCurveMetaData *curve = new avtCurveMetaData;
                    curve->name = s;
                    md->Add(curve);
                }
            }
        }
    }

    // Add expressions to the metadata.
    for(int i = 0; i < expressions.GetNumExpressions(); ++i)
        md->AddExpression(new Expression(expressions[i]));
}


// ****************************************************************************
//  Method: avtTecplotFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
//  Modifications:
//
//    Hank Childs, Sat Mar  5 10:03:47 PST 2005
//    With dynamic load balancing, we may get here after calling FreeResources.
//    Make sure we read the file again if necessary.
//
//    Mark C. Miller, Thu Mar 29 11:28:34 PDT 2007
//    Added support for point meshes where topo dim is zero but spatial dim>1
//
//    Jeremy Meredith, Thu Jun 26 17:26:22 EDT 2008
//    Changed curves to be created as rectilinear grids instead of polydata
//    (so that they work with expressions).
//
//    Jeremy Meredith, Wed May 18 13:31:03 EDT 2011
//    We always expose a point mesh if they have 2 or more spatial dims.
//
//    Jeremy Meredith, Thu May 19 10:46:22 EDT 2011
//    Make point mesh variables on a separate namespace from the normal mesh.
//    Also, fixed a bug with point mesh Z coordinates.
//
//    Mark C. Miller, Thu Jun  4 11:47:43 PDT 2015
//    Adjusted logic for detecting curves defined on FELINESEG elem types
// ****************************************************************************

vtkDataSet *
avtTecplotFileFormat::GetMesh(int domain, const char *meshname)
{
    if (!file_read)
        ReadFile();

    // they might ask for points, no matter what was in the file
    if (string(meshname) == "points/mesh")
    {
        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();

        vtkFloatArray *x = vars[variableNames[Xindex]][domain];
        vtkFloatArray *y = vars[variableNames[Yindex]][domain];
        vtkFloatArray *z = (Zindex >= 0) ? vars[variableNames[Zindex]][domain] : NULL;

        int npts = x->GetNumberOfTuples();

        pts->SetNumberOfPoints(npts);
        pd->SetPoints(pts);
        pts->Delete();
        for (int j = 0 ; j < npts ; j++)
        {
            pts->SetPoint(j,
                          x->GetComponent(j,0),
                          y->GetComponent(j,0),
                          z ? z->GetComponent(j,0) : 0);
        }
 
        vtkCellArray *verts = vtkCellArray::New();
        pd->SetVerts(verts);
        verts->Delete();
        for (int k = 0 ; k < npts ; k++)
        {
            verts->InsertNextCell(1);
            verts->InsertCellPoint(k);
        }

        return pd;
    }

    // otherwise, what they get depends on the file contents
    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension <= 1 && spatialDimension > 1))
    {
        meshes[domain]->Register(NULL);
        return meshes[domain];
    }
    else
    {
        //
        // We're pretty sneaky about 1D plots -- we
        // actual populated every n^2 (roughly) pairing
        // of 1D values in the metadata but only 
        // construct the meshes here upon request.
        //

        if (curveIndices.count(meshname) <= 0)
            EXCEPTION1(InvalidVariableException, meshname);

        int curve = curveIndices[meshname];
        int index1 = curveFirstVar[curve];
        int index2 = curveSecondVar[curve];
        // override domain
        int curveDomain = curveDomains[curve];

        if (index2 < 0)
            EXCEPTION1(InvalidVariableException, meshname);

        vtkFloatArray *var1 = vars[variableNames[index1]][curveDomain];
        vtkFloatArray *var2 = vars[variableNames[index2]][curveDomain];
        int nPts = var1->GetNumberOfTuples();

        vtkFloatArray *vals = vtkFloatArray::New();
        vals->SetNumberOfComponents(1);
        vals->SetNumberOfTuples(nPts);
        vals->SetName(meshname);

        vtkRectilinearGrid *rg =
            vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
        rg->GetPointData()->SetScalars(vals);

        vtkDataArray *xc = rg->GetXCoordinates();
        for (int j = 0 ; j < nPts ; j++)
        {
            xc->SetComponent(j, 0, var2->GetValue(j));
            vals->SetValue(j, var1->GetValue(j));
        }
        vals->Delete();

        return rg;
    }
}


// ****************************************************************************
//  Method: avtTecplotFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
//  Modifications:
//
//    Hank Childs, Sat Mar  5 10:03:47 PST 2005
//    With dynamic load balancing, we may get here after calling FreeResources.
//    Make sure we read the file again if necessary.
//
//    Jeremy Meredith, Thu May 19 10:46:22 EDT 2011
//    Make point mesh variables on a separate namespace from the normal mesh.
//
// ****************************************************************************

vtkDataArray *
avtTecplotFileFormat::GetVar(int domain, const char *vn)
{
    if (!file_read)
        ReadFile();

    string varname(vn);
    if (varname.length() > 7 && varname.substr(0,7) == "points/")
    {
        varname = varname.substr(7);
    }

    vars[varname][domain]->Register(NULL);
    return vars[varname][domain];
}


// ****************************************************************************
//  Method: avtTecplotFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
// ****************************************************************************

vtkDataArray *
avtTecplotFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}
