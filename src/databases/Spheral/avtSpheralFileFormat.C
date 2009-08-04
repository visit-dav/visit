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
//                            avtSpheralFileFormat.C                         //
// ************************************************************************* //

#include <avtSpheralFileFormat.h>

#include <vtkAppendPolyData.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <BadDomainException.h>
#include <DebugStream.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

using std::vector;

// ****************************************************************************
//  Method: avtSpheralFileFormat constructor
//
//  Arguments:
//      fname    The name of the Spheral++ root file.
//
//  Programmer:  Hank Childs
//  Creation:    March 11, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar 17 13:23:27 PST 2003
//    Support for reading cycles/times.
//
//    Hank Childs, Mon Apr  7 18:16:07 PDT 2003
//    Do not read in the meta-data during the constructor.
//
//    Hank Childs, Wed Jun 23 09:10:11 PDT 2004
//    Declare the number of domains as 0, so it will definitely match up with
//    the size of the read_domain vector.
//
// ****************************************************************************

avtSpheralFileFormat::avtSpheralFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    rootfile = fname;
    current_file = rootfile;
    ndomains = 0;
    cycle = 0;
    gotCycle = false;
    dtime = 0.;
    gotTime = false;
    readInMetaData = false;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ReadInMetaData
//
//  Purpose:
//      Reads in the meta-data.
//
//  Programmer: Hank Childs
//  Creation:   April 7, 2003
//
// ****************************************************************************

void
avtSpheralFileFormat::ReadInMetaData(void)
{
    ifstream ifile(rootfile.c_str());

    if (ifile.fail())
    {
        debug1 << "Unable to open file " << rootfile.c_str() << endl;
        EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }
    else
    {
        ParseHeader(ifile);
    }

    readInMetaData = true;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2003
//
// ****************************************************************************

avtSpheralFileFormat::~avtSpheralFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::FreeUpResources
//
//  Purpose:
//      Cleans up the memory allocated for this object.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Jun 23 09:34:43 PDT 2004
//    Avoid dangling pointers.
//
// ****************************************************************************

void
avtSpheralFileFormat::FreeUpResources(void)
{
    int   i, j, k;
    for (i = 0 ; i < ndomains ; i++)
    {
        for (j = 0 ; j < nodeLists.size() ; j++)
        {
            if (cache[i].meshes[j] != NULL)
            {
                cache[i].meshes[j]->Delete();
                cache[i].meshes[j] = NULL;
            }

            for (k = 0 ; k < fields.size() ; k++)
            {
                if (cache[i].fields[j][k] != NULL)
                {
                    cache[i].fields[j][k]->Delete();
                    cache[i].fields[j][k] = NULL;
                }
            }
        }
        read_domain[i] = false;
    }
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::RegisterVariableList
//
//  Purpose:
//      Not all of the variables are defined on all of the node lists.  This 
//      routine determines which node lists are valid for a given set of
//      variables.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Jun 23 07:32:20 PDT 2004
//    Make sure we have read in the meta-data, so we can get field indices
//    properly ['5109].
//
// ****************************************************************************

void
avtSpheralFileFormat::RegisterVariableList(const char *primVar, 
                                           const vector<CharStrRef> &vars2nd)
{
    int   i, j;

    if (!readInMetaData)
    {
        ReadInMetaData();
    }

    //
    // We want every node list (-> mat) unless we have reason not to include
    // it.
    //
    int   nMats = nodeLists.size();
    validNodeLists.resize(nMats);
    for (i = 0 ; i < nMats ; i++)
    {
        validNodeLists[i] = true;
    }

    //
    // Start off by looking at the primary variable.
    //
    int fieldIndex = GetFieldIndexFromName(primVar);
    if (fieldIndex >= 0) // Subsets are < 0
    {
        for (i = 0 ; i < nMats ; i++)
        {
            if (! fieldDefinedOnNodeList[i][fieldIndex])
            {
                validNodeLists[i] = false;
            }
        }
    }

    //
    // Now look at the secondary variables.
    //
    int nVars2nd = vars2nd.size();
    for (j = 0 ; j < nVars2nd ; j++)
    {
        const char *varName = *(vars2nd[j]);
        int fieldIndex = GetFieldIndexFromName(varName);
        if (fieldIndex >= 0) // Subsets are < 0
        {
            for (i = 0 ; i < nMats ; i++)
            {
                if (! fieldDefinedOnNodeList[i][fieldIndex])
                {
                    validNodeLists[i] = false;
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ParseHeader
//
//  Purpose:
//      Parses in the header portion of a Spheral++ file.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar 17 13:23:27 PST 2003
//    Support for reading cycles/times.
//
//    Hank Childs, Thu Jul  3 16:32:17 PDT 2003
//    Stupid bug that screwed up 'single file' databases.
//
// ****************************************************************************

void
avtSpheralFileFormat::ParseHeader(istream &ifile)
{
    char line[1024];
    vector<int> word_starts;
    bool hitEndOfHeader = false;
    bool needPositionField = true;
    int nodeListIndex = -1;
    while (!hitEndOfHeader)
    {
        int nwords = GetLine(ifile, line, word_starts);

        if (strncmp(line, "!Header", strlen("!Header")) == 0)
        {
            continue;
        }
        else if (strncmp(line, "!EndHeader", strlen("!EndHeader")) == 0)
        {
            hitEndOfHeader = true;
            continue;
        }
        else if (strncmp(line, "!Cycle", strlen("!Cycle")) == 0)
        {
            cycle = atoi(line + word_starts[1]);
            gotCycle = true;
            continue;
        }
        else if (strncmp(line, "!Time", strlen("!Time")) == 0)
        {
            dtime = atof(line + word_starts[1]);
            gotTime = true;
            continue;
        }
        else if (strncmp(line, "!NodeList", strlen("!NodeList")) == 0)
        {
            ParseNodeList(line, nwords, word_starts);
            needPositionField = true;
            nodeListIndex++;
        }
        else if (strncmp(line, "!Field", strlen("!Field")) == 0)
        {
            ParseField(line, nwords, word_starts, needPositionField,
                       nodeListIndex);
            needPositionField = false;
        }
    }

    //
    // The next non-comment line should either be ASCIIData or FileList.
    //
    int nwords = GetLine(ifile, line, word_starts);
    if (strstr(line, "!ASCIIData") != 0)
    {
        // This is a normal, single domain file.  The only purpose to
        // 'keepGoing' was to see if we had multiple domains.
        ndomains = 1;
        read_domain.push_back(false);
        domain_files.push_back(rootfile);
    }
    else if (strstr(line, "!FileList") != 0)
    {
        if (nwords <= 1)
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        ndomains = atoi(line + word_starts[1]);
        if (ndomains == 0 && line[word_starts[1]] != '0')
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        DetermineSubFiles(ifile, ndomains);
    }
    else
    {
        EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }

    //
    // Since the data is non-random access, we plan to read in everything
    // requested.  Create a cache to hold this.
    //
    cache.resize(ndomains);
    for (int i = 0 ; i < ndomains ; i++)
    {
        cache[i].meshes.resize(nodeLists.size());
        cache[i].fields.resize(nodeLists.size());
        for (int j = 0 ; j < nodeLists.size() ; j++)
        {
            cache[i].fields[j].resize(fields.size());
        }
    }
}


// ****************************************************************************
//  Function: ConvertStringToVarType
//
//  Purpose:
//      Converts a text string to the avtVarType enumerated type.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2003
//
// ****************************************************************************

avtVarType
ConvertStringToVarType(const char *varType)
{
    if (strcmp(varType, "Scalar") == 0)
    {
        return AVT_SCALAR_VAR;
    }
    else if (strcmp(varType, "Vector") == 0)
    {
        return AVT_VECTOR_VAR;
    }
    else if (strcmp(varType, "Tensor") == 0)
    {
        return AVT_TENSOR_VAR;
    }
    else if (strcmp(varType, "SymTensor") == 0)
    {
        return AVT_SYMMETRIC_TENSOR_VAR;
    }

    return AVT_UNKNOWN_TYPE;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ParseField
//
//  Purpose:
//      Parses in a field.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2003
//
// ****************************************************************************

void
avtSpheralFileFormat::ParseField(char *line, int nwords,
                                 vector<int> &word_starts,
                                 bool needPositionField, int nodeListIndex)
{
    //
    // Make sure that this can even be a validly parsible 'Field'.
    //
    if (nwords < 3)
    {
        debug1 << "All fields must be of the form !Field name type "
               << "[dim1] [dim2]." << endl;
        EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }

    char *fieldName = line + word_starts[1];

    //
    // Determine what type of variable it is -- scalar, vector, etc.
    //
    char *varType = line + word_starts[2];
    avtVarType vt = ConvertStringToVarType(varType);
    if (vt == AVT_UNKNOWN_TYPE)
    {
        debug1 << "Unable to determine variable type of " << varType << endl;
        EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }

    //
    // The number of words is based on the variable type.  Check to make sure
    // that we have enough words a'priori.
    //
    int dims_listed = 0;
    switch (vt)
    {
       case AVT_SCALAR_VAR:
         dims_listed = 0;
         break;

       case AVT_VECTOR_VAR:
         dims_listed = 1;
         break;

       case AVT_TENSOR_VAR:
       case AVT_SYMMETRIC_TENSOR_VAR:
         dims_listed = 2;
         break;

       default:
         EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }
    if ((3+dims_listed) > nwords)
    {
        debug1 << "There are not enough words to make a valid field." << endl;
        EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }

    if (needPositionField)
    {
        //
        // Sanity check.  The only possible position fields are vectors.
        //
        if (needPositionField && vt != AVT_VECTOR_VAR)
        {
            debug1 << "The first field following the node list is the "
                   << "position variable.  It must be a vector." << endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }

        //
        // Get the vector dimension.
        //
        int dim = atoi(line + word_starts[3]);
        if (dim == 0 && line[word_starts[3]] != '0')
        {
            debug1 << "Internal error parsing dim of field " 
                   << fieldName << endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }

        //
        // Now register this vector as the position field.
        //
        positionField[nodeListIndex] = fieldName;
        positionDimension[nodeListIndex] = dim;
    }
    else
    {
        int fieldIndex = -1;
        for (int i = 0 ; i < fields.size() ; i++)
        {
            if (fields[i] == fieldName)
            {
                fieldIndex = i;
                break;
            }
        }
        if (fieldIndex < 0)
        {
            fieldIndex = fields.size();
            fields.push_back(fieldName);
            fieldType.push_back(AVT_UNKNOWN_TYPE);
            fieldDim1.push_back(-1);
            fieldDim2.push_back(-1);
            for (int i = 0 ; i < fieldDefinedOnNodeList.size() ; i++)
            {
                 fieldDefinedOnNodeList[i].push_back(false);
            }
        }

        fieldDefinedOnNodeList[nodeListIndex][fieldIndex] = true;
 
        int dim1 = 0, dim2 = 0;
        if (dims_listed >= 1)
        {
            dim1 = atoi(line + word_starts[3]);
            if (dim1 == 0 && line[word_starts[3]] != '0')
            {
                debug1 << "Internal error parsing dim of field " 
                       << fieldName << endl;
                EXCEPTION1(InvalidFilesException, rootfile.c_str());
            }
        }
        if (dims_listed >= 2)
        {
            dim2 = atoi(line + word_starts[4]);
            if (dim2 == 0 && line[word_starts[4]] != '0')
            {
                debug1 << "Internal error parsing 2nd dim of field " 
                       << fieldName << endl;
                EXCEPTION1(InvalidFilesException, rootfile.c_str());
            }
        }

        //
        // Check for special restrictions on the dimensions...
        //
        if (vt == AVT_SCALAR_VAR)
        {
             dim1 = 1;
        }
        else if (vt == AVT_VECTOR_VAR && dim1 < 2)
        {
            debug1 << "Vector vars must have dimension greater than "
                   << dim1 << endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }

        //
        // Register all of the information about this field.
        //
        if (fieldType[fieldIndex] != vt &&
            fieldType[fieldIndex] != AVT_UNKNOWN_TYPE)
        {
            debug1 << "A field has been declared inconsistently." << endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }
        fieldType[fieldIndex] = vt;

        if (fieldDim1[fieldIndex] != dim1 && fieldDim1[fieldIndex] != -1)
        {
            debug1 << "A dimension has been declared inconsisently." << endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }
        fieldDim1[fieldIndex] = dim1;

        if (fieldDim2[fieldIndex] != dim2 && fieldDim2[fieldIndex] != -1)
        {
            debug1 << "A dimension has been declared inconsisently." << endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }
        fieldDim2[fieldIndex] = dim2;
    }
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ParseNodeList
//
//  Purpose:
//      Parses in the node list.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2003
//
// ****************************************************************************

void
avtSpheralFileFormat::ParseNodeList(char *line, int nwords,
                                    vector<int> &word_starts)
{
    //
    // Make sure that this can be a valid 'NodeList'.
    //
    if (nwords <= 1)
    {
        debug1 << "All node lists must be of the form !NodeList name "
               << "[#]." << endl;
        EXCEPTION1(InvalidFilesException, rootfile.c_str());
    }

    nodeLists.push_back(line + word_starts[1]);
           
    //
    // The next field that we encounter will be the 'position' field.
    // Allocate space for that now.
    //
    positionField.push_back("<unknown>");
    positionDimension.push_back(0);
    std::vector<bool> fieldsDefinedOnThisNodeList(fields.size(), false);
    fieldDefinedOnNodeList.push_back(fieldsDefinedOnThisNodeList);

    //
    // The number of nodes is optional.  If we have it, then parse it.
    // Otherwise, use '-1' to indicate that we don't know.
    int nnodes = -1;
    if (nwords >= 3)
    {
        nnodes = atoi(line + word_starts[2]);
        if (nnodes == 0 && line[word_starts[2]] != '0')
        {
            debug1 << "Internal error parsing number of nodes." <<endl;
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }
    }
    nodeListSizes.push_back(nnodes);
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::DetermineSubFiles
//
//  Purpose:
//      Determines the sub-files underneath the root file.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2003
//
// ****************************************************************************

void
avtSpheralFileFormat::DetermineSubFiles(istream &ifile, int ndomains)
{
    char prefix[1024];
    strcpy(prefix, rootfile.c_str());
    int len = strlen(prefix);
    bool found_prefix = false;
    char dir_char = '\0';
    for (int j = len-1 ; j >= 0 ; j--)
    {
        if (prefix[j] == '/' || prefix[j] == '\\')
        {
            dir_char = prefix[j];
            found_prefix = true;
            prefix[j] = '\0';
            break;
        }
    }

    char line[1024];
    vector<int> word_starts;
    for (int i = 0 ; i < ndomains ; i++)
    {
        int nwords = GetLine(ifile, line, word_starts);
        if (nwords != 1)
        {
            EXCEPTION1(InvalidFilesException, rootfile.c_str());
        }
        char full_filename[1024];
        if (found_prefix)
            sprintf(full_filename, "%s%c%s", prefix, dir_char,
                                             line + word_starts[0]);
        else
            strcpy(full_filename, line + word_starts[0]);
        domain_files.push_back(full_filename);
        read_domain.push_back(false);
    }
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetMesh
//
//  Purpose:
//      Returns the line associated with a domain number.
//
//  Arguments:
//      dom     The domain number.
//      name    The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:16:07 PDT 2003
//    Make sure we have read in the meta-data, since that is no longer done
//    in the constructor.
//
// ****************************************************************************

vtkDataSet *
avtSpheralFileFormat::GetMesh(int dom, const char *name)
{
    if (!readInMetaData)
    {
        ReadInMetaData();
    }

    if (!read_domain[dom])
    {
        ReadDomain(dom);
    }

    vtkDataSet *rv = NULL;

    if (doMaterialSelection)
    {
        int nodeListIndex = GetNodeListIndexFromName(materialName);
        if (validNodeLists[nodeListIndex] &&
            cache[dom].meshes[nodeListIndex] != NULL)
        {
            rv = cache[dom].meshes[nodeListIndex];
            rv->Register(NULL);
        }
    }
    else
    {
        //
        // We have multiple node lists in play here.  Append all that apply.
        //
        vtkAppendPolyData *appender = vtkAppendPolyData::New();
        vtkPolyData *one_dataset = NULL;
        int nInputs = 0;
        for (int i = 0 ; i < nodeLists.size() ; i++)
        {
            if (validNodeLists[i] && cache[dom].meshes[i] != NULL)
            {
                one_dataset = cache[dom].meshes[i];
                appender->AddInput(one_dataset);
                nInputs++;
            }
        }

        if (nInputs > 0)
        {
            if (nInputs == 1)
            {
                rv = one_dataset;
            }
            else
            {
                rv = appender->GetOutput();
                rv->Update();
            }
            rv->Register(NULL);
        }

        appender->Delete();
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      field     The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     March 11, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:16:07 PDT 2003
//    Make sure we have read in the meta-data, since that is no longer done
//    in the constructor.
//
// ****************************************************************************

vtkDataArray *
avtSpheralFileFormat::GetVar(int dom, const char *field)
{
    if (!readInMetaData)
    {
        ReadInMetaData();
    }

    if (!read_domain[dom])
    {
        ReadDomain(dom);
    }

    vtkDataArray *rv = NULL;
    int fieldIndex = GetFieldIndexFromName(field);

    if (doMaterialSelection)
    {
        int nodeListIndex = GetNodeListIndexFromName(materialName);
        if (validNodeLists[nodeListIndex] &&
            cache[dom].fields[nodeListIndex][fieldIndex] != NULL)
        {
            rv = cache[dom].fields[nodeListIndex][fieldIndex];
            rv->Register(NULL);
        }
    }
    else
    {
        //
        // We have multiple node lists in play here.  Append all that apply.
        //
        vtkDataArray *one_array = NULL;
        int nInputs = 0;
        int nTuples = 0;
        for (int i = 0 ; i < nodeLists.size() ; i++)
        {
            if (validNodeLists[i] && cache[dom].fields[i][fieldIndex] != NULL)
            {
                one_array = cache[dom].fields[i][fieldIndex];
                nTuples += one_array->GetNumberOfTuples();
                nInputs++;
            }
        }

        if (nInputs > 0)
        {
            if (nInputs == 1)
            {
                rv = one_array;
                rv->Register(NULL);
            }
            else
            {
                rv = vtkFloatArray::New();
                rv->SetNumberOfComponents(
                                       one_array->GetNumberOfComponents());
                rv->SetNumberOfTuples(nTuples);
                int currentTuple = 0;
                for (int i = 0 ; i < nodeLists.size() ; i++)
                {
                    if (validNodeLists[i] && 
                        cache[dom].fields[i][fieldIndex] != NULL)
                    {
                        vtkDataArray *from = cache[dom].fields[i][fieldIndex];
                        int nT = from->GetNumberOfTuples();
                        double tuple[24];
                        for (int j = 0 ; j < nT ; j++)
                        {
                            from->GetTuple(j, tuple);
                            rv->SetTuple(currentTuple++, tuple);
                        }
                    }
                }
            }
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetNodeListIndexFromName
//
//  Purpose:
//      Turns a name of a node list into an index.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

int
avtSpheralFileFormat::GetNodeListIndexFromName(const char *name)
{
    for (int i = 0 ; i < nodeLists.size() ; i++)
    {
        if (strcmp(nodeLists[i].c_str(), name) == 0)
        {
            return i;
        }
    }

    debug1 << "Unable to identify node list " << name << endl;
    EXCEPTION1(InvalidFilesException, current_file.c_str());
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetFieldIndexFromName
//
//  Purpose:
//      Turns a name of a field into an index.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Mar 28 08:30:00 PST 2003
//    Add support for the mesh name as a field (this happens when you do a
//    mesh plot).
//
//    Hank Childs, Tue May 23 11:42:55 PDT 2006
//    Accept "Materials" as well as "Materials(Node List)".
//
// ****************************************************************************

int
avtSpheralFileFormat::GetFieldIndexFromName(const char *name)
{
    for (int i = 0 ; i < fields.size() ; i++)
    {
        if (strcmp(fields[i].c_str(), name) == 0)
        {
            return i;
        }
    }

    if (strcmp(name, "Materials(Node List)") == 0)
        return -1;
    if (strcmp(name, "Materials") == 0)
        return -1;
    if (strcmp(name, "Processor Decomposition(Node List)") == 0)
        return -1;
    if (strcmp(name, "Node List") == 0)
        return -2;

    debug1 << "Unable to identify field " << name << endl;
    EXCEPTION1(InvalidFilesException, current_file.c_str());
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetVectorVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     March 13, 2003
//
// ****************************************************************************

vtkDataArray *
avtSpheralFileFormat::GetVectorVar(int dom, const char *name)
{
    //
    // GetVectorVar and GetVar should really be combined into one function.
    //
    return GetVar(dom, name);
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this Spheral++ file.  
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar 17 13:23:27 PST 2003
//    Support for setting cycles/times.
//
//    Hank Childs, Mon Apr  7 18:16:07 PDT 2003
//    Make sure we have read in the meta-data, since that is no longer done
//    in the constructor.
//
//    Hank Childs, Mon Sep 22 14:05:50 PDT 2003
//    Add support for tensors.
//
// ****************************************************************************

void
avtSpheralFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readInMetaData)
    {
        ReadInMetaData();
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Node List";
    mesh->meshType = AVT_POINT_MESH;
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = positionDimension[0];
    mesh->topologicalDimension = 0;
    mesh->blockTitle = "Processor Decomposition";
    mesh->blockPieceName = "Processor";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    avtMaterialMetaData *mmd = new avtMaterialMetaData("Materials",
                                    "Node List", nodeLists.size(), nodeLists);
    md->Add(mmd);

    for (int i = 0 ; i < fields.size() ; i++)
    {
        if (fieldType[i] == AVT_SCALAR_VAR)
        {
            AddScalarVarToMetaData(md, fields[i], "Node List", AVT_NODECENT,
                                   NULL);
        }
        else if (fieldType[i] == AVT_VECTOR_VAR)
        {
            AddVectorVarToMetaData(md, fields[i], "Node List", AVT_NODECENT,
                                   fieldDim1[i], NULL);
        }
        else if (fieldType[i] == AVT_TENSOR_VAR)
        {
            AddTensorVarToMetaData(md, fields[i], "Node List", AVT_NODECENT,
                                   fieldDim1[i]);
        }
        else if (fieldType[i] == AVT_SYMMETRIC_TENSOR_VAR)
        {
            AddSymmetricTensorVarToMetaData(md, fields[i], "Node List",
                                            AVT_NODECENT, fieldDim1[i]);
        }
    }

    if (gotCycle)
        metadata->SetCycle(timestep, cycle);
    if (gotTime)
        metadata->SetTime(timestep, dtime);
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ReadDomain
//
//  Purpose:
//      Reads in a domain of a Spheral++ file.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar 17 13:09:24 PST 2003
//    Make sure we mark a domain as 'read' after we read it.
//
//    Kathleen Bonnell, Tue Jul 19 17:45:54 PDT 2005
//    Make sure we don't pass along 'empty' datasets (no points).
//
// ****************************************************************************

void
avtSpheralFileFormat::ReadDomain(int dom)
{
    if (dom >= ndomains || dom < 0)
    {
        EXCEPTION2(BadDomainException, dom, ndomains);
    }

    ifstream ifile(domain_files[dom].c_str());
    current_file = domain_files[dom];
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    // We only care about the body of the file, not the header.  Just
    // "fast forward" through the header.
    bool keepGoing = true;
    char line[1024];
    vector<int> offsets;
    int nwords;
    while (keepGoing)
    {
        nwords = GetLine(ifile, line, offsets);
        if (strcmp(line + offsets[0], "!EndHeader") == 0)
        {
            keepGoing = false;
        }
    }

    nwords = GetLine(ifile, line, offsets);
    if (strcmp(line + offsets[0], "!ASCIIData") != 0)
    {
        debug1 << "Can only read ASCII data." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    for (int i = 0 ; i < nodeLists.size() ; i++)
    {
        vtkPolyData *pdata = ReadNodeList(ifile, i);
        nodeListSizes[i] = pdata->GetNumberOfPoints();
        if (nodeListSizes[i] != 0)
            cache[dom].meshes[i] = pdata;
        else 
            cache[dom].meshes[i] = NULL; 
     

        int nFieldsToRead = 0;
        int j;
        for (j = 0 ; j < fields.size() ; j++)
            if (fieldDefinedOnNodeList[i][j])
                nFieldsToRead++;

        for (j = 0 ; j < nFieldsToRead ; j++)
        {
            int index = -1;
            vtkDataArray *da = ReadField(ifile, i, index);
            cache[dom].fields[i][index] = da;
        }
    }

    read_domain[dom] = true;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ReadField
//
//  Purpose:
//      Reads in one field for one domain.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar 17 13:09:24 PST 2003
//    2D vectors in VTK must be represented as 3D.
//
//    Hank Childs, Mon Sep 22 14:07:02 PDT 2003
//    Add support for tensors.
//
// ****************************************************************************

vtkDataArray *
avtSpheralFileFormat::ReadField(istream &ifile, int nodeListIndex, 
                                int &fieldIndex)
{
    //
    // Read in the line describing the field.
    //
    vector<int> offsets;
    char line[1024];
    int nwords = GetLine(ifile, line, offsets);
    if (nwords < 3)
    {
        debug1 << "Line cannot contain a valid Field declaration." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }
    
    if (strcmp(line + offsets[0], "!Field") != 0)
    {
        debug1 << "Field not in the proper place." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }
    const char *field = line + offsets[1];
    fieldIndex = GetFieldIndexFromName(field);

    int nNodes = nodeListSizes[nodeListIndex];

    vtkDataArray *rv = NULL;

    if (fieldType[fieldIndex] == AVT_SCALAR_VAR)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(nNodes);
        for (int i = 0 ; i < nNodes ; i++)
        {
            nwords = GetLine(ifile, line, offsets);
            if (nwords != 1)
            {
                debug1 << "Scalars have only one component.  Likely parsing "
                       << "error." << endl;
                EXCEPTION1(InvalidFilesException, current_file.c_str());
            }
            float val = atof(line + offsets[0]);
            arr->SetTuple1(i, val);
        }
        rv = arr;
    }
    else if (fieldType[fieldIndex] == AVT_VECTOR_VAR)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        int actual_dim  = fieldDim1[fieldIndex];
        int working_dim = (actual_dim == 2 ? 3 : actual_dim);
        float *vals = new float[working_dim];
        arr->SetNumberOfComponents(working_dim);
        arr->SetNumberOfTuples(nNodes);
        for (int i = 0 ; i < nNodes ; i++)
        {
            nwords = GetLine(ifile, line, offsets);
            if (nwords != actual_dim)
            {
                debug1 << "Incorrect number of components for vector." << endl;
                EXCEPTION1(InvalidFilesException, current_file.c_str());
            }
            int j;
            for (j = 0 ; j < actual_dim ; j++)
                vals[j] = atof(line + offsets[j]);
            for (j = actual_dim ; j < working_dim ; j++)
                vals[j] = 0.;
            arr->SetTuple(i, vals);
        }
        delete [] vals;
        rv = arr;
    }
    else if (fieldType[fieldIndex] == AVT_SYMMETRIC_TENSOR_VAR)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        int dim  = fieldDim1[fieldIndex];
        float vals[9];
        arr->SetNumberOfComponents(9);
        arr->SetNumberOfTuples(nNodes);
        if (dim == 2)
        {
            for (int i = 0 ; i < nNodes ; i++)
            {
                nwords = GetLine(ifile, line, offsets);
                if (nwords != 4)
                {
                    debug1 << "Incorrect number of components for symm-tensor."
                           << endl;
                    EXCEPTION1(InvalidFilesException, current_file.c_str());
                }
                vals[0] = atof(line + offsets[0]);
                vals[1] = atof(line + offsets[1]);
                vals[2] = 0.;
                vals[3] = atof(line + offsets[2]);
                vals[4] = atof(line + offsets[3]);
                vals[5] = 0.;
                vals[6] = 0.;
                vals[7] = 0.;
                vals[8] = 0.;
                arr->SetTuple(i, vals);
            }
        }
        else
        {
            for (int i = 0 ; i < nNodes ; i++)
            {
                nwords = GetLine(ifile, line, offsets);
                if (nwords != 9)
                {
                    debug1 << "Incorrect number of components for symm-tensor."
                           << endl;
                    EXCEPTION1(InvalidFilesException, current_file.c_str());
                }
                vals[0] = atof(line + offsets[0]);
                vals[1] = atof(line + offsets[1]);
                vals[2] = atof(line + offsets[2]);
                vals[3] = atof(line + offsets[3]);
                vals[4] = atof(line + offsets[4]);
                vals[5] = atof(line + offsets[5]);
                vals[6] = atof(line + offsets[6]);
                vals[7] = atof(line + offsets[7]);
                vals[8] = atof(line + offsets[8]);
                arr->SetTuple(i, vals);
            }
        }
        rv = arr;
    }
    else if (fieldType[fieldIndex] == AVT_TENSOR_VAR)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        int dim  = fieldDim1[fieldIndex];
        float vals[9];
        arr->SetNumberOfComponents(9);
        arr->SetNumberOfTuples(nNodes);
        if (dim == 2)
        {
            for (int i = 0 ; i < nNodes ; i++)
            {
                nwords = GetLine(ifile, line, offsets);
                if (nwords != 4)
                {
                    debug1 << "Incorrect number of components for tensor."
                           << endl;
                    EXCEPTION1(InvalidFilesException, current_file.c_str());
                }
                vals[0] = atof(line + offsets[0]);
                vals[1] = atof(line + offsets[1]);
                vals[2] = 0.;
                vals[3] = atof(line + offsets[2]);
                vals[4] = atof(line + offsets[3]);
                vals[5] = 0.;
                vals[6] = 0.;
                vals[7] = 0.;
                vals[8] = 0.;
                arr->SetTuple(i, vals);
            }
        }
        else
        {
            for (int i = 0 ; i < nNodes ; i++)
            {
                nwords = GetLine(ifile, line, offsets);
                if (nwords != 9)
                {
                    debug1 << "Incorrect number of components for symm-tensor."
                           << endl;
                    EXCEPTION1(InvalidFilesException, current_file.c_str());
                }
                vals[0] = atof(line + offsets[0]);
                vals[1] = atof(line + offsets[1]);
                vals[2] = atof(line + offsets[2]);
                vals[3] = atof(line + offsets[3]);
                vals[4] = atof(line + offsets[4]);
                vals[5] = atof(line + offsets[5]);
                vals[6] = atof(line + offsets[6]);
                vals[7] = atof(line + offsets[7]);
                vals[8] = atof(line + offsets[8]);
                arr->SetTuple(i, vals);
            }
        }
        rv = arr;
    }
    else
    {
        for (int i = 0 ; i < nNodes ; i++)
            nwords = GetLine(ifile, line, offsets);
    }

    if (rv != NULL)
        rv->SetName(fields[fieldIndex].c_str());

    return rv;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::ReadNodeList
//
//  Purpose:
//      Reads in a node list for one domain.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Mon May 24 08:42:53 PDT 2004
//    Allow for node lists to be of zero-length, because there might be
//    multiple node lists, and one of the node lists is empty on this processor
//    only.
//
// ****************************************************************************

vtkPolyData *
avtSpheralFileFormat::ReadNodeList(istream &ifile, int nodeListIndex)
{
    //
    // Read in the line describing the node list.
    //
    vector<int> offsets;
    char line[1024];
    int nwords = GetLine(ifile, line, offsets);
    if (nwords < 3)
    {
        debug1 << "Line cannot contain a valid NodeList declaration." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    if (strcmp(line + offsets[0], "!NodeList") != 0)
    {
        debug1 << "Node list not in the proper place." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }
    if (strcmp(line + offsets[1], nodeLists[nodeListIndex].c_str()) != 0)
    { 
        debug1 << "Node list out of order." << endl;
        debug1 << "Expecting node list: " << nodeLists[nodeListIndex].c_str()
               << ", but got: " << line + offsets[1] << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    int nNodes = atoi(line + offsets[2]);
    if (nNodes < 0)
    {
        debug1 << "Determined number of nodes to be non-positive.  Probably "
               << "an internal parsing error." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    //
    // Now read in the line describing the position vector.
    //
    nwords = GetLine(ifile, line, offsets);
    if (nwords < 4)
    {
        debug1 << "Line cannot contain a valid position vector declaration."
               << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }
    if (strcmp(line + offsets[0], "!Field") != 0)
    {
        debug1 << "Could not locate position field." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }
    if (strcmp(line + offsets[1], positionField[nodeListIndex].c_str()) != 0)
    {
        debug1 << "Internal discrepancy between position field names." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }
    int dim = atoi(line + offsets[3]);
    if (dim != 2 && dim != 3)
    {
        debug1 << "Only supports 2D and 3D points." << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    //
    // Read in all of the actual points in the position field.
    //
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nNodes);
    for (int i = 0 ; i < nNodes ; i++)
    {
        nwords = GetLine(ifile, line, offsets);
        if (nwords != dim)
        {
            debug1 << "Could not parse position field." << endl;
            EXCEPTION1(InvalidFilesException, current_file.c_str());
        }
        float pt[3];
        for (int j = 0 ; j < 3 ; j++)
        {
            if (j < dim)
                pt[j] = atof(line + offsets[j]);
            else
                pt[j] = 0.;
        }
        pts->SetPoint(i, pt);
    }

    vtkPolyData *pdata = vtkPolyData::New();
    pdata->SetPoints(pts);
    pts->Delete();

    pdata->Allocate(nNodes);
    vtkIdType onevertex[1];
    for (int k = 0 ; k < nNodes ; k++)
    {
         onevertex[0] = k;
         pdata->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }
    return pdata;
}



// ****************************************************************************
//  Method: avtSpheralFileFormat::GetLine
//
//  Purpose:
//      Removes whitespace from a line and puts a NULL character between
//      words.  Also returns the position of each word.  Also rejects comment
//      lines.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2003
//
// ****************************************************************************

int
avtSpheralFileFormat::GetLine(istream &ifile, char *line, vector<int> &offsets)
{
    if (ifile.eof())
    {
        debug1 << "File ended prematurely" << endl;
        EXCEPTION1(InvalidFilesException, current_file.c_str());
    }

    ifile.getline(line, 1024);
    offsets.clear();

    char buff[1024];
    int nwords = 0;

    bool inWord = false;
    bool inQuotes = false;
    int buffOffset = 0;
    int nchar = strlen(line);
    int i;
    for (i = 0 ; i < nchar ; i++)
    {
        if (line[i] == '\"')
        {
            inQuotes = (inQuotes ? false : true);
        }

        bool is_space = isspace(line[i]);
        if (inQuotes)
            is_space = false;

        if (inWord)
        {
            if (is_space)
            {
                buff[buffOffset++] = '\0';
                inWord = false;
            }
            else
            {
                if (line[i] != '\"')
                    buff[buffOffset++] = line[i];
            }
        }
        else
        {
            if (!is_space)
            {
                inWord = true;
                offsets.push_back(buffOffset);
                if (line[i] != '\"')
                    buff[buffOffset++] = line[i];
                nwords++;
            }
        }
    }

    // Make sure we have a trailing '\0'
    buff[buffOffset++] = '\0';

    for (i = 0 ; i < buffOffset ; i++)
        line[i] = buff[i];

    //
    // If we have a comment line, then just call the routine again and get
    // the next line.
    //
    if (line[0] == '#')
        return GetLine(ifile, line, offsets);

    return nwords;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetTimeFromFilename
//
//  Purpose:
//      Parses out the time from a filename.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2006
//
// ****************************************************************************

double
avtSpheralFileFormat::GetTimeFromFilename(const char *f) const
{
    if (f == NULL || f[0] == '\0')
        return 0.;

    if (strstr(f, "time=") == NULL)
        return FORMAT_INVALID_TIME;

    double rv = atof(strstr(f, "time=") + strlen("time="));
    return rv;
}


// ****************************************************************************
//  Method: avtSpheralFileFormat::GetCycleFromFilename
//
//  Purpose:
//      Parses out the cycle from a filename.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2006
//
// ****************************************************************************

int
avtSpheralFileFormat::GetCycleFromFilename(const char *f) const
{
    if (f == NULL || f[0] == '\0')
        return 0;

    if (strstr(f, "cycle=") == NULL)
        return FORMAT_INVALID_CYCLE;

    int rv = atoi(strstr(f, "cycle=") + strlen("cycle="));
    return rv;
}


