// ************************************************************************* //
//                     avtDecorationsMapper.C                                //
// ************************************************************************* //

#include <avtDecorationsMapper.h>

#include <float.h>
#include <vector>

#include <vtkDataObjectCollection.h>
#include <avtDecorationsDrawable.h>

#include <DebugStream.h>
#include <NoInputException.h>

using std::vector;


// ****************************************************************************
//  Method: avtDecorationsMapper constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
// ****************************************************************************

avtDecorationsMapper::avtDecorationsMapper()
{
    drawable = NULL;
}


// ****************************************************************************
//  Method: avtDecorationsMapper destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

avtDecorationsMapper::~avtDecorationsMapper()
{
    ClearSelf();
}


// ****************************************************************************
//  Method: avtDecorationsMapper::ChangedInput
//
//  Purpose:
//      A hook from avtDatasetSink telling us the input has changed.  We will
//      invalidate the drawable we previously had.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsMapper::ChangedInput(void)
{
    MapperChangedInput();
    ClearSelf();
}


// ****************************************************************************
//  Method: avtDecorationsMapper::InputIsReady
//
//  Purpose:
//      Since we now know that the input is ready, this sets up the mappers.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtDecorationsMapper::InputIsReady(void)
{
    SetUpMappers();
}


// ****************************************************************************
//  Method: avtDecorationsMapper::MapperChangedInput
//
//  Purpose:
//      This is a hook to allow derived types of avtDecorationsMapper to reset their
//      state.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsMapper::MapperChangedInput(void)
{
    ;
}


// ****************************************************************************
//  Method: avtDecorationsMapper::ClearSelf
//
//  Purpose:
//      Cleans up memory so a new drawable can be created.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsMapper::ClearSelf(void)
{
    //
    // This probably doesn't need to be done, but it will guarantee that we are
    // never in an inconsistent state (where we have a valid drawable, but no
    // mappers).
    //
    drawable = NULL;
    if (! actors.empty())
    {
        actors.clear();
    }
}


// ****************************************************************************
//  Method: avtDecorationsMapper::GetDrawable
//
//  Purpose:
//      Gets a drawable from the input dataset.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

avtDrawable_p
avtDecorationsMapper::GetDrawable(void)
{
    avtDataObject_p input = GetInput();
    if (*drawable == NULL || *input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return drawable;
}


// ****************************************************************************
//  Method: avtDecorationsMapper::SetUpMappers
//
//  Purpose:
//      Sets up the mappers and creates a drawable based on the input.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsMapper::SetUpMappers(void)
{
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    avtDataTree_p tree = GetInputDataTree();
   
    vtkDataSet **children;
    int nc;
    children = tree->GetAllLeaves(nc);

    SetUpFilters(nc);
    for (int i = 0; i < nc; i++)
    {
        //
        //  The derived types must performs whatever actions
        //  are necessary for this dataset. They must also 
        /// populate the actors list.
        //
        SetDatasetInput(children[i], i);
    }
    // this was allocated in GetAllLeaves, need to free it now
    delete [] children;

    CustomizeMappers();

    avtDecorationsDrawable *gd = new avtDecorationsDrawable(actors);
    gd->SetMapper(this);
    drawable = gd;
}


// ****************************************************************************
//  Method: avtDecorationsMapper::SetUpFilters
//
//  Purpose:
//      Sets up any filters that should be inserted into the pipeline before
//      the vtkMappers.  This is a stub for the derived classes.
//
//  Arguments:
//      <unnamed>   The number of domains.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtDecorationsMapper::SetUpFilters(int)
{
    ;
}


