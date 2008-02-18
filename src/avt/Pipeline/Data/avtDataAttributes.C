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
//                             avtDataAttributes.C                           //
// ************************************************************************* //

#include <avtDataAttributes.h>

#include <algorithm>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtExtents.h>
#include <avtWebpage.h>

#include <BufferConnection.h>
#include <PlotInfoAttributes.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>
#include <InvalidMergeException.h>

using     std::string;
using     std::vector;
using     std::sort;


// ****************************************************************************
//  Method: avtDataAttributes constructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Removed unneeded fields.  Made all extents use avtExtents.
//
//    Hank Childs, Tue Sep 11 09:14:01 PDT 2001
//    Initialize more data members.
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Initialize current extents. 
//
//    Hank Childs, Fri Dec 21 07:18:33 PST 2001
//    Initialize varname.
//
//    Hank Childs, Fri Mar  8 16:39:30 PST 2002
//    Initialize cycle, dtime, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Initialize cellOrigin and blockOrigin.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Initialize containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Initialize containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Initialize transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Initialize windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Initialize labels.
//
//    Hank Childs, Mon Feb 23 08:59:42 PST 2004
//    Allow for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Initialize containsOriginalNodes, invTransform, canUseInvTransform.
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Initialize keepNodeZoneArrays.
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Initialize containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Initialize meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Initialize numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Initialize mirOccurred.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick.
//
//    Jeremy Meredith, Thu Aug 25 11:07:00 PDT 2005
//    Added groupOrigin.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Added fullDBName.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 08:48:40 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Initialize containsExteriorBoundaryGhosts.
//
// ****************************************************************************

avtDataAttributes::avtDataAttributes()
{
    trueSpatial               = NULL;
    cumulativeTrueSpatial     = NULL;
    currentSpatial            = NULL;
    effectiveSpatial          = NULL;
    cumulativeCurrentSpatial  = NULL;

    canUseCumulativeAsTrueOrCurrent = false;

    activeVariable         = -1;
    cellOrigin             = 0;
    nodeOrigin             = 0;
    blockOrigin            = 0;
    groupOrigin            = 0;
    topologicalDimension   = -1;
    spatialDimension       = -1;

    cycle                  = 0;
    cycleIsAccurate        = false;
    dtime                  = 0.;
    timeIsAccurate         = false;
    
    xLabel                 = "X-Axis";
    yLabel                 = "Y-Axis";
    zLabel                 = "Z-Axis";

    meshname               = "<unknown>";
    filename               = "<unknown>";
    fullDBName             = "<unknown>";
    containsGhostZones     = AVT_MAYBE_GHOSTS;
    containsExteriorBoundaryGhosts = false;
    containsOriginalCells  = false;
    containsOriginalNodes  = false;
    keepNodeZoneArrays     = false;
    containsGlobalZoneIds  = false;
    containsGlobalNodeIds  = false;

    SetTopologicalDimension(3);
    SetSpatialDimension(3);

    invTransform = NULL;
    canUseInvTransform = true;

    transform = NULL;
    canUseTransform = true;

    windowMode = WINMODE_NONE;
    adaptsToAnyWindowMode = false;

    numStates = 1;
    mirOccurred = false;
    canUseOrigZones = true;
    origElementsRequiredForPick = false;
    meshCoordType = AVT_XY;
    meshType = AVT_UNKNOWN_MESH;
    nodesAreCritical = false;
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<3; j++)
        {
            unitCellVectors[i*3+j] = (i==j) ? 1.0 : 0.0;
        }
    }
    for (int m=0; m<4; m++)
    {
        for (int n=0; n<4; n++)
        {
            rectilinearGridTransform[m*4+n] = (m==n) ? 1.0 : 0.0;
        }
    }
    rectilinearGridHasTransform = false;
    plotInfoAtts = NULL;
}


// ****************************************************************************
//  Method: avtDataAttributes destructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//      
//  Modifications:
//
//    Hank Childs, Mon Feb 23 09:18:58 PST 2004
//    Moved all code in the former destructor to DestructSelf.  Also moved
//    appropriate comments.
//
// ****************************************************************************

avtDataAttributes::~avtDataAttributes()
{
    DestructSelf();
}


// ****************************************************************************
//  Method: avtDataAttributes::DestructSelf
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//      
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Delete current extents. 
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Delete transform.
//
//    Hank Childs, Mon Feb 23 08:59:42 PST 2004
//    Allow for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Delete invTransform.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

void
avtDataAttributes::DestructSelf(void)
{
    spatialDimension = -1;
    if (trueSpatial != NULL)
    {
        delete trueSpatial;
        trueSpatial = NULL;
    }
    if (cumulativeTrueSpatial != NULL)
    {
        delete cumulativeTrueSpatial;
        cumulativeTrueSpatial = NULL;
    }
    if (effectiveSpatial != NULL)
    {
        delete effectiveSpatial;
        effectiveSpatial = NULL;
    }
    if (currentSpatial != NULL)
    {
        delete currentSpatial;
        currentSpatial = NULL;
    }
    if (cumulativeCurrentSpatial != NULL)
    {
        delete cumulativeCurrentSpatial;
        cumulativeCurrentSpatial = NULL;
    }

    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].trueData != NULL)
        {
            delete variables[i].trueData;
            variables[i].trueData = NULL;
        }
        if (variables[i].cumulativeTrueData != NULL)
        {
            delete variables[i].cumulativeTrueData;
            variables[i].cumulativeTrueData = NULL;
        }
        if (variables[i].effectiveData != NULL)
        {
            delete variables[i].effectiveData;
            variables[i].effectiveData = NULL;
        }
        if (variables[i].currentData != NULL)
        {
            delete variables[i].currentData;
            variables[i].currentData = NULL;
        }
        if (variables[i].cumulativeCurrentData != NULL)
        {
            delete variables[i].cumulativeCurrentData;
            variables[i].cumulativeCurrentData = NULL;
        }
        if (variables[i].componentExtents != NULL)
        {
            delete variables[i].componentExtents;
            variables[i].componentExtents = NULL;
        }
    }
    variables.clear();

    if (invTransform != NULL)
    {
        delete invTransform;
        invTransform = NULL;
    }
    if (transform != NULL)
    {
        delete transform;
        transform = NULL;
    }
    if (plotInfoAtts != NULL)
    {
        delete plotInfoAtts;
        plotInfoAtts = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::Print
//
//  Purpose:
//      Prints out the contents of the data attributes.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Mar 24 09:18:39 PDT 2004
//    Fixed for Windows compiler.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Print containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Print VarInfo's treatAsASCII. 
//
//    Brad Whitlock, Wed Jul 21 15:25:11 PST 2004
//    Added variable units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added vector of bools for which data selections were applied
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
//    Hank Childs, Thu Aug  4 11:40:24 PDT 2005
//    Print out vartype, subnames.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick.
//
//    Jeremy Meredith, Thu Aug 25 11:07:11 PDT 2005
//    Added group origin.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Added fullDBName.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Hank Childs, Fri Jan 12 13:00:31 PST 2007
//    Added binRange.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 08:48:40 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new axis array window mode.
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

void
avtDataAttributes::Print(ostream &out)
{
    out << "Spatial dimension = " << spatialDimension << endl;
    out << "Topological dimension = " << topologicalDimension << endl;
    out << "Cell origin = " << cellOrigin << endl;
    out << "Node origin = " << nodeOrigin << endl;
    out << "Block origin = " << blockOrigin << endl;
    out << "Group origin = " << groupOrigin << endl;
    if (!timeIsAccurate)
        out << "Time is not known. Suspected to be " << dtime << endl;
    else
        out << "Time = " << dtime << endl;
    if (!cycleIsAccurate)
        out << "Cycle is not known. Suspected to be " << cycle << endl;
    else
        out << "Cycle = " << cycle << endl;

    if (containsOriginalCells)
        out << "This dataset contains the original cells list." << endl;
    else
        out << "This dataset's connectivity has changed." << endl;

    if (containsOriginalNodes)
        out << "This dataset contains the original nodes list." << endl;

    if (containsGlobalZoneIds)
        out << "This dataset contains the global zones list." << endl;

    if (containsGlobalNodeIds)
        out << "This dataset contains the global nodes list." << endl;

    switch (containsGhostZones)
    {
      case AVT_NO_GHOSTS:
        out << "There are no ghost zones in this dataset." << endl;
        break;
      case AVT_HAS_GHOSTS:
        out << "There are ghost zones in this dataset." << endl;
        break;
      case AVT_CREATED_GHOSTS:
        out << "There are generated ghost zones in this dataset." << endl;
        break;
      case AVT_MAYBE_GHOSTS:
        out << "There maybe ghost zones in this dataset." << endl;
        break;
    }
    if (containsExteriorBoundaryGhosts)
        out << "There are ghost zones on the exterior of the boundary." <<endl;

    if (!canUseInvTransform)
        out << "An operation has been performed on this data that prevents "
            << "using an invTransform" << endl;
    if (invTransform != NULL)
        out << invTransform << endl;

    if (!canUseTransform)
        out << "An operation has been performed on this data that prevents "
            << "using a transform" << endl;
    if (transform != NULL)
        out << transform << endl;

    if (canUseCumulativeAsTrueOrCurrent)
        out << "The cumulative extents can be used as true or current" << endl;
    else
        out << "The cumulative extents can not be used as true or current" 
            << endl;

    switch (windowMode)
    {
      case WINMODE_2D:
        out << "The window mode is 2D" << endl;
        break;
      case WINMODE_3D:
        out << "The window mode is 3D" << endl;
        break;
      case WINMODE_CURVE:
        out << "The window mode is curve" << endl;
        break;
      case WINMODE_AXISARRAY:
        out << "The window mode is axis-array" << endl;
        break;
      case WINMODE_NONE:
        out << "The window mode is none" << endl;
        break;
    }
    if (adaptsToAnyWindowMode)
        out << "This plot can adapt to any window mode" << endl;

    if (labels.size() > 0)
    {
        out << "The labels are: ";
        for (int i = 0 ; i < labels.size() ; i++)
        {
            out << labels[i].c_str();
            if (i < labels.size()-1)
               out << ", ";
        }
        out << endl;
    }

    out << "The mesh's name is " << meshname.c_str() << endl;
    out << "The filename is " << filename.c_str() << endl;
    out << "The full db name is " << fullDBName.c_str() << endl;
    out << "The X-units are " << xUnits.c_str() << endl;
    out << "The Y-units are " << yUnits.c_str() << endl;
    out << "The Z-units are " << zUnits.c_str() << endl;
    out << "The X-labels are " << xLabel.c_str() << endl;
    out << "The Y-labels are " << yLabel.c_str() << endl;
    out << "The Z-labels are " << zLabel.c_str() << endl;

    if (trueSpatial != NULL)
    {
        out << "True spatial = " << endl;
        trueSpatial->Print(out);
    }
    if (cumulativeTrueSpatial != NULL)
    {
        out << "Cumulative true spatial = " << endl;
        cumulativeTrueSpatial->Print(out);
    }
    if (effectiveSpatial != NULL)
    {
        out << "Effective spatial = " << endl;
        effectiveSpatial->Print(out);
    }
    if (currentSpatial != NULL)
    {
        out << "Current spatial = " << endl;
        currentSpatial->Print(out);
    }
    if (cumulativeCurrentSpatial != NULL)
    {
        out << "Cumulative current spatial = " << endl;
        cumulativeCurrentSpatial->Print(out);
    }

    int i;
    for (i = 0 ; i < variables.size() ; i++)
    {
        out << "Variable = " << variables[i].varname.c_str() << endl;
        out << "Variable type = ";
        switch (variables[i].vartype)
        {
          case AVT_MESH:
            out << "mesh";
            break;
          case AVT_SCALAR_VAR:
            out << "scalar";
            break;
          case AVT_VECTOR_VAR:
            out << "vector";
            break;
          case AVT_TENSOR_VAR:
            out << "tensor";
            break;
          case AVT_SYMMETRIC_TENSOR_VAR:
            out << "symmetric tensor";
            break;
          case AVT_ARRAY_VAR:
            out << "array";
            break;
          case AVT_LABEL_VAR:
            out << "label";
            break;
          case AVT_MATERIAL:
            out << "material";
            break;
          case AVT_MATSPECIES:
            out << "species";
            break;
          case AVT_CURVE:
            out << "curve";
            break;
          case AVT_UNKNOWN_TYPE:
          default:
            out << "unknown";
            break;
        }
        out << endl;
        if (variables[i].subnames.size() != 0)
        {
            out << "Variable subnames = " << endl;
            for (int j = 0 ; j < variables[i].subnames.size() ; j++)
            {
                out << variables[i].subnames[j].c_str();
                if (j < variables[i].subnames.size()-1)
                    out << ", ";
            }
            out << endl;
        }
        if (variables[i].binRange.size() != 0)
        {
            out << "Bin ranges = " << endl;
            for (int j = 0 ; j < variables[i].binRange.size() ; j++)
            {
                out << variables[i].binRange[j];
                if (j < variables[i].binRange.size()-1)
                    out << ", ";
            }
            out << endl;
        }
        if(variables[i].varunits != "")
            out << "Units = " << variables[i].varunits.c_str() << endl;
        out << "Dimension = " << variables[i].dimension << endl;
        switch (variables[i].centering)
        {
          case AVT_NODECENT:
            out << "Centering is nodal." << endl;
            break;
          case AVT_ZONECENT:
            out << "Centering is zonal." << endl;
            break;
          case AVT_NO_VARIABLE:
            out << "Centering is <no-variable> ... this shouldn't happen." 
                << endl;
            break;
          case AVT_UNKNOWN_CENT:
            out << "Centering is unknown." << endl;
            break;
        }
        if (variables[i].treatAsASCII)
            out << "Treat as ASCII." << endl;

        out << "Used for axis " << variables[i].useForAxis << endl;

        if (variables[i].trueData != NULL)
        {
            out << "True data = " << endl;
            variables[i].trueData->Print(out);
        }
        if (variables[i].cumulativeTrueData != NULL)
        {
            out << "Cumulative true data = " << endl;
            variables[i].cumulativeTrueData->Print(out);
        }
        if (variables[i].effectiveData != NULL)
        {
            out << "Effective data = " << endl;
            variables[i].effectiveData->Print(out);
        }
        if (variables[i].currentData != NULL)
        {
            out << "Current data = " << endl;
            variables[i].currentData->Print(out);
        }
        if (variables[i].cumulativeCurrentData != NULL)
        {
            out << "Cumulative current data = " << endl;
            variables[i].cumulativeCurrentData->Print(out);
        }
        if (variables[i].componentExtents != NULL)
        {
            out << "Component extents = " << endl;
            variables[i].componentExtents->Print(out);
        }
    }

    out << "Selections Applied: ";
    for (i = 0; i < selectionsApplied.size(); i++)
        out << (selectionsApplied[i] ? "T " : "F ");
    out << endl;

    out << "Num states: " << numStates << endl;
    if (mirOccurred)
        out << "Material Interace Reconstruction occurred. " << endl;
    if (canUseOrigZones)
        out << "Original Zones cannot be used for Pick." << endl;
    if (origElementsRequiredForPick)
        out << "Original Nodes are required for Pick." << endl;

    switch (meshCoordType)
    {
      case AVT_XY:
        out << "The mesh coord type is XY " << endl;
        break; 
      case AVT_RZ:
        out << "The mesh coord type is RZ " << endl;
        break; 
      case AVT_ZR:
        out << "The mesh coord type is ZR " << endl;
        break; 
    }
    switch (meshType)
    {
      case AVT_RECTILINEAR_MESH:
        out << "The mesh type is RECTILINEAR " << endl;
        break; 
      case AVT_CURVILINEAR_MESH:
        out << "The mesh type is CURVILINEAR " << endl;
        break; 
      case AVT_UNSTRUCTURED_MESH:
        out << "The mesh type is UNSTRUCTURED " << endl;
        break; 
      case AVT_POINT_MESH:
        out << "The mesh type is POINT " << endl;
        break; 
      case AVT_SURFACE_MESH:
        out << "The mesh type is SURFACE " << endl;
        break; 
      case AVT_CSG_MESH:
        out << "The mesh type is CSG " << endl;
        break; 
      case AVT_AMR_MESH:
        out << "The mesh type is AMR " << endl;
        break; 
      case AVT_UNKNOWN_MESH:
        out << "The mesh type is UNKNOWN " << endl;
        break; 
    }

    if (nodesAreCritical)
    {
        out << "This mesh is primarily a cell-based mesh." << endl;
    }
    else
    {
        out << "This mesh is primarily a point-based mesh." << endl;
    }

    for (i=0; i<3; i++)
    {
        out << "Unit cell vector #"<<i<<" is "
            << unitCellVectors[i*3+0] << " "
            << unitCellVectors[i*3+1] << " "
            << unitCellVectors[i*3+2] << endl;
    }

    out << "Rectilinear grids "
        << (rectilinearGridHasTransform ? "do" : "do not")
        << " have an implicit transform." << endl;
    if (rectilinearGridHasTransform)
    {
        out << "The rectilinear grid transform is:" << endl;
        for (i=0; i<4; i++)
        {
            for (int j=0; j<4; j++)
            {
                out << "  " << rectilinearGridTransform[i*4+j];
            }
            out << endl;
        }
    }

    out << "PlotInfoAttributes: ";
    if (plotInfoAtts == NULL)
        out << "Not Set";
    else 
        plotInfoAtts->PrintSelf(out);
    out << endl;
}


// ****************************************************************************
//  Method: avtDataAttributes::Copy
//
//  Purpose:
//      Copies the data attributes from the argument to this object.
//
//  Arguments:
//      di      Data attributes from another object.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Wed Sep 19 14:54:15 PDT 2001 
//    Copy over labels. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Copy over current extents. 
//
//    Hank Childs, Fri Dec 21 07:18:33 PST 2001
//    Copy over varname.
//
//    Hank Childs, Fri Mar  8 16:39:30 PST 2002
//    Copy over cycle, dtime, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Copy cellOrigin and blockOrigin.
//
//    Hank Childs, Thu Sep 26 21:55:02 PDT 2002
//    Copy units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Copy containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Copy containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Copy transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Copy windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Copy labels.
//
//    Hank Childs, Mon Feb 23 09:18:58 PST 2004
//    Copy all variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Copy containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Copy varinfo's treatAsASCII.
//
//    Brad Whitlock, Tue Jul 20 14:02:32 PST 2004
//    Copied variable units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
//    Hank Childs, Thu Aug  4 13:27:43 PDT 2005
//    Added vartype, subnames.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Added fullDBName.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Hank Childs, Fri Jan 12 13:00:31 PST 2007
//    Added binRanges.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 10:20:04 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

void
avtDataAttributes::Copy(const avtDataAttributes &di)
{
    DestructSelf();

    SetTopologicalDimension(di.topologicalDimension);
    SetSpatialDimension(di.spatialDimension);

    SetCellOrigin(di.cellOrigin);
    SetNodeOrigin(di.nodeOrigin);
    SetBlockOrigin(di.blockOrigin);
    SetGroupOrigin(di.groupOrigin);
    if (di.cycleIsAccurate)
    {
        SetCycle(di.cycle);
    }
    else
    {
        cycleIsAccurate = false;
    }
    if (di.timeIsAccurate)
    {
        SetTime(di.dtime);
    }
    else
    {
        timeIsAccurate = false;
    }

    SetMeshname(di.GetMeshname());
    SetFilename(di.GetFilename());
    SetFullDBName(di.GetFullDBName());
    SetXUnits(di.GetXUnits());
    SetYUnits(di.GetYUnits());
    SetZUnits(di.GetZUnits());
    SetXLabel(di.GetXLabel());
    SetYLabel(di.GetYLabel());
    SetZLabel(di.GetZLabel());

    *(trueSpatial)               = *(di.trueSpatial);
    *(cumulativeTrueSpatial)     = *(di.cumulativeTrueSpatial);
    *(effectiveSpatial)          = *(di.effectiveSpatial);
    *(currentSpatial)            = *(di.currentSpatial);
    *(cumulativeCurrentSpatial)  = *(di.cumulativeCurrentSpatial);

    canUseCumulativeAsTrueOrCurrent = di.canUseCumulativeAsTrueOrCurrent;
    for (int i = 0 ; i < di.variables.size() ; i++)
    {
        const char *vname = di.variables[i].varname.c_str();
        AddVariable(vname, di.variables[i].varunits);
        SetVariableType(di.variables[i].vartype, vname);
        SetVariableSubnames(di.variables[i].subnames, vname);
        SetVariableBinRanges(di.variables[i].binRange, vname);
        SetVariableDimension(di.variables[i].dimension, vname);
        SetCentering(di.variables[i].centering, vname);
        SetTreatAsASCII(di.variables[i].treatAsASCII, vname);
        SetUseForAxis(di.variables[i].useForAxis, vname);
        *(variables[i].trueData)              = *(di.variables[i].trueData);
        *(variables[i].cumulativeTrueData)    = 
                                      *(di.variables[i].cumulativeTrueData);
        *(variables[i].effectiveData)         =
                                      *(di.variables[i].effectiveData);
        *(variables[i].currentData)           = *(di.variables[i].currentData);
        *(variables[i].cumulativeCurrentData) = 
                                      *(di.variables[i].cumulativeCurrentData);
        *(variables[i].componentExtents) = *(di.variables[i].componentExtents);
    }
    activeVariable = di.activeVariable;

    labels = di.labels;
    SetContainsGhostZones(di.GetContainsGhostZones());
    SetContainsExteriorBoundaryGhosts(di.GetContainsExteriorBoundaryGhosts());
    SetContainsOriginalCells(di.GetContainsOriginalCells());
    SetContainsOriginalNodes(di.GetContainsOriginalNodes());
    SetKeepNodeZoneArrays(di.GetKeepNodeZoneArrays());
    SetContainsGlobalZoneIds(di.GetContainsGlobalZoneIds());
    SetContainsGlobalNodeIds(di.GetContainsGlobalNodeIds());
    CopyInvTransform(di.invTransform);
    canUseInvTransform = di.canUseInvTransform;
    CopyTransform(di.transform);
    canUseTransform = di.canUseTransform;
    windowMode = di.windowMode;
    adaptsToAnyWindowMode = di.adaptsToAnyWindowMode;
    selectionsApplied = di.selectionsApplied;
    numStates = di.numStates;
    mirOccurred = di.mirOccurred;
    canUseOrigZones = di.canUseOrigZones;
    origElementsRequiredForPick = di.origElementsRequiredForPick;
    meshCoordType = di.meshCoordType;
    meshType = di.meshType;
    nodesAreCritical = di.nodesAreCritical;
    for (int j=0; j<9; j++)
        unitCellVectors[j] = di.unitCellVectors[j];
    rectilinearGridHasTransform = di.rectilinearGridHasTransform;
    for (int k=0; k<16; k++)
        rectilinearGridTransform[k] = di.rectilinearGridTransform[k];
    SetPlotInfoAtts(di.plotInfoAtts);
}


// ****************************************************************************
//  Method: avtDataAttributes::Merge
//
//  Purpose:
//      Merges two dataset attributes.
//
//  Arguments:
//      da      The data attributes to merge in this object.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Wed Sep 19 14:54:15 PDT 2001 
//    Call MergeLabels method. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Merge current extents. 
//
//    Hank Childs, Fri Mar  8 16:39:30 PST 2002
//    Merge cycle and dtime.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Merge cellOrigin and blockOrigin.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Merge containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Merge containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Merge transform.
//
//    Kathleen Bonnell, Wed Jun 18 17:50:20 PDT 2003 
//    Account for avtGhostType AVT_CREATED_GHOSTS. 
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Merge windowMode.
//
//    Mark C. Miller, Thu Jan 29 19:40:25 PST 2004
//    Added bools to ignore this or that. We might need to ignore this or that
//    if in parallel, we can't guarentee that one of the two operands to the
//    Merge operator has valid data members for ALL of them. Right now, the
//    only attribute that is 'ignored' is variable dimension. If we discover
//    that other circumstances arise other attribute data members can't be
//    guraenteed to be valid, we can add more logic here. Added the
//    support macro, CHECK_OR_IGNORE()
// 
//    Hank Childs, Mon Feb 23 09:18:58 PST 2004
//    Account for multiple variables.  Remove CHECK_OR_IGNORE, since the
//    logic got too complex for multiple variables.  Note that this is a
//    broader change than what Mark made on 1/04.  I have spoken with Mark and
//    he believes it is a reasonable thing to do.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Merge containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Test for equivalent treatAsASCII values.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
//    Hank Childs, Thu Aug  4 11:09:38 PDT 2005
//    Added cast to int, since size_t is not automatically cast to int with
//    presence of new InvalidMergeException signature for doubles.  Also add
//    support for vartypes and subnames.
//
//    Hank Childs, Thu Aug  4 13:27:43 PDT 2005
//    Added vartype, subnames.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick.
//
//    Jeremy Meredith, Thu Aug 25 11:07:42 PDT 2005
//    Added groupOrigin.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 10:20:04 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

void
avtDataAttributes::Merge(const avtDataAttributes &da,
   bool ignoreThis, bool ignoreThat)
{
    int   i;

    if (ignoreThat)
    {
        //
        // The argument da is not valid.  So we are going to stay with what
        // we have.  Just return.
        //
        return;
    }
    if (ignoreThis)
    {
        //
        // We know that the argument da is valid.  In addition, we are not
        // valid.  So just copy 'da' and return.
        //
        Copy(da);
        return;
    }

    //
    // From here on, we can assume that both the 'this' object and the 'da'
    // object are valid.  If not, we would have already returned.
    //

    if (variables.size() != da.variables.size())
    {
        EXCEPTION2(InvalidMergeException, (int) variables.size(), 
                   (int) da.variables.size());
    }
    for (i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname != da.variables[i].varname)
        {
            EXCEPTION0(InvalidMergeException);
        }
        if (variables[i].vartype != da.variables[i].vartype)
        {
            EXCEPTION2(InvalidMergeException, variables[i].vartype,
                       da.variables[i].vartype);
        }
        if (variables[i].centering != da.variables[i].centering)
        {
            EXCEPTION2(InvalidMergeException, variables[i].centering,
                       da.variables[i].centering);
        }
        if (variables[i].treatAsASCII != da.variables[i].treatAsASCII)
        {
            EXCEPTION0(InvalidMergeException);
        }
        if (variables[i].useForAxis != da.variables[i].useForAxis)
        {
            EXCEPTION0(InvalidMergeException);
        }
        if (variables[i].dimension != da.variables[i].dimension)
        {
            EXCEPTION2(InvalidMergeException, variables[i].dimension,
                       da.variables[i].dimension);
        }
        if (variables[i].subnames.size() != da.variables[i].subnames.size())
        {
            EXCEPTION2(InvalidMergeException, 
                       (int) variables[i].subnames.size(),
                       (int) da.variables[i].subnames.size());
        }
        for (int j = 0 ; j < variables[i].subnames.size() ; j++)
            if (variables[i].subnames[j] != da.variables[i].subnames[j])
            {
                EXCEPTION0(InvalidMergeException);
                EXCEPTION0(InvalidMergeException);
            }
        if (variables[i].binRange.size() != da.variables[i].binRange.size())
        {
            EXCEPTION2(InvalidMergeException, 
                       (int) variables[i].binRange.size(),
                       (int) da.variables[i].binRange.size());
        }
        for (int j = 0 ; j < variables[i].binRange.size() ; j++)
            if (variables[i].binRange[j] != da.variables[i].binRange[j])
            {
                EXCEPTION0(InvalidMergeException);
            }
    }
    if (activeVariable != da.activeVariable)
    {
        EXCEPTION2(InvalidMergeException, activeVariable, da.activeVariable);
    }

    if (cellOrigin != da.cellOrigin)
    {
        EXCEPTION2(InvalidMergeException, cellOrigin, da.cellOrigin);
    }
    if (nodeOrigin != da.nodeOrigin)
    {
        EXCEPTION2(InvalidMergeException, nodeOrigin, da.nodeOrigin);
    }
    if (blockOrigin != da.blockOrigin)
    {
        EXCEPTION2(InvalidMergeException, blockOrigin, da.blockOrigin);
    }
    if (groupOrigin != da.groupOrigin)
    {
        EXCEPTION2(InvalidMergeException, groupOrigin, da.groupOrigin);
    }

    if (spatialDimension != da.spatialDimension)
    {
        EXCEPTION2(InvalidMergeException, spatialDimension,
                   da.spatialDimension);
    }
    if (topologicalDimension != da.topologicalDimension)
    {
        EXCEPTION2(InvalidMergeException, topologicalDimension,
                   da.topologicalDimension);
    }
    if (cycleIsAccurate && da.cycleIsAccurate && cycle != da.cycle)
    {
        EXCEPTION2(InvalidMergeException, cycle, da.cycle);
    }
    if (timeIsAccurate && da.timeIsAccurate && dtime != da.dtime)
    {
        EXCEPTION2(InvalidMergeException, dtime, da.dtime);
    }
    if (windowMode != da.windowMode)
    {
        EXCEPTION2(InvalidMergeException, windowMode, da.windowMode);
    }
    if (adaptsToAnyWindowMode != da.adaptsToAnyWindowMode)
    {
        EXCEPTION2(InvalidMergeException, adaptsToAnyWindowMode, 
                                          da.adaptsToAnyWindowMode);
    }
    if (numStates != da.numStates)
    {
        EXCEPTION2(InvalidMergeException, numStates, da.numStates);
    }

    if (selectionsApplied.size() != da.selectionsApplied.size())
    {
        EXCEPTION2(InvalidMergeException, (int) selectionsApplied.size(),
                                        (int) da.selectionsApplied.size());
    }
    else
    {
        for (int i = 0; i < selectionsApplied.size(); i++)
        {
            if (selectionsApplied[i] != da.selectionsApplied[i])
            {
                EXCEPTION2(InvalidMergeException, selectionsApplied[i], 
                                               da.selectionsApplied[i]);
            }
        }
    }
    if (meshCoordType != da.meshCoordType)
    {
        EXCEPTION2(InvalidMergeException, meshCoordType, da.meshCoordType);
    }
    if (meshType != da.meshType)
    {
        EXCEPTION2(InvalidMergeException, meshType, da.meshType);
    }

    if (nodesAreCritical != da.nodesAreCritical)
    {
        EXCEPTION2(InvalidMergeException,nodesAreCritical,da.nodesAreCritical);
    }

    if (rectilinearGridHasTransform != da.rectilinearGridHasTransform)
    {
        EXCEPTION2(InvalidMergeException,
                   rectilinearGridHasTransform,
                   da.rectilinearGridHasTransform);
    }

    if (GetContainsGhostZones() == AVT_MAYBE_GHOSTS)
    {
        if (da.GetContainsGhostZones() == AVT_HAS_GHOSTS)
        {
            SetContainsGhostZones(AVT_HAS_GHOSTS);
        }
        else if (da.GetContainsGhostZones() == AVT_CREATED_GHOSTS)
        {
            SetContainsGhostZones(AVT_CREATED_GHOSTS);
        }
    }
    else if (GetContainsGhostZones() == AVT_NO_GHOSTS)
    {
        SetContainsGhostZones(da.GetContainsGhostZones());
    }

    if (!GetContainsExteriorBoundaryGhosts())
    {
        SetContainsExteriorBoundaryGhosts(
                                       da.GetContainsExteriorBoundaryGhosts());
    }

    if (!GetContainsOriginalCells()) 
    {
        SetContainsOriginalCells(da.GetContainsOriginalCells());
    }
    if (!GetContainsOriginalNodes()) 
    {
        SetContainsOriginalNodes(da.GetContainsOriginalNodes());
    }
    if (!GetKeepNodeZoneArrays()) 
    {
        SetKeepNodeZoneArrays(da.GetKeepNodeZoneArrays());
    }
    if (!GetContainsGlobalZoneIds()) 
    {
        SetContainsGlobalZoneIds(da.GetContainsGlobalZoneIds());
    }
    if (!GetContainsGlobalNodeIds()) 
    {
        SetContainsGlobalNodeIds(da.GetContainsGlobalNodeIds());
    }


    canUseCumulativeAsTrueOrCurrent &= da.canUseCumulativeAsTrueOrCurrent;

    trueSpatial->Merge(*(da.trueSpatial));
    cumulativeTrueSpatial->Merge(*(da.cumulativeTrueSpatial));
    effectiveSpatial->Merge(*(da.effectiveSpatial));
    currentSpatial->Merge(*(da.currentSpatial));
    cumulativeCurrentSpatial->Merge(*(da.cumulativeCurrentSpatial));

    for (i = 0 ; i < variables.size() ; i++)
    {
        variables[i].trueData->Merge(*(da.variables[i].trueData));
        variables[i].cumulativeTrueData->Merge(
                                        *(da.variables[i].cumulativeTrueData));
        variables[i].effectiveData->Merge(*(da.variables[i].effectiveData));
        variables[i].currentData->Merge(*(da.variables[i].currentData));
        variables[i].cumulativeCurrentData->Merge(
                                     *(da.variables[i].cumulativeCurrentData));
        variables[i].componentExtents->Merge(
                                        *(da.variables[i].componentExtents));
    }

    MergeLabels(da.labels);
    MergeInvTransform(da.invTransform);
    MergeTransform(da.transform);
    canUseInvTransform &= da.canUseInvTransform;
    canUseTransform &= da.canUseTransform;
    mirOccurred |= da.mirOccurred;
    canUseOrigZones &= da.canUseOrigZones;
    origElementsRequiredForPick |= da.origElementsRequiredForPick;
    SetPlotInfoAtts(da.plotInfoAtts);

    // there's no good answer for unitCellVectors or rectilinearGridTransform
}


// ****************************************************************************
//  Method: avtDataAttributes::GetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 12:18:16 PST 2001
//    Don't use effective spatial extents.
//
// ****************************************************************************

bool
avtDataAttributes::GetSpatialExtents(double *buff)
{
    if (trueSpatial->HasExtents())
    {
        trueSpatial->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        cumulativeTrueSpatial->HasExtents())
    {
        cumulativeTrueSpatial->CopyTo(buff);
        return true;
    }

    return false;
}

// ****************************************************************************
//  Method: avtDataAttributes::GetAnySpatialExtents
//
//  Purpose:
//      Gets any spatial extents for the data object if any exists starting
//      with most useful to least useful
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Mark C. Miller
//  Creation:    February 29, 2004
//
// ****************************************************************************

bool
avtDataAttributes::GetAnySpatialExtents(double *buff)
{
    if (trueSpatial->HasExtents())
    {
        trueSpatial->CopyTo(buff);
        return true;
    }

    if (cumulativeTrueSpatial->HasExtents())
    {
        cumulativeTrueSpatial->CopyTo(buff);
        return true;
    }

    if (currentSpatial->HasExtents())
    {
        currentSpatial->CopyTo(buff);
        return true;
    }

    if (cumulativeCurrentSpatial->HasExtents())
    {
        cumulativeCurrentSpatial->CopyTo(buff);
        return true;
    }

    if (effectiveSpatial->HasExtents())
    {
        effectiveSpatial->CopyTo(buff);
        return true;
    }

    return false;
}

// ****************************************************************************
//  Method: avtDataAttributes::GetDataExtents
//
//  Purpose:
//      Gets the data extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 12:18:16 PST 2001
//    Don't use effective data extents.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
// ****************************************************************************

bool
avtDataAttributes::GetDataExtents(double *buff, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
        return false;

    if (variables[index].trueData->HasExtents())
    {
        variables[index].trueData->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        variables[index].cumulativeTrueData->HasExtents())
    {
        variables[index].cumulativeTrueData->CopyTo(buff);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetTrueDataExtents
//
//  Purpose:
//      Gets the true data extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetTrueDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].trueData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCumulativeTrueDataExtents
//
//  Purpose:
//      Gets the cumulative true data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetCumulativeTrueDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].cumulativeTrueData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableComponentExtents
//
//  Purpose:
//      Gets the component extents for an array variable.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  7, 2008
//
//  Modifications:
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetVariableComponentExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].componentExtents;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetEffectiveDataExtents
//
//  Purpose:
//      Gets the effective data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetEffectiveDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].effectiveData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCurrentDataExtents
//
//  Purpose:
//      Gets the current data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetCurrentDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].currentData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCumulativeCurrentDataExtents
//
//  Purpose:
//      Gets the cumulative current data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetCumulativeCurrentDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].cumulativeCurrentData;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTopologicalDimension
//
//  Purpose:
//      Sets the topological dimension for the mesh.
//
//  Arguments:
//      td       The new topological dimension.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

void
avtDataAttributes::SetTopologicalDimension(int td)
{
    topologicalDimension = td;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetSpatialDimension
//
//  Purpose:
//      Sets the spatial dimension for the mesh.
//
//  Arguments:
//      td       The new spatial dimension.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Oct  3 10:57:13 PDT 2001
//    Add currentSpatial, cumulativeCurrentSpatial.
//
// ****************************************************************************

void
avtDataAttributes::SetSpatialDimension(int td)
{
    if (td == spatialDimension)
    {
        return;
    }

    spatialDimension = td;

    if (trueSpatial != NULL)
    {
        delete trueSpatial;
    }
    trueSpatial = new avtExtents(spatialDimension);

    if (cumulativeTrueSpatial != NULL)
    {
        delete cumulativeTrueSpatial;
    }
    cumulativeTrueSpatial = new avtExtents(spatialDimension);

    if (effectiveSpatial != NULL)
    {
        delete effectiveSpatial;
    }
    effectiveSpatial = new avtExtents(spatialDimension);

    if (currentSpatial != NULL)
    {
        delete currentSpatial;
    }
    currentSpatial = new avtExtents(spatialDimension);

    if (cumulativeCurrentSpatial != NULL)
    {
        delete cumulativeCurrentSpatial;
    }
    cumulativeCurrentSpatial = new avtExtents(spatialDimension);
}


// ****************************************************************************
//  Method: avtDataAttributes::SetVariableDimension
//
//  Purpose:
//      Sets the variable dimension.
//
//  Arguments:
//      vd       The new variable dimension.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Oct  3 10:57:13 PDT 2001
//    Add currentData, cumulativeCurrentData.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 10:32:04 PST 2004 
//    DataExtents now always have dimension of 1. 
//
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

void
avtDataAttributes::SetVariableDimension(int vd, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set dimension of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    if (vd == variables[index].dimension)
    {
        return;
    }

    variables[index].dimension  = vd;

    if (variables[index].trueData != NULL)
    {
        delete variables[index].trueData;
    }
    variables[index].trueData = new avtExtents(1);

    if (variables[index].cumulativeTrueData != NULL)
    {
        delete variables[index].cumulativeTrueData;
    }
    variables[index].cumulativeTrueData = new avtExtents(1);

    if (variables[index].effectiveData != NULL)
    {
        delete variables[index].effectiveData;
    }
    variables[index].effectiveData =new avtExtents(1);

    if (variables[index].currentData != NULL)
    {
        delete variables[index].currentData;
    }
    variables[index].currentData = new avtExtents(1);

    if (variables[index].cumulativeCurrentData != NULL)
    {
        delete variables[index].cumulativeCurrentData;
    }
    variables[index].cumulativeCurrentData = new avtExtents(1);

    if (variables[index].componentExtents != NULL)
    {
        delete variables[index].componentExtents;
    }
    variables[index].componentExtents = new avtExtents(vd);
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableDimension
//
//  Purpose:
//      Gets the variable dimension of a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

int
avtDataAttributes::GetVariableDimension(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve dimension of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].dimension;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetCentering
//
//  Purpose:
//      Sets the centering type.
//
//  Arguments:
//      cen        The new centering type.
//
//  Programmer:    Hank Childs
//  Creation:      March 19, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Fri Oct 22 13:57:58 PDT 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

void
avtDataAttributes::SetCentering(avtCentering cen, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the centering of a non-existent variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set centering of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].centering = cen;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCentering
//
//  Purpose:
//      Gets the variable centering of a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Fri Oct 22 13:57:58 PDT 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtCentering
avtDataAttributes::GetCentering(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to get the centering of a non-existent variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve centering of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].centering;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetVariableType
//
//  Purpose:
//      Sets the variable type.
//
//  Arguments:
//      vt         The new variable type.
//
//  Programmer:    Hank Childs
//  Creation:      August 4, 2005
//
// ****************************************************************************

void
avtDataAttributes::SetVariableType(avtVarType vt, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable type of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set variable type of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].vartype = vt;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableType
//
//  Purpose:
//      Gets the variable type of a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2005
//
// ****************************************************************************

avtVarType
avtDataAttributes::GetVariableType(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable type of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve variable type of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].vartype;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetVariableSubnames
//
//  Purpose:
//      Sets the variable subnames.
//
//  Arguments:
//      sn         The new subnames.
//
//  Programmer:    Hank Childs
//  Creation:      August 4, 2005
//
// ****************************************************************************

void
avtDataAttributes::SetVariableSubnames(const std::vector<std::string> &sn,
                                       const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable type of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set variable subnames of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].subnames = sn;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableSubnames
//
//  Purpose:
//      Gets the variable subnames of a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2005
//
// ****************************************************************************

const std::vector<std::string> &
avtDataAttributes::GetVariableSubnames(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable type of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve variable subnames of non-";
        reason = reason +  "existent variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].subnames;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetVariableBinRanges
//
//  Purpose:
//      Sets the variable bin ranges.
//
//  Arguments:
//      bn         The new bin ranges.
//
//  Programmer:    Hank Childs
//  Creation:      January 12, 2007
//
// ****************************************************************************

void
avtDataAttributes::SetVariableBinRanges(const std::vector<double> &bn,
                                       const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable type of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set bin ranges of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].binRange = bn;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableBinRanges
//
//  Purpose:
//      Gets the variable bin ranges for a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   January 12, 2007
//
// ****************************************************************************

const std::vector<double> &
avtDataAttributes::GetVariableBinRanges(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable type of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve variable bin ranges of non-";
        reason = reason +  "existent variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].binRange;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetCellOrigin
//
//  Purpose:
//      Sets the cell origin.
//
//  Arguments:
//      origin     The new cell origin.
//
//  Programmer:    Hank Childs
//  Creation:      June 16, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetCellOrigin(int origin)
{
    cellOrigin = origin;
}

void
avtDataAttributes::SetNodeOrigin(int origin)
{
    nodeOrigin = origin;
}

// ****************************************************************************
//  Method: avtDataAttributes::SetBlockOrigin
//
//  Purpose:
//      Sets the block origin.
//
//  Arguments:
//      origin     The new block origin.
//
//  Programmer:    Hank Childs
//  Creation:      June 16, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetBlockOrigin(int origin)
{
    blockOrigin = origin;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetGroupOrigin
//
//  Purpose:
//      Sets the group origin.
//
//  Arguments:
//      origin     The new group origin.
//
//  Programmer:    Jeremy Meredith
//  Creation:      August 25, 2005
//
// ****************************************************************************

void
avtDataAttributes::SetGroupOrigin(int origin)
{
    groupOrigin = origin;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetCycle
//
//  Purpose:
//      Sets the cycle number.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetCycle(int c)
{
    cycle = c;
    cycleIsAccurate = true;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTime
//
//  Purpose:
//      Sets the problem time.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetTime(double d)
{
    dtime = d;
    timeIsAccurate = true;
}


// ****************************************************************************
//  Method: avtDataAttributes::Write
//
//  Purpose:
//      Writes the data attributes object out to a stream (string).
//
//  Arguments:
//      str      The string to write to.
//      wrtr     The writer that has information/methods about the destination
//               format.
//
//  Progammer: Hank Childs
//  Creation:  March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sat May 26 10:06:58 PDT 2001
//    Made use of avtDataObjectString::Append instead of string::append to
//    prevent bottleneck.
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Hank Childs, Wed Sep 19 10:24:02 PDT 2001
//    Make use of new array writing facilities.
//
//    Kathleen Bonnell, Thu Sep 20 14:24 PDT 2001 
//    Call WriteLabels method. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Write current extents. 
//
//    Hank Childs, Fri Mar  8 17:02:10 PST 2002
//    Write the cycle, time, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Write cellOrigin and blockOrigin.
//
//    Hank Childs, Thu Sep 26 21:55:02 PDT 2002
//    Write units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Write containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Write containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Write transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Write windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Write labels.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Write containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Write treatAsASCII. 
//
//    Brad Whitlock, Tue Jul 20 14:13:37 PST 2004
//    Added units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
//    Hank Childs, Thu Aug  4 13:27:43 PDT 2005
//    Added vartype, subnames.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick.
//
//    Jeremy Meredith, Thu Aug 25 11:09:40 PDT 2005
//    Added group origin.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Added fullDBName.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Hank Childs, Fri Jan 12 13:00:31 PST 2007
//    Added binRanges.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin. Re-wrote code to be a little easier to manage
//    by using i++ indexing rather than literal ints and basei in the for
//    loop for variables.
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 10:20:04 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
//    Kathleen Bonnell, Tue Feb 12 11:47:08 PST 2008
//    Check for binRange size before writing.
//
// ****************************************************************************

void
avtDataAttributes::Write(avtDataObjectString &str,
                         const avtDataObjectWriter *wrtr)
{
    int   i, j;

    int varSize = 7;
    int numVals = 31 + varSize*variables.size();
    int *vals = new int[numVals];
    i = 0;
    vals[i++] = topologicalDimension;
    vals[i++] = spatialDimension;
    vals[i++] = cellOrigin;
    vals[i++] = nodeOrigin;
    vals[i++] = blockOrigin;
    vals[i++] = groupOrigin;
    vals[i++] = cycle;
    vals[i++] = (cycleIsAccurate ? 1 : 0);
    vals[i++] = (timeIsAccurate ? 1 : 0);
    vals[i++] = (int) containsGhostZones;
    vals[i++] = (int) containsExteriorBoundaryGhosts;
    vals[i++] = (containsOriginalCells ? 1 : 0);
    vals[i++] = (containsOriginalNodes ? 1 : 0);
    vals[i++] = (keepNodeZoneArrays ? 1 : 0);
    vals[i++] = (containsGlobalZoneIds ? 1 : 0);
    vals[i++] = (containsGlobalNodeIds ? 1 : 0);
    vals[i++] = (canUseInvTransform ? 1 : 0);
    vals[i++] = (canUseTransform ? 1 : 0);
    vals[i++] = (canUseCumulativeAsTrueOrCurrent ? 1 : 0);
    vals[i++] = windowMode;
    vals[i++] = (adaptsToAnyWindowMode ? 1 : 0);
    vals[i++] = numStates;
    vals[i++] = mirOccurred;
    vals[i++] = canUseOrigZones;
    vals[i++] = origElementsRequiredForPick;
    vals[i++] = meshCoordType;
    vals[i++] = meshType;
    vals[i++] = (nodesAreCritical ? 1 : 0);
    vals[i++] = (rectilinearGridHasTransform ? 1 : 0);
    vals[i++] = activeVariable;
    vals[i++] = variables.size();
    int basei = i;
    for (i = 0 ; i < variables.size() ; i++)
    {
        vals[basei+varSize*i]   = variables[i].dimension;
        vals[basei+varSize*i+1] = variables[i].centering;
        vals[basei+varSize*i+2] = (variables[i].treatAsASCII ? 1 : 0);
        vals[basei+varSize*i+3] = variables[i].vartype;
        vals[basei+varSize*i+4] = variables[i].subnames.size();
        vals[basei+varSize*i+5] = variables[i].binRange.size();
        vals[basei+varSize*i+6] = variables[i].useForAxis;
    }
    wrtr->WriteInt(str, vals, numVals);
    wrtr->WriteDouble(str, dtime);

    trueSpatial->Write(str, wrtr);
    cumulativeTrueSpatial->Write(str, wrtr);
    effectiveSpatial->Write(str, wrtr);
    currentSpatial->Write(str, wrtr);
    cumulativeCurrentSpatial->Write(str, wrtr);

    for (i = 0 ; i < variables.size() ; i++)
    {
        // Write the variable name
        wrtr->WriteInt(str, variables[i].varname.size());
        str.Append((char *) variables[i].varname.c_str(),
                   variables[i].varname.size(),
                   avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

        // Write the units name.
        int unitlen = variables[i].varunits.size();
        wrtr->WriteInt(str, unitlen);
        if(unitlen > 0)
        {
            str.Append((char *) variables[i].varunits.c_str(), unitlen,
                     avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
        }

        // Write the subnames (if any).  Number of subnames already
        // communicated in mass "int" writing phase.
        for (j = 0 ; j < variables[i].subnames.size() ; j++)
        {
            wrtr->WriteInt(str, variables[i].subnames[j].size());
            str.Append((char *) variables[i].subnames[j].c_str(),
                     variables[i].subnames[j].size(),
                     avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
        }
        // Write the binRanges (if any).  Number of binRanges already
        // communicated in mass "int" writing phase.
        if (variables[i].binRange.size() > 0)
        {
            wrtr->WriteDouble(str, &(variables[i].binRange[0]), 
                              variables[i].binRange.size());
        }
        variables[i].trueData->Write(str, wrtr);
        variables[i].cumulativeTrueData->Write(str, wrtr);
        variables[i].effectiveData->Write(str, wrtr);
        variables[i].currentData->Write(str, wrtr);
        variables[i].cumulativeCurrentData->Write(str, wrtr);
        variables[i].componentExtents->Write(str, wrtr);
    }

    wrtr->WriteInt(str, meshname.size());
    str.Append((char *) meshname.c_str(), meshname.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, filename.size());
    str.Append((char *) filename.c_str(), filename.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, fullDBName.size());
    str.Append((char *) fullDBName.c_str(), fullDBName.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, xUnits.size());
    str.Append((char *) xUnits.c_str(), xUnits.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, yUnits.size());
    str.Append((char *) yUnits.c_str(), yUnits.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, zUnits.size());
    str.Append((char *) zUnits.c_str(), zUnits.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, xLabel.size());
    str.Append((char *) xLabel.c_str(), xLabel.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, yLabel.size());
    str.Append((char *) yLabel.c_str(), yLabel.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, zLabel.size());
    str.Append((char *) zLabel.c_str(), zLabel.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    for (i = 0; i < 9 ; i++)
        wrtr->WriteDouble(str, unitCellVectors[i]);

    for (i = 0; i < 16 ; i++)
        wrtr->WriteDouble(str, rectilinearGridTransform[i]);

    wrtr->WriteInt(str, selectionsApplied.size());
    for (i = 0; i < selectionsApplied.size(); i++)
        wrtr->WriteInt(str, selectionsApplied[i] ? 1 : 0);

    WriteLabels(str, wrtr);
    WriteInvTransform(str, wrtr);
    WriteTransform(str, wrtr);

    WritePlotInfoAtts(str, wrtr);

    delete [] vals;
}


// ****************************************************************************
//  Method: avtDataAttributes::Read
//
//  Purpose:
//      Read the data attributes object from a stream (string).
//
//  Arguments:
//      input  The string to read from.
//
//  Returns:   The amount read.
//
//  Progammer: Hank Childs
//  Creation:  March 25, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Sep 20 14:24 PDT 2001 
//    Call ReadLabels method. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Read current extents. 
//
//    Hank Childs, Fri Mar  8 17:02:10 PST 2002
//    Read the cycle, time, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Read cellOrigin and blockOrigin.
//
//    Hank Childs, Thu Sep 26 21:55:02 PDT 2002
//    Read units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Read containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Read containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Read transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Read windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Read labels.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Read containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Read treatAsASCII. 
//
//    Brad Whitlock, Tue Jul 20 14:03:19 PST 2004
//    Added units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
//    Hank Childs, Thu Mar  3 16:20:49 PST 2005
//    Clear the selectionsApplied array before pushing new entries back.
//
//    Hank Childs, Fri Aug  5 16:19:40 PDT 2005
//    Read variable type and subnames.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick.
//
//    Jeremy Meredith, Thu Aug 25 11:09:34 PDT 2005
//    Added group origin.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Added fullDBName.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Hank Childs, Wed May 24 11:43:23 PDT 2006
//    Check in fix suggested by Jeremy Meredith.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added plotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Hank Childs, Fri Jan 12 13:11:26 PST 2007
//    Added binRange.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 10:20:04 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

int
avtDataAttributes::Read(char *input)
{
    int     i, j;
    int     size = 0;
    int     tmp;
    double  dtmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetTopologicalDimension(tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetSpatialDimension(tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCellOrigin(tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetNodeOrigin(tmp);
  
    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetBlockOrigin(tmp);
  
    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetGroupOrigin(tmp);
  
    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    cycle = tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    cycleIsAccurate = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    timeIsAccurate = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsGhostZones( (avtGhostType) tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsExteriorBoundaryGhosts( (bool) tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsOriginalCells(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsOriginalNodes(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetKeepNodeZoneArrays(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsGlobalZoneIds(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsGlobalNodeIds(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseInvTransform(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseTransform(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseCumulativeAsTrueOrCurrent(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    windowMode = (WINDOW_MODE) tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetAdaptsToAnyWindowMode(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    numStates = tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    mirOccurred = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    canUseOrigZones = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    origElementsRequiredForPick = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    meshCoordType = (avtMeshCoordType)tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    meshType = (avtMeshType)tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    nodesAreCritical = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    rectilinearGridHasTransform = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    activeVariable = tmp;

    int numVars;
    memcpy(&numVars, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    int *varDims = new int[numVars];
    avtCentering *centerings = new avtCentering[numVars];
    bool *ascii = new bool[numVars];
    avtVarType *vartypes = new avtVarType[numVars];
    int *subnames_size = new int[numVars];
    int *binRange_size = new int[numVars];
    int *useForAxis = new int[numVars];
    for (i = 0 ; i < numVars ; i++)
    {
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        varDims[i] = tmp;

        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        centerings[i] = (avtCentering) tmp;

        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        ascii[i] = (tmp != 0 ? true : false);

        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        vartypes[i] = (avtVarType) tmp;

        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        subnames_size[i] = tmp;

        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        binRange_size[i] = tmp;

        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        useForAxis[i] = tmp;
    }

    memcpy(&dtmp, input, sizeof(double));
    input += sizeof(double); size += sizeof(double);
    dtime = dtmp;

    int s;
    s = trueSpatial->Read(input);
    input += s; size += s;
    s = cumulativeTrueSpatial->Read(input);
    input += s; size += s;
    s = effectiveSpatial->Read(input);
    input += s; size += s;
    s = currentSpatial->Read(input);
    input += s; size += s;
    s = cumulativeCurrentSpatial->Read(input);
    input += s; size += s;

    for (i = 0 ; i < numVars ; i++)
    {
        // Get the name of the variable
        int varname_length;
        memcpy(&varname_length, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        string varname(input, varname_length);
        size += varname_length;
        input += varname_length;

        // Get the length of the name of the units.
        int unit_length;
        memcpy(&unit_length, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        // Add the variable.
        if(unit_length > 0)
        {
            string units(input, unit_length);
            AddVariable(varname, units);
        }
        else
            AddVariable(varname);
        // Adjust the size and input pointer in accordance with the
        // size of the units that we had. We do it here in case
        // we didn't have any units.
        size += unit_length;
        input += unit_length;

        // Get the subnames.
        std::vector<std::string> subnames;
        if (subnames_size[i] > 0)
        {
            subnames.resize(subnames_size[i]);
            for (j = 0 ; j < subnames_size[i] ; j++)
            {
                int len;
                memcpy(&len, input, sizeof(int));
                input += sizeof(int); size += sizeof(int);
                subnames[j] = string(input, len);
                size += len;
                input += len; 
            }
            SetVariableSubnames(subnames, varname.c_str());
        }

        std::vector<double> br;
        if (binRange_size[i] > 0)
        {
            br.resize(binRange_size[i]);
            for (j = 0 ; j < binRange_size[i] ; j++)
            {
                memcpy(&dtmp, input, sizeof(double));
                input += sizeof(double); size += sizeof(double);
                br[j] = dtmp;
            }
            SetVariableBinRanges(br, varname.c_str());
        }
        SetCentering(centerings[i], varname.c_str());
        SetVariableDimension(varDims[i], varname.c_str());
        SetTreatAsASCII(ascii[i], varname.c_str());
        SetUseForAxis(useForAxis[i], varname.c_str());
        SetVariableType(vartypes[i], varname.c_str());
 
        s = variables[i].trueData->Read(input);
        input += s; size += s;
        s = variables[i].cumulativeTrueData->Read(input);
        input += s; size += s;
        s = variables[i].effectiveData->Read(input);
        input += s; size += s;
        s = variables[i].currentData->Read(input);
        input += s; size += s;
        s = variables[i].cumulativeCurrentData->Read(input);
        input += s; size += s;
        s = variables[i].componentExtents->Read(input);
        input += s; size += s;
    }
    delete [] varDims;
    delete [] centerings;
    delete [] ascii;
    delete [] useForAxis;
    delete [] vartypes;
    delete [] subnames_size;
    delete [] binRange_size;

    int meshnameSize;
    memcpy(&meshnameSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string l2(input, meshnameSize);
    meshname = l2;
    size += meshnameSize;
    input += meshnameSize;

    int filenameSize;
    memcpy(&filenameSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string l(input, filenameSize);
    filename = l;
    size += filenameSize;
    input += filenameSize;

    int fullDBNameSize;
    memcpy(&fullDBNameSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string l3(input, fullDBNameSize);
    fullDBName = l3;
    size += fullDBNameSize;
    input += fullDBNameSize;

    int unitSize;
    memcpy(&unitSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string xu(input, unitSize);
    xUnits = xu;
    size += unitSize;
    input += unitSize;
    memcpy(&unitSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string yu(input, unitSize);
    yUnits = yu;
    size += unitSize;
    input += unitSize;
    memcpy(&unitSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string zu(input, unitSize);
    zUnits = zu;
    size += unitSize;
    input += unitSize;

    int labelSize;
    memcpy(&labelSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string xl(input, labelSize);
    xLabel = xl;
    size += labelSize;
    input += labelSize;
    memcpy(&labelSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string yl(input, labelSize);
    yLabel = yl;
    size += labelSize;
    input += labelSize;
    memcpy(&labelSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string zl(input, labelSize);
    zLabel = zl;
    size += labelSize;
    input += labelSize;

    for (i = 0; i < 9 ; i++)
    {
        memcpy(&dtmp, input, sizeof(double));
        input += sizeof(double); size += sizeof(double);
        unitCellVectors[i] = dtmp;
    }

    for (i = 0; i < 16 ; i++)
    {
        memcpy(&dtmp, input, sizeof(double));
        input += sizeof(double); size += sizeof(double);
        rectilinearGridTransform[i] = dtmp;
    }

    int selectionsSize;
    memcpy(&selectionsSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    selectionsApplied.clear();
    for (i = 0; i < selectionsSize; i++)
    {
        int tmp;
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        selectionsApplied.push_back(tmp);
    }

    s = ReadLabels(input); 
    input += s; size += s;

    s = ReadInvTransform(input); 
    input += s; 
    size  += s;

    s = ReadTransform(input); 
    input += s; 
    size  += s;

    s = ReadPlotInfoAtts(input); 
    input += s; 
    size  += s;

    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::MergeLabels
//
//  Purpose:
//    Merged labels with passed labels list.  
//
//  Arguments:
//    l        The list of labels with which to merge. 
//
//  Progammer: Kathleen Bonnell 
//  Creation:  September 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Mar 18 20:20:48 PST 2004
//    Re-wrote to avoid a quadratic performance.
//
// ****************************************************************************

void
avtDataAttributes::MergeLabels(const vector<string> &l)
{
    if (labels.size() == l.size())
    {
        bool foundDifference = false;
        for (int i = 0 ; i < labels.size() ; i++)
            if (labels[i] != l[i])
            {
                foundDifference = true;
                break;
            }
        if (!foundDifference)
            return;
    }

    vector<string> list1 = l;
    vector<string> list2 = labels;
    sort(list1.begin(), list1.end());
    sort(list2.begin(), list2.end());
    int list1_counter = 0;
    int list2_counter = 0;
    vector<string> master_list;
    while ((list1_counter < list1.size()) || (list2_counter < list2.size()))
    {
        if ((list1_counter < list1.size()) && (list2_counter < list2.size()))
        {
            if (list1[list1_counter] == list2[list2_counter])
            {
                master_list.push_back(list1[list1_counter]);
                list1_counter++;
                list2_counter++;
            }
            else if (list1[list1_counter] < list2[list2_counter])
                master_list.push_back(list1[list1_counter++]);
            else
                master_list.push_back(list2[list2_counter++]);
        }
        else if (list1_counter < list1.size())
            master_list.push_back(list1[list1_counter++]);
        else
            master_list.push_back(list2[list2_counter++]);
    }

    labels = master_list;
}


// ****************************************************************************
//  Method: avtDataAttributes::WriteLabels
//
//  Purpose:
//      Writes the data object information to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 19, 20001
//
// ****************************************************************************

void
avtDataAttributes::WriteLabels(avtDataObjectString &str,
                                const avtDataObjectWriter *wrtr)
{
    wrtr->WriteInt(str, labels.size());
    for (int i = 0; i < labels.size(); i++)
    {
       wrtr->WriteInt(str, labels[i].size());
       str.Append((char *) labels[i].c_str(), labels[i].size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadLabels
//
//  Purpose:
//    Reads the label information from a stream.
//
//  Arguments:
//    input     The string (stream) to read from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

int
avtDataAttributes::ReadLabels(char *input)
{
    int size = 0;
    int numLabels;
    memcpy(&numLabels, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    int labelSize;
    for (int i = 0; i < numLabels; i++)
    {
        memcpy(&labelSize, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        string l(input, labelSize);
        labels.push_back(l);
        size += labelSize;
        input += labelSize;
    }
    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetLabels
//
//  Purpose:
//    Sets the labels according to the passed argument.
//
//  Arguments:
//    l         The labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
avtDataAttributes::SetLabels(const vector<string> &l)
{
    if (!labels.empty())
    {
        labels.clear();
    }

    for (int i = 0; i < l.size(); i++)
    {
        labels.push_back(l[i]);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::GetLabels
//
//  Purpose:
//    Returns the member labels.
//
//  Arguments:
//    l         A place to store the labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
avtDataAttributes::GetLabels(vector<string> &l)
{
    if (!l.empty())
    {
        l.clear();
    }

    for (int i = 0; i < labels.size(); i++)
    {
        l.push_back(labels[i]);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCurrentDataExtents
//
//  Purpose:
//      Gets the current data extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 2, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 08:59:42 PST 2004
//    Allow for multiple variables.
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Fri Oct 22 13:57:58 PDT 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

bool
avtDataAttributes::GetCurrentDataExtents(double *buff, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    if (variables[index].currentData->HasExtents())
    {
        variables[index].currentData->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        variables[index].cumulativeCurrentData->HasExtents())
    {
        variables[index].cumulativeCurrentData->CopyTo(buff);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCurrentSpatialExtents
//
//  Purpose:
//      Gets the current spatial extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 2, 2001
//
// ****************************************************************************

bool
avtDataAttributes::GetCurrentSpatialExtents(double *buff)
{
    if (currentSpatial->HasExtents())
    {
        currentSpatial->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        cumulativeCurrentSpatial->HasExtents())
    {
        cumulativeCurrentSpatial->CopyTo(buff);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableName
//
//  Purpose:
//      Returns the name of the active variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableName(void) const
{
    if (activeVariable < 0)
    {
        string reason = "Attempting to retrieve non-existent";
        reason = reason +  " active variable.\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[activeVariable].varname;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableName
//
//  Purpose:
//      Returns the name of the variable at an index.
//
//  Programmer: Hank Childs
//  Creation:   February 24, 2004
//
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableName(int index) const
{
    if (index < 0 || index >= variables.size())
    {
        EXCEPTION2(BadIndexException, index, variables.size());
    }

    return variables[index].varname;
}

// ****************************************************************************
// Method: avtDataAttributes::GetVariableUnits
//
// Purpose: 
//   Returns the active variable's units.
//
// Returns:    A reference to the active variable's units.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 16:36:00 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 27 14:44:20 PDT 2004
//   Allow retrieval by varname.
//   
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableUnits(const char *varname) const
{
    if (varname == NULL)
    {
        if (activeVariable < 0)
        {
            string reason = "Attempting to retrieve non-existent"
                            " active variable.\n";
            EXCEPTION1(ImproperUseException, reason);
        }

        return variables[activeVariable].varunits;
    }
    else
    {
        int index = VariableNameToIndex(varname);
        if (index < 0 || index >= variables.size())
        {
            EXCEPTION2(BadIndexException, index, variables.size());
        }

        return variables[index].varunits;
    }
}

// ****************************************************************************
// Method: avtDataAttributes::GetVariableUnits
//
// Purpose: 
//   Returns the units for the variable at an index.
//
// Arguments:
//   index : The index of the variable whose units we want.
//
// Returns:    A reference to the variable units.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 12:22:22 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableUnits(int index) const
{
    if (index < 0 || index >= variables.size())
    {
        EXCEPTION2(BadIndexException, index, variables.size());
    }

    return variables[index].varunits;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetNumberOfVariables
//
//  Purpose:
//      Returns the number of variables in this object.
//
//  Programmer: Hank Childs
//  Creation:   February 24, 2004
//
// ****************************************************************************

int
avtDataAttributes::GetNumberOfVariables(void) const
{
    return variables.size();
}


// ****************************************************************************
//  Method: avtDataAttributes::SetActiveVariable
//
//  Purpose:
//      Sets the active variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

void
avtDataAttributes::SetActiveVariable(const char *v)
{
    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == v)
        {
            activeVariable = i;
            break;
        }
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::AddVariable
//
//  Purpose:
//      Adds a variable to the data attributes.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004
//    Initialize treatAsASCII.
//
//    Brad Whitlock, Tue Jul 20 12:24:28 PDT 2004
//    Added the units argument so units can be passed in if they are known.
//
//    Hank Childs, Fri Aug  5 16:29:41 PDT 2005
//    Initialize vartype.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//
// ****************************************************************************

void
avtDataAttributes::AddVariable(const std::string &s)
{
    AddVariable(s, "");
}

void
avtDataAttributes::AddVariable(const std::string &s, const std::string &units)
{
    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == s)
        {
            //
            // We already have this variable -- just return.
            //
            return;
        }
    }

    VarInfo new_var;
    new_var.varname = s;
    new_var.varunits = units;
    new_var.dimension = -1;
    new_var.centering = AVT_UNKNOWN_CENT;
    new_var.vartype = AVT_UNKNOWN_TYPE;
    new_var.treatAsASCII = false;
    new_var.trueData = NULL;
    new_var.cumulativeTrueData = NULL;
    new_var.effectiveData = NULL;
    new_var.currentData = NULL;
    new_var.cumulativeCurrentData = NULL;
    new_var.useForAxis = -1;
    new_var.componentExtents = NULL;
    variables.push_back(new_var);
}

// ****************************************************************************
//  Method: avtDataAttributes::ValidVariable
//
//  Purpose:
//      Determines if a variable is valid (meaning it is contained in this
//      object).
//
//  Arguments:
//      vname   A variable name.
//
//  Returns:    true if we have information for vname, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

bool
avtDataAttributes::ValidVariable(const std::string &vname) const
{
    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == vname)
        {
            return true;
        }
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::ValidActiveVariable
//
//  Purpose:
//      Determines if there is a valid active variable.
//
//  Returns:    true if we have information for vname, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

bool
avtDataAttributes::ValidActiveVariable(void) const
{
    if ((activeVariable >= 0) && (activeVariable < variables.size()))
        return true;

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::RemoveVariable
//
//  Purpose:
//      Removes a variable from the data attributes.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

void
avtDataAttributes::RemoveVariable(const std::string &s)
{
    vector<VarInfo> new_vars;

    bool haveActiveVar = false;
    string activeVar;
    if (activeVariable >= 0)
    {
        haveActiveVar = true;
        activeVar = variables[activeVariable].varname;
    }

    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname != s)
            new_vars.push_back(variables[i]);
    }
    variables = new_vars;

    if (haveActiveVar)
    {
        activeVariable = VariableNameToIndex(activeVar.c_str());
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::VariableNameToIndex
//
//  Purpose:
//      Converts a variable name into an index.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

int
avtDataAttributes::VariableNameToIndex(const char *vname) const
{
    if (vname == NULL)
        return activeVariable;
    if (vname[0] == '\0')
        return activeVariable;

    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == vname)
            return i;
    }

    return -1;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetInvTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Notes:
//    If the passed argument is NULL, no change occurs.
//
//    If this object's transformed matrix is NULL, a new matrix is constructed 
//    from the passed argument.
//
//    If this object's transform matrix is NOT NULL,  then it is multiplied
//    by the matrix created from the passed argument.  This allows for 
//    multiple transforms to occur in the same pipeline.
//   
//  Arguments:
//    D         The values of the transform matrix.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::SetInvTransform(const double *D)
{
   if (D == NULL)
       return;

   if (invTransform == NULL)
   {
       invTransform = new avtMatrix(D);
   }
   else
   {
       *invTransform = *invTransform * avtMatrix(D);
   }
}


// ****************************************************************************
//  Method: avtDataAttributes::CopyTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Arguments:
//    m         The transform matrix to be copied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::CopyInvTransform(const avtMatrix *m)
{
   if (m == NULL)
       return;

   if (invTransform == NULL)
       invTransform = new avtMatrix();

   *invTransform = *m;
}


// ****************************************************************************
//  Method: avtDataAttributes::HasInvTransform
//
//  Purpose:
//      Returns whether or not there is a transfrom matrix in this object.
//
//  Returns:    true if it has transform, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003 
//
// ****************************************************************************

bool
avtDataAttributes::HasInvTransform()
{
    return invTransform != NULL;
}


// ****************************************************************************
//  Method: avtDataAttributes::MergeInvTransform
//
//  Purpose:
//    Merged the transform matrix with passed matrix.  This performs
//    multiplication of the matrices if neither are NULL. 
//
//  Arguments:
//    m        The transform matrix with which to merge. 
//
//  Progammer: Kathleen Bonnell 
//  Creation:  April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::MergeInvTransform(const avtMatrix *m)
{
    if (m == NULL)
        return;

    if (invTransform == NULL)
        invTransform = new avtMatrix(*m);
    else
        *invTransform = (*invTransform) * (*m);
}


// ****************************************************************************
//  Method: avtDataAttributes::WriteInvTransform
//
//  Purpose:
//      Writes the transform matrix to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003
//
// ****************************************************************************

void
avtDataAttributes::WriteInvTransform(avtDataObjectString &str,
                                const avtDataObjectWriter *wrtr)
{
    int has = (int)HasInvTransform();

    wrtr->WriteInt(str, has);

    if (has)
    {
       double *d = (*invTransform)[0];    
       wrtr->WriteDouble(str, d, 16);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadInvTransform
//
//  Purpose:
//    Reads the transform matrix from a stream.
//
//  Arguments:
//    input     The string (stream) to read from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

int
avtDataAttributes::ReadInvTransform(char *input)
{
    int size = 0;
    int hasInvTransform;
    memcpy(&hasInvTransform, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    if (hasInvTransform)
    {
        double d[16];
        int s = sizeof(double) * 16;
        memcpy(d, input, s);
        input += s; 
        size += s;
        SetInvTransform(d);
    }
    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Notes:
//    If the passed argument is NULL, no change occurs.
//
//    If this object's transformed matrix is NULL, a new matrix is constructed 
//    from the passed argument.
//
//    If this object's transform matrix is NOT NULL,  then it is multiplied
//    by the matrix created from the passed argument.  This allows for 
//    multiple transforms to occur in the same pipeline.
//   
//  Arguments:
//    D         The values of the transform matrix.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::SetTransform(const double *D)
{
   if (D == NULL)
       return;

   if (transform == NULL)
   {
       transform = new avtMatrix(D);
   }
   else
   {
       *transform = *transform * avtMatrix(D);
   }
}


// ****************************************************************************
//  Method: avtDataAttributes::CopyTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Arguments:
//    m         The transform matrix to be copied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::CopyTransform(const avtMatrix *m)
{
   if (m == NULL)
       return;

   if (transform == NULL)
       transform = new avtMatrix();

   *transform = *m;
}


// ****************************************************************************
//  Method: avtDataAttributes::HasTransform
//
//  Purpose:
//      Returns whether or not there is a transfrom matrix in this object.
//
//  Returns:    true if it has transform, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003 
//
// ****************************************************************************

bool
avtDataAttributes::HasTransform()
{
    return transform != NULL;
}


// ****************************************************************************
//  Method: avtDataAttributes::MergeTransform
//
//  Purpose:
//    Merged the transform matrix with passed matrix.  This performs
//    multiplication of the matrices if neither are NULL. 
//
//  Arguments:
//    m        The transform matrix with which to merge. 
//
//  Progammer: Kathleen Bonnell 
//  Creation:  April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::MergeTransform(const avtMatrix *m)
{
    if (m == NULL)
        return;

    if (transform == NULL)
        transform = new avtMatrix(*m);
    else
        *transform = (*transform) * (*m);
}


// ****************************************************************************
//  Method: avtDataAttributes::WriteTransform
//
//  Purpose:
//      Writes the transform matrix to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003
//
// ****************************************************************************

void
avtDataAttributes::WriteTransform(avtDataObjectString &str,
                                const avtDataObjectWriter *wrtr)
{
    int has = (int)HasTransform();

    wrtr->WriteInt(str, has);

    if (has)
    {
       double *d = (*transform)[0];    
       wrtr->WriteDouble(str, d, 16);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadTransform
//
//  Purpose:
//    Reads the transform matrix from a stream.
//
//  Arguments:
//    input     The string (stream) to read from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

int
avtDataAttributes::ReadTransform(char *input)
{
    int size = 0;
    int hasTransform;
    memcpy(&hasTransform, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    if (hasTransform)
    {
        double d[16];
        int s = sizeof(double) * 16;
        memcpy(d, input, s);
        input += s; 
        size += s;
        SetTransform(d);
    }
    return size;
}

// ****************************************************************************
//  Method: avtDataAttributes::ClearAllUseForAxis
//
//  Purpose:
//    Sets all variables to not be associated with *any* axis.
//
//  Arguments:
//    none
//
//  Programmer:    Jeremy Meredith
//  Creation:      January 30, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtDataAttributes::ClearAllUseForAxis()
{
    for (int i=0; i<variables.size(); i++)
        variables[i].useForAxis = -1;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetUseForAxis
//
//  Purpose:
//    Sets which acis (e.g. in a parallel coordinates plot) this variable
//    should be used for.
//
//  Arguments:
//    ufa          The new useForAxis value
//
//  Programmer:    Jeremy Meredith
//  Creation:      January 30, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtDataAttributes::SetUseForAxis(const int ufa, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the useForAxis value of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set UseForAxis of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].useForAxis = ufa;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetUseForAxis
//
//  Purpose:
//    Gets the value specifying which axis (e.g. in a parallel coordinates
//    plot) this variable is used for.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 30, 2008
//
//  Modifications:
//
// ****************************************************************************

int
avtDataAttributes::GetUseForAxis(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to get the useForAxis of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve UseForAxis of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].useForAxis;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTreatAsASCII
//
//  Purpose:
//    Sets whether or not this var's data should be treated as ascii values. 
//
//  Arguments:
//    ascii       The new treatAsASCII value,
//
//  Programmer:    Kathleen Bonnell 
//  Creation:      July 21, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

void
avtDataAttributes::SetTreatAsASCII(const bool ascii, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the treatAsASCII value of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set TreatAsASCII of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].treatAsASCII = ascii;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetTreatAsASCII
//
//  Purpose:
//    Gets the value specifying if the variable's data should be treated as 
//    ascii values.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 21, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

bool
avtDataAttributes::GetTreatAsASCII(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to get the treatAsASCII of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve TreatAsASCII of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].treatAsASCII;
}

// ****************************************************************************
//  Method: avtDataAttributes::SetSelectionsApplied
//
//  Purpose: Sets the vector of bools indicating which selections have been
//     applied.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

void
avtDataAttributes::SetSelectionsApplied(std::vector<bool> &selsApplied)
{
    selectionsApplied = selsApplied;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetSelectionApplied
//
//  Purpose: Given the id of a data selection, returns the flag indicating
//     if it was applied. 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

bool
avtDataAttributes::GetSelectionApplied(int selID) const
{
    if (selID < 0 || selID >= selectionsApplied.size())
        return false;
    else
        return selectionsApplied[selID];
}

// ****************************************************************************
//  Method: avtDataAttributes::GetSelectionsApplied
//
//  Purpose: Returns the whole vector of bools for which selections  were
//  applied
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

const vector<bool> &
avtDataAttributes::GetSelectionsApplied() const
{
    return selectionsApplied;
}


// ****************************************************************************
//  Method: avtDataAttributes::TransformSpatialExtents
//
//  Purpose:
//      Transforms all of the spatial extents using a callback function.
//      The advantage of this routine is that all of the different extents
//      flavors don't have to be enumerated by the caller.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtDataAttributes::TransformSpatialExtents(avtDataAttributes &outAtts,
              void (*ProjectExtentsCallback)(const double *, double *, void *),
              void *args)
{
    double in[6], out[6]; // 6 is biggest possible -- not necessarily using
                          // all 6 -- up to callback function to decide.

    if (GetTrueSpatialExtents()->HasExtents())
    {
        GetTrueSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetTrueSpatialExtents()->Set(out);
    }

    if (GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        GetCumulativeTrueSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetCumulativeTrueSpatialExtents()->Set(out);
    }

    if (GetEffectiveSpatialExtents()->HasExtents())
    {
        GetEffectiveSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetEffectiveSpatialExtents()->Set(out);
    }

    if (GetCurrentSpatialExtents()->HasExtents())
    {
        GetCurrentSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetCurrentSpatialExtents()->Set(out);
    }

    if (GetCumulativeCurrentSpatialExtents()->HasExtents())
    {
        GetCumulativeCurrentSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetCumulativeCurrentSpatialExtents()->Set(out);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::WritePlotInfoAtts
//
//  Purpose:
//      Writes the data object information to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 20, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 27 12:21:47 PST 2006
//    Removed PlotInfoAtts specific code, call WriteAtts on writer instead.
//
// ****************************************************************************

void
avtDataAttributes::WritePlotInfoAtts(avtDataObjectString &str,
                                     const avtDataObjectWriter *wrtr)
{
    wrtr->WriteAtts(str, plotInfoAtts);
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadPlotInfoAtts
//
//  Purpose:
//    Reads the label information from a stream.
//
//  Arguments:
//    input     The string (stream) to write to.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 20, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 27 12:32:24 PST 2006
//    Correctly delete plotInfoAtts.
//
// ****************************************************************************

int
avtDataAttributes::ReadPlotInfoAtts(char *input)
{
    int size = 0;
    int piaSize;
    memcpy(&piaSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    if (piaSize == 0)
    {
        if (plotInfoAtts != NULL)
        {
            delete plotInfoAtts;
            plotInfoAtts = NULL;
        }
        return size;
    }
    if (plotInfoAtts == NULL)
    {
        plotInfoAtts = new PlotInfoAttributes();
    }
    unsigned char *b = new unsigned char[piaSize];
    for (int i = 0; i < piaSize; i++)
    {
        b[i] = (unsigned char)input[i];
    }
    input += piaSize;
    size += piaSize;

    BufferConnection buf;
    buf.Append(b, piaSize);
    plotInfoAtts->Read(buf);
    delete [] b;

    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetPlotInfoAtts
//
//  Purpose:
//    Sets the PlotInfoAtts according to the passed argument.
//
//  Arguments:
//    pia       The new plotInfoAtts.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 20, 2006
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 27 12:32:24 PST 2006
//    Delete plotInfoAtts if necessary when pia is NULL.
//
// ****************************************************************************

void
avtDataAttributes::SetPlotInfoAtts(const PlotInfoAttributes *pia)
{
    if (pia == NULL)
    {
        if (plotInfoAtts != NULL)
        {
            delete plotInfoAtts;
            plotInfoAtts = NULL;
        }
        return;
    }
    if (plotInfoAtts == NULL)
    {
        plotInfoAtts = new PlotInfoAttributes();
    }
    *plotInfoAtts = *pia;
}


// ****************************************************************************
//  Method: avtDataAttributes::DebugDump
//
//  Purpose:
//    Dump the attributes to a webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin. Fixed apparent problem in outputting cellOrigin
//    in outputting cellOrigin (it didn't before). 
//
//    Hank Childs, Sun Oct 28 09:42:50 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added ability for variables to be associated with an axis.
//
//    Jeremy Meredith, Thu Feb  7 17:52:59 EST 2008
//    Added component extents for array variables.
//    Also display subnames for array variables.
//    Added support for arbitrarily large numbers of dimensions for extents.
//
// ****************************************************************************

static const char *
YesOrNo(bool b)
{
    static const char *yes_str = "yes";
    static const char *no_str  = "no";
    if (b)
        return yes_str;

    return no_str;
}

static void ExtentsToString(avtExtents *exts, char *str)
{
    int dim = exts->GetDimension();
    double *e = new double[dim*2];
    exts->CopyTo(e);
    if (!exts->HasExtents())
        strcpy(str, "not set");
    else
    {
        strcpy(str, "(");
        char tmp[1000];
        for (int i=0; i<dim; i++)
        {
            sprintf(tmp, "%e -> %e", e[i*2+0], e[i*2+1]);
            strcat(str, tmp);
            if (i<dim-1)
                strcat(str, ", ");
            else
                strcat(str, ")");
        }
    }
    delete[] e;
}


void
avtDataAttributes::DebugDump(avtWebpage *webpage)
{
    char str[1024];

    webpage->AddSubheading("Basic data attributes");
    webpage->StartTable();
    webpage->AddTableHeader2("Field", "Value");
    sprintf(str, "%d", spatialDimension);
    webpage->AddTableEntry2("Spatial Dimension", str);
    sprintf(str, "%d", topologicalDimension);
    webpage->AddTableEntry2("Topological Dimension", str);
    sprintf(str, "%d", cellOrigin);
    webpage->AddTableEntry2("Cell Origin", str);
    sprintf(str, "%d", nodeOrigin);
    webpage->AddTableEntry2("Node Origin", str);
    switch (containsGhostZones)
    {
      case AVT_NO_GHOSTS:
        strcpy(str, "None");
        break;
      case AVT_HAS_GHOSTS:
        strcpy(str, "Yes");
        break;
      case AVT_CREATED_GHOSTS:
        strcpy(str, "Yes (created by VisIt)");
        break;
      case AVT_MAYBE_GHOSTS:
        strcpy(str, "Unknown");
        break;
    }
    webpage->AddTableEntry2("Ghosts", str);
    webpage->AddTableEntry2("Contains exterior boundary ghosts?",
                            YesOrNo(containsExteriorBoundaryGhosts));
    webpage->EndTable();

    webpage->AddSubheading("Data attributes that rarely change");
    webpage->StartTable();
    webpage->AddTableHeader2("Field", "Value");
    webpage->AddTableEntry2("Contains global zone ids?", 
                            YesOrNo(containsGlobalZoneIds));
    webpage->AddTableEntry2("Contains global node ids?", 
                            YesOrNo(containsGlobalNodeIds));
    webpage->AddTableEntry2("Cell Origin", str);
    sprintf(str, "%d", blockOrigin);
    webpage->AddTableEntry2("Block Origin", str);
    sprintf(str, "%d", groupOrigin);
    webpage->AddTableEntry2("Group Origin", str);
    webpage->AddTableEntry2("Contains original cells?", 
                            YesOrNo(containsOriginalCells));
    webpage->AddTableEntry2("Contains original nodes?", 
                            YesOrNo(containsOriginalNodes));
    webpage->AddTableEntry2("Should keep node and zone arrays?", 
                            YesOrNo(keepNodeZoneArrays));
    webpage->AddTableEntry2("Has interface reconstruction occurred?",
                            YesOrNo(mirOccurred));
    webpage->AddTableEntry2("Can use original zones?",
                            YesOrNo(canUseOrigZones));
    webpage->AddTableEntry2("Are the original elements required for pick?",
                            YesOrNo(origElementsRequiredForPick));
    switch (meshCoordType)
    {
      case AVT_XY:
        strcpy(str, "XY (cartesian)");
        break;
      case AVT_ZR:
        strcpy(str, "ZR (cylindrical)");
        break;
      case AVT_RZ:
        strcpy(str, "RZ (cylindrical)");
        break;
     }
    webpage->AddTableEntry2("Coordinate type", str);

    switch (meshType)
    {
      case AVT_RECTILINEAR_MESH:
        strcpy(str, "Rectilinear");
        break;
      case AVT_CURVILINEAR_MESH:
        strcpy(str, "Curvilinear");
        break;
      case AVT_UNSTRUCTURED_MESH:
        strcpy(str, "Unstructured");
        break;
      case AVT_POINT_MESH:
        strcpy(str, "Point");
        break;
      case AVT_SURFACE_MESH:
        strcpy(str, "Surface");
        break;
      case AVT_CSG_MESH:
        strcpy(str, "CSG");
        break;
      case AVT_AMR_MESH:
        strcpy(str, "AMR");
        break;
      case AVT_UNKNOWN_MESH:
        strcpy(str, "Unkown mesh type");
        break;
     }
    webpage->AddTableEntry2("Mesh type", str);

    webpage->AddTableEntry2("Are the nodes critical?",
                            YesOrNo(nodesAreCritical));
    webpage->AddTableEntry2("Is there an implied rectilinear grid transform?",
                            YesOrNo(rectilinearGridHasTransform));
    webpage->AddTableEntry2("X Units", xUnits.c_str());
    webpage->AddTableEntry2("Y Units", yUnits.c_str());
    webpage->AddTableEntry2("Z Units", zUnits.c_str());
    webpage->AddTableEntry2("X Label", xLabel.c_str());
    webpage->AddTableEntry2("Y Label", yLabel.c_str());
    webpage->AddTableEntry2("Z Label", zLabel.c_str());
    webpage->EndTable();
    webpage->AddSubheading("File information");
    webpage->StartTable();
    webpage->AddTableHeader2("Field", "Value");
    webpage->AddTableEntry2("Database name", fullDBName.c_str());
    webpage->AddTableEntry2("File name", filename.c_str());
    webpage->AddTableEntry2("Mesh name", meshname.c_str());
    sprintf(str, "%d", numStates);
    webpage->AddTableEntry2("Number of time slices?", str);
    if (timeIsAccurate)
        sprintf(str, "%f", dtime);
    else
        sprintf(str, "%f (guess)", dtime);
    webpage->AddTableEntry2("Time", str);
    if (cycleIsAccurate)
        sprintf(str, "%d", cycle);
    else
        sprintf(str, "%d (guess)", cycle);
    webpage->AddTableEntry2("Cycle", str);
    webpage->EndTable();

    webpage->AddSubheading("Spatial extents attributes");
    webpage->StartTable();
    webpage->AddTableHeader2("Field", "Value");
    ExtentsToString(trueSpatial, str);
    webpage->AddTableEntry2("True spatial extents", str);
    ExtentsToString(cumulativeTrueSpatial, str);
    webpage->AddTableEntry2("Cumulative true spatial extents", str);
    ExtentsToString(effectiveSpatial, str);
    webpage->AddTableEntry2("Effective spatial extents", str);
    ExtentsToString(currentSpatial, str);
    webpage->AddTableEntry2("Current spatial extents", str);
    ExtentsToString(cumulativeCurrentSpatial, str);
    webpage->AddTableEntry2("Cumulative current spatial extents", str);
    webpage->AddTableEntry2("Can use the cumulative extents are true or current extents?", 
                            YesOrNo(canUseCumulativeAsTrueOrCurrent));
    webpage->EndTable();

    webpage->AddSubheading("Variable attributes");
    if (variables.size() > 0)
    {
        webpage->StartTable();
        webpage->AddTableHeader3("Variable", "Field", "Value");
        for (int i = 0 ; i < variables.size() ; i++)
        {
            webpage->AddTableEntry3(variables[i].varname.c_str(), NULL, NULL);
            webpage->AddTableEntry3(NULL, "Type", 
                                   avtVarTypeToString(variables[i].vartype).c_str());
            webpage->AddTableEntry3(NULL, "Units", variables[i].varunits.c_str());
            sprintf(str, "%d", variables[i].dimension);
            webpage->AddTableEntry3(NULL, "Dimension", str);
            switch (variables[i].centering)
            {
              case AVT_NODECENT:
                strcpy(str, "nodal");
                break;
              case AVT_ZONECENT:
                strcpy(str, "zonal");
                break;
              default:
                strcpy(str, "unknown");
                break;
            }
            webpage->AddTableEntry3(NULL, "Centering", str);
            webpage->AddTableEntry3(NULL, "Treat variable as ASCII characters?",
                                    YesOrNo(variables[i].treatAsASCII));
            sprintf(str, "%d", variables[i].useForAxis);
            webpage->AddTableEntry3(NULL, "Use for axis", str);
            ExtentsToString(variables[i].cumulativeTrueData, str);
            webpage->AddTableEntry3(NULL, "Cumulative true data extents", str);
            ExtentsToString(variables[i].effectiveData, str);
            webpage->AddTableEntry3(NULL, "Effective data extents", str);
            ExtentsToString(variables[i].currentData, str);
            webpage->AddTableEntry3(NULL, "Current data extents", str);
            ExtentsToString(variables[i].cumulativeCurrentData, str);
            webpage->AddTableEntry3(NULL, "Cumulative current data extents", str);
            ExtentsToString(variables[i].componentExtents, str);
            webpage->AddTableEntry3(NULL, "Component extents", str);
            if (variables[i].subnames.size() != 0)
            {
                for (int j = 0 ; j < variables[i].subnames.size() ; j++)
                {
                    sprintf(str, "Variable subname[%d]", j);
                    webpage->AddTableEntry3(NULL, str,
                                            variables[i].subnames[j].c_str());
                }
            }
        }
        webpage->EndTable();
    }
    else
    {
        webpage->AddSubheading("--> No variables!");
    }
}


