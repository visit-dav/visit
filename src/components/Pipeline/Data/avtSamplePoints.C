// ************************************************************************* //
//                              avtSamplePoints.C                            //
// ************************************************************************* //

#include <avtSamplePoints.h>

#include <avtCellList.h>
#include <avtSamplePointsSource.h>
#include <avtVolume.h>

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
// ****************************************************************************

avtSamplePoints::avtSamplePoints(avtDataObjectSource *up)
    : avtDataObject(up)
{
    volume   = NULL;
    celllist = NULL;
    numVars  = -1;
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
//      nv      The number of variables to sample over.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2001
//
// ****************************************************************************

void
avtSamplePoints::SetNumberOfVariables(int nv)
{
    numVars = nv;
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
// ****************************************************************************

avtCellList *
avtSamplePoints::GetCellList(void)
{
    if (numVars < 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    if (celllist == NULL)
    {
        celllist = new avtCellList(numVars);
    }

    return celllist;
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
// ****************************************************************************

void
avtSamplePoints::SetVolume(int width, int height, int depth)
{
    if (volume != NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    volume   = new avtVolume(width, height, depth, numVars);
}


