#include <SiloReader.h>
#include <InvalidVariableException.h>

#include <avtDatabaseMetaData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

// ****************************************************************************
// Method: SiloReader::Identify
//
// Purpose: 
//   Tests to see if the file is likely to be a Silo file by looking for
//   symbol names that are often in Silo files.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 08:41:35 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
SiloReader::Identify()
{
    bool isSilo = SymbolExists("_meshtv_defvars") ||
                  SymbolExists("_meshtv_info") ||
                  SymbolExists("_fileinfo");

    return isSilo;
}

vtkDataSet *
SiloReader::GetMesh(const char *varName)
{
    EXCEPTION1(InvalidVariableException, varName);
//    return NULL;
}

vtkDataArray *
SiloReader::GetVar(const char *varName)
{
    EXCEPTION1(InvalidVariableException, varName);
//    return NULL;
}

vtkDataArray *
SiloReader::GetVectorVar(const char *varName)
{
    EXCEPTION1(InvalidVariableException, varName);
//    return NULL;
}
