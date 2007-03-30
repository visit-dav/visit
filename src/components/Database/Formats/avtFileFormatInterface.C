// ************************************************************************* //
//                          avtFileFormatInterface.C                         //
// ************************************************************************* //

#include <avtFileFormatInterface.h>

#include <avtFileFormat.h>


using     std::vector;


// ****************************************************************************
//  Method: avtFileFormatInterface::SetCache
//
//  Purpose:
//      Sets the cache for each file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::SetCache(avtVariableCache *cache)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->SetCache(cache);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::RegisterVariableList
//
//  Purpose:
//      Registers a variable list with each file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::RegisterVariableList(const char *var,
                                             const vector<CharStrRef> &vars2nd)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->RegisterVariableList(var, vars2nd);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::PopulateIOInformation
//
//  Purpose:
//      Collect information about the best way to do I/O.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::PopulateIOInformation(avtIOInformation &ioInfo)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        //
        // There is an assumption here that each file format has as much info
        // as the next about the best I/O layout.
        //
        avtFileFormat *ff = GetFormat(0);
        ff->PopulateIOInformation(ioInfo);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::GetType
//
//  Purpose:
//      Gets the type of file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

const char *
avtFileFormatInterface::GetType(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->GetType();
    }

    return "Unknown Format";
}


// ****************************************************************************
//  Method: avtFileFormatInterface::HasVarsDefinedOnSubMeshes
//
//  Purpose:
//      Declares whether or not some variables are defined on sub-meshes
//      only.  This can affect caching strategies.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasVarsDefinedOnSubMeshes(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->HasVarsDefinedOnSubMeshes();
    }

    return false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::PerformsMaterialSelection
//
//  Purpose:
//      Determines whether or not the underlying file format wants to do its
//      own material selection.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************
 
bool
avtFileFormatInterface::PerformsMaterialSelection(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->PerformsMaterialSelection();
    }

    return false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOff
//
//  Purpose:
//      Tells the file format that it should not do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::TurnMaterialSelectionOff(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->TurnMaterialSelectionOff();
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOn
//
//  Purpose:
//      Tells the file format that it should do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::TurnMaterialSelectionOn(const char *matname)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->TurnMaterialSelectionOn(matname);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::CanDoDynamicLoadBalancing
//
//  Purpose:
//      Tells the file format that it should do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::CanDoDynamicLoadBalancing(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        if (! ff->CanDoDynamicLoadBalancing())
        {
            return false;
        }
    }

    return true;
}


