// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPickByNodeQuery.C                            //
// ************************************************************************* //

#include <avtPickByNodeQuery.h>

#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkIntArray.h>
#include <vtkVisItUtility.h>

#include <avtMatrix.h>
#include <avtOriginatingSource.h>
#include <avtVector.h>
#include <PickVarInfo.h>

#include <BadNodeException.h>
#include <float.h>


// ****************************************************************************
//  Method: avtPickByNodeQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:    May 10, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPickByNodeQuery::avtPickByNodeQuery()
{
}


// ****************************************************************************
//  Method: avtPickByNodeQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtPickByNodeQuery::~avtPickByNodeQuery()
{
}


// ****************************************************************************
//  Method: avtPickByNodeQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul  8 16:42:05 PDT 2004
//    Changed the way that nodeid is modified when accounting for ghost zones.
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004 
//    Removed 'needRealId' test, no longer needed (we are reporting ghost
//    zones when ghostType == AVT_HAS_GHOSTS). 
//
//    Kathleen Bonnell, Wed Dec 15 17:24:27 PST 2004 
//    Added logic to handle case when chosen node is to be considered global.
//
//    Kathleen Bonnell, Mon Dec 20 08:33:16 PST 2004 
//    Added early return when DB should determine local id from global, and 
//    pickAtts not fulfilled when returned from the query.
//
//    Kathleen Bonnell, Tue Feb  8 08:29:44 PST 2005 
//    Added test for presence of avtGlobalNodeNumbers array, create error
//    message when not available but should be.
//
//    Hank Childs, Thu Mar 10 10:36:51 PST 2005
//    Fix memory leak.
//
//    Kathleen Bonnell, Wed May 11 17:14:03 PDT 200 
//    Convert varnmames to Global names when the DB supplied the ids. 
//
//    Kathleen Bonnell, Tue Jan 30 16:25:23 PST 2007 
//    Send nodeid to RetrieveVarInfo.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Kathleen Bonnell, Mon Oct  8 18:54:53 PDT 2007 
//    Completed support for node origin. 
//
//    Kathleen Biagas, Tue Jul 22 11:38:50 MST 2014
//    Don't convert to Global ids unless user wants ids shown as global.
//
//    Matt Larsen, Mon Jul 3 07:15:55 PDT 2017
//    Adding support for picking a node by label. 
//
//    Alister Maguire, Wed Apr 10 11:31:03 PDT 2019
//    Don't replace nodeid with GetCurrentNodeForOriginal if we are
//    picking by label. 
//
//    Alister Maguire, Fri Oct 11 15:37:24 PDT 2019
//    Set the real element id for direct database lookups. 
//
//    Alister Maguire, Thu Jul 15 14:38:13 PDT 2021
//    If we were unable to retrieve variable information from the database,
//    query the current dataset.
//
// ****************************************************************************

void
avtPickByNodeQuery::Execute(vtkDataSet *ds, const int dom)
{
    bool pickByLabel = pickAtts.GetElementLabel() != "";

    if (pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    if (!pickAtts.GetElementIsGlobal())
    {
        if (dom != pickAtts.GetDomain())
            return;
    }
    else if (ds->GetPointData()->GetArray("avtGlobalNodeNumbers") == NULL && 
             !pickByLabel)
    {
        pickAtts.SetDomain(-1);
        pickAtts.SetElementNumber(-1);
        pickAtts.SetErrorMessage("Pick could not be performed because a global"
                                 " node id was specified for Pick but the mesh"
                                 " does not contain global node information.");
        pickAtts.SetError(true);
        return; 
    }

    int nodeid;

    if(pickByLabel)
    {
        std::string nodeLabel = pickAtts.GetElementLabel();
        bool isZone = false;
        int id;
        bool error = GetElementIdByLabel(nodeLabel, isZone, id, dom); 

        if(error)
        {
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            pickAtts.SetErrorMessage("Pick could not be performed because a node "
                                     "label was specified for Pick but the mesh "
                                     "does not contain node label information.");
            pickAtts.SetError(true);
            return;
        }

        if( id != -1 )
        {
            nodeid = id;
        }
        else
        {
            return;
        }
    }
    else
    {
        nodeid = pickAtts.GetElementNumber();
    }

    int usernodeid = nodeid;
    int maxEls = ds->GetNumberOfPoints(); 

    // Verify the node number is in range.
    if (!pickAtts.GetElementIsGlobal() && (nodeid < 0 || nodeid >= maxEls))
    {
        EXCEPTION2(BadNodeException, nodeid+nodeOrigin, maxEls+nodeOrigin);
    } 

    bool DBsuppliedNodeId = true;
    if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
    {
        if (pickAtts.GetElementIsGlobal() && !pickByLabel)
        {
            usernodeid = vtkVisItUtility::GetLocalElementForGlobal(ds, 
                          usernodeid, false);
            if (usernodeid == -1)
                return;
            nodeid = usernodeid;
            pickAtts.SetGlobalElement(pickAtts.GetElementNumber());
            DBsuppliedNodeId = false;
        }
        GetNodeCoords(ds, nodeid);    
        if (RetrieveZones(ds, nodeid))
        {
            RetrieveVarInfo(ds, nodeid);
            pickAtts.SetFulfilled(true);
        }
        else
        {
            // the incidient zones could not be found, 
            // no further processing required.
            // SetDomain and ElementNumber to -1 to indicate failure. 
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            pickAtts.SetErrorMessage("Pick encountered an internal "
                "error (could not find incident elements).\n"
                "Please contact a VisIt developer"); 
            pickAtts.SetError(true);
            return; 
        }
    }

    if (pickAtts.GetElementIsGlobal())
        pickAtts.SetDomain(dom);

    //
    //  Allow the database to add any missing information.
    // 
    src->Query(&pickAtts);

    if (!pickAtts.GetFulfilled())
        return;

    //
    // There are times when we aren't able to retrieve information
    // for a patricular variable from the database query. For example,
    // expressions are unavailable for query. In those cases, we can
    // try to directly query the current dataset.
    //
    bool needVarInfo = false;
    int numVars      = pickAtts.GetNumVarInfos();

    for (int varNum = 0; varNum < numVars; ++varNum)
    {
        if (pickAtts.GetVarInfo(varNum).HasInfo() == 0)
        {
            needVarInfo = true;
            break;
        }
    }

    if (pickAtts.GetElementIsGlobal() && DBsuppliedNodeId && !pickByLabel)
    {
        nodeid = GetCurrentNodeForOriginal(ds, pickAtts.GetElementNumber());
        usernodeid = nodeid;
        if (pickAtts.GetShowGlobalIds())
            ConvertElNamesToGlobal();
    }
    pickAtts.SetRealElementNumber(nodeid);
    pickAtts.SetElementNumber(usernodeid + nodeOrigin);
    if (pickAtts.GetMatSelected())
    {
        //
        // The zone numbers stored in IncidentElements are not the correct
        // ones to use with this dataset ... get the correct ones to use
        // with RetrieveVarInfo, then reset them.
        //
        intVector pickedZones = pickAtts.GetIncidentElements();
        intVector currentZones = GetCurrentZoneForOriginal(ds, pickedZones); 
        if (!pickAtts.GetElementIsGlobal())
            RetrieveVarInfo(ds, pickAtts.GetElementNumber(), currentZones); 
        else 
            RetrieveVarInfo(ds, nodeid, currentZones); 
    }
    else if (needVarInfo)
    {
        //
        // The incident elements should be correct. We just need to get
        // some more variable information.
        //
        RetrieveVarInfo(ds, nodeid, pickAtts.GetIncidentElements());
    }

    //
    // Set the domain and zone of pickAtts in relation to the
    // blockOrigin and cellOrigin of the problem.  
    // 
    if (singleDomain)
    {
        //
        // Indicate that there was only one domain.
        // We don't report the domain number for single-domain problems.
        //
        pickAtts.SetDomain(-1);
    }
    else
    {
        pickAtts.SetDomain(dom+blockOrigin);
    }

    //
    // PickPoint determines where on the screen the pick letter will be
    // placed.  This should be the actual node coordinates if the plot was 
    // NOT transformed.
    //
    // If the plot was transformed && the transform is available, 
    // then transform the node coordinates.
    //
    // If the plot was transformed && inverseTransform is NOT available,
    // there is no way to determine the location of the picked node in
    // transformed space, so  ????????
    //
    double coord[3];
    double *p = pickAtts.GetCellPoint();
    if (p[0] == FLT_MAX)
    {
        ds->GetPoint(nodeid, coord);
        pickAtts.SetCellPoint(coord);
    }
    else
    {
        coord[0] = p[0];
        coord[1] = p[1];
        coord[2] = p[2];
    }
    if (transform != NULL)
    {
        avtVector v1(coord);
        v1 = (*transform) *v1;
        coord[0] = v1.x;
        coord[1] = v1.y;
        coord[2] = v1.z;
        pickAtts.SetPickPoint(coord);
    }
    else if (pickAtts.GetNeedTransformMessage())
    {
        //
        // Points were transformed, but we don't need the message because
        // we are displaying the real node coords to the user in pick output.
        //
        pickAtts.SetNeedTransformMessage(false);
        //
        // We need to query the actual data to find a location for the
        // pick letter.  
        //
        pickAtts.SetNeedActualCoords(true);
        if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
        {
            //
            // We have used a 'ghosted' id to query here, but have set the
            // element number back to what the user requested.  Tell the
            // locator code that it should use the RealElementNumber in
            // determining the node location.
            //
            pickAtts.SetRealElementNumber(nodeid);
        }
    }
    else
    {
        pickAtts.SetPickPoint(coord);
    }
}


// ****************************************************************************
//  Method: avtPickByNodeQuery::Preparation
//
//  Purpose:
//    Allows this pick query to modify pickAtts before filters are applied.    
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 17:24:27 PST 2004
//    Set domain to -1 (so that no domain-restriction is performed) if
//    the node is global.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDatAttributes arg.
//
//    Kathleen Bonnell, Mon Oct  8 18:54:53 PDT 2007 
//    Add support for node origin. 
//
// ****************************************************************************

void
avtPickByNodeQuery::Preparation(const avtDataAttributes &)
{
    if (!pickAtts.GetElementIsGlobal())
    {
        int dom = pickAtts.GetDomain() - blockOrigin;
        pickAtts.SetDomain(dom < 0 ? 0 : dom);
        int node = pickAtts.GetElementNumber() - nodeOrigin;
        pickAtts.SetElementNumber(node < 0 ? 0 : node);
    }
    else 
    {
        pickAtts.SetDomain(-1);
    }
}


// ****************************************************************************
//  Method: avtPickByNodeQuery::SetTransform
//
//  Purpose:
//    Set the transform needed by this pick. 
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPickByNodeQuery::SetTransform(const avtMatrix *m)
{
    transform = m;
}

