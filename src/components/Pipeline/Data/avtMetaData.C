// ************************************************************************* //
//                                avtMetaData.C                              //
// ************************************************************************* //

#include <avtMetaData.h>

#include <stdlib.h>
#include <float.h>

#include <vtkDataSet.h>

#include <avtCommonDataFunctions.h>
#include <avtFacelist.h>
#include <avtIntervalTree.h>
#include <avtTerminatingSource.h>
#include <avtTypes.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtMetaData constructor
//
//  Arguments:
//      s       The source this meta-data corresponds to.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
// ****************************************************************************

avtMetaData::avtMetaData(avtTerminatingSource *s)
{
    source = s;
}


// ****************************************************************************
//  Method: avtMetaData destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMetaData::~avtMetaData()
{
    ;
}


// ****************************************************************************
//  Method: avtMetaData::GetDataExtents
//
//  Purpose:
//      Gets the data extents for the dataset.  An interval tree is always
//      returned, even if it has to be calculated.
//
//  Returns:    An interval tree of the data extents.
//     
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 08:16:49 PDT 2001
//    Removed reference to domain lists.
//
// ****************************************************************************

avtIntervalTree *
avtMetaData::GetDataExtents(void)
{
    VoidRefList list;
    avtPipelineSpecification_p spec = GetPipelineSpecification();
    source->GetVariableAuxiliaryData(AUXILIARY_DATA_DATA_EXTENTS, NULL,
                                     spec, list);
    if (list.nList == 0)
    {
        return NULL;
    }
    if (list.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }

    avtIntervalTree *tree = (avtIntervalTree *) *(list.list[0]);

    return tree;
}


// ****************************************************************************
//  Method: avtMetaData::GetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents for the dataset.  An interval tree is always
//      returned, even if it has to be calculated.
//
//  Returns:    An interval tree of the spatial extents.
//     
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 08:16:49 PDT 2001
//    Removed reference to domain lists.
//
// ****************************************************************************

avtIntervalTree *
avtMetaData::GetSpatialExtents(void)
{
    VoidRefList list;
    avtPipelineSpecification_p spec = GetPipelineSpecification();
    source->GetMeshAuxiliaryData(AUXILIARY_DATA_SPATIAL_EXTENTS, NULL,
                                     spec, list);
    if (list.nList == 0)
    {
        return NULL;
    }
    if (list.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }

    avtIntervalTree *tree = (avtIntervalTree *) *(list.list[0]);
    return tree;
}


// ****************************************************************************
//  Method: avtMetaData::GetExternalFacelist
//
//  Purpose:
//      Gets the face list from the source.
//
//  Note:     Most meta-data objects are calculated if they are not available.
//            This is not true for facelists, so NULL is a possible output.
//
//  Returns:  The facelist for the dataset, if one exists.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 4 14:38:03 PST 2002
//    Changed CopyTo to a template function.
//
//    Hank Childs, Tue Sep 24 11:43:06 PDT 2002
//    Return a normal pointer to a facelist, not a reference counted one.
//
// ****************************************************************************

avtFacelist *
avtMetaData::GetExternalFacelist(int domain)
{
    VoidRefList list;
    avtPipelineSpecification_p spec = GetPipelineSpecification(domain);
    source->GetMeshAuxiliaryData(AUXILIARY_DATA_EXTERNAL_FACELIST, NULL,
                                 spec, list);
    if (list.nList == 0)
    {
        return NULL;
    }
    if (list.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }

    return (avtFacelist *) *(list.list[0]);
}


// ****************************************************************************
//  Method: avtMetaData::GetMaterial
//
//  Purpose:
//      Gets an avtMaterial object.
//
//  Returns:  The material for the dataset, if one exists.
//
//  Programmer: Hank Childs
//  Creation:   August 12, 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 28 08:05:38 PDT 2004
//    Added optional timestep argument.
//
// ****************************************************************************

avtMaterial *
avtMetaData::GetMaterial(int domain, int timestep)
{
    VoidRefList list;
    avtPipelineSpecification_p spec = GetPipelineSpecification(domain);
    //
    // If a timestep has been specified, set it in DataSpecification.
    //
    if (timestep != -1)
        spec->GetDataSpecification()->SetTimestep(timestep);

    source->GetMaterialAuxiliaryData(AUXILIARY_DATA_MATERIAL, NULL, spec,list);
    if (list.nList == 0)
    {
        return NULL;
    }
    if (list.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }

    return (avtMaterial *) *(list.list[0]);
}


// ****************************************************************************
//  Method: avtMetaData::GetSpecies
//
//  Purpose:
//      Gets an avtSpecies object.
//
//  Returns:  The species for the dataset, if one exists.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  8, 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 28 08:05:38 PDT 2004
//    Added optional timestep argument.
//
// ****************************************************************************

avtSpecies *
avtMetaData::GetSpecies(int domain, int timestep)
{
    VoidRefList list;
    avtPipelineSpecification_p spec = GetPipelineSpecification(domain);
    //
    // If a timestep has been specified, set it in DataSpecification.
    //
    if (timestep != -1)
        spec->GetDataSpecification()->SetTimestep(timestep);
    source->GetSpeciesAuxiliaryData(AUXILIARY_DATA_SPECIES, NULL, spec,list);
    if (list.nList == 0)
    {
        return NULL;
    }
    if (list.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    return (avtSpecies *) *(list.list[0]);
}


// ****************************************************************************
//  Method: avtMetaData::GetPipelineSpecification
//
//  Purpose:
//      Gets a pipeline specification corresponding to all data.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtPipelineSpecification_p
avtMetaData::GetPipelineSpecification(void)
{
    //
    // -1 means get an object for the entire dataset.
    //
    return GetPipelineSpecification(-1);
}


// ****************************************************************************
//  Method: avtMetaData::GetPipelineSpecification
//
//  Purpose:
//      Gets a pipeline specification corresponding to all data.
//
//  Arguments:
//      domain  The identifier for the chunk we are interested in.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtPipelineSpecification_p
avtMetaData::GetPipelineSpecification(int domain)
{
    //
    // Get the data from the source that represents all data.  Then override it
    // to have only the domain we are interested in.
    //
    avtDataSpecification_p ds = source->GetFullDataSpecification();
    avtDataSpecification_p alldata = new avtDataSpecification(ds, domain);

    //
    // Create a pipeline with index -1.  This is the index reserved for an
    // ad-hoc pipeline and it is only to meet the interface.  The load balancer
    // should be aware of this.
    //
    avtPipelineSpecification_p ps = new avtPipelineSpecification(alldata, -1);
    ps->UseLoadBalancing(false);

    return ps;
}


