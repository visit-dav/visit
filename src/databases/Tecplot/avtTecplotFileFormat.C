/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
            next_char == '='  ||
            next_char == '('  ||
            next_char == ')'  ||
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
            next_char == '='  ||
            next_char == '('  ||
            next_char == ')'  ||
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

    vector<vtkFloatArray*> allScalars;
    vector<float*> allptr;

    for (int v=0; v<numTotalVars; v++)
    {
        int numVals = (variableCellCentered[v] ? numElements : numNodes);

        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(numVals);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        vars[allVariableNames[v]].push_back(scalars);

        allScalars.push_back(scalars);
        allptr.push_back(ptr);
    }

    for (i=0; i<numNodes; i++)
    {
        bool doneWithCellCentered = (i>=numElements);
        for (int v = 0; v < numTotalVars; v++)
        {
            if (variableCellCentered[v] && doneWithCellCentered)
                continue;

            float val = atof(GetNextToken().c_str());
            if (v==Xindex)
            {
                pts[3*i + 0] = val;
            }
            else if (v==Yindex)
            {
                pts[3*i + 1] = val;
            }
            else if (v==Zindex)
            {
                pts[3*i + 2] = val;
            }

            allptr[v][i] = val;
        }
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

    for (int v = 0; v < numTotalVars; v++)
    {
        int numVals = (variableCellCentered[v] ? numElements : numNodes);

        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(numVals);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<numVals; i++)
            ptr[i] = atof(GetNextToken().c_str());
        vars[allVariableNames[v]].push_back(scalars);

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
// ****************************************************************************
vtkUnstructuredGrid *
avtTecplotFileFormat::ParseElements(int numElements, const string &elemType)
{
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

    // construct the cell arrays
    int nelempts = -1;
    int idtype = -1;
    if (elemType == "BRICK")
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
    else if (elemType == "POINT" || elemType == "")
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
    int *cl = cellLocations->GetPointer(0);

    int offset = 0;
    for (int c=0; c<numElements; c++)
    {
        *ct++ = idtype;

        *nl++ = nelempts;
        // 1-origin connectivity array
        for (int j=0; j<nelempts; j++)
            *nl++ = idtype == VTK_VERTEX ?  c : atoi(GetNextToken().c_str())-1;
        
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
// ****************************************************************************
void
avtTecplotFileFormat::ParseFEBLOCK(int numNodes, int numElements,
                                   const string &elemType)
{
    vtkPoints *points = ParseArraysBlock(numNodes,numElements);
    vtkUnstructuredGrid *ugrid = ParseElements(numElements, elemType);
    ugrid->SetPoints(points);
    points->Delete();

    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension == 0 && spatialDimension > 1))
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
// ****************************************************************************
void
avtTecplotFileFormat::ParseFEPOINT(int numNodes, int numElements,
                                   const string &elemType)
{
    vtkPoints *points = ParseArraysPoint(numNodes,numElements);
    vtkUnstructuredGrid *ugrid = ParseElements(numElements, elemType);
    ugrid->SetPoints(points);
    points->Delete();

    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension == 0 && spatialDimension > 1))
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
        (topologicalDimension == 0 && spatialDimension > 1))
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

    // If we have fewer topological dimenions than spatial dimensions,
    // then the only reasonable thing is zero topological dimension.
    if (topologicalDimensionOfZone < spatialDimension)
        topologicalDimensionOfZone = 0;

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
            (topologicalDimension == 0 && spatialDimension > 1))
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
// ****************************************************************************

void
avtTecplotFileFormat::ReadFile()
{
    file.open(filename.c_str());
    string tok = GetNextToken();
    int zoneIndex = 0;
    int numVars = 0;
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
            title = GetNextToken();
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
        }
        else if (tok == "VARIABLES")
        {
            int guessedXindex = -1;
            int guessedYindex = -1;
            int guessedZindex = -1;

            // variable lists
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
                allVariableNames.push_back(tok);
                numTotalVars++;
                numVars = variableNames.size();
                tok = GetNextToken();
            }
            if (numTotalVars==0)
            {
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
                    allVariableNames.push_back(tok);
                    numTotalVars++;
                    numVars = variableNames.size();
                    if (next_char_eol)
                    {
                        tok = GetNextToken();
                        break;
                    }
                    else
                        tok = GetNextToken();
                }
            }

            // Default the centering to nodal
            variableCellCentered.clear();
            variableCellCentered.resize(numTotalVars, 0);

            // If we didn't find an exact match for coordinate axis vars, guess
            if (Xindex < 0) Xindex = guessedXindex;
            if (Yindex < 0) Yindex = guessedYindex;
            if (Zindex < 0) Zindex = guessedZindex;

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

            got_next_token_already = true;
        }
        else if (tok == "ZONE")
        {
            // Parse a zone!
            char untitledZoneTitle[40];
            sprintf(untitledZoneTitle, "zone%05d", zoneIndex);

            string zoneTitle = untitledZoneTitle;
            string elemType = "";
            string format = "";
            int numNodes = 0;
            int numElements = 0;
            int numI = 1, numJ = 1, numK = 1;

            tok = GetNextToken();
            while (!(tok != "T"  &&
                     tok != "I"  &&
                     tok != "J"  &&
                     tok != "K"  &&
                     tok != "N"  &&
                     tok != "E"  &&
                     tok != "ET" &&
                     tok != "F"  &&
                     tok != "ZONETYPE"  &&
                     tok != "DATAPACKING"  &&
                     tok != "VARLOCATION"  &&
                     tok != "DT" &&
                     tok != "D"))
            {
                if (tok == "T")
                {
                    zoneTitle = GetNextToken();
                    if (!token_was_string)
                    {
                        EXCEPTION2(InvalidFilesException, filename,
                                   "Zone titles MUST be quoted.");
                    }
                }
                else if (tok == "I")
                {
                    numI = atoi(GetNextToken().c_str());
                }
                else if (tok == "J")
                {
                    numJ = atoi(GetNextToken().c_str());
                }
                else if (tok == "K")
                {
                    numK = atoi(GetNextToken().c_str());
                }
                else if (tok == "N")
                {
                    numNodes = atoi(GetNextToken().c_str());
                }
                else if (tok == "E")
                {
                    numElements = atoi(GetNextToken().c_str());
                }
                else if (tok == "ET" || tok == "ZONETYPE")
                {
                    elemType = GetNextToken();
                }
                else if (tok == "F" || tok == "DATAPACKING")
                {
                    format = GetNextToken();
                }
                else if (tok == "VARLOCATION")
                {
                    string centering;
                    variableCellCentered.clear();
                    variableCellCentered.resize(numTotalVars, 0);
                    for (int i=0; i<numTotalVars; i++)
                    {
                        centering = GetNextToken();
                        if (centering == "CELLCENTERED")
                            variableCellCentered[i] = 1;
                    }
                }
                else if (tok == "DT")
                {
                    for (int i=0; i<numTotalVars; i++)
                        GetNextToken();
                }
                else if (tok == "D")
                {
                    EXCEPTION2(InvalidFilesException, filename,
                               "Tecplot Zone record parameter 'D' is "
                               "currently unsupported.  Please contact a "
                               "VisIt developer if you need support for this "
                               "parameter.");
                }
                tok = GetNextToken();
            }
            PushBackToken(tok);

            // New flavors of the tecplot format let you specify
            // simply "point" or "block" for the format, and by
            // adding a zonetype which starts with FE
            // (e.g. "FETETRAHEDRON"), switch to an FE
            // parsing mode.   Ugh.
            if (elemType.length() > 2 && elemType.substr(0,2) == "FE")
            {
                if (format == "POINT")
                    format = "FEPOINT";
                if (format == "BLOCK")
                    format = "FEBLOCK";
            }

            zoneTitles.push_back(zoneTitle);
            if (format=="FEBLOCK")
            {
                ParseFEBLOCK(numNodes, numElements, elemType);
            }
            else if (format=="FEPOINT")
            {
                ParseFEPOINT(numNodes, numElements, elemType);
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
// ****************************************************************************

avtTecplotFileFormat::avtTecplotFileFormat(const char *fname)
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
    allVariableNames.clear();
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
// ****************************************************************************

void
avtTecplotFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!file_read)
        ReadFile();

    if ((topologicalDimension==2 || topologicalDimension==3) ||
        (topologicalDimension==0 && spatialDimension > 1))
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh";
        mesh->topologicalDimension = topologicalDimension;
        mesh->spatialDimension = spatialDimension;
        mesh->meshType = DetermineAVTMeshType();
        mesh->numBlocks = meshes.size();
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
            if (Xindex < 0)
            {
                for (unsigned int i=0; i<allVariableNames.size(); i++)
                {
                    for (unsigned int j=0; j<allVariableNames.size(); j++)
                    {
                        if (i==j) 
                            continue;
                        if (zoneTitles.size() > 1)
                        {
                            sprintf(s, "%s/%s vs/%s",
                                    zoneTitles[z].c_str(),
                                    allVariableNames[i].c_str(),
                                    allVariableNames[j].c_str());
                        }
                        else
                        {
                            sprintf(s, "%s vs/%s",
                                    allVariableNames[i].c_str(),
                                    allVariableNames[j].c_str());
                        }
                        curveIndices[s] = curveNames.size();
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
            else
            {
                for (int i=0; i<(int)allVariableNames.size(); i++)
                {
                    if (i==Xindex)
                        continue;

                    if (zoneTitles.size() > 1)
                    {
                        sprintf(s, "%s/%s vs/X",
                                zoneTitles[z].c_str(),
                                allVariableNames[i].c_str());
                    }
                    else
                    {
                        sprintf(s, "%s vs/X", allVariableNames[i].c_str());
                    }
                    curveIndices[s] = curveNames.size();
                    curveNames.push_back(s);
                    curveDomains.push_back(z);
                    curveFirstVar.push_back(i);
                    curveSecondVar.push_back(-1);
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
// ****************************************************************************

vtkDataSet *
avtTecplotFileFormat::GetMesh(int domain, const char *meshname)
{
    if (!file_read)
        ReadFile();

    if ((topologicalDimension == 2 || topologicalDimension == 3) ||
        (topologicalDimension == 0 && spatialDimension > 1))
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

        vtkFloatArray *var1 = vars[allVariableNames[index1]][curveDomain];
        vtkFloatArray *var2 = vars[allVariableNames[index2]][curveDomain];
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
// ****************************************************************************

vtkDataArray *
avtTecplotFileFormat::GetVar(int domain, const char *varname)
{
    if (!file_read)
        ReadFile();

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
