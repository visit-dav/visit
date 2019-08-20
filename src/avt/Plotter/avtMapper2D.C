// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtMapper2D.C                                //
// ************************************************************************* //

#include <avtMapper2D.h>

#include <vtkActor2D.h>
#include <vtkDataSet.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkMapper2D.h>
#include <vtkProperty2D.h>

#include <avtGeometryDrawable.h>
#include <avtTransparencyActor.h>

#include <ColorAttribute.h>
#include <DebugStream.h>



// ****************************************************************************
//  Method: avtMapper2D constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

avtMapper2D::avtMapper2D() : avtMapperBase()
{
    mappers  = NULL;
    actors   = NULL;
}


// ****************************************************************************
//  Method: avtMapper2D destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

avtMapper2D::~avtMapper2D()
{
}


// ****************************************************************************
//  Method: avtMapper2D::SetUpDrawable
//
//  Purpose:
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper2D::SetUpDrawable()
{
    avtGeometryDrawable *gd = new avtGeometryDrawable(nMappers, actors);
    gd->SetMapper(this);
    drawable = gd;
}


// ****************************************************************************
//  Method: avtMapper2D::ClearSelf
//
//  Purpose:
//      Cleans up memory so a new drawable can be created.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper2D::ClearSelf(void)
{
    avtMapperBase::ClearSelf();

    if (mappers != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->Delete();
            }
        }
        delete [] mappers;
    }
    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                actors[i]->Delete();
            }
        }
        delete [] actors;
    }

    //
    // Nothing should be done with the transparency actor -- we don't own it.
    //
    mappers = NULL;
    actors = NULL;
}


// ****************************************************************************
//  Method: avtMapper2D::CreateActorMapperPairs
//
//  Purpose:
//      Sets up the mappers and creates a drawable based on the input.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper2D::CreateActorMapperPairs(vtkDataSet **children)
{
    mappers  = new vtkMapper2D*[nMappers];
    actors   = new vtkActor2D*[nMappers];

    SetUpFilters(nMappers);

    for (int i = 0; i < nMappers; i++)
    {
        // We might have some dummy data (SR-mode).  If so, just continue.
        if (children[i] == NULL || children[i]->GetNumberOfCells() <= 0)
        {
            mappers[i] = NULL;
            actors[i]  = NULL;
            continue;
        }
        mappers[i] = CreateMapper2D();
        vtkAlgorithmOutput * outputPort = InsertFilters(children[i], i);
        if (outputPort != NULL)
            mappers[i]->SetInputConnection(outputPort);
        else
            mappers[i]->SetInputDataObject(children[i]);
        actors[i]  = vtkActor2D::New();
        actors[i]->SetMapper(mappers[i]);
    }
}

// ****************************************************************************
//  Method: avtMapper2D::SetUpFilters
//
//  Purpose:
//      Sets up any filters that should be inserted into the pipeline before
//      the vtkMapper2Ds.  This is a stub for the derived classes.
//
//  Arguments:
//      <unnamed>   The number of domains.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtMapper2D::SetUpFilters(int)
{
    ;
}


// ****************************************************************************
//  Method: avtMapper2D::InsertFilters
//
//  Purpose:
//      Allows the derived types to insert filters into the pipeline before
//      the vtkMapper2D.  This is a stub for the derived classes.
//
//  Arguments:
//      ds          The input dataset.
//      <unnamed>   The index of the domain.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

vtkAlgorithmOutput *
avtMapper2D::InsertFilters(vtkDataSet *ds, int)
{
    return NULL;
}


// ****************************************************************************
//  Method: avtMapper2D::CreateMapper
//
//  Purpose:
//      A hook to allow derived types to insert their own types of mappers.
// 
//  Returns:    A standard vtkMapper2D.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017 
//
// ****************************************************************************

vtkMapper2D *
avtMapper2D::CreateMapper2D()
{
    return vtkPolyDataMapper2D::New();
}


// ****************************************************************************
//  Method: avtMapper2D::SetTransparencyActor
//
//  Purpose:
//      Sets the transparency actor.
//
//  Arguments:
//      act      The transparency actor.  It is associated with a specific
//               visualization window and is not owned by this object.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

int 
avtMapper2D::SetTransparencyActor(avtTransparencyActor *act)
{
    // do nothing for now
    return transparencyIndex;
}

