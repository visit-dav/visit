// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

#include <Expression.h>

void
avtXXXXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Add a mesh called "mesh" to the metadata object.

    // Add scalars to the metadata object.

    // Add expression definitions to the metadata object.
    Expression *e0 = new Expression;
    e0->SetName("speed");
    e0->SetDefinition("{u,v,w}");
    e0->SetType(Expression::VectorMeshVar);
    e0->SetHidden(false);
    md->AddExpression(e0);

    Expression *e1 = new Expression;
    e1->SetName("density");
    e1->SetDefinition("mass/volume");
    e1->SetType(Expression::ScalarMeshVar);
    e1->SetHidden(false);
    md->AddExpression(e1);

    // Add other objects to the metadata object.
}
