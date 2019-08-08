// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtSamplePoints.C                            //
// ************************************************************************* //

#include <avtSamplePoints.h>

#include <avtCellList.h>
#include <avtSamplePointsSource.h>
#include <avtVolume.h>
#include <avtWebpage.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtSamplePoints constructor
//
//  Arguments:
//      up      The upstream source for this data object.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jan  8 12:52:20 PST 2006
//    Initialize useWeightingScheme.
//
//    Hank Childs, Thu May 31 10:40:50 PDT 2007
//    Remove reference to removed data member "numVars".
//
// ****************************************************************************

avtSamplePoints::avtSamplePoints(avtDataObjectSource *up)
    : avtDataObject(up)
{
    volume   = NULL;
    celllist = NULL;
    useWeightingScheme = false;
}


// ****************************************************************************
//  Method: avtSamplePoints destructor
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

avtSamplePoints::~avtSamplePoints()
{
    if (celllist != NULL)
    {
        delete celllist;
    }
    if (volume != NULL)
    {
        delete volume;
    }
}


// ****************************************************************************
//  Method: avtSamplePoints::SetNumberOfVariables
//
//  Purpose:
//      Sets the number of variables that we will sample over.
//
//  Arguments:
//      vn      The variable names.
//      vs      The size of each variable (i.e. 1 for scalar, etc.)
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2001
//
//  Modifications:
//
//    Hank Childs, Thu May 31 08:49:53 PDT 2007
//    Add support for non-scalar variables.
//
// ****************************************************************************

void
avtSamplePoints::SetNumberOfVariables(std::vector<int> &vs,
                                      std::vector<std::string> &vn)
{
    varnames = vn;
    varsize  = vs;
}


// ****************************************************************************
//  Method: avtSamplePoints::GetNumberOfVariables
//
//  Purpose:
//      Gets the "number of variables".  This will treat a single vector as
//      "three variables", since the sample points data object has limited
//      understanding of anything except scalars.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2007
//
// ****************************************************************************

int
avtSamplePoints::GetNumberOfVariables(void)
{
    int nv = 0;
    for (size_t i = 0 ; i < varsize.size() ; i++)
        nv += varsize[i];
    return nv;
}


// ****************************************************************************
//  Method: avtSamplePoints::GetVariableName
//
//  Arguments:
//      idx     The index of the variable name to get.
//
//  Returns:    The name of the idx'th variable.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2007
//
// ****************************************************************************

const std::string &
avtSamplePoints::GetVariableName(int idx)
{
    if (idx < 0 || idx >= (int)varnames.size())
        EXCEPTION2(BadIndexException, idx, (int)varnames.size());

    return varnames[idx];
}


// ****************************************************************************
//  Method: avtSamplePoints::GetVariableSize
//
//  Arguments:
//      idx     The index of the variable size to get.
//
//  Returns:    The size of the idx'th variable.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2007
//
// ****************************************************************************

int
avtSamplePoints::GetVariableSize(int idx)
{
    if (idx < 0 || idx >= (int)varsize.size())
        EXCEPTION2(BadIndexException, idx, (int)varsize.size());

    return varsize[idx];
}


// ****************************************************************************
//  Method: avtSamplePoints::GetCellList
//
//  Purpose:
//      Gets the cell list.
//
//  Returns:    The cell list.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2001
//
//  Modifications:
//
//    Hank Childs, Thu May 31 10:40:50 PDT 2007
//    Remove reference to removed data member "numVars".
//
//    Hank Childs, Wed Jan 16 08:47:59 PST 2008
//    Create better error message.
//
// ****************************************************************************

avtCellList *
avtSamplePoints::GetCellList(void)
{
    if (varnames.size() <= 0)
    {
        EXCEPTION1(VisItException, 
            "Degenerate case: asked to resample a data set with no variables."
            "  This has not been implemented.");
    }

    if (celllist == NULL)
    {
        celllist = new avtCellList(GetNumberOfVariables());
    }

    return celllist;
}


// ****************************************************************************
//  Method: avtSamplePoints::ResetCellList
//
//  Purpose:
//      Resets the cell list.
//
//  Programmer: Hank Childs
//  Creation:   December 12, 2004
//
// ****************************************************************************

void
avtSamplePoints::ResetCellList(void)
{
    if (celllist != NULL)
        delete celllist;
    celllist = NULL;
}


// ****************************************************************************
//  Method: avtSamplePoints::SetVolume
//
//  Purpose:
//      Sets the size of the output volume.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 14:51:34 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Sun Dec  4 19:18:37 PST 2005
//    Add support for weighting schemes.
//
//    Hank Childs, Thu May 31 10:40:50 PDT 2007
//    Remove reference to removed data member "numVars".
//
// ****************************************************************************

void
avtSamplePoints::SetVolume(int width, int height, int depth)
{
    if (volume != NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    int nv = GetNumberOfVariables(); // counts a vector as 3 vars.
    if (useWeightingScheme)
        nv++;
    volume = new avtVolume(width, height, depth, nv);
}


// ****************************************************************************
//  Method: avtSamplePoints::DebugDump
//
//  Purpose:
//      Does a DebugDump.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2007
//
// ****************************************************************************

void
avtSamplePoints::DebugDump(avtWebpage *webpage, const char *prefix)
{
    avtDataObject::DebugDump(webpage, prefix);
    webpage->AddSubheading("Sample points variable tracking");
    webpage->StartTable();
    webpage->AddTableHeader2("Name", "Size");
    char str[1024];
    for (size_t i = 0 ; i < varnames.size() ; i++)
    {
        snprintf(str, 1024, "%d", varsize[i]);
        webpage->AddTableEntry2(varnames[i].c_str(), str);
    }
    webpage->EndTable();
}


