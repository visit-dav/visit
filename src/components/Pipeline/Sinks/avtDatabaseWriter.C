// ************************************************************************* //
//                            avtDatabaseWriter.C                            //
// ************************************************************************* //

#include <avtDatabaseWriter.h>

#include <Expression.h>

#include <avtDatabaseMetaData.h>
#include <avtTerminatingSource.h>

#include <NoInputException.h>


// ****************************************************************************
//  Method: avtDatabaseWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
// ****************************************************************************

avtDatabaseWriter::avtDatabaseWriter()
{
    shouldAlwaysDoMIR = false;
    hasMaterialsInProblem = false;
    mustGetMaterialsAdditionally = false;
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
// ****************************************************************************

void
avtDatabaseWriter::Write(const std::string &filename,
                         const avtDatabaseMetaData *md)
{
    int  i;

    avtDataObject_p dob = GetInput();
    if (*dob == NULL)
        EXCEPTION0(NoInputException);

    //
    // We will need a pipeline specification to force an update. Get that here.
    //
    avtTerminatingSource *src = dob->GetTerminatingSource();
    avtPipelineSpecification_p spec = src->GetGeneralPipelineSpecification();
    avtDataSpecification_p ds = spec->GetDataSpecification();
    const avtMeshMetaData *mmd = md->GetMesh(0);

    //
    // We want to process all of the variables in the dataset, so dummy up the
    // data specification to include every variable in the dataset that
    // pertains to mesh 0.
    //
    std::vector<std::string> scalarList;
    std::vector<std::string> vectorList;
    std::vector<std::string> materialList;
    for (i = 0 ; i < md->GetNumScalars() ; i++)
    {
        const avtScalarMetaData *smd = md->GetScalar(i);
        if (md->MeshForVar(smd->name) == mmd->name)
        {
            ds->AddSecondaryVariable(smd->name.c_str());
            scalarList.push_back(smd->name);
        }
    }
    for (i = 0 ; i < md->GetNumVectors() ; i++)
    {
        const avtVectorMetaData *vmd = md->GetVector(i);
        if (md->MeshForVar(vmd->name) == mmd->name)
        {
            ds->AddSecondaryVariable(vmd->name.c_str());
            vectorList.push_back(vmd->name);
        }
    }
    for (i = 0 ; i < md->GetNumMaterials() ; i++)
    {
        const avtMaterialMetaData *mat_md = md->GetMaterial(i);
        if (md->MeshForVar(mat_md->name) == mmd->name)
        {
            hasMaterialsInProblem = true;
            mustGetMaterialsAdditionally = true;
            if (shouldAlwaysDoMIR || !CanHandleMaterials())
            {
                ds->ForceMaterialInterfaceReconstructionOn();
                mustGetMaterialsAdditionally = false;
            }
            else
            {
                mustGetMaterialsAdditionally = true;
            }
            materialList.push_back(mat_md->name);
        }
    }

/* 
 * Expressions currently only work on the engine, so don't add them.
 *
    // We only want the expressions that correspond to the mesh we are
    // operating on.  If there is more than one mesh, then we don't 
    // really know, so don't add expressions.
    if (md->GetNumMeshes() == 1)
    {
        for (i = 0 ; i < md->GetNumberOfExpressions() ; i++)
        {
            const Expression *expr = md->GetExpression(i);
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
 *
 */

    //
    // Actually force the read of the data.
    //
    dob->Update(spec);

    //
    // Call virtual function that the derived type re-defines to do the
    // actual writing.  All of the remaining code is devoted to writing out
    // the file.
    //
    OpenFile(filename);
    WriteHeaders(md, scalarList, vectorList, materialList);

    avtDataTree_p rootnode = GetInputDataTree();
    std::vector<avtDataTree_p> nodelist;
    nodelist.push_back(rootnode);

    //
    // This 'for' loop is a bit tricky.  We are adding to the nodelist as we
    // go, so nodelist.size() keeps going.  This is in lieu of recursion.
    //
    cerr << "About to iterate over chunks..." << endl;
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
            int chunk = dt->GetDataRepresentation().GetDomain();
            cerr << "Working on chunk " << chunk << endl;
            WriteChunk(in_ds, chunk);
        }
    }

    CloseFile();
}


