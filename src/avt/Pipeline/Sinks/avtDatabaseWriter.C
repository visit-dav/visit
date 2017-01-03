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
//                            avtDatabaseWriter.C                            //
// ************************************************************************* //

#include <avtDatabaseWriter.h>

#include <Expression.h>
#include <ParsingExprList.h>

#include <vtkAlgorithm.h>
#include <vtkAppendPolyData.h>
#include <vtkDataSet.h>
#include <vtkGeometryFilter.h>
#include <vtkDataSetWriter.h>
#include <vtkPointData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkCharArray.h>
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkVisItPolyDataNormals.h>

#include <avtCompactTreeFilter.h>
#include <avtDatabaseMetaData.h>
#include <avtParallel.h>
#include <avtParallelContext.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>
#include <NoInputException.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <string>
#include <vector>


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
//    Hank Childs, Fri Sep  7 17:54:21 PDT 2012
//    Initialize shouldOutputZonal.
//
//    Mark C. Miller, Tue Jun 14 10:39:39 PDT 2016
//    Added writeContextHasNoDataProcs.
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
    shouldOutputZonal = false;
    nTargetChunks = 1;
    targetTotalZones = 1;
    savedContract = NULL;

    writeContextHasNoDataProcs = false;
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
//  Method: avtDatabaseWriter::SetOutputZonal
//
//  Purpose:
//      Tells the writer to output zonal variables.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2012
//
// ****************************************************************************

bool
avtDatabaseWriter::SetOutputZonal(bool val)
{
    shouldOutputZonal = val;
    return (shouldOutputZonal ? SupportsOutputZonal() : true);
}

// ****************************************************************************
// Method: avtDatabaseWriter::ApplyVariablessToContract
//
// Purpose:
//   This method lets a writer change the contract in response to its variable
//   needs.
//
// Arguments:
//   c0       : The initial version of the contract.
//   meshname : The name of the mesh for the pipeline.
//   vars     : The variable names that are needed.
//   changed  : Return a value indicating whether the contract changed due to
//              calling this method.
//
// Returns:    A new version of the contract.
//
// Note:       Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 16:05:50 PDT 2014
//
// Modifications:
//
// ****************************************************************************

avtContract_p
avtDatabaseWriter::ApplyVariablesToContract(avtContract_p c0, 
    const std::string &meshname, const std::vector<std::string> &vars,
    bool &changed)
{
    avtContract_p c1 = new avtContract(c0);
    avtDataRequest_p ds = c1->GetDataRequest();

    for(size_t i = 0; i < vars.size(); ++i)
    {
        if(vars[i] == std::string(ds->GetVariable()) ||
           ds->HasSecondaryVariable(vars[i].c_str()))
        {
            continue;
        }

        ds->AddSecondaryVariable(vars[i].c_str());
        changed = true;
    }

    return c1;
}

// ****************************************************************************
// Method: avtDatabaseWriter::ApplyMaterialsToContract
//
// Purpose:
//   This method lets a writer change the contract in response to its material
//   needs.
//
// Arguments:
//   c0       : The initial version of the contract.
//   meshname : The name of the mesh for the pipeline.
//   mats     : The material object names.
//   changed  : Return a value indicating whether the contract changed due to
//              calling this method.
//
// Returns:    A new version of the contract.
//
// Note:       This method the hasMaterialsInProblem and 
//             mustGetMaterialsAdditionally flags. Subclasses will use this to
//             make contract changes if they need to request materials, for
//             example.
//
//             Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 16:05:50 PDT 2014
//
// Modifications:
//
// ****************************************************************************

avtContract_p
avtDatabaseWriter::ApplyMaterialsToContract(avtContract_p c0, 
    const std::string &meshname, const std::vector<std::string> &mats,
    bool &changed)
{
    changed = false;
    if(!mats.empty())
    {
        // The default behavior when there are materials is to just set these 
        // flags for derived types:

        // Tell the derived types that there are materials in the dataset.
        hasMaterialsInProblem = true;

        // Tell the derived types that we're not going to force MIR so they
        // should get the avtMaterial themselves.
        mustGetMaterialsAdditionally = true;
    }
    return c0;
}

// ****************************************************************************
// Method: avtDatabaseWriter::GetMaterials
//
// Purpose:
//   Get the materials for pipeline's mesh.
//
// Arguments:
//   needsExecute : True if we know that the pipeline will already need to be
//                  executed again to get the data we need.
//   meshname     : The name of the mesh whose materials we're considering.
//   md           : The database metadata.
//   materialList : The material list that we make.
//
// Returns:    
//
// Note:       This default method only gets the materials if we already know
//             that the pipeline will re-execute. This preserves prior behavior
//             while allowing certain file formats to disregard the needsExecute
//             flag.
//
//
//             Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 14:54:09 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::GetMaterials(bool needsExecute, const std::string &meshname,
    const avtDatabaseMetaData *md,
    std::vector<std::string> &materialList)
{
    // If we know that the pipeline already requires a re-execute. Get the 
    // material information while we're at it.
    if(needsExecute)
    {
        // Add materials to the list.
        for (int i = 0 ; i < md->GetNumMaterials() ; i++)
        {
            const avtMaterialMetaData *mat_md = md->GetMaterial(i);
            if (md->MeshForVar(mat_md->name) == meshname)
                materialList.push_back(mat_md->name);
        }
    }
}

// ****************************************************************************
// Method: avtDatabaseWriter::GetMeshName
//
// Purpose:
//   Get the name of the pipeline's mesh.
//
// Arguments:
//   md : The database metadata.
//
// Returns:    The name of the mesh.
//
// Note:       Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 15:39:13 PDT 2014
//
// Modifications:
//
// ****************************************************************************

std::string
avtDatabaseWriter::GetMeshName(const avtDatabaseMetaData *md) const
{
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    std::string activeMeshName = atts.GetMeshname(); 
    std::string meshname;
    if (md->GetNumMeshes() > 0)
    {
        if (!activeMeshName.empty())
            meshname = activeMeshName;
        else
            meshname = md->GetMesh(0)->name;
    }
    else if (md->GetNumCurves() > 0)
        meshname = md->GetCurve(0)->name;
    else
        EXCEPTION1(ImproperUseException,
                   "No meshes or curves appear to exist");
    return meshname;
}

// ****************************************************************************
// Method: avtDatabaseWriter::GetTime
//
// Purpose:
//   Get the time of the pipeline.
//
// Returns:    The time of the pipetime.
//
// Programmer: Allen Sanderson
// Creation:   April 4 2015
//
// ****************************************************************************

double
avtDatabaseWriter::GetTime() const
{
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    return atts.GetTime(); 
}

// ****************************************************************************
// Method: avtDatabaseWriter::GetCycle
//
// Purpose:
//   Get the cycle of the pipeline
//
// Returns:    The cycle of the pipeline.
//
// Programmer: Allen Sanderson
// Creation:   April 4 2015
//
// ****************************************************************************

int
avtDatabaseWriter::GetCycle() const
{
    const avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    return atts.GetCycle();
}

// ****************************************************************************
// Method: avtDatabaseWriter::GetVariables
//
// Purpose:
//   Get the list of scalars and vectors that we will export.
//
// Arguments:
//   meshname         : The name of the mesh whose variables can be used.
//   md               : The database metadata.
//   varlist          : The list of variables that we want to export. This can 
//                      contain a list of user-specified variables or it can be
//                      empty, which is part of requesting all variables.
//   allVars          : True if we want to get all variables. If false then 
//                      we pay more attention to varlist.
//   allowExpressions : True if we want to allow expressions to be exported
//                      when we make a variable list that includes more variables.
//   defaultVars      : The list of variables to be substituted when we see the
//                      "default" variable name.
//   scalarList       : The list of scalars that we output.
//   vectorList       : The list of vectors that we output.
//
// Returns: GETVARIABLES_ADD_NO_VARS        0: We collected variable names but
//                                             they are not eligible for insertion
//                                             into the contract.
//          GETVARIABLES_ADD_VARS           1: We collected variable names and
//                                             they are not eligible for insertion
//                                             into the contract.
//          GETVARIABLES_ADD_VARS_AND_CLEAR 2: We collected variable names and
//                                             they are not eligible for insertion
//                                             into the contract. We will also
//                                             clear any variables that were
//                                             already in the contract.
//
// Note:       Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 15:29:53 PDT 2014
//
// Modifications:
//   Brad Whitlock, Mon Jun 16 18:05:22 PDT 2014
//   Be more permissive about adding whichever expressions the user wants.
//
// ****************************************************************************

#define GETVARIABLES_ADD_NO_VARS        0
#define GETVARIABLES_ADD_VARS           1
#define GETVARIABLES_ADD_VARS_AND_CLEAR 2

int
avtDatabaseWriter::GetVariables(const std::string &meshname,
    const avtDatabaseMetaData *md,
    const std::vector<std::string> &varlist0, bool allVars, bool allowExpressions,
    const std::vector<std::string> &defaultVars,
    std::vector<std::string> &scalarList,
    std::vector<std::string> &vectorList)
{
    int retval;
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    std::vector<std::string> varlist(varlist0);

    // See if the only variable in the list is "default". If so then
    // there is no point to re-executing.
    bool onlyDefault = varlist.size() == 1 && varlist[0] == "default";
    if (allVars || (varlist.size() > 0 && !onlyDefault))
    {
        if (varlist.size() > 0)
        {
            // Any variable that we add to scalarList and vectorList will be eligible
            // for addition to the contract. We will also clear secondary variables.
            retval = GETVARIABLES_ADD_VARS_AND_CLEAR;

            //
            // Expand "default" to the proper set of variables.
            //
            std::vector<std::string> expandedVarList;
            for (size_t j = 0 ; j < varlist.size() ; j++)
            {
                if (varlist[j] == "default")
                {
                    // Replace "default" with a list of variables.
                    for(size_t i = 0; i < defaultVars.size(); ++i)
                    {
                        expandedVarList.push_back(defaultVars[i]);
                    }
                }
                else
                    expandedVarList.push_back(varlist[j]);
            }
            varlist = expandedVarList;

            //
            // Only process the variables that the user has requested. 
            //
            for (size_t j = 0 ; j < varlist.size() ; j++)
            {
                for (int i = 0 ; i < md->GetNumScalars() ; i++)
                {
                    const avtScalarMetaData *smd = md->GetScalar(i);
                    if (smd->name == varlist[j])
                       scalarList.push_back(smd->name);
                }
                for (int i = 0 ; i < md->GetNumVectors() ; i++)
                {
                    const avtVectorMetaData *vmd = md->GetVector(i);
                    if (vmd->name == varlist[j])
                       vectorList.push_back(vmd->name);
                }
                if (allowExpressions)
                {
                    ParsingExprList *pel = ParsingExprList::Instance();
                    ExpressionList *el = pel->GetList();
                    int index = 0;
                    for (int i = 0 ; i < el->GetNumExpressions() ; i++)
                    {
                        const Expression &expr = el->GetExpressions(i);
                        if (expr.GetName() == varlist[j])
                        {
                            bool canAdd = false;
                            if (md->GetNumMeshes() == 1)
                            {
                                // only one mesh, so can safely assume that
                                // the expression is defined on this mesh
                                canAdd = true;
                            }
                            else if (varlist.size() == 1 &&
                                    atts.GetVariableName() == varlist[j])
                            {
                                // expression is the active var, so must be 
                                // defined on the active mesh
                                canAdd = true;
                            }
                            else if ((index = varlist[j].find("mesh_quality") != std::string::npos) &&
                                (varlist[j].find(meshname, index+12) != std::string::npos))
                            {
                                // expression is mesh_quality expression
                                // defined on the active mesh
                                canAdd = true;
                            }
                            else
                            {
                                TRY
                                {
                                    std::string m = md->MeshForVar(varlist[j]);
                                    canAdd = m == meshname;
                                }
                                CATCH(VisItException)
                                {
                                }
                                ENDTRY
                            }
                            if (canAdd)
                            {
                                Expression::ExprType type = expr.GetType();
                                if (type == Expression::ScalarMeshVar)
                                    scalarList.push_back(varlist[j]);
                                else if (type == Expression::VectorMeshVar)
                                    vectorList.push_back(varlist[j]);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // Any variable that we add to scalarList and vectorList will be eligible
            // for addition to the contract.
            retval = GETVARIABLES_ADD_VARS;

            //
            // We want to process all of the variables in the dataset, so dummy
            // up the data specification to include every variable in the 
            // dataset that pertains to the active mesh 
            //
            for (int i = 0 ; i < md->GetNumScalars() ; i++)
            {
                const avtScalarMetaData *smd = md->GetScalar(i);
                if (md->MeshForVar(smd->name) == meshname)
                {
                    scalarList.push_back(smd->name);
                }
            }
            for (int i = 0 ; i < md->GetNumVectors() ; i++)
            {
                const avtVectorMetaData *vmd = md->GetVector(i);
                if (md->MeshForVar(vmd->name) == meshname)
                {
                    vectorList.push_back(vmd->name);
                }
            }
    
            if (allowExpressions)
            {
                std::string::size_type index = 0;
                for (int i = 0 ; i < md->GetNumberOfExpressions() ; i++)
                {
                    const Expression *expr = md->GetExpression(i);
                    if (expr->GetAutoExpression())
                        continue;
                    bool canAdd = false;
                    std::string varname = expr->GetName();
                    //
                    // We only want the expressions that correspond to the 
                    // mesh we are operating on.  If there is more than one 
                    // mesh, we don't really know unless the meshname is
                    // encoded in the varname, as with mesh_quality 
                    // expressions. So test for that condition.
                    //
                    if (md->GetNumMeshes() == 1)
                    {
                        // only 1 mesh so can safely assume this
                        // expression is defined on this mesh
                        canAdd = true;
                    }
                    else if ((index = varname.find("mesh_quality")) != std::string::npos &&
                             varname.find(meshname, index+12) != std::string::npos)
                    {
                        // mesh_quality expression  
                        // defined on the active mesh
                        canAdd = true;
                    }
                    else
                    {
                        TRY
                        {
                            std::string m = md->MeshForVar(varname);
                            canAdd = m == meshname;
                        }
                        CATCH(VisItException)
                        {
                        }
                        ENDTRY
                    }
                    if (canAdd)
                    {
                        Expression::ExprType type = expr->GetType();
                        if (type == Expression::ScalarMeshVar)
                            scalarList.push_back(varname);
                        else if (type == Expression::VectorMeshVar)
                            vectorList.push_back(varname);
                    }
                }
            }
        }
    }
    else
    {
        //
        // In this case, the idea is that we're going to directly export the
        // pipeline's data object, probably not forcing a pipeline update. As
        // such, get the names of the variables already in the pipeline. Note 
        // that the variables might include synthetic variables that were created
        // by the pipeline and have no hope of being read from a re-execute. So,
        // we're doing this pass to get the variables but they will not be added
        // to any subsequent pipeline execution since we do not want to affect
        // the pipeline in that way.
        //
        retval = GETVARIABLES_ADD_NO_VARS;

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

    return retval;
}

// ****************************************************************************
//  Method: avtDatabaseWriter::Write
//
//  Purpose:
//      Writes out a database making use of virtual function calls.
//
//  Note: This method is used in visitconvert.
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
    Write("", filename, md, varlist, true, false, 1);
}

void
avtDatabaseWriter::Write(const std::string &plotName,
                         const std::string &filename,
                         const avtDatabaseMetaData *md,
                         std::vector<std::string> &varlist, bool allVars)
{
    Write(plotName, filename, md, varlist, allVars, false, 1);
}

void
avtDatabaseWriter::SetWriteContext(avtParallelContext &ctx)
{
    writeContext = ctx;
}

avtParallelContext &
avtDatabaseWriter::GetWriteContext()
{
    return writeContext;
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
//    Kathleen Bonnell, Wed May  2 09:25:16 PDT 2007 
//    Allow expression when there is more than 1 mesh -- if either the
//    chosen var is the active var, or it's a mesh_quality expression
//    defined on the active Mesh.  Change how meshname is determined.
//
//    Jeremy Meredith, Wed Aug  6 18:07:45 EDT 2008
//    Put extra parens to clarfy combined if-assignment statement.
//
//    Brad Whitlock, Tue Jan 21 15:31:11 PST 2014
//    Pass the plot name.
//
//    Brad Whitlock, Fri Mar 14 15:26:59 PDT 2014
//    I moved a lot of code to helper methods so we can override bits of the
//    export process.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Mon Jun 16 18:06:06 PDT 2014
//    Add some more debugging output.
//
//    Brad Whitlock, Thu Aug  6 16:55:50 PDT 2015
//    Added support for writing data in parallel as groups.
//    
//    Mark C. Miller, Tue Jun 14 10:40:00 PDT 2016
//    Added logic to set writeContextHasNoDataProcs
// ****************************************************************************

void
avtDatabaseWriter::Write(const std::string &plotName,
                         const std::string &filename,
                         const avtDatabaseMetaData *md,
                         std::vector<std::string> &varlist, bool allVars,
                         bool writeUsingGroups, int groupSize)
{
    const char *line = 
"=============================================================================";
    const char *mName = "avtDatabaseWriter::Write: ";
    avtDataObject_p dob = GetInput();
    if (*dob == NULL)
        EXCEPTION0(NoInputException);

    // Print the input options.
    debug5 << line << endl;
    debug5 << "EXPORT START" << endl;
    debug5 << line << endl;
    debug5 << mName << "plotName=" << plotName
           << ", filename=" << filename
           << ", varlist={";
    for(size_t i = 0; i < varlist.size(); ++i)
        debug5 << varlist[i] << ", ";
    debug5 << "}, allVars=" << (allVars?"true":"false") << endl;

    // Let the derived types decide whether the input data is suitable.
    CheckCompatibility(plotName);

    // Get the name of the mesh whose variables we're considering.
    std::string meshname(GetMeshName(md));
    debug5 << mName << "meshname=" << meshname << endl;

    // Get the contract.
    avtContract_p spec;
    if (*savedContract)
        spec = savedContract;
    else
        spec = dob->GetOriginatingSource()->GetGeneralContract();
    avtDataRequest_p ds = spec->GetDataRequest();

    // Get an expansion of the default vars.
    std::vector<std::string> dv = GetDefaultVariables(ds);
    debug5 << mName << "default variables={";
    for(size_t i = 0; i < dv.size(); ++i)
        debug5 << dv[i] << ", ";
    debug5 << "}" << endl;

    // Get the list of variables that we need to fulfill the export.
    std::vector<std::string> scalarList;
    std::vector<std::string> vectorList;
    int varMode = GetVariables(meshname, md, varlist, allVars,
                               !shouldNeverDoExpressions, dv, 
                               scalarList, vectorList);

    // Print the results of GetVariables.
    debug5 << mName << "After GetVariables:\n\tscalarList={";
    for(size_t i = 0; i < scalarList.size(); ++i)
        debug5 << scalarList[i] << ", ";
    debug5 << "}" << endl;
    debug5 << "\tvectorList={";
    for(size_t i = 0; i < vectorList.size(); ++i)
        debug5 << vectorList[i] << ", ";
    debug5 << "}" << endl;

    bool needsExecute = false;

    // Change the contract so it has what we need.
    if(varMode == GETVARIABLES_ADD_VARS_AND_CLEAR)
    {
        ds->RemoveAllSecondaryVariables();
        needsExecute = true;
        debug5 << mName << "NEED TO CLEAR SECONDARY VARIABLES" << endl;
    }

    // If the variables are eligible for insertion into the contract, insert
    // them and see if that changes the contract.
    if(varMode == GETVARIABLES_ADD_VARS_AND_CLEAR ||
       varMode == GETVARIABLES_ADD_VARS)
    {
        bool scalarsCausedChange = false, vectorsCausedChange = false;
        spec = ApplyVariablesToContract(spec, meshname, scalarList, scalarsCausedChange);
        if(scalarsCausedChange)
        {
            debug5 << mName << "SCALARS CAUSED CHANGE TO PIPELINE" << endl;
        }

        spec = ApplyVariablesToContract(spec, meshname, vectorList, vectorsCausedChange);
        if(vectorsCausedChange)
        {
            debug5 << mName << "VECTORS CAUSED CHANGE TO PIPELINE" << endl;
        }

        needsExecute |= (scalarsCausedChange || vectorsCausedChange);
    }

    // Get the materials if the format can support materials in some way.
    std::vector<std::string> materialList;
    if(CanHandleMaterials())
    {
        GetMaterials(needsExecute, meshname, md, materialList);

        // Let materials contribute to the contract.
        bool materialsCausedChange = false;
        spec = ApplyMaterialsToContract(spec, meshname, materialList, materialsCausedChange);
        if(materialsCausedChange)
        {
            debug5 << mName << "MATERIALS CAUSED CHANGE TO PIPELINE" << endl;
        }

        needsExecute |= materialsCausedChange;
    }

    // If the contract changed, re-execute.
    if(needsExecute)
    {
        debug5 << endl;
        debug5 << "THE PIPELINE MUST REEXECUTE" << endl;
        debug5 << line << endl << endl;

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

    debug5 << endl;
    debug5 << "DETERMINE WRITE PARTITIONS" << endl;
    debug5 << line << endl << endl;

    //
    // We need to know which ranks have data.
    //
    int *nDatasetsInput = new int[PAR_Size()];
    memset(nDatasetsInput, 0, sizeof(int) * PAR_Size());
    int nMyDatasets = GetInputDataTree()->GetNumberOfLeaves();
    nDatasetsInput[PAR_Rank()] = nMyDatasets;
    int *nDatasets = new int[PAR_Size()];
    SumIntArrayAcrossAllProcessors(nDatasetsInput, nDatasets, PAR_Size());
    // Reuse the buffer as "group".
    int *group = nDatasetsInput;

    // Sum up the total number of dataset chunks.
    int numTotalChunks = 0;
    for(int i = 0; i < PAR_Size(); ++i)
        numTotalChunks += nDatasets[i];

    // If there is no data across the entire set of ranks, we can't export.
    if(numTotalChunks == 0)
    {
        delete [] group;
        delete [] nDatasets;
        EXCEPTION1(ImproperUseException, "Dataset to export was empty. "
                   "It is possible an invalid variable was requested.");
    }

    // Get the sum of the number of datasets up to this processor.
    int startIndex = 0;
    for(int i = 0; i < PAR_Rank(); ++i)
        startIndex += nDatasets[i];

    // Save the current parallel write context. This just makes it available to 
    // derived classes via the GetWriteContext() method.
    avtParallelContext oldContext(writeContext);
    int oldWriteContextHasNoDataProcs(writeContextHasNoDataProcs);

    // Make a message that that we might use to coordinate some writing. We
    // call this on all ranks to stay in sync.
    int tag = writeContext.GetUniqueMessageTag();

    //
    // Scan through the number of datasets and assign a color for each 
    // MPI rank. We "color" them in groups of groupSize, starting with 1.
    // Ranks that have no data get color 0.
    //
    if(writeUsingGroups && groupSize > 0)
    {
        debug5 << "Grouping ranks into write groups:" << endl;
        int nGroups = 1, count = 0;
        std::set<int> unique;
        bool someProcsHaveNoData = false;
        for(int i = 0; i < writeContext.Size(); ++i)
        {
            if(nDatasets[i] > 0)
            {
                if(count >= groupSize)
                {
                    count = 0;
                    ++nGroups;
                }

                group[i] = nGroups;
                ++count;
            }
            else
            {
                group[i] = 0;
                someProcsHaveNoData = true;
            }

            unique.insert(group[i]);
            debug5 << "\trank[" << i << "] nds=" << nDatasets[i] << ", group=" << group[i] << endl;
        }

        // Create a new communicator for the write group based on the colors.
        writeContext = oldContext.Split(group[writeContext.Rank()], unique.size());
        writeContextHasNoDataProcs = someProcsHaveNoData;
    }

    TRY
    {
        debug5 << endl;
        debug5 << "WRITING TO FILES" << endl;
        debug5 << line << endl << endl;

        if((writeUsingGroups && groupSize > 0) && nMyDatasets == 0)
        {
            // If we're doing write grouping then we want to skip the write step
            // for ranks with 0 datasets so they don't write junk empty files.
            debug5 << "This rank is in a group with no data and can skip export." << endl;
        }
        else
        {
            // Write the data out using the parallel context we create for writing.
            // That parallel context may represent several groups of MPI ranks, with
            // each group writing their own data file.
            GroupWrite(plotName, filename, 
                       md, scalarList, vectorList, materialList,
                       numTotalChunks, startIndex,
                       tag, writeUsingGroups, groupSize);
        }

        delete [] group;
        delete [] nDatasets;

        // Restore the old parallel write context.
        SetWriteContext(oldContext);

        // Writer root file
        WriteRootFile();
    }
    CATCHALL
    {
        delete [] group;
        delete [] nDatasets;

        // Restore the old parallel write context.
        writeContext = oldContext;
        writeContextHasNoDataProcs = oldWriteContextHasNoDataProcs;

        RETHROW;
    }
    ENDTRY

    debug5 << line << endl;
    debug5 << "EXPORT END" << endl;
    debug5 << line << endl << endl;
}

// ****************************************************************************
// Method: avtDatabaseWriter::GroupWrite
//
// Purpose:
//   Invokes the methods that enable derived classes to write the datasets.
//
// Arguments:
//
// Returns:    
//
// Note:       I moved this from the Write method.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug  6 16:34:16 PDT 2015
//
// Modifications:
//
// ****************************************************************************
void
avtDatabaseWriter::GroupWrite(const std::string &plotName,
    const std::string &filename,
    const avtDatabaseMetaData *md,
    const std::vector<std::string> &scalarList,
    const std::vector<std::string> &vectorList,
    const std::vector<std::string> &materialList,
    int numTotalChunks, int startIndex,
    int tag, bool writeUsingGroups, int groupSize)
{
    const char *mName = "avtDatabaseWriter::GroupWrite: ";

    //
    // Determine the method of data combination.
    //
    CombineMode mode = GetCombineMode(plotName);

    //
    // Call virtual function that the derived type re-defines to do the
    // actual writing.  All of the remaining code is devoted to writing out
    // the file.
    //
    if(mode == CombineAll)
    {
        debug5 << mName << "Write mode: CombineAll" << endl;

        if(writeContext.Rank() == 0)
        {
            OpenFile(filename, 1);
            WriteHeaders(md, scalarList, vectorList, materialList);
            BeginPlot(plotName);
        }

        // We turn all datasets to polydata and collect them all in a
        // single dataset on rank 0 of the write group.
        vtkPolyData *pd = CreateSinglePolyData(writeContext, GetInputDataTree());

        if(writeContext.Rank() == 0)
        {
            TRY
            {
                // There is only ever 1 chunk to write using this combination mode.
                int domain = 0;
                std::string label("combined");
                WriteChunk(pd, 0, domain, label);
            }
            CATCH(VisItException)
            {
                if(pd != NULL)
                    pd->Delete();
                RETHROW;
            }
            ENDTRY

            EndPlot(plotName);
            CloseFile();
        }

        if(pd != NULL)
            pd->Delete();
    }
    else if(mode == CombineLike || mode == CombineNoneGather)
    {
        debug5 << mName << "Write mode: "
               << (mode == CombineLike ? "CombineLike" : "CombineNoneGather") << endl;

        bool skipCompact = mode == CombineNoneGather;

        // Use the compact tree filter logic to combine like datasets.
        // If we take this path then data are being aggregated onto
        // rank 0 of the ranks that are part of the parallel writeContext.
        avtDataTree_p combined = avtCompactTreeFilter::Execute(
            writeContext,
            GetInput(),
            false, // executionDependsOnDLB
            true,  // parallelMerge
            skipCompact,
            false, // createCleanPolyData,
            0.,    // tolerance,
            avtCompactTreeFilter::Never, // compactDomainMode,
            0      // compactDomainThreshold
            );

        // At this point, all data is on rank0 of the write group.

        // We just write on rank 0 of the write group. The data was already
        // gathered to rank 0 of the write group when we called 
        // avtCompactTreeFilter::Execute.
        if(writeContext.Rank() == 0)
        {
            int nds = 0;
            vtkDataSet **ds = combined->GetAllLeaves(nds);

            // Get some more information about the datasets.
            std::vector<std::string> labels;
            combined->GetAllLabels(labels);
            std::vector<int> domainIds;
            combined->GetAllDomainIds(domainIds);

            if(nds != static_cast<int>(labels.size()))
            {
                EXCEPTION1(ImproperUseException, "The number of labels does not match the number of domains.");
            }
            if(nds != static_cast<int>(domainIds.size()))
            {
                EXCEPTION1(ImproperUseException, "The number of domainIds does not match the number of domains.");
            }
#if 1
            debug5 << "CombinedData: number of datasets = " << nds << endl;
            debug5 << "CombinedData: numLabels = " << labels.size() << endl;
            for(size_t i = 0; i < labels.size(); ++i)
                debug5 << "\tlabels[" << i << "] = " << labels[i] << endl;
#endif
            OpenFile(filename, nds);
            WriteHeaders(md, scalarList, vectorList, materialList);
            BeginPlot(plotName);

            for(int i = 0; i < nds; ++i)
            {
                std::vector<vtkPolyData *> pds;
                TRY
                {
                    // If ds is not polydata, convert.
                    pds = ConvertDatasetsIntoPolyData(&ds[i], 1);
                    WriteChunk(pds[0], i, domainIds[i], labels[i]);
                    pds[0]->Delete();
                }
                CATCHALL
                {
                    delete [] ds;
                    pds[0]->Delete();
                    RETHROW;
                }
                ENDTRY
            }

            delete [] ds;
            EndPlot(plotName);
            CloseFile();
        }
    }
    else // CombineNone
    {
        debug5 << mName << "Write mode: CombineNone" << endl;
        int nWritten = 0;

        // NOTE: Plugins are not allowed to use collective communication in
        //       the methods coming up.

        // If we're using write groups or the format appends data from each
        // rank into the output file, then we iterate over the ranks in
        // the group so they do their I/O one after the next. All write
        // groups do this at the same time.
        if((writeUsingGroups && groupSize > 0) || SequentialOutput())
            WaitForTurn(tag, nWritten);

        OpenFile(filename, numTotalChunks);
        WriteHeaders(md, scalarList, vectorList, materialList);
        BeginPlot(plotName);

        //
        // This 'for' loop is a bit tricky.  We are adding to the nodelist as we
        // go, so nodelist.size() keeps going.  This is in lieu of recursion.
        //
        std::vector<avtDataTree_p> nodelist;
        nodelist.push_back(GetInputDataTree());
        int chunkID = startIndex;
        for (size_t cur_index = 0 ; cur_index < nodelist.size() ; cur_index++)
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
                int domainId = dt->GetDataRepresentation().GetDomain();
                std::string label(dt->GetDataRepresentation().GetLabel());
                WriteChunk(in_ds, chunkID, domainId, label);
                chunkID++;
                ++nWritten;
            }
        }

        EndPlot(plotName);
        CloseFile();

        // The current rank has written its data. Message the next rank
        // in the group so it can write its data...
        if((writeUsingGroups && groupSize > 0) || SequentialOutput())
            GrantTurn(tag, nWritten);
    }
}

// ****************************************************************************
// Method: avtDatabaseWriter::WriteChunk
//
// Purpose:
//   Write a chunk of data to the file format.
//
// Arguments:
//   ds      : The data to write.
//   chunkId : The chunk id of the data (will be 0..N-1 over all chunks 
//             across all ranks.)
//   domainId : The original domain id that generated the data.
//   label    : The label of the data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 21 13:17:30 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::WriteChunk(vtkDataSet *ds, int chunkId, 
    int /*domainId*/, const std::string &/*label*/)
{
    // Call the old method signature so we can use readers that have not 
    // been changed.
    debug5 << "Calling DEPRECATED WriteChunk method." << endl;
    WriteChunk(ds, chunkId);
}

// ****************************************************************************
// Method: avtDatabaseWriter::WaitForTurn
//
// Purpose:
//   This method is called when we need to wait for our turn to do I/O.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 10 12:20:24 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::WaitForTurn(int tag, int &nWritten)
{
#ifdef PARALLEL
    // Wait for a turn.
    if(writeContext.Rank() > 0)
    {
        MPI_Status status;
        MPI_Recv((void*)&nWritten, 1, MPI_INT, writeContext.Rank()-1, tag, 
                 writeContext.GetCommunicator(), &status);
        debug5 << "avtDatabaseWriter::WaitForTurn: It is rank " << writeContext.Rank() << "'s turn now. We've written "
               << nWritten << " chunks." << endl;
    }
#endif
}

// ****************************************************************************
// Method: avtDatabaseWriter::GrantTurn
//
// Purpose:
//   This method is called when we want to tell the next rank that its turn
//   has come.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 10 12:20:24 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::GrantTurn(int tag, int &nWritten)
{
#ifdef PARALLEL
    // All ranks except the last rank send a message to the next rank in the 
    // communicator when it is time for that rank's turn.
    int nextRank = writeContext.Rank()+1;
    if(nextRank < writeContext.Size())
    {
        debug5 << "avtDatabaseWriter::GrantTurn: messaging rank " << nextRank << endl;
        MPI_Send((void*)&nWritten, 1, MPI_INT, nextRank,
                 tag, writeContext.GetCommunicator());
    }
    else
    {
        debug5 << "avtDatabaseWriter::GrantTurn: " << nWritten << " chunks written." << endl;
    }
#endif
}

//****************************************************************************
// Method:  avtDatabaseWriter::GetCombineMode
//
// Purpose:
//   Tells the writer if we're doing any combination of geometry.
//
// Returns: The type of geometry combination that we want for the specified
//          plot.
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer:  Brad Whitlock
// Creation:    Tue Jan 21 16:58:52 PST 2014
//
// Modifications:
//
//****************************************************************************

avtDatabaseWriter::CombineMode
avtDatabaseWriter::GetCombineMode(const std::string &) const
{
    return CombineNone;
}

//****************************************************************************
// Method:  avtDatabaseWriter::CreateTrianglePolyData
//
// Purpose:
//   Tells the writer whether we're creating triangle polydata.
//
// Returns: A flag indicating whether we're creating triangle polydata.
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer:  Brad Whitlock
// Creation:    Tue Jan 21 16:58:52 PST 2014
//
// Modifications:
//
//****************************************************************************

bool
avtDatabaseWriter::CreateTrianglePolyData() const
{
    return false;
}

// ****************************************************************************
// Method: avtDatabaseWriter::CreateNormals
//
// Purpose:
//   Tell the writer to create normals if they do not exist prior to writing
//   out the data.
//
// Returns:    True if we want normals to be created for polydata.
//
// Note:       Normal creation only happens when the combine mode is CombineLike
//             or CombineAll.
//             Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar  5 15:45:09 PST 2014
//
// Modifications:
//
// ****************************************************************************

bool
avtDatabaseWriter::CreateNormals() const
{
    return false;
}

// ****************************************************************************
// Method: avtDatabaseWriter::SequentialOutput
//
// Purpose:
//   Tell the writer whether the format needs MPI-rank sequential access to
//   write the output file.
//
// Returns:    True if we want to allow just one domain in a write group to
//             write its data at any one time.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar  5 15:45:09 PST 2014
//
// Modifications:
//
// ****************************************************************************

bool
avtDatabaseWriter::SequentialOutput() const
{
    return false;
}

//****************************************************************************
// Method:  avtDatabaseWriter::CreateSinglePolyData
//
// Purpose:
//   Create a single polydata from all of the datasets, taking care of movement
//   of data from other ranks to rank 0.
//
// Arguments:
//   context  : The parallel context to use.
//   rootnode : The data tree that we're converting/combining.
//
// Returns: A single polydata object.  Caller must "Delete".
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer:  Brad Whitlock
// Creation:    Tue Jan 21 16:58:52 PST 2014
//
// Modifications:
//
//****************************************************************************

vtkPolyData *
avtDatabaseWriter::CreateSinglePolyData(avtParallelContext &context, avtDataTree_p root)
{
    // Get all of the leaves.
    int nds = 0;
    vtkDataSet **ds = root->GetAllLeaves(nds);

    // Convert all datasets into polydata.
    std::vector<vtkPolyData *> pds = ConvertDatasetsIntoPolyData(ds, nds);
    if(ds != NULL)
        delete [] ds;

    // Send the polydatas to rank 0.
    std::vector<vtkPolyData *> allpds = SendPolyDataToRank0(context, pds);

    // We do not need pds anymore.
    for(size_t i = 0; i < pds.size(); ++i)
        pds[i]->Delete();
    pds.clear();

    // Combine all of the poly datas into a single polydata.
    vtkPolyData *pd = CombinePolyData(allpds);

    // We do not need allpds anymore.
    for(size_t i = 0; i < allpds.size(); ++i)
        allpds[i]->Delete();
    allpds.clear();

    return pd;
}

//****************************************************************************
// Method:  avtDatabaseWriter::ConvertDatasetsIntoPolyData
//
// Purpose:
//   Convert a bunch of datasets into polydata.
//
// Arguments:
//   ds  : An array of polydata objects.
//   nds : The number of polydata objects.
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer:  Brad Whitlock
// Creation:    Tue Jan 21 16:58:52 PST 2014
//
// Modifications:
//   Brad Whitlock, Wed Mar  5 16:00:55 PST 2014
//   I added support for generating normals if they do not exist.
//
//****************************************************************************

std::vector<vtkPolyData *>
avtDatabaseWriter::ConvertDatasetsIntoPolyData(vtkDataSet **ds, int nds)
{
    std::vector<vtkPolyData *> pds;

    for(int i = 0; i < nds; ++i)
    {
        vtkPolyData *pd = vtkPolyData::SafeDownCast(ds[i]);
        if(pd == NULL)
        {
            vtkGeometryFilter *geom = vtkGeometryFilter::New();
            vtkTriangleFilter *tri = vtkTriangleFilter::New();
            geom->SetInputData(ds[i]);
            if(CreateTrianglePolyData())
            {
                // Convert geom output polydata to triangles.
                tri->SetInputConnection(geom->GetOutputPort());        
                tri->Update();
                pd = tri->GetOutput();
                pd->Register(NULL);
            }
            else
            {
                // Use geom output polydata.
                geom->Update();
                pd = geom->GetOutput();
                pd->Register(NULL);
            }
            geom->Delete();
            tri->Delete();
        }
        else if(CreateTrianglePolyData())
        {
            // Convert existing polydata to triangles.
            vtkTriangleFilter *tri = vtkTriangleFilter::New();
            tri->SetInputData(pd);        
            tri->Update();
            pd = tri->GetOutput();
            pd->Register(NULL);
            tri->Delete();
        }
        else
        {
            // Take existing polydata as-is
            pd->Register(NULL);
        }

        // Create point normals.
        if(CreateNormals())
        {
            if(pd->GetPointData()->GetNormals() == NULL &&
               pd->GetPointData()->GetArray("Normals") == NULL)
            {
                vtkVisItPolyDataNormals *pdn = vtkVisItPolyDataNormals::New();
                pdn->SetNormalTypeToPoint();
                pdn->SetInputData(pd);
                pdn->Update();
                pd->Delete();
                pd = pdn->GetOutput();
                pd->Register(NULL);
                pdn->Delete();
            }
        }

        pds.push_back(pd);
    }

    return pds;
}

//****************************************************************************
// Method:  avtDatabaseWriter::CombinePolyData
//
// Purpose:
//   Combine a vector of polydata objects into a single polydata.
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer:  Brad Whitlock
// Creation:    Tue Jan 21 16:58:52 PST 2014
//
// Modifications:
//
//****************************************************************************

vtkPolyData *
avtDatabaseWriter::CombinePolyData(const std::vector<vtkPolyData *> &pds)
{
    vtkPolyData *pd = NULL;
    if(!pds.empty())
    {
        vtkAppendPolyData *f = vtkAppendPolyData::New();
        for(size_t i = 0; i < pds.size(); ++i)
            f->AddInputData(pds[i]);
        f->Update();
        pd = f->GetOutput();
        pd->Register(NULL);
        f->Delete();
    }
    return pd;
}

//****************************************************************************
// Method:  avtDatabaseWriter::SendPolyDataToRank0
//
// Purpose:
//   Move all the data to rank0
//
// Programmer:  Dave Pugmire
// Creation:    April 15, 2013
//
// Modifications:
//
//****************************************************************************

std::vector<vtkPolyData *>
avtDatabaseWriter::SendPolyDataToRank0(avtParallelContext &context,
    const std::vector<vtkPolyData *> &pds)
{
    std::vector<vtkPolyData *> outputpds;

#ifdef PARALLEL
    int par_size = context.Size();
    int par_rank = context.Rank();

    int *inA = new int[par_size], *outA = new int[par_size];
    for (int i = 0; i < par_size; i++)
        inA[i] = outA[i] = 0;

    if (par_rank != 0)
    {
        vtkPolyDataWriter *writer = NULL;
        int len = 0;
        
        if (pds.size() == 0)
            len = 0;
        else
        {
            vtkAppendPolyData *f = vtkAppendPolyData::New();

            writer = vtkPolyDataWriter::New();
            writer->WriteToOutputStringOn();
            writer->SetFileTypeToBinary();
            if (pds.size() == 1)
                writer->SetInputData(pds[0]);
            else
            {
                for(size_t i = 0; i < pds.size(); ++i)
                    f->AddInputData(pds[i]);
                
                writer->SetInputConnection(f->GetOutputPort());
            }
            
            writer->Write();
            len = writer->GetOutputStringLength();
            f->Delete();
        }

        // Send the lengths to rank 0.
        inA[par_rank] = len;
        MPI_Reduce(inA, outA, par_size, MPI_INT, MPI_SUM, 0, context.GetCommunicator());

        // Send the string to rank 0.
        if (len > 0)
        {
            char *data = writer->GetOutputString();
            MPI_Send(data, len, MPI_CHAR, 0, 0, context.GetCommunicator());
            writer->Delete();
        }
    }
    else
    {
        // Copy pds into outputpds for rank 0's polydatas.
        for(size_t i = 0; i < pds.size(); ++i)
        {
            pds[i]->Register(NULL);
            outputpds.push_back(pds[i]);
        }

        // Get the string lengths from other ranks.
        MPI_Reduce(inA, outA, par_size, MPI_INT, MPI_SUM, 0, context.GetCommunicator());

        // Get the strings from other ranks.
        for (int i = 1; i < par_size; i++)
        {
            if (outA[i] > 0)
            {
                char *data = new char[outA[i]];
                MPI_Status stat;
                MPI_Recv(data, outA[i], MPI_CHAR, i, 0, context.GetCommunicator(), &stat);

                vtkPolyDataReader *rdr = vtkPolyDataReader::New();
                rdr->ReadFromInputStringOn();
                vtkCharArray *charArray = vtkCharArray::New();
                charArray->SetArray(data, outA[i], 1);
                rdr->SetInputArray(charArray);
                rdr->Update();
                vtkPolyData *pd = rdr->GetOutput();
                pd->Register(NULL);
                outputpds.push_back(pd);

                delete [] data;
                rdr->Delete();
                charArray->Delete();
            }
        }
    }

    delete [] inA;
    delete [] outA;
#else
    for(size_t i = 0; i < pds.size(); ++i)
    {
        pds[i]->Register(NULL);
        outputpds.push_back(pds[i]);
    }
#endif

    return outputpds;
}

// ****************************************************************************
//  Method:  avtDatabaseWriter::GetDefaultVariables
//
//  Purpose:
//    Use the data request to determine the variables to use for "default".
//
//  Arguments:
//    ds : The data request.
//
//  Note:    Work partially supported by DOE Grant SC0007548.
//
//  Returns: A vector of variable names.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Jan 21 15:50:40 PST 2014
//
// ****************************************************************************

std::vector<std::string>
avtDatabaseWriter::GetDefaultVariables(avtDataRequest_p ds)
{
    std::vector<std::string> vars;
    vars.push_back(ds->GetVariable());
    return vars;
}

// ****************************************************************************
// Method: avtDatabaseWriter::CheckCompatibility
//
// Purpose:
//   Look at the plot type and the data attributes to determine whether we
//   should even start exporting.
//
// Arguments:
//   plotName : Then name of the plot type being exported.
//
// Returns:    
//
// Note:       Throw an exception if there is a problem that would prevent
//             the writer from exporting the data.
//             Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 16:12:27 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::CheckCompatibility(const std::string &plotName)
{
}

// ****************************************************************************
// Method: avtDatabaseWriter::BeginPlot
//
// Purpose:
//   This method is called just before data will be written using WriteChunk.
//
// Arguments:
//   plotName : The name (type) of the plot being exported.
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar  5 13:25:36 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::BeginPlot(const std::string &)
{
}

// ****************************************************************************
// Method: avtDatabaseWriter::EndPlot
//
// Purpose:
//   This method is called just after all data from WriteChunk has been written.
//
// Arguments:
//   plotName : The name (type) of the plot being exported.
//
// Note:    Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar  5 13:25:36 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::EndPlot(const std::string &)
{
}

// ****************************************************************************
// Method: avtDatabaseWriter::WriteRootFile
//
// Purpose:
//   This method writes out a root file to collect a bunch of chunk files into
//   something more convenient for VisIt to read.
//
// Arguments:
//
// Note: This is called after all chunks have been written and the write
//       context has been restored to the global context.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 21 10:23:31 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
avtDatabaseWriter::WriteRootFile()
{
}

// ****************************************************************************
//  Method:  avtDatabaseWriter::SetContractToUse
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
avtDatabaseWriter::SetContractToUse(avtContract_p ps)
{
    savedContract = ps;
}
