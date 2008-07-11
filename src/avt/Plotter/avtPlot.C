/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                                    avtPlot.C                              //
// ************************************************************************* //

#include <avtPlot.h>

#include <avtCallback.h>
#include <avtCompactTreeFilter.h>
#include <avtCondenseDatasetFilter.h>
#include <avtCurrentExtentFilter.h>
#include <avtMeshLogFilter.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtDataObjectString.h>
#include <avtDataSetWriter.h>
#include <avtDrawer.h>
#include <avtExtents.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtImageDrawable.h>
#include <avtImageMapper.h>
#include <avtSourceFromAVTDataset.h>
#include <avtSourceFromDataset.h>
#include <avtOriginatingSource.h>
#include <avtVertexNormalsFilter.h>
#include <avtSmoothPolyDataFilter.h>

#include <vtkPolyData.h>
#include <vtkDataSet.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>

#include <climits>

#define SIZECUTOFF 1000000


// ****************************************************************************
//  Method: avtPlot constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Mar  1 13:52:16 PST 2001
//    Added a relevantPointsFilter and a facelistFilter.
//
//    Jeremy Meredith, Tue Jun  5 20:41:21 PDT 2001
//    Added initialization of needsRecalculation.
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001 
//    Added initialization of ghostZoneFilter.
//
//    Kathleen Bonnell, Fri Jul 20 07:26:55 PDT 2001 
//    Added combined ghostZone and facelist filter, removed
//    separate filters.
//
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Initialized silr to NULL.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Initialized compactTreeFilter. 
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//    Initialized currentExtentFilter. 
//
//    Hank Childs, Tue Nov 20 14:48:18 PST 2001
//    Initialized index.
//
//    Hank Childs, Mon Dec 31 11:36:28 PST 2001
//    Added vertex normals filter.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002   
//    Initialized intermediateDataObject. 
//
//    Jeremy Meredith, Tue Dec 10 09:15:35 PST 2002
//    Initialize avtSmoothPolyDataFilter.
//
//    Brad Whitlock, Tue Jul 20 16:10:51 PST 2004
//    Added variable units.
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Added cellCountMultiplierForSRThreshold
//
//    Kathleen Bonnell, Tue Nov  2 10:18:16 PST 2004 
//    Initialize meshType. 
//
//    Kathleen Bonnell, Wed Nov  3 16:51:24 PST 2004 
//    Removed meshType, added topologicalDim and spatialDim.
//
//    Kathleen Bonnell, Thu Mar 22 15:45:21 PDT 2007 
//    Added logMeshFilter, xScaleMode, yScaleMode, havePerformedLogX, 
//    havePerformedLogY.
//
// ****************************************************************************

avtPlot::avtPlot()
{
    needsRecalculation = true;
    actor              = new avtActor;
    behavior           = new avtBehavior;
    actor->SetBehavior(behavior);

    drawer             = NULL;
    condenseDatasetFilter      = new avtCondenseDatasetFilter;
    ghostZoneAndFacelistFilter = new avtGhostZoneAndFacelistFilter;
    compactTreeFilter          = new avtCompactTreeFilter;
    currentExtentFilter        = new avtCurrentExtentFilter;
    logMeshFilter              = new avtMeshLogFilter;
    vertexNormalsFilter        = new avtVertexNormalsFilter;
    smooth                     = new avtSmoothPolyDataFilter();
    varname                = NULL;
    varunits               = NULL;
    silr                   = NULL;
    index                  = -1;
    intermediateDataObject = NULL;
    cellCountMultiplierForSRThreshold = 0.0; // an invalid value
    topologicalDim = -1;
    spatialDim = -1;
    xScaleMode2D = LINEAR;
    yScaleMode2D = LINEAR;
    havePerformedLogX2D = false;
    havePerformedLogY2D = false;
    xScaleModeCurve = LINEAR;
    yScaleModeCurve = LINEAR;
    havePerformedLogXCurve = false;
    havePerformedLogYCurve = false;
}


// ****************************************************************************
//  Method: avtPlot destructor
//
//  Programmer: Hank Childs
//  Creation:   January 8, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Mar  1 13:52:16 PST 2001
//    Added a relevantPointsFilter and a facelistFilter.
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001 
//    Added a ghostZoneFilter.
//
//    Kathleen Bonnell, Fri Jul 20 07:26:55 PDT 2001 
//    Added combined ghostZone and facelist filter, removed 
//    separate filters. 
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added compactTreeFilter. 
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//    Added currentExtentFilter. 
//
//    Hank Childs, Mon Dec 31 11:36:28 PST 2001
//    Added vertex normals filter.
//
//    Jeremy Meredith, Tue Dec 10 09:16:39 PST 2002
//    Added poly data smoothing filter.
//
//    Brad Whitlock, Tue Jul 20 16:11:26 PST 2004
//    Added variable units.
//
//    Kathleen Bonnell, Thu Mar 22 15:45:21 PDT 2007 
//    Added logMeshFilter.
//
// ****************************************************************************

avtPlot::~avtPlot()
{
    if (drawer != NULL)
    {
        delete drawer;
        drawer = NULL;
    }
    if (condenseDatasetFilter != NULL)
    {
        delete condenseDatasetFilter;
        condenseDatasetFilter = NULL;
    }
    if (ghostZoneAndFacelistFilter != NULL)
    {
        delete ghostZoneAndFacelistFilter;
        ghostZoneAndFacelistFilter = NULL;
    }
    if (compactTreeFilter != NULL)
    {
        delete compactTreeFilter;
        compactTreeFilter = NULL;
    }
    if (currentExtentFilter != NULL)
    {
        delete currentExtentFilter;
        currentExtentFilter = NULL;
    }
    if (logMeshFilter != NULL)
    {
        delete logMeshFilter;
        logMeshFilter = NULL;
    }
    if (vertexNormalsFilter != NULL)
    {
        delete vertexNormalsFilter;
        vertexNormalsFilter = NULL;
    }
    if (smooth != NULL)
    {
        delete smooth;
        smooth = NULL;
    }
    if (varname != NULL)
    {
        delete [] varname;
        varname = NULL;
    }
    if (varunits != NULL)
    {
        delete [] varunits;
        varunits = NULL;
    }
}


// ****************************************************************************
//  Method: avtPlot::SetDataExtents
//
//  Purpose:
//      Set the data extents for the plot to use.
//
//  Programmer: Eric Brugger
//  Creation:   March 19, 2004
//
// ****************************************************************************

void
avtPlot::SetDataExtents(const vector<double> &extents)
{
    dataExtents = extents;
}


// ****************************************************************************
//  Method: avtPlot::GetDataExtents
//
//  Purpose:
//      Get the data extents that the plot used.
//
//  Programmer: Eric Brugger
//  Creation:   March 19, 2004
//
// ****************************************************************************

void
avtPlot::GetDataExtents(vector<double> &extents)
{
    extents = dataExtents;
}


// ****************************************************************************
//  Method: avtPlot::SetVarName
//
//  Purpose:
//      Sets the variable name for the plot.
//
//  Arguments:
//      name     The name of the variable.
//
//  Programmer:  Hank Childs
//  Creation:    March 9, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Feb 24 13:47:47 PST 2004
//    Fixed memory leak.
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Added code to set cellCountMultiplierForSRThreshold back to zero
//
// ****************************************************************************

void
avtPlot::SetVarName(const char *name)
{
    if (varname != NULL)
    {
        delete [] varname;
        varname = NULL;
    }
    if (name != NULL)
    {
        varname = new char[strlen(name)+1];
        strcpy(varname, name);
    }
    avtLegend_p legend = GetLegend();
    if (*legend != NULL)
    {
        legend->SetVarName(varname);
    }
    needsRecalculation = true;
    cellCountMultiplierForSRThreshold = 0.0;
}

// ****************************************************************************
// Method: avtPlot::SetVarUnits
//
// Purpose: 
//   Sets the units for the plot.
//
// Arguments:
//   name : The name of the units.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 16:12:30 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtPlot::SetVarUnits(const char *name)
{
    if (varunits != NULL)
    {
        delete [] varunits;
        varunits = NULL;
    }
    if (name != NULL)
    {
        varunits = new char[strlen(name)+1];
        strcpy(varunits, name);
    }
    avtLegend_p legend = GetLegend();
    if (*legend != NULL)
    {
        legend->SetVarUnits(varunits);
        legend->Update();
    }
}

// ****************************************************************************
//  Method: avtPlot::Execute
//
//  Purpose: Public wrapper for engine's execute method
//
//  Note: If you are trying to change the behavior of the Engine's execute
//  method, this is NOT the place to do it. Modify the one just below this one
//
//  Programmer: Mark C. Miller 
//  Creation:   February 11, 2003
//
// ****************************************************************************
avtDataObjectWriter_p
avtPlot::Execute(avtDataObject_p input, avtContract_p spec,
                 const WindowAttributes *atts)
{
   return Execute(input, spec, atts, false);
}

// ****************************************************************************
//  Method: avtPlot::Execute
//
//  Purpose:
//      Performs the execution that should take place on the engine.
//
//  Arguments:
//      input   The input dataset.
//
//  Returns:    A DataObjectWriter that contains the output.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  4 14:30:26 PST 2001
//    Added code to create imageOutput from geometryOutput if the geometry's 
//    size is greater than SIZECUTOFF.
//
//    Kathleen Bonnell, Mon Jan  8 11:48:07 PST 2001 
//    Revised code to reflect new version of avtDrawer.  
//   
//    Jeremy Meredith, Thu Mar  1 13:52:16 PST 2001
//    Added a relevantPointsFilter and a facelistFilter directly to this class.
//
//    Hank Childs, Sun Mar 25 11:44:12 PST 2001
//    Account for new interface with data object information.
//
//    Hank Childs, Tue Mar 27 14:32:54 PST 2001
//    Only apply a facelist filter if this is a surface plot.
//
//    Jeremy Meredith, Thu Jul 26 12:31:54 PDT 2001
//    Changed usage of the writer to reflect the fact that it is now
//    and originating sink (to enable dynamic load balancing).
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 200 
//    Added call to CompactTree method. 
//    
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//    Added call to SetCurrentExtents method. 
//    
//    Jeremy Meredith, Thu Nov  8 11:07:14 PST 2001
//    Added WindowAttributes.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002   
//    Place a copy of the data in intermediateDataObject, to be used
//    by queries. 
//
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002 
//    Split ApplyOperators into two methods (added ApplyRendering
//    Transformation).  The output from ApplyOperators is the data stored in
//    intermediateDataObject, and is the portion of the plot that is 
//    (possibly) queryable.
//    
//    Mark C. Miller, Tue Feb 11 08:33:07 PST 2003
//    Added boolean argument indicating if its being called by the
//    CombinedExecute method or not. Made it a protected method. Introduced
//    a public wrapper for this now protected method.
//    
//    Hank Childs, Tue Feb 24 13:47:47 PST 2004
//    Remove old code regarding the "avtDrawer", since its functionality is
//    now totally subsumed by scalable rendering.  Also set the name of the
//    variable for this plot.
//
//    Brad Whitlock, Tue Jul 20 16:39:07 PST 2004
//    Added code to set the units for the plot.
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Added call to set cell count multiplier for SR threshold
//
//    Kathleen Bonnell, Fri Sep  3 10:07:11 PDT 2004
//    Added test for ValidActiveVariable for attempting to retrieve Units,
//    to avoid unncessary exception catches. 
//
//    Kathleen Bonnell, Fri Jan  7 13:00:32 PST 2005 
//    Removed unnecessary TRY-CATCH block. 
//
//    Hank Childs, Wed Mar  2 11:11:59 PST 2005
//    Pass a pipeline specification to the sink, not a data specification.
//
//    Hank Childs, Thu May 25 16:45:41 PDT 2006
//    Tell the intermediate data object that it is not transient (we need it
//    for queries).
//
//    Hank Childs, Fri Feb  1 13:14:21 PST 2008
//    Make use of UtilizeRenderingFilters.
//
// ****************************************************************************

avtDataObjectWriter_p
avtPlot::Execute(avtDataObject_p input, avtContract_p spec,
                 const WindowAttributes *atts, const bool combinedExecute)
{
    SetVarName(spec->GetDataRequest()->GetVariable());

    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    avtCallback::SetCurrentWindowAtts(*atts);

    //
    // Only one of geometryOutput and imageOutput will be populated by
    // ApplyOperators.  Most will return geometry.  Only volume rendering
    // currently returns images.
    //
    avtDataObject_p dob = ApplyOperators(input);
    CopyTo(intermediateDataObject, dob);
    intermediateDataObject->SetTransientStatus(false);
    dob = ApplyRenderingTransformation(dob);

    if (UtilizeRenderingFilters() && 
        strcmp(dob->GetType(), "avtDataset") == 0)
    {
        dob = ReduceGeometry(dob);
        dob = CompactTree(dob);
        dob = SetCurrentExtents(dob);
    }

    spec = EnhanceSpecification(spec);

    avtDataObjectWriter_p writer = dob->InstantiateWriter();
    writer->SetInput(dob);

    // if we're being called by the CombinedExecute method, don't actually
    // do any work with the writer.
    if (!combinedExecute)
    {
       writer->Execute(spec);
    }

    //
    // Try setting the plot's units based on the information in the dob.
    //
    if (dob->GetInfo().GetAttributes().ValidActiveVariable())
    {
        std::string dobunits = dob->GetInfo().GetAttributes().GetVariableUnits();
        if(dobunits == "")
            SetVarUnits(NULL);
        else
            SetVarUnits(dobunits.c_str());
    }
    else
    { 
        SetVarUnits(NULL);
    }

    SetCellCountMultiplierForSRThreshold(dob);

    return writer;
}


// ****************************************************************************
//  Method: avtPlot::Execute
//
//  Purpose: Public wrapper for viewer's execute method
//
//  Note: If you are looking for the place to modify the viewer's execute
//  method, this is NOT the place to do it. Modify the method below this
//  one.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 11, 2003 
//
// ****************************************************************************

avtActor_p
avtPlot::Execute(avtDataObjectReader_p reader)
{
   return Execute(reader, NULL);
}

// ****************************************************************************
//  Method: avtPlot::Execute
//
//  Purpose:
//      Performs the viewer portion of execute for an avtPlot.  This will apply
//      a mapper if we are in geometry mode and then create an actor that can
//      be added to a VisWindow.
//
//  Arguments:
//      reader  A reader that contains geometry or an image.
//   
//  Returns:    An actor created from the data object input.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jan  5 10:57:29 PST 2001
//    When geometry is sent across the network, populate the current data
//    and spatial extents before sending them into the theater.
//
//    Hank Childs, Sun Mar 25 12:04:27 PST 2001
//    Extents now take doubles, not floats.
//
//    Brad Whitlock, Thu Apr 4 16:40:17 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Kathleen Bonnell, Fri Jul 12 16:45:42 PDT 2002  
//    Added support for decorations. 
//
//    Mark C. Miller, Tue Feb 11 08:33:07 PST 2003
//    Added avtDataObject argument indicating if its being called by the
//    CombinedExecute method or not. Made it a protected method. Introduced
//    a public wrapper for this now protected method.
//
//    Eric Brugger, Wed Aug 20 09:52:57 PDT 2003
//    Set the window mode based on the spatial dimension.
//
//    Kathleen Bonnell, Wed Nov  3 16:51:24 PST 2004
//    Save Spatial and Topological dimension for use by derived types. 
//
//    Hank Childs, Wed Dec  1 07:33:05 PST 2004
//    If plot is image based, then we should not add the drawable to the vis
//    window.  Instead add a null data actor.
//
//    Hank Childs, Mon Feb 28 15:16:58 PST 2005
//    Reset the guide function after it is set to NULL.  This is important
//    for DLB with SR.
//
//    Hank Childs, Wed Mar  2 11:11:59 PST 2005
//    Pass a pipeline specification to the sink, not a data specification.
//
//    Hank Childs, Sun Mar 13 11:13:01 PST 2005
//    Fix memory leak.
//
//    Brad Whitlock, Wed Feb 7 12:19:17 PDT 2007
//    Removed avtTheater.
//
//    Kathleen Bonnell, Thu Mar 22 15:45:21 PDT 2007 
//    Added Call to SetScaleMode(performs log scale on mesh when requested).
//
//    Kathleen Bonnell, Tue Sep 25 07:57:01 PDT 2007 
//    Added separate calls to SetScaleMode, based on spatial/topo dims, so
//    that correct scale modes get used.
//
//    Kathleen Bonnell, Thu Jul 10 16:22:11 PDT 2008
//    Test for Curve plots should test for topo dim 1, not 2.
//
// ****************************************************************************

avtActor_p
avtPlot::Execute(avtDataObjectReader_p reader, avtDataObject_p dob)
{
    avtDrawable_p drawable = NULL;
    avtDrawable_p decorations = NULL;
    avtDataObjectInformation  info;

    //
    // Whether it is an image or geometry, get the information about the
    // dataset and create a drawable.  If it is geometry, creating a drawable
    // means creating a mapper.
    //
    bool haveDatasetReader = (*reader != NULL) && reader->InputIsDataset();
    bool haveDatasetObject = (*dob != NULL) && 
                                 (strcmp(dob->GetType(),"avtDataset")==0);
    bool haveDataset = haveDatasetReader || haveDatasetObject;
    if (haveDataset && !PlotIsImageBased())
    {   
        avtDataset_p geometry;

        debug2 << "avtPlot::Execute Receiving Polygon Data" << endl;

        if (*dob != NULL)
        {
            avtDataObject_p new_dob = dob->Clone();
            new_dob->SetSource(NULL);
            CopyTo(geometry, new_dob);
        }
        else
            geometry = reader->GetDatasetOutput();

        topologicalDim = geometry->GetInfo().GetAttributes().GetTopologicalDimension();
        spatialDim = geometry->GetInfo().GetAttributes().GetSpatialDimension();
        avtMapper *mapper = GetMapper();
        avtDataObject_p geo;
        CopyTo(geo, geometry);

        // Before we get the drawable, we must do an update.
        avtOriginatingSource *src = NULL;
        if (*dob != NULL)
           src = dob->GetOriginatingSource();
        else
           src = geo->GetOriginatingSource();
        avtContract_p ds = 
              new avtContract(src->GetFullDataRequest(), 0);

        avtDataObject_p sd;
        if (spatialDim == 2 && topologicalDim == 1)  
            sd = SetScaleMode(geo, xScaleModeCurve, yScaleModeCurve,
                              havePerformedLogXCurve, havePerformedLogYCurve);
        else 
            sd = SetScaleMode(geo, xScaleMode2D, yScaleMode2D,
                              havePerformedLogX2D, havePerformedLogY2D);
        sd->Update(ds);

        mapper->SetInput(sd);

        GuideFunction foo;
        void *args;
        // Turn off the load balancer.
        if (*dob != NULL)
        {
           mapper->GetGuideFunction(foo,args);
           mapper->SetGuideFunction(NULL,NULL);
        }
        mapper->Execute(ds);
        if (*dob != NULL)
           mapper->SetGuideFunction(foo,args);

        // We must call get extents after the execute.
        info.Copy(geometry->GetInfo());

        drawable = mapper->GetDrawable();
        avtDecorationsMapper *decoMapper = GetDecorationsMapper();
        if (decoMapper != NULL)
        {
            decoMapper->SetInput(sd);
            decoMapper->Execute(ds);
            decorations = decoMapper->GetDrawable();
        }
    }
    else if (((*reader != NULL) && reader->InputIsImage()) ||
             ((*dob != NULL) && !strcmp(dob->GetType(),"avtImage")))
    {   
        avtImage_p  image;
    
        debug2 << "avtPlot::Execute Receiving Image Data" << endl;

        if (*dob != NULL)
           CopyTo(image, dob);
        else
           image = reader->GetImageOutput();

        info.Copy(image->GetInfo());
        avtImageMapper mapper;
        avtDataObject_p img;
        CopyTo(img, image);
        mapper.SetInput(img);
        drawable = mapper.GetDrawable();
    }
    else if (!PlotIsImageBased() && ((*reader != NULL) && 
             reader->InputIsNullData()))
    {
        // proceed as though we had geometry & create a geometry drawable.
        debug2 << "avtPlot::Execute Receiving Null Data" << endl;
        avtNullData_p nullData = reader->GetNullDataOutput();

        // create some bogus geometry data
        vtkPolyData *emptyPolyData = vtkPolyData::New();
        avtSourceFromDataset emptySource((vtkDataSet**)&emptyPolyData,1);
        emptyPolyData->Delete();
        avtDataObject_p geo = emptySource.GetOutput();

        // although we're creating bogus geometry data here, we still need
        // to pass valid dobInfo through the mapper
        avtMapper *mapper = GetMapper();
        avtDataObjectInformation& geoInfo = geo->GetInfo();
        geoInfo.Copy(nullData->GetInfo());
        mapper->SetInput(geo);

        // Before we get the drawable, we must do an update.
        avtOriginatingSource *src = geo->GetOriginatingSource();
        avtContract_p ds = 
              new avtContract(src->GetFullDataRequest(), 0);
        mapper->Execute(ds);

        info.Copy(nullData->GetInfo());

        drawable = mapper->GetDrawable();
        avtDecorationsMapper *decoMapper = GetDecorationsMapper();
        if (decoMapper != NULL)
        {
            decoMapper->SetInput(geo);
            decoMapper->Execute(ds);
            decorations = decoMapper->GetDrawable();
        }
    }
    else if (PlotIsImageBased())
    {
        debug2 << "avtPlot::Execute Plot is Image Based" << endl;
        avtDataObject_p working_dob;
        if (*dob != NULL)
            working_dob = dob;
        else if (*reader != NULL)
            working_dob = reader->GetOutput();
        else
            EXCEPTION0(ImproperUseException);

        // create some bogus geometry data
        vtkPolyData *emptyPolyData = vtkPolyData::New();
        avtSourceFromDataset emptySource((vtkDataSet**)&emptyPolyData,1);
        emptyPolyData->Delete();
        avtDataObject_p geo = emptySource.GetOutput();

        // although we're creating bogus geometry data here, we still need
        // to pass valid dobInfo through the mapper
        avtMapper *mapper = GetMapper();
        avtDataObjectInformation& geoInfo = geo->GetInfo();
        geoInfo.Copy(working_dob->GetInfo());
        mapper->SetInput(geo);

        // Before we get the drawable, we must do an update.
        avtOriginatingSource *src = geo->GetOriginatingSource();
        avtContract_p ds = 
              new avtContract(src->GetFullDataRequest(), 0);
        GuideFunction foo;
        void *args;
        // Turn off the load balancer.
        if (*dob != NULL)
        {
           mapper->GetGuideFunction(foo,args);
           mapper->SetGuideFunction(NULL,NULL);
        }
        mapper->Execute(ds);
        if (*dob != NULL)
           mapper->SetGuideFunction(foo,args);

        drawable = mapper->GetDrawable();
        info.Copy(working_dob->GetInfo());
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    //
    // Add behavior to our drawable. Set the window mode based on the 
    // spatial dimension.  Call CustomizeBehavior to give the derived 
    // types a chance to add legends, etc.
    //
    actor->SetDrawable(drawable);
    actor->SetDecorations(decorations);
    behavior->SetInfo(info);
    if (behavior->GetInfo().GetAttributes().GetSpatialDimension() == 2)
        behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_2D);
    else
        behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_3D);
    CustomizeBehavior();

    //
    // Allow a plot to customize its mapper based on the dataset that it is
    // going to send into it.
    //
    CustomizeMapper(info);

    return actor;
}

// ****************************************************************************
//  Method: avtPlot::Execute
//
//  Purpose:
//      Performs the engine AND viewer portions of execute for an avtPlot in
//      succession but skipping some of the intermediary data serialization
//      and copy steps.
//
//      This code is written to simply call the previously existing viewer
//      and engine execute methods with some flags to skip some steps.
//
//  Returns:    An actor created from the data object input.
//
//  Programmer: Mark C. Miller
//  Creation:   February 11. 2003  
//
// ****************************************************************************
avtActor_p
avtPlot::CombinedExecute(avtDataObject_p input, avtContract_p spec,
                 const WindowAttributes *atts)
{
   avtDataObjectWriter_p writer = Execute(input, spec, atts, true);
   writer->GetInput()->GetInfo().ParallelMerge(writer);
   return Execute(NULL, writer->GetInput());
}


// ****************************************************************************
//  Method: avtPlot::ReduceGeometry
//
//  Purpose:
//      Reduces geometry by adding a facelist filter if appropriate.
//
//  Arguments:
//      curDS   The current dataset (may need to be reduced).
//
//  Returns:    The reduced dataset.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Apr 10 14:01:42 PDT 2001
//    Correct test for debug statement.
//
//    Kathleen Bonnell, Wed Jun 13 15:01:47 PDT 2001 
//    Added ghostZoneFilter. 
//
//    Hank Childs, Tue Jun 26 18:53:48 PDT 2001
//    Only apply a relevant points filter if the target geometry is not points.
//
//    Kathleen Bonnell, Fri Jun 29 14:37:18 PDT 2001 
//    Moved ghostZoneFilter after facelistFilter. 
//
//    Kathleen Bonnell, Fri Jul 20 07:26:55 PDT 2001 
//    Added ghostZoneAndFacelistFilter in place of separate facelistFilter 
//    and ghostZoneFilter.
//
//    Hank Childs, Tue Sep  4 16:04:18 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Hank Childs, Mon Dec 31 11:36:28 PST 2001
//    Add vertex normals if appropriate.
//
//    Jeremy Meredith, Wed Jan  2 12:10:06 PST 2002
//    Changed the test for vertex normals.
//
//    Hank Childs, Wed Feb 27 13:01:47 PST 2002
//    Allowed 2D plots to have facelists taken as well.
//
//    Hank Childs, Tue Aug 13 09:14:52 PDT 2002
//    Calculate the vertex normals after we through out the irrelevant points
//    (the vertex normals operates on all normals).
//
//    Hank Childs, Mon Sep 30 13:34:48 PDT 2002
//    Only apply the ghost zone and facelist filter if necessary.
//
//    Jeremy Meredith, Tue Dec 10 10:26:55 PST 2002
//    Added code to smooth after the facelist filter if the subclass
//    requests it.
//
//    Hank Childs, Fri Jul 25 21:21:08 PDT 2003
//    Rename relevantPointsFilter to condenseDatasetFilter.
//
//    Jeremy Meredith, Wed Aug 13 18:11:45 PDT 2003
//    Made it use always use the normals filter, but only
//    do cell normals if we don't want them averaged to the nodes.
//
//    Hank Childs, Tue Feb 24 13:47:47 PST 2004
//    Defer more decisions to the vertex normals filter, since it has more
//    information.
//
//    Hank Childs, Tue Jun 14 18:17:02 PDT 2005
//    Always do the relevant points when we have point meshes, since the
//    glypher glyphs the points in the point list, not the vertices in the
//    cell list.
//
// ****************************************************************************

avtDataObject_p
avtPlot::ReduceGeometry(avtDataObject_p curDS)
{
    avtDataObject_p rv = curDS;

    //
    // avtGhostZoneAndFacelistFilter applies a facelist and ghostzone
    // filter in correct order depending upon the data.  
    //
    // If we have a 3D dataset, but actually want a surface, then make
    // sure we apply facelist filter, otherwise not.
    //
    avtDataAttributes &atts = curDS->GetInfo().GetAttributes();
    bool useFace = atts.GetTopologicalDimension() == 3 &&
                   TargetTopologicalDimension() == 2;
    if (atts.GetTopologicalDimension() == 2 &&
        TargetTopologicalDimension() == 2)
    {
        useFace = true;
    }

    bool hasGhosts = (atts.GetContainsGhostZones() != AVT_NO_GHOSTS
                     ? true : false);
    if (hasGhosts || useFace)
    {
        ghostZoneAndFacelistFilter->SetInput(rv);
        ((avtGhostZoneAndFacelistFilter*)ghostZoneAndFacelistFilter)
             ->SetUseFaceFilter(useFace);
        rv = ghostZoneAndFacelistFilter->GetOutput();
    }

    condenseDatasetFilter->SetInput(rv);
    if (atts.GetTopologicalDimension() == 0)
        condenseDatasetFilter->BypassHeuristic(true);
    rv = condenseDatasetFilter->GetOutput();

    //
    // Only smooth here if the derived plot type tells us to.
    //
    if (GetSmoothingLevel() > 0)
    {
        smooth->SetSmoothingLevel(GetSmoothingLevel());
        smooth->SetInput(rv);
        rv = smooth->GetOutput();
    }

    //
    // The vertex normals filter will decide whether or not normals are 
    // necessary and what kind of normals (point, cell) to do.
    //
    vertexNormalsFilter->SetInput(rv);
    rv = vertexNormalsFilter->GetOutput();

    avtDataAttributes &outAtts = rv->GetInfo().GetAttributes();
    if (outAtts.GetTopologicalDimension() != TargetTopologicalDimension())
    {
        debug1 << GetName() << ": Did not hit target topological dimension: "
               << TargetTopologicalDimension() << endl;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtPlot::CompactTree
//
//  Purpose:
//    Compacts that data by using a compact tree filter. 
//
//  Arguments:
//    curDS     The current dataset. 
//
//  Returns:    The dataset with it's tree compacted.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 18, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Oct 12 11:38:41 PDT 2001
//    Set flag specifiying that execution depends on DLB. 
// ****************************************************************************

avtDataObject_p
avtPlot::CompactTree(avtDataObject_p curDS)
{
    avtDataObject_p rv = curDS;
    compactTreeFilter->SetInput(rv);
    ((avtCompactTreeFilter*)compactTreeFilter)->DLBDependentExecutionON();
    rv = compactTreeFilter->GetOutput();
    return rv;
}


// ****************************************************************************
//  Method: avtPlot::CustomizeMapper
//
//  Purpose:
//      Customizes the mappers based on data object information.  This is a
//      stub that allows the derived type to not define this method.
//
//  Arguments:
//      <unnamed>   The data object information of the object going into the
//                  mapper.
//
//  Programmer: Hank Childs
//  Creation:   April 7, 2001
//
// ****************************************************************************

void
avtPlot::CustomizeMapper(avtDataObjectInformation &)
{
    ;
}


// ****************************************************************************
//  Method:  avtPlot::NeedsRecalculation
//
//  Purpose:
//    Return true if the plot needs recalulation on the engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  5, 2001
//
//  Modifications:
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Added checks against SIL restrictions as well.
//
//    Eric Brugger, Fri Sep  7 13:38:22 PDT 2001
//    Removed the SIL restriction argument.
//
// ****************************************************************************

bool
avtPlot::NeedsRecalculation(void)
{
    return needsRecalculation;
}


// ****************************************************************************
//  Method: avtPlot::SetCurrentSILRestriction
//
//  Purpose:
//      Set the SIL restriction that the plot just executed for.  This can be
//      used later when trying to determine if a SIL should be re-calculated.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2001
//
// ****************************************************************************

void
avtPlot::SetCurrentSILRestriction(avtSILRestriction_p s)
{
    silr = s;
}


// ****************************************************************************
//  Method: avtPlot::GetCurrentSILRestriction
//
//  Purpose:
//      Get the SIL restriction that the plot just executed for.
//
//  Returns:    The current SIL restriction.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2001
//
// ****************************************************************************

avtSILRestriction_p
avtPlot::GetCurrentSILRestriction(void)
{
    return silr;
}


// ****************************************************************************
//  Method: avtPlot::SetColorTable
//
//  Purpose: 
//      This method allows the plot to lookup the colors for the named
//      colormap and do things with them. This is a stub that allows the derived
//      type to not define this method.
//
//  Arguments:
//      ctName : The name of the color table to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 14 16:44:03 PST 2001
//
// ****************************************************************************

bool
avtPlot::SetColorTable(const char *)
{
    return false;
}


// ****************************************************************************
//  Method: avtPlot::SetBackgroundColor
//
//  Purpose: 
//    This method allows the plot to set the background color. 
//    This is a stub that allows the derived type to not define this method.
//
//  Arguments:
//    bg        The background color. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 25, 2001 
//
// ****************************************************************************

bool
avtPlot::SetBackgroundColor(const double *)
{
    return false;
}


// ****************************************************************************
//  Method: avtPlot::SetForegroundColor
//
//  Purpose: 
//    This method allows the plot to set the foreground color. 
//    This is a stub that allows the derived type to not define this method.
//
//  Arguments:
//    fg        The foreground color. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 26, 2001 
//
// ****************************************************************************

bool
avtPlot::SetForegroundColor(const double *)
{
    return false;
}


// ****************************************************************************
//  Method: avtPlot::SetCurrentExtents
//
//  Purpose: 
//    This method sets the current extents using the current extent filter. 
//
//  Arguments:
//    curDS     The data object. 
//
//  Returns:    The data object with its current extents set. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 2, 2001 
//
// ****************************************************************************

avtDataObject_p
avtPlot::SetCurrentExtents(avtDataObject_p curDS)
{
    avtDataObject_p rv = curDS;
    currentExtentFilter->SetInput(rv);
    rv = currentExtentFilter->GetOutput();
    return rv;
}


// ****************************************************************************
//  Method: avtPlot::SetScaleMode
//
//  Purpose: 
//    This method sets the mesh scaling using the MeshLog filter. 
//
//  Arguments:
//    curDS             The data object to be scaled. 
//    xScaleMode        The scale mode for the x-axis.
//    yScaleMode        The scale mode for the y-axis.
//    havePerformedLogX Has this data object already been log scaled in x?
//    havePerformedLogY Has this data object already been log scaled in y?
//
//  Returns:    The data object with (possibly) log scaling applied.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2007 
// 
//  Modifications:
//    Kathleen Bonnell, Tue Apr  3 16:06:54 PDT 2007
//    Made execution dependent upon this plot allowing curve view scaling.
//
//    Kathleen Bonnell, Tue Sep 25 07:57:01 PDT 2007 
//    Added ScaleMode and bool args so this method could be used for 2d or
//    curve, depending on the args passed in. 
//
// ****************************************************************************

avtDataObject_p
avtPlot::SetScaleMode(avtDataObject_p curDS, ScaleMode xScaleMode, 
                      ScaleMode yScaleMode,
                      bool &havePerformedLogX, bool &havePerformedLogY)
{
    if (!havePerformedLogX && !havePerformedLogY && 
        xScaleMode == LINEAR && yScaleMode == LINEAR)
    {
        return curDS;
    }

    avtDataObject_p rv = curDS;
    if (logMeshFilter != NULL)
    {
       // create a fresh filter
       delete logMeshFilter;
       logMeshFilter = new avtMeshLogFilter();
    }

    ScaleMode useXScaleMode, useYScaleMode;
    bool useInvLogX = false;
    bool useInvLogY = false;
    if (!havePerformedLogX)
    {
        useXScaleMode = xScaleMode;
    }
    else 
    {
        // we've already performed a log scale on the data, 
        // if we want linear now, then we must do an inverse log, 
        // if we want log now, then we don't transform by specifying linear
        //useXScaleMode = (xScaleMode == LINEAR ? INVLOG : LINEAR);
        if (xScaleMode == LINEAR)
        {
            useXScaleMode = LOG;
            useInvLogX = true;
        }
        else
        {
            useXScaleMode = LINEAR; 
        }
    }
    if (!havePerformedLogY)
    {
        useYScaleMode = yScaleMode;
    }
    else 
    {
        // we've already performed a log scale on the data, 
        // if we want linear now, then we must do an inverse log, 
        // if we want log now, then we don't transform by specifying linear
        //useYScaleMode = (yScaleMode == LINEAR ? INVLOG : LINEAR);
        if (yScaleMode == LINEAR)
        {
            useYScaleMode = LOG;
            useInvLogY = true;
        }
        else 
        {
            useYScaleMode = LINEAR;
        }
    }

    logMeshFilter->SetInput(rv);
    logMeshFilter->SetXScaleMode(useXScaleMode);
    logMeshFilter->SetYScaleMode(useYScaleMode);
    logMeshFilter->SetUseInvLogX(useInvLogX);
    logMeshFilter->SetUseInvLogY(useInvLogY);
    rv = logMeshFilter->GetOutput();
    havePerformedLogX = (xScaleMode == LOG);
    havePerformedLogY = (yScaleMode == LOG);
    return rv;
}


// ****************************************************************************
//  Method: avtPlot::EnhanceSpecification
//
//  Purpose:
//      This is a hook for derived types to enhance a pipeline specification.
//      (like read in an extra variable).  Most plots do not want to do this,
//      so this is defined so the derived types don't have to.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2001
//
// ****************************************************************************

avtContract_p
avtPlot::EnhanceSpecification(avtContract_p spec)
{
    return spec;
}


// ****************************************************************************
//  Method: avtPlot::RequiresReExecuteForQuery
//
//  Purpose: 
//    Returns whether or not Re-execution of the pipeline is required
//    in order to perform a query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 25 11:18:43 PST 2003
//    Renamed from "GetTransformedPoints" to reflect actual usage.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Added args needInvT, and needZones.
//
// ****************************************************************************

bool
avtPlot::RequiresReExecuteForQuery(const bool needInvT, const bool needZones)
{
    return behavior->RequiresReExecuteForQuery(needInvT, needZones);
}


// ****************************************************************************
//  Method: avtPlot::GetDecorationsMapper
//
//  Purpose:
//      This is a hook for derived types to add decorations to their
//      render window.  Most plots do not want to do this,
//      so this is defined so the derived types don't have to.
//
//  Programmer: Kathleen Bonnel 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

avtDecorationsMapper *
avtPlot::GetDecorationsMapper()
{
    return NULL;
}


// ****************************************************************************
//  Method: avtPlot::ReleaseData
//
//  Purpose:
//      Tell all of the filters contained within the plot to release their
//      data.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2002
//
// ****************************************************************************

void
avtPlot::ReleaseData(void)
{
    condenseDatasetFilter->ReleaseData();
    ghostZoneAndFacelistFilter->ReleaseData();
    compactTreeFilter->ReleaseData();
    currentExtentFilter->ReleaseData();
    logMeshFilter->ReleaseData();
    vertexNormalsFilter->ReleaseData();
    if (GetMapper() != NULL)
    {
        GetMapper()->ReleaseData();
        GetMapper()->SetInput(NULL);
    }
    avtDataObjectInformation doi;
    behavior->SetInfo(doi);
    actor->SetDrawable(NULL);
    actor->SetDecorations(NULL);
}

// ****************************************************************************
//  Method: avtPlot::SetCellCountMultiplierForSRThreshold
//
//  Purpose: Default method to set a plot's cell count multiplier for SR
//      threshold. By default, all plot's that don't override this method
//      will set a value of 1.0.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 23, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Nov 24 16:49:39 PST 2004
//    If a plot is image based, set the cell count multiplier to be very high.
//
// ****************************************************************************

void
avtPlot::SetCellCountMultiplierForSRThreshold(const avtDataObject_p)
{
    if (PlotIsImageBased())
        cellCountMultiplierForSRThreshold = INT_MAX;
    else
        cellCountMultiplierForSRThreshold = 1.0;
}

// ****************************************************************************
//  Method: avtPlot::GetCellCountMultiplierForSRThreshold
//
//  Purpose: Return the cell count multiplier for SR threshold. However, throw
//  an exception if this information is requested but hasn't already been set.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 23, 2004 
//
// ****************************************************************************

float
avtPlot::GetCellCountMultiplierForSRThreshold() const
{
    if (cellCountMultiplierForSRThreshold == 0.0)
    {
        EXCEPTION1(ImproperUseException, "The plot has not been executed such "
            "that the cell count multiplier for SR threshold can be computed");
    }

    return cellCountMultiplierForSRThreshold;
}


// ****************************************************************************
//  Method: avtPlot::GetPlotInfoAtts
//
//  Purpose: 
//    Returns the PlotInfoAttributes associated with this plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 20, 2006
//
//  Modifications:
//
// ****************************************************************************

const PlotInfoAttributes *
avtPlot::GetPlotInfoAtts()
{
    return behavior->GetPlotInfoAtts();
}

// ****************************************************************************
//  Method: avtPlot::SetScaleMode
//
//  Purpose: 
//    Sets the scale modes. 
//
//  Arguments:
//    ds        The scale mode for the x-axis.
//    rs        The scale mdoe for the y-axis.
//    wm        The window mode to which the scaling applies.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2007 
//
//  Modifications:
//    Kathleen Bonnell, Tue Apr  3 16:06:54 PDT 2007
//    Don't set scale modes if this plot doesn't support curve view scaling.
//
//    Kathleen Bonnell, Wed May  9 16:58:50 PDT 2007 
//    Added support for 2D log scaling.
//
//    Kathleen Bonnell, Tue Sep 25 07:57:01 PDT 2007 
//    2D and Curve modes now stored separately.
//
// ****************************************************************************

bool
avtPlot::SetScaleMode(ScaleMode ds, ScaleMode rs, WINDOW_MODE wm)
{
    bool retval = false;
    if (wm == WINMODE_CURVE && CanDoCurveViewScaling()) 
    {
        xScaleModeCurve = ds;
        yScaleModeCurve = rs;
        retval = true;
    }
    else if (wm == WINMODE_2D && CanDo2DViewScaling())
    {
        xScaleMode2D = ds;
        yScaleMode2D = rs;
        retval = true;
    }
    return retval;
}


// ****************************************************************************
//  Method: avtPlot::ScaleModeRequiresUpdate
//
//  Purpose: 
//    Determines if changing to passed scale modes would require this
//    plot to reexecute (viewer). 
//
//  Arguments:
//    wm    The windowmode that the scale modes apply to.
//    ds    The scale mode for the x-axis.
//    rs    The scale mode for the y-axis.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 27, 2007 
//
//  Modifications:
//
// ****************************************************************************

bool
avtPlot::ScaleModeRequiresUpdate(WINDOW_MODE wm, ScaleMode ds, ScaleMode rs)
{
    if (wm == WINMODE_CURVE)
    {
        return ( (havePerformedLogXCurve != (ds == LOG)) ||
                 (havePerformedLogYCurve != (rs == LOG)) ); 
    }
    else 
    {
        return ( (havePerformedLogX2D != (ds == LOG)) ||
                 (havePerformedLogY2D != (rs == LOG)) ); 
    }
}
