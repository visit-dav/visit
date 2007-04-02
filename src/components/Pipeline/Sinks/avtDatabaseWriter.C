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
//                            avtDatabaseWriter.C                            //
// ************************************************************************* //

#include <avtDatabaseWriter.h>

#include <Expression.h>
#include <ParsingExprList.h>

#include <avtDatabaseMetaData.h>
#include <avtParallel.h>
#include <avtTerminatingSource.h>

#include <NoInputException.h>


// ****************************************************************************
//  Method: avtDatabaseWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Sep 11 12:14:31 PDT 2004
//    Initialized new data members for target chunks, total zones.
//
//    Jeremy Meredith, Wed Feb 16 15:01:40 PST 2005
//    Initialized members to disable MIR and Expressions.
//
//    Jeremy Meredith, Tue Mar 27 17:16:23 EDT 2007
//    Added a saved pipeline spec in case we need to re-execute the
//    pipeline to get the requested variables.
//
// ****************************************************************************

avtDatabaseWriter::avtDatabaseWriter()
{
    shouldAlwaysDoMIR = false;
    shouldNeverDoMIR = false;
    shouldNeverDoExpressions = false;
    hasMaterialsInProblem = false;
    mustGetMaterialsAdditionally = false;

    shouldChangeChunks = false;
    shouldChangeTotalZones = false;
    nTargetChunks = 1;
    targetTotalZones = 1;
    savedPipelineSpec = NULL;
}


// ****************************************************************************
//  Method: avtDatabaseWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatabaseWriter::~avtDatabaseWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatabaseWriter::SetTargetChunks
//
//  Purpose:
//      Tells the writer what its target number of chunks is.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
// ****************************************************************************

bool
avtDatabaseWriter::SetTargetChunks(int nChunks)
{
    shouldChangeChunks = true;
    nTargetChunks = nChunks;
    return SupportsTargetChunks();
}


// ****************************************************************************
//  Method: avtDatabaseWriter::SetTargetZones
//
//  Purpose:
//      Tells the writer what its target number of zones is.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
// ****************************************************************************

bool
avtDatabaseWriter::SetTargetZones(VISIT_LONG_LONG nZones)
{
    shouldChangeTotalZones = true;
    targetTotalZones = nZones;
    return SupportsTargetZones();
}


// ****************************************************************************
//  Method: avtDatabaseWriter::Write
//
//  Purpose:
//      Writes out a database making use of virtual function calls.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
// ****************************************************************************

void
avtDatabaseWriter::Write(const std::string &filename,
                         const avtDatabaseMetaData *md)
{
    std::vector<std::string> varlist;
    Write(filename, md, varlist);
}


// ****************************************************************************
//  Method: avtDatabaseWriter::Write
//
//  Purpose:
//      Writes out a database making use of virtual function calls.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Sep 11 12:14:31 PDT 2004
//    Added argument for variable list.
//
//    Hank Childs, Wed Dec 22 11:25:07 PST 2004
//    Enable expressions, since they now work inside of convert.
//
//    Jeremy Meredith, Wed Feb 16 15:01:40 PST 2005
//    Added ability to disable MIR and Expressions.
//
//    Hank Childs, Thu May 26 15:45:48 PDT 2005
//    Do not write auto-expressions.  Also allow for re-execution to be turned
//    off.
//
//    Hank Childs, Thu Jul 21 16:11:55 PDT 2005
//    Add more support for expressions.  Fix typo with vectors/scalars.
//
//    Hank Childs, Fri Aug 19 15:47:58 PDT 2005
//    If the database does domain decomposition, then we need to change the
//    chunk id to be our processor id.
//
//    Hank Childs, Thu Sep 15 14:13:34 PDT 2005
//    Don't do interface reconstruction if all variables are requested ['6587].
//
//    Hank Childs, Thu Mar 30 12:05:56 PST 2006
//    Do not assume we are writing a mesh.  It might be a curve.
//
//    Jeremy Meredith, Tue Mar 27 17:11:14 EDT 2007
//    Fixed a number of bugs:  (a) in parallel with all or specified
//    variables, all processors execute and save all domains, (b) chunk
//    IDs are not guaranteed to be unique or sequential, (c) the number
//    of domains/blocks is incorrectly taken from the metadata instead
//    of using what was actually in the data tree.
//
//    Hank Childs, Wed Mar 28 10:29:36 PDT 2007
//    Add support for "default".
//
// ****************************************************************************

void
avtDatabaseWriter::Write(const std::string &filename,
                         const avtDatabaseMetaData *md,
                         std::vector<std::string> &varlist, bool allVars)
{
    int  i, j;

    avtDataObject_p dob = GetInput();
    if (*dob == NULL)
        EXCEPTION0(NoInputException);

    std::vector<std::string> scalarList;
    std::vector<std::string> vectorList;
    std::vector<std::string> materialList;

    if (allVars || varlist.size() > 0)
    {
        //
        // We will need a pipeline specification to force an update. Get that 
        // here.
        //
        avtTerminatingSource *src = dob->GetTerminatingSource();
        avtPipelineSpecification_p spec;
        if (*savedPipelineSpec)
            spec = savedPipelineSpec;
        else
            spec = src->GetGeneralPipelineSpecification();
        avtDataSpecification_p ds = spec->GetDataSpecification();
        std::string meshname;
        if (md->GetNumMeshes() > 0)
            meshname = md->GetMesh(0)->name;
        else if (md->GetNumCurves() > 0)
            meshname = md->GetCurve(0)->name;
        else
            EXCEPTION1(ImproperUseException,
                       "No meshes or curves appear to exist");
    
        //
        // We want to process all of the variables in the dataset, so dummy up
        // the data specification to include every variable in the dataset that
        // pertains to mesh 0.
        //
        if (varlist.size() > 0)
        {
            for (j = 0 ; j < varlist.size() ; j++)
            {
                if (varlist[j] == "default")
                    varlist[j] = ds->GetVariable();

                for (i = 0 ; i < md->GetNumScalars() ; i++)
                {
                    const avtScalarMetaData *smd = md->GetScalar(i);
                    if (smd->name == varlist[j])
                       scalarList.push_back(smd->name);
                }
                for (i = 0 ; i < md->GetNumVectors() ; i++)
                {
                    const avtVectorMetaData *vmd = md->GetVector(i);
                    if (vmd->name == varlist[j])
                       vectorList.push_back(vmd->name);
                }
                if (md->GetNumMeshes() == 1 && !shouldNeverDoExpressions)
                {
                    ParsingExprList *pel = ParsingExprList::Instance();
                    ExpressionList *el = pel->GetList();
                    for (i = 0 ; i < el->GetNumExpressions() ; i++)
                    {
                        const Expression &expr = el->GetExpressions(i);
                        if (expr.GetName() == varlist[j])
                        {
                            Expression::ExprType type = expr.GetType();
                            if (type == Expression::ScalarMeshVar)
                                scalarList.push_back(varlist[j]);
                            else if (type == Expression::VectorMeshVar)
                                vectorList.push_back(varlist[j]);
                        }
                    }
                }
                ds->AddSecondaryVariable(varlist[j].c_str());
            }
        }
        else
        {
            for (i = 0 ; i < md->GetNumScalars() ; i++)
            {
                const avtScalarMetaData *smd = md->GetScalar(i);
                if (md->MeshForVar(smd->name) == meshname)
                {
                    ds->AddSecondaryVariable(smd->name.c_str());
                    scalarList.push_back(smd->name);
                }
            }
            for (i = 0 ; i < md->GetNumVectors() ; i++)
            {
                const avtVectorMetaData *vmd = md->GetVector(i);
                if (md->MeshForVar(vmd->name) == meshname)
                {
                    ds->AddSecondaryVariable(vmd->name.c_str());
                    vectorList.push_back(vmd->name);
                }
            }
    
            //
            // We only want the expressions that correspond to the mesh we are
            // operating on.  If there is more than one mesh, then we don't 
            // really know, so don't add expressions.
            //
            if (md->GetNumMeshes() == 1 && !shouldNeverDoExpressions)
            {
                for (i = 0 ; i < md->GetNumberOfExpressions() ; i++)
                {
                    const Expression *expr = md->GetExpression(i);
                    if (expr->GetAutoExpression())
                        continue;
                    Expression::ExprType type = expr->GetType();
                    bool shouldAdd = false;
                    if (type == Expression::ScalarMeshVar)
                    {
                        shouldAdd = true;
                        scalarList.push_back(expr->GetName());
                    }
                    else if (type == Expression::VectorMeshVar)
                    {
                        shouldAdd = true;
                        vectorList.push_back(expr->GetName());
                    }
                    if (shouldAdd)
                        ds->AddSecondaryVariable(expr->GetName().c_str());
                }
            }
        }

        for (i = 0 ; i < md->GetNumMaterials() ; i++)
        {
            const avtMaterialMetaData *mat_md = md->GetMaterial(i);
            if (md->MeshForVar(mat_md->name) == meshname)
            {
                hasMaterialsInProblem = true;
                mustGetMaterialsAdditionally = true;
/* Doing this interface reconstruction almost always is the WRONG thing to do.
 * So remove this code until we decide to put it back in.
                if (!shouldNeverDoMIR &&
                    (shouldAlwaysDoMIR || !CanHandleMaterials()))
                {
                    ds->ForceMaterialInterfaceReconstructionOn();
                    mustGetMaterialsAdditionally = false;
                }
                else
                {
                    mustGetMaterialsAdditionally = true;
                }
 */
                materialList.push_back(mat_md->name);
            }
        }
    
    
        //
        // Actually force the read of the data.
        //
        // Note: if we attempt to export in the middle
        // of an already existing pipeline (e.g. via a
        // hypothetical Export Operator, this call 
        // needs to be skipped.)
        //
        dob->Update(spec);
    }
    else
    {
        avtDataAttributes &atts = dob->GetInfo().GetAttributes();
        int nVars = atts.GetNumberOfVariables();
        for (int i = 0 ; i < nVars ; i++)
        {
            const std::string &name = atts.GetVariableName(i);
            int dim = atts.GetVariableDimension(name.c_str());
            if (dim == 1)
                scalarList.push_back(name);
            else if (dim == 3)
                vectorList.push_back(name);
        }
    }

    //
    // Get the data tree
    //
    avtDataTree_p rootnode = GetInputDataTree();

    //
    // In parallel, we need to find a start chunk ID for this processor
    // to guarantee that chunk IDs go from 0..n-1 across all processors.
    // 
    int  numMyChunks = rootnode->GetNumberOfLeaves();
    int  arrlen = PAR_Size()+1;
    int *startIndexArrayTmp = new int[arrlen];
    int *startIndexArray = new int[arrlen];
    for (int i=0; i<arrlen; i++)
        startIndexArrayTmp[i] = (i>PAR_Rank()) ? numMyChunks : 0;
    SumIntArrayAcrossAllProcessors(startIndexArrayTmp,startIndexArray,arrlen);
    int numTotalChunks = startIndexArray[arrlen-1];
    int startIndex = startIndexArray[PAR_Rank()];
    delete[] startIndexArrayTmp;
    delete[] startIndexArray;

    if (numTotalChunks == 0)
    {
        EXCEPTION1(ImproperUseException, "Dataset to export was empty. "
                   "It is possible an invalid variable was requested.");
    }

    //
    // Call virtual function that the derived type re-defines to do the
    // actual writing.  All of the remaining code is devoted to writing out
    // the file.
    //
    OpenFile(filename, numTotalChunks);
    WriteHeaders(md, scalarList, vectorList, materialList);

    std::vector<avtDataTree_p> nodelist;
    nodelist.push_back(rootnode);

    //
    // This 'for' loop is a bit tricky.  We are adding to the nodelist as we
    // go, so nodelist.size() keeps going.  This is in lieu of recursion.
    //
    int chunkID = startIndex;
    for (int cur_index = 0 ; cur_index < nodelist.size() ; cur_index++)
    {
        avtDataTree_p dt = nodelist[cur_index];
        if (*dt == NULL)
            continue;

        int nc = dt->GetNChildren();
        if (nc > 0)
            for (int i = 0 ; i < nc ; i++)
                nodelist.push_back(dt->GetChild(i));
        else
        {
            vtkDataSet *in_ds = dt->GetDataRepresentation().GetDataVTK();
            WriteChunk(in_ds, chunkID);
            chunkID++;
        }
    }

    CloseFile();
}


// ****************************************************************************
//  Method:  avtDatabaseWriter::SetPipelineSpecToUse
//
//  Purpose:
//    Save a pipeline specification to use when re-executing a pipeline.
///   If this is not set by the caller, then all processors will
//    get all domains if the pipeline re-executes, which is a problem
//    in parallel.
//
//  Arguments:
//    ps         the pipeline specification to use later
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 28, 2007
//
// ****************************************************************************
void
avtDatabaseWriter::SetPipelineSpecToUse(avtPipelineSpecification_p ps)
{
    savedPipelineSpec = ps;
}
