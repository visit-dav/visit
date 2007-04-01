// ************************************************************************* //
//                            avtDatabaseMetaData.C                          //
// ************************************************************************* //

#include <iostream.h>

#include <avtDatabaseMetaData.h>
#include <ParsingExprList.h>
#include <ExprNode.h>
#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>


inline void   Indent(ostream &, int);


// ****************************************************************************
//  Method: avtMeshMetaData default constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun 24 12:30:18 PDT 2002
//    Initialized the data members to some reasonable value.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Initialized disjointElements.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Initialized containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Initialized containsOriginalCells. 
//
// ****************************************************************************

avtMeshMetaData::avtMeshMetaData()
    : AttributeSubject("sssiiiiiibFFs*ii*ssbsssibb")
{
    blockTitle = "domains";
    blockPieceName = "domain";
    numBlocks = 1;
    blockOrigin = 0;
    cellOrigin = 0;
    numGroups = 0;
    spatialDimension = 3;
    topologicalDimension = 3;
    meshType = AVT_UNKNOWN_MESH;
    hasSpatialExtents = false;
    groupTitle = "groups";
    groupPieceName = "group";
    disjointElements = false;
    containsGhostZones = AVT_MAYBE_GHOSTS;
    containsOriginalCells = false;
    validVariable = true;
}

// ****************************************************************************
//  Method: avtMeshMetaData constructor
//
//  Arguments:
//      extents     Mesh extents as <min_x, max_x, min_y, max_y, min_z, max_z>.
//      s           The name of the mesh.
//      nb          The number of blocks.
//      bo          The block origin.
//      sd          The spacial dimensionality.
//      td          The topological dimensionality.
//      mt          The type of mesh.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May 28 13:57:49 PDT 2002
//    Initialized blockPieceName and blockTitle.
//
//    Hank Childs, Sun Jun 16 19:31:03 PDT 2002 
//    Added argument for cell origin.
//
//    Hank Childs, Mon Jun 24 12:21:15 PDT 2002
//    Initialize numGroups.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Initialized disjointElements.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Initialized containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Initialized containsOriginalCells. 
//
// ****************************************************************************

avtMeshMetaData::avtMeshMetaData(const float *extents, std::string s, int nb,
                                 int bo, int co,int sd, int td, avtMeshType mt)
    : AttributeSubject("sssiiiiiibFFs*ii*ssbsssibb")
{
    name                 = s;
    numBlocks            = nb;
    blockOrigin          = bo;
    cellOrigin           = co;
    topologicalDimension = td;
    spatialDimension     = sd;
    meshType             = mt;
    numGroups            = 0;
    SetExtents(extents);
    blockTitle           = "domains";
    blockPieceName       = "domain";
    groupTitle           = "groups";
    groupPieceName       = "group";
    disjointElements     = false;
    containsGhostZones   = AVT_MAYBE_GHOSTS;
    containsOriginalCells   = false; 
    validVariable        = true;
}


// ****************************************************************************
//  Method: avtMeshMetaData constructor
//
//  Arguments:
//      s           The name of the mesh.
//      nb          The number of blocks.
//      bo          The block origin.
//      sd          The spacial dimensionality.
//      td          The topological dimensionality.
//      mt          The type of mesh.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May 28 13:57:49 PDT 2002
//    Initialized blockPieceName and blockTitle.
//
//    Hank Childs, Sun Jun 16 19:31:03 PDT 2002 
//    Added argument for cell origin.
//
//    Hank Childs, Mon Jun 24 12:21:15 PDT 2002
//    Initialize numGroups.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Initialized disjointElements.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Initialized containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Initialized containsOriginalCells. 
//
// ****************************************************************************

avtMeshMetaData::avtMeshMetaData(std::string s, int nb, int bo, int co, int sd,
                                 int td, avtMeshType mt)
    : AttributeSubject("sssiiiiiibFFs*ii*ssbsssibb")
{
    name                 = s;
    numBlocks            = nb;
    blockOrigin          = bo;
    cellOrigin           = co;
    topologicalDimension = td;
    spatialDimension     = sd;
    meshType             = mt;

    hasSpatialExtents    = false;
    blockTitle           = "domains";
    blockPieceName       = "domain";
    numGroups            = 0;
    groupTitle           = "groups";
    groupPieceName       = "group";
    disjointElements     = false;
    containsGhostZones   = AVT_MAYBE_GHOSTS;
    containsOriginalCells = false;
    validVariable        = true;
}


// ****************************************************************************
//  Method: avtMeshMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 10:16:24 PDT 2001
//    Copied over blockNames.
//
//    Hank Childs, Tue May 28 13:57:49 PDT 2002
//    Copied over blockPieceName and blockTitle.
//
//    Hank Childs, Sun Jun 16 19:31:03 PDT 2002 
//    Copied over cellOrigin.
//
//    Hank Childs, Mon Jun 24 12:21:15 PDT 2002
//    Copied over numGroups, groupIds.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Copied over disjointElements.
//
//    Hank Childs, Thu Sep 26 22:01:55 PDT 2002
//    Copied over units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Copied over containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Copied over containsOriginalCells. 
//
// ****************************************************************************

avtMeshMetaData::avtMeshMetaData(const avtMeshMetaData &rhs)
    : AttributeSubject("sssiiiiiibFFs*ii*ssbsssibb")
{
    name                     = rhs.name;
    numBlocks                = rhs.numBlocks;
    blockOrigin              = rhs.blockOrigin;
    cellOrigin               = rhs.cellOrigin;
    spatialDimension         = rhs.spatialDimension;
    topologicalDimension     = rhs.topologicalDimension;
    meshType                 = rhs.meshType;
    blockNames               = rhs.blockNames;
    blockTitle               = rhs.blockTitle;
    blockPieceName           = rhs.blockPieceName;
    hasSpatialExtents        = rhs.hasSpatialExtents;
    for (int i=0;i<3;i++)
    {
        minSpatialExtents[i] = rhs.minSpatialExtents[i];
        maxSpatialExtents[i] = rhs.maxSpatialExtents[i];
    }
    numGroups                = rhs.numGroups;
    groupIds                 = rhs.groupIds;
    groupTitle               = rhs.groupTitle;
    groupPieceName           = rhs.groupPieceName;
    disjointElements         = rhs.disjointElements;
    containsGhostZones       = rhs.containsGhostZones;
    containsOriginalCells    = rhs.containsOriginalCells;
    xUnits                   = rhs.xUnits;
    yUnits                   = rhs.yUnits;
    zUnits                   = rhs.zUnits;
    validVariable            = rhs.validVariable;
}


// ****************************************************************************
// Method: avtMeshMetaData destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:11:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

avtMeshMetaData::~avtMeshMetaData()
{
}

// ****************************************************************************
//  Method: avtMeshMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 10:16:24 PDT 2001
//    Copied over blockNames.
//
//    Hank Childs, Tue May 28 13:57:49 PDT 2002
//    Copied over blockPieceName and blockTitle.
//
//    Hank Childs, Sun Jun 16 19:31:03 PDT 2002 
//    Copied over cellOrigin.
//
//    Hank Childs, Mon Jun 24 12:21:15 PDT 2002
//    Copied over numGroups.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Copied over disjointElements.
//
//    Hank Childs, Thu Sep 26 22:01:55 PDT 2002
//    Copied over units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Copied over containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Copied validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Copied containsOriginalCells. 
//
// ****************************************************************************

const avtMeshMetaData &
avtMeshMetaData::operator=(const avtMeshMetaData &rhs)
{
    name                     = rhs.name;
    numBlocks                = rhs.numBlocks;
    blockOrigin              = rhs.blockOrigin;
    cellOrigin               = rhs.cellOrigin;
    spatialDimension         = rhs.spatialDimension;
    topologicalDimension     = rhs.topologicalDimension;
    meshType                 = rhs.meshType;
    blockNames               = rhs.blockNames;
    blockTitle               = rhs.blockTitle;
    blockPieceName           = rhs.blockPieceName;
    hasSpatialExtents        = rhs.hasSpatialExtents;
    for (int i=0;i<3;i++)
    {
        minSpatialExtents[i] = rhs.minSpatialExtents[i];    
        maxSpatialExtents[i] = rhs.maxSpatialExtents[i];
    }
    numGroups                = rhs.numGroups;
    groupIds                 = rhs.groupIds;
    groupTitle               = rhs.groupTitle;
    groupPieceName           = rhs.groupPieceName;
    disjointElements         = rhs.disjointElements;
    containsGhostZones       = rhs.containsGhostZones;
    containsOriginalCells    = rhs.containsOriginalCells;
    xUnits                   = rhs.xUnits;
    yUnits                   = rhs.yUnits;
    zUnits                   = rhs.zUnits;
    validVariable            = rhs.validVariable;

    return *this;
}


// ****************************************************************************
//  Method: avtMeshMetaData::SelectAll
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 10:16:24 PDT 2001
//    Added block names.
//
//    Hank Childs, Tue May 28 13:57:49 PDT 2002
//    Added blockPieceName and blockTitle.
//
//    Hank Childs, Mon Jun 24 12:21:15 PDT 2002
//    Added numGroups, groupIds.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Added disjointElements.
//
//    Hank Childs, Thu Sep 26 22:01:55 PDT 2002
//    Add units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Add containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Add validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Add containsOriginalCells. 
//
// ****************************************************************************

void
avtMeshMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&blockPieceName);
    Select(2, (void*)&blockTitle);
    Select(3, (void*)&numBlocks);
    Select(4, (void*)&blockOrigin);
    Select(5, (void*)&cellOrigin);
    Select(6, (void*)&spatialDimension);
    Select(7, (void*)&topologicalDimension);
    Select(8, (void*)&meshType);
    Select(9, (void*)&hasSpatialExtents);
    Select(10, (void*)minSpatialExtents, 3);
    Select(11, (void*)maxSpatialExtents, 3);
    Select(12, (void*)&blockNames);
    Select(13, (void*)&numGroups);
    Select(14, (void*)&groupIds);
    Select(15, (void*)&groupTitle);
    Select(16, (void*)&groupPieceName);
    Select(17, (void*)&disjointElements);
    Select(18, (void*)&xUnits);
    Select(19, (void*)&yUnits);
    Select(20, (void*)&zUnits);
    Select(21, (void*)&containsGhostZones);
    Select(22, (void*)&containsOriginalCells);
    Select(23, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtMeshMetaData::SetExtents
//
//  Purpose:
//      Sets the extents of the mesh.
//
//  Arguments:
//      extents     Mesh extents as <min_x, max_x, min_y, max_y, min_z, max_z>.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May  1 12:53:10 PDT 2001
//    Check for NULL extents.
//
// ****************************************************************************

void
avtMeshMetaData::SetExtents(const float *extents)
{
    if (extents == NULL)
    {
        hasSpatialExtents = false;
    }
    else
    {
        hasSpatialExtents = true;
        for (int i = 0 ; i < spatialDimension ; i++)
        {
            minSpatialExtents[i] = extents[2*i];
            maxSpatialExtents[i] = extents[2*i + 1];
        }
    }
}


// ****************************************************************************
//  Method: avtMeshMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    August 28, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 10:16:24 PDT 2001
//    Added block names.
//
//    Hank Childs, Tue May 28 13:57:49 PDT 2002
//    Added blockPieceName and blockTitle.
//
//    Hank Childs, Sun Jun 16 19:31:03 PDT 2002 
//    Added cell origin.
//
//    Hank Childs, Mon Jun 24 12:21:15 PDT 2002
//    Added numGroups.
//
//    Hank Childs, Sun Aug 18 10:54:26 PDT 2002
//    Added disjointElements.
//
//    Brad Whitlock, Tue Aug 20 15:11:14 PST 2002
//    Changed printing a little.
//
//    Hank Childs, Thu Sep 26 22:01:55 PDT 2002
//    Added units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002 
//    Added containsGhostZones.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Added containsOriginalCells. 
//
// ****************************************************************************

void
avtMeshMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;
    Indent(out, indent);
    out << "Number of blocks = " << numBlocks << endl;
    Indent(out, indent);
    out << "Block origin = " << blockOrigin << endl;
    Indent(out, indent);
    out << "Cell origin = " << cellOrigin 
        << " (origin within one block of the cells)." << endl;
    Indent(out, indent);
    out << "Title for domain hierarchy is " << blockTitle.c_str() << endl;
    Indent(out, indent);
    out << "Title for individual piece in domain hierarchy is "
        << blockPieceName.c_str() << endl;

    Indent(out, indent);
    out << "Number of groups = " << numGroups << endl;
    if(numGroups > 0)
    {
        Indent(out, indent);
        out << "Group ids are:";
        for (int i = 0 ; i < groupIds.size() ; i++)
        {
            out << groupIds[i];
            if(i  < groupIds.size() - 1)
                out << ", ";
        }
        out << endl;
    }
    Indent(out, indent);
    out << "Title for group hierarchy is " << groupTitle.c_str() << endl;
    Indent(out, indent);
    out << "Title for individual piece in group hierarchy is "
        << groupPieceName.c_str() << endl;

    Indent(out, indent);
    out << "Mesh type is ";
    switch (meshType)
    {
      case AVT_UNSTRUCTURED_MESH:
        out << "Unstructured Mesh";
        break;

      case AVT_RECTILINEAR_MESH:
        out << "Rectilinear Mesh";
        break;

      case AVT_CURVILINEAR_MESH:
        out << "Curvilinear Mesh";
        break;

      case AVT_POINT_MESH:
        out << "Point Mesh";
        break;

      case AVT_UNKNOWN_MESH:
      default:
        out << "Unknown";
        break;
    }
    out << "." << endl;

    Indent(out, indent);
    out << "Spatial Dimension = " << spatialDimension << endl;
    Indent(out, indent);
    out << "Topological Dimension = " << topologicalDimension << endl;
    if (hasSpatialExtents)
    {
        Indent(out, indent);
        out << "Extents are: (";
        for (int j = 0 ; j < spatialDimension ; j++)
        {
            out << "(" << minSpatialExtents[j] << ", " << maxSpatialExtents[j]
                << ")";
            if(j < spatialDimension-1)
                out << ", ";
        }
        out << ")" << endl;
    }
    else
    {
        Indent(out, indent);
        out << "The spatial extents are not set." << endl;
    }

    if (blockNames.size() == numBlocks)
    {
        Indent(out, indent);
        out << "Block names: " << endl;
        for (int i = 0 ; i < numBlocks ; i++)
        {
            Indent(out, indent);
            out << "\t" << blockNames[i].c_str() << endl;
        }
    }
    else
    {
        Indent(out, indent);
        out << "There are no names set with the blocks." << endl;
    }

    Indent(out, indent);
    out << "Disjoint elements " << (disjointElements ? "true" : "false") 
        << endl;

    Indent(out, indent);
    out << "Contains ghost zones " << containsGhostZones << endl;

    Indent(out, indent);
    out << "Contains original cells " << containsOriginalCells << endl;

    Indent(out, indent);
    out << "Units =  x: \"" << xUnits.c_str()
        << "\", y: \"" << yUnits.c_str()
        << "\", z: \"" << zUnits.c_str() << "\"." << endl;

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtScalarMetaData default constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData()
    : AttributeSubject("ssibffb")
{
    validVariable = true;
}
   

// ****************************************************************************
//  Method: avtScalarMetaData constructor
//
//  Arguments:
//      n           The name of the scalar variable.
//      mn          The name of the mesh the scalar var is defined on.
//      c           The centering of the variable.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(std::string n, std::string mn, 
                                     avtCentering c)
    : AttributeSubject("ssibffb")
{
    name           = n;
    meshName       = mn;
    centering      = c;
    hasDataExtents = false;
    validVariable  = true;
}
   

// ****************************************************************************
//  Method: avtScalarMetaData constructor
//
//  Arguments:
//      n           The name of the scalar variable.
//      mn          The name of the mesh the scalar var is defined on.
//      c           The centering of the variable.
//      min         The minimum value of the scalar variable.
//      max         The maximum value of the scalar variable.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(std::string n, std::string mn,
                                     avtCentering c, float min, float max)
    : AttributeSubject("ssibffb")
{
    name           = n;
    meshName       = mn;
    centering      = c;
    validVariable  = true;

    float  extents[2] = { min, max };
    SetExtents(extents);
}


// ****************************************************************************
//  Method: avtScalarMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(const avtScalarMetaData &rhs)
    : AttributeSubject("ssibffb")
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    hasDataExtents = rhs.hasDataExtents;
    minDataExtents = rhs.minDataExtents;
    maxDataExtents = rhs.maxDataExtents;
    validVariable  = rhs.validVariable;
}


// ****************************************************************************
// Method: avtScalarMetaData destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:11:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData::~avtScalarMetaData()
{
}


// ****************************************************************************
//  Method: avtScalarMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Copied validVariable.
//
// ****************************************************************************

const avtScalarMetaData &
avtScalarMetaData::operator=(const avtScalarMetaData &rhs)
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    hasDataExtents = rhs.hasDataExtents;
    minDataExtents = rhs.minDataExtents;
    maxDataExtents = rhs.maxDataExtents;
    validVariable  = rhs.validVariable;
    return *this;
}


// ****************************************************************************
//  Method: avtScalarMetaData::SelectAll
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
// ****************************************************************************

void
avtScalarMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&meshName);
    Select(2, (void*)&centering);
    Select(3, (void*)&hasDataExtents);
    Select(4, (void*)&minDataExtents);
    Select(5, (void*)&maxDataExtents);
    Select(6, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtScalarMetaData::SetExtents
//
//  Purpose:
//      Sets the extents of the scalar variable.
//
//  Arguments:
//      extents     Extents as <min value, max value>.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May  1 12:53:10 PDT 2001
//    Check for NULL extents.
//
// ****************************************************************************

void
avtScalarMetaData::SetExtents(const float *extents)
{
    if (extents == NULL)
    {
        hasDataExtents = false;
    }
    else
    {
        hasDataExtents = true;
        minDataExtents = extents[0];
        maxDataExtents = extents[1];
    }
}


// ****************************************************************************
//  Method: avtScalarMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    August 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Print out validVariable.
//
// ****************************************************************************

void
avtScalarMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    Indent(out, indent);
    out << "Mesh is = " << meshName.c_str() << endl;

    Indent(out, indent);
    out << "Centering = ";
    switch (centering)
    {
      case AVT_NODECENT:
        out << "node centered.";
        break;

      case AVT_ZONECENT:
        out << "zone centered.";
        break;

      case AVT_UNKNOWN_CENT:
      default:
        out << "unknowing centering.";
        break;
    }
    out << endl;

    if (hasDataExtents)
    {
        Indent(out, indent);
        out << "Extents are: (" << minDataExtents << ", " 
            << maxDataExtents << ")" << endl;
    }
    else
    {
        Indent(out, indent);
        out << "The extents are not set." << endl;
    }

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtVectorMetaData default constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtVectorMetaData::avtVectorMetaData()
    : AttributeSubject("ssiibffb")
{
    varDim = 0;
    validVariable = true;
}


// ****************************************************************************
//  Method: avtVectorMetaData constructor
//
//  Arguments:
//      n           The name of the vector variable.
//      mn          The name of the mesh the vector var is defined on.
//      c           The centering of the variable.
//      vd          The dimension of the variable.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtVectorMetaData::avtVectorMetaData(std::string n, std::string mn, 
                                     avtCentering c, int vd)
    : AttributeSubject("ssiibffb")
{
    name           = n;
    meshName       = mn;
    centering      = c;
    varDim         = vd;
    hasDataExtents = false;
    validVariable  = true;
}


// ****************************************************************************
//  Method: avtVectorMetaData constructor
//
//  Arguments:
//      n           The name of the vector variable.
//      mn          The name of the mesh the vector var is defined on.
//      c           The centering of the variable.
//      vd          The dimension of the variable.
//      extents     The extents of the variable.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtVectorMetaData::avtVectorMetaData(std::string n, std::string mn,
                                     avtCentering c, int vd,
                                     const float *extents)
    : AttributeSubject("ssiibffb")
{
    name           = n;
    meshName       = mn;
    centering      = c;
    varDim         = vd;
    validVariable  = true;
    SetExtents(extents);
}


// ****************************************************************************
//  Method: avtVectorMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Copied validVariable.
//
//    Hank Childs, Sat Sep 20 08:32:38 PDT 2003
//    Copy over varDim.
//
// ****************************************************************************

avtVectorMetaData::avtVectorMetaData(const avtVectorMetaData &rhs)
    : AttributeSubject("ssiibffb")
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    varDim         = rhs.varDim;
    hasDataExtents = rhs.hasDataExtents;
    minDataExtents = rhs.minDataExtents; // safe on a std::vector<float>
    maxDataExtents = rhs.maxDataExtents; // safe on a std::vector<float>
    validVariable  = rhs.validVariable;
}


// ****************************************************************************
// Method: avtVectorMetaData destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:11:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

avtVectorMetaData::~avtVectorMetaData()
{
}

// ****************************************************************************
//  Method: avtVectorMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Copied validVariable.
//
//    Hank Childs, Sat Sep 20 08:32:38 PDT 2003
//    Copy over varDim.
//
// ****************************************************************************

const avtVectorMetaData &
avtVectorMetaData::operator=(const avtVectorMetaData &rhs)
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    varDim         = rhs.varDim;
    hasDataExtents = rhs.hasDataExtents;
    minDataExtents = rhs.minDataExtents; // safe on a std::vector<float>
    maxDataExtents = rhs.maxDataExtents; // safe on a std::vector<float>
    validVariable  = rhs.validVariable;
    return *this;
}


// ****************************************************************************
//  Method: avtVectorMetaData::SelectAll
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtVectorMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&meshName);
    Select(2, (void*)&centering);
    Select(3, (void*)&varDim);
    Select(4, (void*)&hasDataExtents);
    Select(5, (void*)&minDataExtents);
    Select(6, (void*)&maxDataExtents);
    Select(7, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtVectorMetaData::SetExtents
//
//  Purpose:
//      Sets the extents of the :vector
//
//  Arguments:
//      extents     vector extents as <min_v1, max_v1, min_v2, max_v2, ...>.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May  1 12:53:10 PDT 2001
//    Check for NULL extents.
//
//    Kathleen Bonnell, Thu Mar 11 10:59:14 PST 2004 
//    DataExtents now only has 2 components. 
//
// ****************************************************************************

void
avtVectorMetaData::SetExtents(const float *extents)
{
    if (extents == NULL)
    {
        hasDataExtents = false;
    }
    else
    {
        hasDataExtents = true;
        minDataExtents = extents[0];
        maxDataExtents = extents[1];
    }
}


// ****************************************************************************
//  Method: avtVectorMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    August 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtVectorMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    Indent(out, indent);
    out << "Mesh is = " << meshName.c_str() << endl;

    Indent(out, indent);
    out << "Centering = ";
    switch (centering)
    {
      case AVT_NODECENT:
        out << "node centered.";
        break;

      case AVT_ZONECENT:
        out << "zone centered.";
        break;

      case AVT_UNKNOWN_CENT:
      default:
        out << "unknowing centering.";
        break;
    }
    out << endl;

    Indent(out, indent);
    out << "Variable Dimension = " << varDim << endl;
    if (hasDataExtents)
    {
        Indent(out, indent);
        out << "Extents are: ( ";
        out << minDataExtents << ", " << maxDataExtents;
        out << ")" << endl;
    }
    else
    {
        Indent(out, indent);
        out << "The extents are not set." << endl;
    }

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtTensorMetaData default constructor
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

avtTensorMetaData::avtTensorMetaData()
    : AttributeSubject("ssiib")
{
    dim = 0;
    validVariable = true;
}


// ****************************************************************************
//  Method: avtTensorMetaData constructor
//
//  Arguments:
//      n           The name of the tensor variable.
//      mn          The name of the mesh the tensor var is defined on.
//      c           The centering of the variable.
//      vd          The dimension of the variable.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

avtTensorMetaData::avtTensorMetaData(std::string n, std::string mn, 
                                     avtCentering c, int vd)
    : AttributeSubject("ssiib")
{
    name           = n;
    meshName       = mn;
    centering      = c;
    dim            = vd;
    validVariable  = true;
}


// ****************************************************************************
//  Method: avtTensorMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

avtTensorMetaData::avtTensorMetaData(const avtTensorMetaData &rhs)
    : AttributeSubject("ssiib")
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    dim            = rhs.dim;
    validVariable  = rhs.validVariable;
}


// ****************************************************************************
// Method: avtTensorMetaData destructor
//
// Programmer: Hank Childs
// Creation:   September 20, 2003
//
// ****************************************************************************

avtTensorMetaData::~avtTensorMetaData()
{
}

// ****************************************************************************
//  Method: avtTensorMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

const avtTensorMetaData &
avtTensorMetaData::operator=(const avtTensorMetaData &rhs)
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    dim            = rhs.dim;
    validVariable  = rhs.validVariable;
    return *this;
}


// ****************************************************************************
//  Method: avtTensorMetaData::SelectAll
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtTensorMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&meshName);
    Select(2, (void*)&centering);
    Select(3, (void*)&dim);
    Select(4, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtTensorMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    September 20, 2003
//
// ****************************************************************************

void
avtTensorMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    Indent(out, indent);
    out << "Mesh is = " << meshName.c_str() << endl;

    Indent(out, indent);
    out << "Centering = ";
    switch (centering)
    {
      case AVT_NODECENT:
        out << "node centered.";
        break;

      case AVT_ZONECENT:
        out << "zone centered.";
        break;

      case AVT_UNKNOWN_CENT:
      default:
        out << "unknowing centering.";
        break;
    }
    out << endl;

    Indent(out, indent);
    out << "Variable Dimension = " << dim << endl;

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtSymmetricTensorMetaData default constructor
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

avtSymmetricTensorMetaData::avtSymmetricTensorMetaData()
    : AttributeSubject("ssiib")
{
    dim = 0;
    validVariable = true;
}


// ****************************************************************************
//  Method: avtSymmetricTensorMetaData constructor
//
//  Arguments:
//      n           The name of the tensor variable.
//      mn          The name of the mesh the tensor var is defined on.
//      c           The centering of the variable.
//      vd          The dimension of the variable.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

avtSymmetricTensorMetaData::avtSymmetricTensorMetaData(std::string n,
                                        std::string mn, avtCentering c, int vd)
    : AttributeSubject("ssiib")
{
    name           = n;
    meshName       = mn;
    centering      = c;
    dim            = vd;
    validVariable  = true;
}


// ****************************************************************************
//  Method: avtSymmetricTensorMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

avtSymmetricTensorMetaData::avtSymmetricTensorMetaData(
                                         const avtSymmetricTensorMetaData &rhs)
    : AttributeSubject("ssiib")
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    dim            = rhs.dim;
    validVariable  = rhs.validVariable;
}


// ****************************************************************************
// Method: avtSymmetricTensorMetaData destructor
//
// Programmer: Hank Childs
// Creation:   September 20, 2003
//
// ****************************************************************************

avtSymmetricTensorMetaData::~avtSymmetricTensorMetaData()
{
}

// ****************************************************************************
//  Method: avtSymmetricTensorMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

const avtSymmetricTensorMetaData &
avtSymmetricTensorMetaData::operator=(const avtSymmetricTensorMetaData &rhs)
{
    name           = rhs.name;
    meshName       = rhs.meshName;
    centering      = rhs.centering;
    dim            = rhs.dim;
    validVariable  = rhs.validVariable;
    return *this;
}


// ****************************************************************************
//  Method: avtSymmetricTensorMetaData::SelectAll
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtSymmetricTensorMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&meshName);
    Select(2, (void*)&centering);
    Select(3, (void*)&dim);
    Select(4, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtSymmetricTensorMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    September 20, 2003
//
// ****************************************************************************

void
avtSymmetricTensorMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    Indent(out, indent);
    out << "Mesh is = " << meshName.c_str() << endl;

    Indent(out, indent);
    out << "Centering = ";
    switch (centering)
    {
      case AVT_NODECENT:
        out << "node centered.";
        break;

      case AVT_ZONECENT:
        out << "zone centered.";
        break;

      case AVT_UNKNOWN_CENT:
      default:
        out << "unknowing centering.";
        break;
    }
    out << endl;

    Indent(out, indent);
    out << "Variable Dimension = " << dim << endl;

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtMaterialMetaData default constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Brad Whitlock, Thu Oct 23 16:10:39 PST 2003
//    I made validVariable be true by default.
//
// ****************************************************************************

avtMaterialMetaData::avtMaterialMetaData()
    : AttributeSubject("ssis*b")
{
    validVariable = true;
}


// ****************************************************************************
//  Method: avtMaterialMetaData constructor
//
//  Arguments:
//      n            The name of the material.
//      mesh         The name of the mesh the material is defined on.
//      nm           The number of materials.
//      names        The name of each material.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtMaterialMetaData::avtMaterialMetaData(std::string n,std::string mesh,int nm, 
                                         std::vector<std::string> names)
    : AttributeSubject("ssis*b")
{
    name          = n;
    meshName      = mesh;
    numMaterials  = nm;
    materialNames = names;
    validVariable = true;
}


// ****************************************************************************
//  Method: avtMaterialMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtMaterialMetaData::avtMaterialMetaData(const avtMaterialMetaData &rhs)
    : AttributeSubject("ssis*b")
{
    name          = rhs.name;
    meshName      = rhs.meshName;
    numMaterials  = rhs.numMaterials;
    materialNames = rhs.materialNames; // safe on a std::vector<std::string>
    validVariable = rhs.validVariable;
}


// ****************************************************************************
// Method: avtMaterialMetaData destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:11:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

avtMaterialMetaData::~avtMaterialMetaData()
{
}


// ****************************************************************************
//  Method: avtMaterialMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

const avtMaterialMetaData &
avtMaterialMetaData::operator=(const avtMaterialMetaData &rhs)
{
    name          = rhs.name;
    meshName      = rhs.meshName;
    numMaterials  = rhs.numMaterials;
    materialNames = rhs.materialNames; // safe on a std::vector<std::string>
    validVariable = rhs.validVariable;
    return *this;
}


// ****************************************************************************
//  Method: avtMaterialMetaData::SelectAll
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtMaterialMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&meshName);
    Select(2, (void*)&numMaterials);
    Select(3, (void*)&materialNames);
    Select(4, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtMaterialMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    August 28, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Aug 20 15:18:26 PST 2002
//    Changed to remove a trailing comma.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtMaterialMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    Indent(out, indent);
    out << "Mesh Name = " << meshName.c_str() << endl;
    Indent(out, indent);
    out << "Number of Materials = " << numMaterials << endl;

    Indent(out, indent);
    out << "The materials names are = ";
    for (int i = 0; i < materialNames.size() ; ++i)
    {
        out << "\"" << materialNames[i].c_str() << "\"";
        if(i < materialNames.size() - 1)
            out << ", ";
    }
    out << endl;

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtMatSpeciesMetaData default constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtMatSpeciesMetaData::avtMatSpeciesMetaData()
    : AttributeSubject("is*b")
{
    validVariable = true;
}


// ****************************************************************************
//  Method: avtMatSpeciesMetaData constructor
//
//  Arguments:
//      ns           The number of species for this material.
//      sn           The name of each species for this material.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtMatSpeciesMetaData::avtMatSpeciesMetaData(int ns,
                                                   std::vector<std::string> sn)
    : AttributeSubject("is*b")
{
    numSpecies    = ns;
    speciesNames  = sn;
    validVariable = true;
}


// ****************************************************************************
//  Method: avtMatSpeciesMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtMatSpeciesMetaData::avtMatSpeciesMetaData(const avtMatSpeciesMetaData &rhs)
    : AttributeSubject("is*b")
{
    numSpecies    = rhs.numSpecies;
    speciesNames  = rhs.speciesNames;
    validVariable = rhs.validVariable;
}


// ****************************************************************************
// Method: avtMatSpeciesMetaData destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:11:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

avtMatSpeciesMetaData::~avtMatSpeciesMetaData()
{
}


// ****************************************************************************
//  Method: avtMatSpeciesMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

const avtMatSpeciesMetaData &
avtMatSpeciesMetaData::operator=(const avtMatSpeciesMetaData &rhs)
{
    numSpecies    = rhs.numSpecies;
    speciesNames  = rhs.speciesNames;
    validVariable = rhs.validVariable;

    return *this;
}


// ****************************************************************************
//  Method: avtMatSpeciesMetaData::SelectAll
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtMatSpeciesMetaData::SelectAll()
{
    Select(0, (void*)&numSpecies);
    Select(1, (void*)&speciesNames);
    Select(2, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtSpeciesMetaData default constructor
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtSpeciesMetaData::avtSpeciesMetaData()
    : AttributeSubject("sssia*b")
{
    validVariable = true;
}


// ****************************************************************************
//  Method: avtSpeciesMetaData constructor
//
//  Arguments:
//      n            The name of the species
//      meshn        The name of the mesh the species is defined on.
//      matn         The name of the material the species is defined on.
//      nummat       The number of materials in matn.
//      ns           The number of species for each material.
//      sn           The name of each species for each material.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
//  Modifications:
//    Hank Childs, Fri Feb 23 13:30:13 PST 2001
//    Added nummat argument.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtSpeciesMetaData::avtSpeciesMetaData(std::string n,
                                    std::string meshn, std::string matn,
                                    int nummat, std::vector<int> ns, 
                                    std::vector< std::vector<std::string> > sn)
    : AttributeSubject("sssia*b")
{
    name         = n;
    meshName     = meshn;
    materialName = matn;
    numMaterials = nummat;
    species.clear();
    for (int i=0; i<ns.size(); i++)
        species.push_back(new avtMatSpeciesMetaData(ns[i], sn[i]));
    validVariable = true;
}


// ****************************************************************************
//  Method: avtSpeciesMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//    Hank Childs, Fri Feb 23 13:30:13 PST 2001
//    Added copy of numMaterials.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
// ****************************************************************************

avtSpeciesMetaData::avtSpeciesMetaData(const avtSpeciesMetaData &rhs)
    : AttributeSubject("sssia*b")
{
    name          = rhs.name;
    meshName      = rhs.meshName;
    materialName  = rhs.materialName;
    numMaterials  = rhs.numMaterials;
    validVariable = rhs.validVariable;
    species.clear();
    for (int i=0; i<rhs.species.size(); i++)
    {
        species.push_back(new avtMatSpeciesMetaData(rhs.species[i]->numSpecies,
                                                rhs.species[i]->speciesNames));
    }
}


// ****************************************************************************
// Method: avtSpeciesMetaData destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:11:43 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 18 09:56:30 PDT 2003
//   I added code to delete the species metadata objects so we don't leak
//   memory.
//
// ****************************************************************************

avtSpeciesMetaData::~avtSpeciesMetaData()
{
    for(int i = 0; i < species.size(); ++i)
        delete species[i];
}


// ****************************************************************************
//  Method: avtSpeciesMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Feb 23 13:30:13 PST 2001
//    Added copy of numMaterials.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Copied validVariable.
//
// ****************************************************************************

const avtSpeciesMetaData &
avtSpeciesMetaData::operator=(const avtSpeciesMetaData &rhs)
{
    name          = rhs.name;
    meshName      = rhs.meshName;
    materialName  = rhs.materialName;
    numMaterials  = rhs.numMaterials;
    validVariable = rhs.validVariable;
    species.clear();
    for (int i=0; i<rhs.species.size(); i++)
    {
        species.push_back(new avtMatSpeciesMetaData(rhs.species[i]->numSpecies,
                                                rhs.species[i]->speciesNames));
    }
    return *this;
}


// ****************************************************************************
//  Method: avtSpeciesMetaData::SelectAll
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtSpeciesMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&meshName);
    Select(2, (void*)&materialName);
    Select(3, (void*)&numMaterials);
    Select(4, (void*)&species);
    Select(5, (void*)&validVariable);
}

// ****************************************************************************
//  Method: avtSpeciesMetaData::SelectAll
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
// ****************************************************************************

AttributeGroup*
avtSpeciesMetaData::CreateSubAttributeGroup(int)
{
    return new avtMatSpeciesMetaData;
}


// ****************************************************************************
//  Method: avtSpeciesMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    August 28, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Aug 20 16:10:53 PST 2002
//    I changed the printing a little bit.
//
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Added validVariable.
//
// ****************************************************************************

void
avtSpeciesMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    Indent(out, indent);
    out << "Mesh Name = " << meshName.c_str() << endl;
    Indent(out, indent);
    out << "Material Name = " << materialName.c_str() << endl;
    Indent(out, indent);
    out << "Number of materials = " << numMaterials << endl;
    for (int i = 0 ; i < species.size() ; i++)
    {
        Indent(out, indent);
        out << "Material " << i << ", number of species = " 
            << species[i]->numSpecies << endl;
        Indent(out, indent);
        out << "Species names are: ";
        for (int j = 0 ; j < species[i]->numSpecies ; j++)
        {
            out << "\"" << species[i]->speciesNames[j].c_str() << "\"";
            if(j < species[i]->numSpecies - 1)
                out << ", ";
        }
        out << endl;
    }

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtCurveMetaData default constructor
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

avtCurveMetaData::avtCurveMetaData()
    : AttributeSubject("sb")
{
    validVariable = true;
}


// ****************************************************************************
//  Method: avtCurveMetaData constructor
//
//  Arguments:
//      n            The name of the curve
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

avtCurveMetaData::avtCurveMetaData(std::string n)
    : AttributeSubject("sb")
{
    name         = n;
    validVariable = true;
}


// ****************************************************************************
//  Method: avtCurveMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

avtCurveMetaData::avtCurveMetaData(const avtCurveMetaData &rhs)
    : AttributeSubject("sb")
{
    name          = rhs.name;
    validVariable = rhs.validVariable;
}


// ****************************************************************************
// Method: avtCurveMetaData destructor
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

avtCurveMetaData::~avtCurveMetaData()
{
}


// ****************************************************************************
//  Method: avtCurveMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

const avtCurveMetaData &
avtCurveMetaData::operator=(const avtCurveMetaData &rhs)
{
    name          = rhs.name;
    validVariable = rhs.validVariable;

    return *this;
}


// ****************************************************************************
//  Method: avtCurveMetaData::SelectAll
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

void
avtCurveMetaData::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtCurveMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

void
avtCurveMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Name = " << name.c_str() << endl;

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}


// ****************************************************************************
//  Method: avtSILCollectionMetaData default constructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILCollectionMetaData::avtSILCollectionMetaData()
   : AttributeSubject("ssiiiii*")
{
   collectionSize = -1;
   collectionIdOfParent = -1;
   indexOfParent = -1;
   collectionIdOfChildren = -1;
}

// ****************************************************************************
//  Method: avtSILCollectionMetaData copy constructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILCollectionMetaData::avtSILCollectionMetaData(
   const avtSILCollectionMetaData &rhs) : AttributeSubject("ssiiiii*")
{
   classOfCollection = rhs.classOfCollection;
   defaultMemberBasename = rhs.defaultMemberBasename;
   collectionSize = rhs.collectionSize;
   collectionIdOfParent = rhs.collectionIdOfParent;
   indexOfParent = rhs.indexOfParent;
   collectionIdOfChildren = collectionIdOfChildren;
   indicesOfChildren = rhs.indicesOfChildren;
}

// ****************************************************************************
//  Method: avtSILCollectionMetaData constructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILCollectionMetaData::avtSILCollectionMetaData(
   std::string _classOfCollection, std::string _defaultMemberBasename,
   int _collectionSize,
   int _collectionIdOfParent, int _indexOfParent,
   int _collectionIdOfChildren, int *_indicesOfChildren)
   : AttributeSubject("ssiiiii*")
{
   if (_collectionSize <= 0)
   {
      EXCEPTION1(ImproperUseException, _classOfCollection);
   }

   classOfCollection = _classOfCollection;
   defaultMemberBasename = _defaultMemberBasename;
   collectionSize = _collectionSize;
   collectionIdOfParent = _collectionIdOfParent;
   indexOfParent = _indexOfParent;
   collectionIdOfChildren = _collectionIdOfChildren;
   if (_indicesOfChildren != NULL)
   {
      indicesOfChildren =  intVector(collectionSize);
      for (int i = 0; i < collectionSize; i++)
         indicesOfChildren[i] = _indicesOfChildren[i];
   }
}

// ****************************************************************************
//  Method: avtSILCollectionMetaData destructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILCollectionMetaData::~avtSILCollectionMetaData()
{
}

// ****************************************************************************
//  Method: avtSILCollectionMetaData operator=
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
const avtSILCollectionMetaData &
avtSILCollectionMetaData::operator=(const avtSILCollectionMetaData &rhs)
{
   classOfCollection = rhs.classOfCollection;
   defaultMemberBasename = rhs.defaultMemberBasename;
   collectionSize = rhs.collectionSize;
   collectionIdOfParent = rhs.collectionIdOfParent;
   indexOfParent = rhs.indexOfParent;
   collectionIdOfChildren = collectionIdOfChildren;
   indicesOfChildren = rhs.indicesOfChildren;

   return *this;
}


// ****************************************************************************
//  Method: avtSILCollectionMetaData::SelectAll 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
void
avtSILCollectionMetaData::SelectAll()
{
   Select(0,&classOfCollection);
   Select(1,&defaultMemberBasename);
   Select(2,&collectionSize);
   Select(3,&collectionIdOfParent);
   Select(4,&indexOfParent);
   Select(5,&collectionIdOfChildren);
   Select(6,&indicesOfChildren);
}

// ****************************************************************************
//  Method: avtSILCollectionMetaData::Print
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
void
avtSILCollectionMetaData::Print(ostream& out, int indent) const
{
    Indent(out, indent);
    out << "Class = " << classOfCollection.c_str() << endl;

    Indent(out, indent);
    out << "Default Member Basename = " << defaultMemberBasename.c_str() << endl;

    Indent(out, indent);
    out << "Size = " << collectionSize << endl;

    Indent(out, indent);
    out << "Parent = (" << collectionIdOfParent << "," <<
            indexOfParent << ")" << endl;

    Indent(out, indent);
    out << "Children are from collection " << collectionIdOfChildren << endl;

    if (indicesOfChildren.size())
    {
       Indent(out, indent);
       out << "Children ids are... ";
       for (int i = 0; i < collectionSize; i++)
       {
          if (!((i+1)%10))
          {
             out << endl;
             Indent(out, indent);
             out << "                   ";
          }
          out << " " << indicesOfChildren[i];
       }
       out << endl;
    }
}

// ****************************************************************************
//  Method: avtSILCollectionMetaData::GetType
//
//  Purpose: returns whether or not the given collection entry represents
//  a collection class, a pure collection (only links in the SIL) or a 
//  collection and sets.
//
//  Programmer:  Mark C. Miller
//  Creation:    14Sep03 
//
// ****************************************************************************
avtSILCollectionMetaData::CollectionType
avtSILCollectionMetaData::GetType(void) const
{
   if      (collectionIdOfParent     == -1 &&
            indexOfParent            == -1 &&
            collectionIdOfChildren   == -1 &&
            indicesOfChildren.size() == 0)
      return Class;
   else if (collectionIdOfParent   != -1 &&
            indexOfParent          != -1 &&
            collectionIdOfChildren != -1)
      return PureCollection;
   else if (collectionIdOfParent     != -1 &&
            indexOfParent            != -1 &&
            collectionIdOfChildren   == -1 &&
            indicesOfChildren.size() == 0)
      return CollectionAndSets;
   else
      return Unknown;
}

// ****************************************************************************
//  Method: avtSILMetaData default constructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILMetaData::avtSILMetaData()
    : AttributeSubject("si*i*ia*")
{
   // initially, we don't know the storage chunk class id
   theStorageChunkClassId = -1;

   // create pre-defined collection classes (currently just "whole")
   int wholeCollectionClassId = collections.size();
   avtSILCollectionMetaData *wholeCollectionClass =
      new avtSILCollectionMetaData("whole", "mesh", 1, -1, -1, -1, NULL);

   collections.push_back(wholeCollectionClass);
   classDisjointFlags.push_back(1);
   classIds.push_back(wholeCollectionClassId);
}

// ****************************************************************************
//  Method: avtSILMetaData constructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILMetaData::avtSILMetaData(std::string _meshName)
    : AttributeSubject("si*i*ia*")
{
   meshName = _meshName;

   // initially, we don't know the storage chunk class id
   theStorageChunkClassId = -1;

   // create pre-defined collection classes (currently just "whole")
   int wholeCollectionClassId = collections.size();
   avtSILCollectionMetaData *wholeCollectionClass =
      new avtSILCollectionMetaData("whole", _meshName, 1, -1, -1, -1, NULL);

   collections.push_back(wholeCollectionClass);
   classDisjointFlags.push_back(1);
   classIds.push_back(wholeCollectionClassId);
}

// ****************************************************************************
//  Method: avtSILMetaData copy constructor 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILMetaData::avtSILMetaData(const avtSILMetaData &rhs)
    : AttributeSubject("si*i*ia*")
{
   meshName = rhs.meshName;
   classIds = rhs.classIds;
   classDisjointFlags = rhs.classDisjointFlags;
   theStorageChunkClassId = rhs.theStorageChunkClassId;
   collections.clear();
   for (int i=0; i<rhs.collections.size(); i++)
      collections.push_back(new avtSILCollectionMetaData(*(rhs.collections[i])));
}

// ****************************************************************************
//  Method: avtSILMetaData destructor
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
avtSILMetaData::~avtSILMetaData()
{
    for(int i = 0; i < collections.size(); ++i)
        delete collections[i];
}

// ****************************************************************************
//  Method: avtSILMetaData operator=
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
const avtSILMetaData &
avtSILMetaData::operator=(const avtSILMetaData &rhs)
{
   meshName = rhs.meshName;
   classIds = rhs.classIds;
   classDisjointFlags = rhs.classDisjointFlags;
   theStorageChunkClassId = rhs.theStorageChunkClassId;
   collections.clear();
   for (int i=0; i<rhs.collections.size(); i++)
      collections.push_back(new avtSILCollectionMetaData(*(rhs.collections[i])));

   return *this;
}

// ****************************************************************************
//  Method: avtSILMetaData operator=
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
void
avtSILMetaData::SelectAll()
{
   Select(0,&meshName);
   Select(1,&classIds);
   Select(2,&classDisjointFlags);
   Select(3,&theStorageChunkClassId);
   Select(4,&collections);
}

// ****************************************************************************
//  Method: avtSILMetaData::CreateSubAttributeGroup 
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
AttributeGroup*
avtSILMetaData::CreateSubAttributeGroup(int)
{
    return new avtSILCollectionMetaData;
}

// ****************************************************************************
//  Method: avtSILMetaData::Print
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
void
avtSILMetaData::Print(ostream& out, int indent) const
{
    int i;

    Indent(out, indent);
    out << "Mesh Name = " << meshName.c_str() << endl;

    Indent(out, indent);
    out << "Number of collection classes = " << classIds.size() << endl;

    Indent(out, indent);
    out << "Collection class names are..." << endl;
    for (i = 0; i < classIds.size(); i++)
    {
       Indent(out, indent+8);
       out << collections[classIds[i]]->GetClassName().c_str() << endl;
    }

    Indent(out, indent);
    out << "The storage chunk class = " <<
       collections[theStorageChunkClassId]->GetClassName().c_str() << endl;

    Indent(out, indent);
    out << "Collection details are..." << endl;
    for (i = 0; i < collections.size(); i++)
       collections[i]->Print(out,indent+8);

}

// ****************************************************************************
//  Method: avtSILMetaData::GetCollectionClassId
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
int
avtSILMetaData::GetCollectionClassId(std::string& className) const
{
   for (int i = 0; i < classIds.size(); i++)
      if (!strcmp(className.c_str(),
                  collections[classIds[i]]->GetClassName().c_str()))
          return classIds[i];
   return -1;
}

// ****************************************************************************
//  Method: avtSILMetaData::AddCollectionClass
//
//  Purpose: Add a collection class to SIL metadata. A collection class is an
//  awful lot like a collection. In fact, it can be thought of as being
//  implemented as a collection on the whole though if hideFromWhole is true,
//  it won't be 'visible' on the whole from inside VisIt.
//
//  Collection class names must be unique. Only one collection class can be
//  created that has isStorageChunkClass true. Finally, the count of the number
//  of members in the collection class is a global count over the SIL of
//  sets of the associated class. For example, if you are creating a collection
//  class for patches in a block-structured AMR mesh, the numMembers you
//  would pass here is the total number of patches over all levels. Each
//  'entry' in the collection class uniquely identifies one of the patches.
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
int
avtSILMetaData::AddCollectionClass(std::string className,
   std::string defaultMemberBasename, int numMembers,
   int pairwiseDisjoint, bool hideFromWhole, bool isStorageChunkClass)
{
   int newCollectionClassId = collections.size();

   // make sure the number of members is sane
   if (numMembers <= 0)
   {
      EXCEPTION1(ImproperUseException, className);
   }

   // make sure the class name is unique
   for (int i = 0; i < classIds.size(); i++)
   {
      if (className == collections[classIds[i]]->GetClassName())
      {
         EXCEPTION1(ImproperUseException, className);
      }
   }

   // create the collection class entry
   avtSILCollectionMetaData *newCollectionClass =
      new avtSILCollectionMetaData(className, defaultMemberBasename,
                                   numMembers, -1, -1, -1, NULL);

   collections.push_back(newCollectionClass);
   classDisjointFlags.push_back(pairwiseDisjoint);
   classIds.push_back(newCollectionClassId);

   if (isStorageChunkClass)
   {
      if (theStorageChunkClassId != -1)
      {
         EXCEPTION1(ImproperUseException, className);
      }

      theStorageChunkClassId = newCollectionClassId;

   }

#if 0
   if (!hideFromWhole)
   {
      // create a collection entry on whole
      avtSILCollectionMetaData *collectionOnWhole =
         new avtSILCollectionMetaData(className, defaultMemberBasename,
                                      numMembers, 0, 0, newCollectionClassId, NULL);
      collections.push_back(collectionOnWhole);
   }
#endif

   return newCollectionClassId;
}

// ****************************************************************************
//  Method: avtSILMetaData::AddCollection 
//
//  Purpose: Add a collection to SIL meta data. A collection can be created
//  'from scratch' or in terms of another collection. In the former mode,
//  the caller does NOT specify either the collectionIdOfChildren of
//  indicesOfChildren arguments. In the later mode, the caller specifies
//  both of these arguments. The indicesOfChildren argument enumerates which
//  sets in the collection identified by collectionIdOfChildren, are in the
//  collection being here defined and, consequently, subsets of the set
//  identified by the pair <collectionIdOfParent,indexOfParent>.
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
int
avtSILMetaData::AddCollection(
   std::string classOfCollection, std::string defaultMemberBasename,
   int collectionSize, int collectionIdOfParent, int indexOfParent,
   int collectionIdOfChildren, int *indicesOfChildren)
{
   int newCollectionId = collections.size();

   avtSILCollectionMetaData *parentCollection = 
      collections[collectionIdOfParent];

   if (indexOfParent < 0 || indexOfParent >= parentCollection->GetSize())
   {
      EXCEPTION2(BadIndexException, indexOfParent, parentCollection->GetSize());
   }

   if (collectionIdOfChildren < 0 && indicesOfChildren != NULL)
   {
      EXCEPTION1(ImproperUseException, classOfCollection);
   }

   if (collectionIdOfChildren >= 0)
   {
      avtSILCollectionMetaData *childrenCollection = 
         collections[collectionIdOfChildren];

      if (classOfCollection != childrenCollection->GetClassName())
      {
         EXCEPTION1(ImproperUseException, classOfCollection);
      }

      if (indicesOfChildren)
      {
         for (int i = 0; i < collectionSize; i++)
         {
            if (indicesOfChildren[i] >= childrenCollection->GetSize())
            {
               EXCEPTION2(BadIndexException, indicesOfChildren[i], childrenCollection->GetSize());
            }
         }
      }
   }

   avtSILCollectionMetaData *newCollection =
      new avtSILCollectionMetaData(classOfCollection, defaultMemberBasename,
             collectionSize, collectionIdOfParent, indexOfParent,
             collectionIdOfChildren, indicesOfChildren);

   collections.push_back(newCollection);

   return newCollectionId;
}

// ****************************************************************************
//  Method: avtSILMetaData::Validate
//
//  Purpose: check a SIL meta data object for errors.
//
//  Programmer:  Mark C. Miller
//  Creation:    02Sep03 
//
// ****************************************************************************
void
avtSILMetaData::Validate()
{
   if (theStorageChunkClassId == -1)
   {
      EXCEPTION0(ImproperUseException);
   }

   for (int i = 0; i < collections.size(); i++)
   {
      avtSILCollectionMetaData *coll = collections[i];

      if ((coll->collectionIdOfChildren > 0) &&
          (collections[coll->collectionIdOfChildren]->GetType() !=
           avtSILCollectionMetaData::Class))
      {
         if (coll->indicesOfChildren.size() == 0)
         {
            EXCEPTION0(ImproperUseException);
         }

         avtSILCollectionMetaData *childColl = collections[coll->collectionIdOfChildren];

         if (coll->collectionSize > childColl->collectionSize)
         {
            EXCEPTION0(ImproperUseException);
         }

         for (int j = 0; j < coll->collectionSize; j++)
         {
            if ((coll->indicesOfChildren[i] < 0) ||
                (coll->indicesOfChildren[i] >= childColl->collectionSize))
            {
               EXCEPTION2(BadIndexException, coll->indicesOfChildren[i],
                          childColl->collectionSize);
            }
         }
      }
   }
}


// ****************************************************************************
//  Method: avtDefaultPlotMetaData default constructor
//
//  Arguments:
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

avtDefaultPlotMetaData::avtDefaultPlotMetaData()
    : AttributeSubject("sss*b")
{ 
    validVariable = false;
}

// ****************************************************************************
//  Method: avtDefaultPlotMetaData constructor
//
//  Arguments:
//      p        The name of the plugin.
//      v        The name of the variable the plot is applied on.
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

avtDefaultPlotMetaData::avtDefaultPlotMetaData(std::string p, std::string v)
    : AttributeSubject("sss*b")
{
    pluginID      = p;
    plotVar       = v;
    validVariable = true;
}

// ****************************************************************************
//  Method: avtDefaultPlotMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

avtDefaultPlotMetaData::avtDefaultPlotMetaData(const avtDefaultPlotMetaData &rhs)
    : AttributeSubject("sss*b")
{
    pluginID       = rhs.pluginID;
    plotVar        = rhs.plotVar;
    plotAttributes = rhs.plotAttributes; 
    validVariable  = rhs.validVariable;
}

// ****************************************************************************
//  Method: avtDefaultPlotMetaData destructor
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    September 04, 2003
//
//  Modifications:
//   
// ****************************************************************************

avtDefaultPlotMetaData::~avtDefaultPlotMetaData()
{
}


// ****************************************************************************
//  Method: avtDefaultPlotMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

const avtDefaultPlotMetaData &
avtDefaultPlotMetaData::operator=(const avtDefaultPlotMetaData &rhs)
{
    pluginID       = rhs.pluginID;
    plotVar        = rhs.plotVar;
    plotAttributes = rhs.plotAttributes; 
    validVariable  = rhs.validVariable;
    return *this;
}


// ****************************************************************************
//  Method: avtDefaultPlotMetaData::AddAttribute
//
//  Arguments:
//      attr  : the attribute to be added
//
//  Programmer: Walter Herrera Jimenez
//  Creation:   September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtDefaultPlotMetaData::AddAttribute(const std::string& attr)
{
    plotAttributes.push_back(attr);
}


// ****************************************************************************
//  Method: avtDefaultPlotMetaData::SelectAll
//
//  Arguments:
//
//  Programmer: Walter Herrera Jimenez
//  Creation:   September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtDefaultPlotMetaData::SelectAll()
{
    Select(0, (void*)&pluginID); 
    Select(1, (void*)&plotVar);
    Select(2, (void*)&plotAttributes);
    Select(3, (void*)&validVariable);
}


// ****************************************************************************
//  Method: avtDefaultPlotMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    September 04, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtDefaultPlotMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "PluginID = " << pluginID.c_str() << endl;

    Indent(out, indent);
    out << "Plot Variable = " << plotVar.c_str() << endl;

    out << "Plot Attributes:" << endl;
    for(int i = 0; i < plotAttributes.size(); ++i)
      out << "\t" << plotAttributes[i].c_str() << endl;
    out << endl;

    if (!validVariable)
    {
        Indent(out, indent);
        out << "THIS IS NOT A VALID VARIABLE." << endl;
    }
}

// ****************************************************************************
//  Method: avtDatabaseMetaData default constructor
//
//  Programmer:  Hank Childs
//  Creation:    September 1, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Aug  2 09:58:05 PDT 2001
//    Initialize numStates.
//
//    Hank Childs, Mon Mar 11 09:22:45 PST 2002
//    Changed temporal extents to doubles.  Added and initialized new data
//    members.
//
//    Brad Whitlock, Tue Mar 25 14:29:26 PST 2003
//    I added the isVirtualDatabase and timeStepNames fields.
//
//    Walter Herrera, Tue Sep 04 15:08:48 PST 2003
//    I added the defaultPlots field.
//
//    Hank Childs, Fri Mar  5 11:21:06 PST 2004
//    Added 's' for fileFormat.
//
// ****************************************************************************

avtDatabaseMetaData::avtDatabaseMetaData()
    : AttributeSubject("ssbddibss*i*i*i*d*a*a*a*a*a*a*a*a*a*aba*")
{
    hasTemporalExtents          = false;
    minTemporalExtents          = 0.;
    maxTemporalExtents          = 0.;
    numStates                   = 0;
    isVirtualDatabase           = false;
    mustRepopulateOnStateChange = false;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData copy constructor
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 19 10:57:04 PDT 2000
//    Copy over cycles.
//
//    Hank Childs, Mon Mar 11 09:22:45 PST 2002
//    Copy over new data members.
//
//    Hank Childs, Wed Sep  4 11:32:48 PDT 2002
//    Copy over expressions.
//
//    Sean Ahern, Fri Dec 13 16:49:10 PST 2002
//    Changed to use the ExpressionList state object.
//
//    Brad Whitlock, Tue Mar 25 14:29:26 PST 2003
//    I added the isVirtualDatabase, timeStepPath, timeStepNames fields.
//
//    Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//    Add support for curves.
//
//    Hank Childs, Thu Aug 14 08:16:07 PDT 2003
//    Copy over the database name.
//
//    Walter Herrera, Tue Sep 04 15:08:48 PST 2003
//    Copy defaultPlots.
//
//    Hank Childs, Sat Sep 20 08:32:38 PDT 2003
//    Copy over tensor data.
//
//    Hank Childs, Fri Mar  5 11:21:06 PST 2004
//    Copy over file format.
//
// ****************************************************************************

avtDatabaseMetaData::avtDatabaseMetaData(const avtDatabaseMetaData &rhs)
    : AttributeSubject("ssbddibss*i*i*i*d*a*a*a*a*a*a*a*a*a*aba*")
{
    databaseName       = rhs.databaseName;
    fileFormat         = rhs.fileFormat;
    hasTemporalExtents = rhs.hasTemporalExtents;
    minTemporalExtents = rhs.minTemporalExtents;
    maxTemporalExtents = rhs.maxTemporalExtents;
    numStates          = rhs.numStates;
    isVirtualDatabase  = rhs.isVirtualDatabase;
    mustRepopulateOnStateChange = rhs.mustRepopulateOnStateChange;
    timeStepPath       = rhs.timeStepPath;
    timeStepNames      = rhs.timeStepNames;
    cyclesAreAccurate  = rhs.cyclesAreAccurate;
    cycles             = rhs.cycles;
    timesAreAccurate   = rhs.timesAreAccurate;
    times              = rhs.times;
    exprList           = rhs.exprList;

    int i;
    for (i=0; i<rhs.meshes.size(); i++)
        meshes.push_back(new avtMeshMetaData(*rhs.meshes[i]));
    for (i=0; i<rhs.scalars.size(); i++)
        scalars.push_back(new avtScalarMetaData(*rhs.scalars[i]));
    for (i=0; i<rhs.vectors.size(); i++)
        vectors.push_back(new avtVectorMetaData(*rhs.vectors[i]));
    for (i=0; i<rhs.tensors.size(); i++)
        tensors.push_back(new avtTensorMetaData(*rhs.tensors[i]));
    for (i=0; i<rhs.symm_tensors.size(); i++)
        symm_tensors.push_back(
                         new avtSymmetricTensorMetaData(*rhs.symm_tensors[i]));
    for (i=0; i<rhs.materials.size(); i++)
        materials.push_back(new avtMaterialMetaData(*rhs.materials[i]));
    for (i=0; i<rhs.species.size(); i++)
        species.push_back(new avtSpeciesMetaData(*rhs.species[i]));
    for (i=0; i<rhs.curves.size(); i++)
        curves.push_back(new avtCurveMetaData(*rhs.curves[i]));
    for (i=0; i<rhs.sils.size(); i++)
        sils.push_back(new avtSILMetaData(*rhs.sils[i]));
    for (i=0; i<rhs.defaultPlots.size(); i++)
        defaultPlots.push_back(new avtDefaultPlotMetaData(*rhs.defaultPlots[i]));
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::operator=
//
//  Arguments:
//      rhs   :  the source object
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 19 10:57:04 PDT 2000
//    Copy over cycles.
//
//    Hank Childs, Mon Mar 11 09:22:45 PST 2002
//    Copy over new data members.
//
//    Hank Childs, Wed Sep  4 11:32:48 PDT 2002
//    Copy over expressions.
//
//    Sean Ahern, Fri Dec 13 16:50:00 PST 2002
//    Changed to use the ExpressionList state object.
//
//    Brad Whitlock, Tue Mar 25 14:31:31 PST 2003
//    I added the isVirtualDatabase, timeStepPath, timeStepNames fields.
//
//    Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//    Add support for curves.
//
//    Hank Childs, Thu Aug 14 08:16:07 PDT 2003
//    Copy over the database name.
//
//    Walter Herrera, Tue Sep 04 15:08:48 PST 2003
//    Copy defaultPlots.
//
//    Hank Childs, Sat Sep 20 08:32:38 PDT 2003
//    Copy over tensor data.
//
//    Hank Childs, Fri Mar  5 11:21:06 PST 2004
//    Copy over file format.
//
// ****************************************************************************

const avtDatabaseMetaData &
avtDatabaseMetaData::operator=(const avtDatabaseMetaData &rhs)
{
    databaseName       = rhs.databaseName;
    fileFormat         = rhs.fileFormat;
    hasTemporalExtents = rhs.hasTemporalExtents;
    minTemporalExtents = rhs.minTemporalExtents;
    maxTemporalExtents = rhs.maxTemporalExtents;
    numStates          = rhs.numStates;
    isVirtualDatabase  = rhs.isVirtualDatabase;
    mustRepopulateOnStateChange = rhs.mustRepopulateOnStateChange;
    timeStepPath       = rhs.timeStepPath;
    timeStepNames      = rhs.timeStepNames;
    cyclesAreAccurate  = rhs.cyclesAreAccurate;
    cycles             = rhs.cycles;
    timesAreAccurate   = rhs.timesAreAccurate;
    times              = rhs.times;
    exprList           = rhs.exprList;

    int i;
    for (i=0; i<meshes.size(); i++)
        delete meshes[i];
    meshes.clear();
    for (i=0; i<scalars.size(); i++)
        delete scalars[i];
    scalars.clear();
    for (i=0; i<vectors.size(); i++)
        delete vectors[i];
    vectors.clear();
    for (i=0; i<tensors.size(); i++)
        delete tensors[i];
    tensors.clear();
    for (i=0; i<symm_tensors.size(); i++)
        delete symm_tensors[i];
    symm_tensors.clear();
    for (i=0; i<materials.size(); i++)
        delete materials[i];
    materials.clear();
    for (i=0; i<species.size(); i++)
        delete species[i];
    species.clear();
    for (i=0; i<curves.size(); i++)
        delete curves[i];
    curves.clear();
    for (i=0; i<sils.size(); i++)
        delete sils[i];
    sils.clear();
    for (i=0; i<defaultPlots.size(); i++)
        delete defaultPlots[i];
    defaultPlots.clear();

    for (i=0; i<rhs.meshes.size(); i++)
        meshes.push_back(new avtMeshMetaData(*rhs.meshes[i]));
    for (i=0; i<rhs.scalars.size(); i++)
        scalars.push_back(new avtScalarMetaData(*rhs.scalars[i]));
    for (i=0; i<rhs.vectors.size(); i++)
        vectors.push_back(new avtVectorMetaData(*rhs.vectors[i]));
    for (i=0; i<rhs.tensors.size(); i++)
        tensors.push_back(new avtTensorMetaData(*rhs.tensors[i]));
    for (i=0; i<rhs.symm_tensors.size(); i++)
        symm_tensors.push_back(
                         new avtSymmetricTensorMetaData(*rhs.symm_tensors[i]));
    for (i=0; i<rhs.materials.size(); i++)
        materials.push_back(new avtMaterialMetaData(*rhs.materials[i]));
    for (i=0; i<rhs.species.size(); i++)
        species.push_back(new avtSpeciesMetaData(*rhs.species[i]));
    for (i=0; i<rhs.curves.size(); i++)
        curves.push_back(new avtCurveMetaData(*rhs.curves[i]));
    for (i=0; i<rhs.sils.size(); i++)
        sils.push_back(new avtSILMetaData(*rhs.sils[i]));
    for (i=0; i<rhs.defaultPlots.size(); i++)
        defaultPlots.push_back(
                             new avtDefaultPlotMetaData(*rhs.defaultPlots[i]));

    return *this;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData destructor
//
//  Programmer:  Hank Childs
//  Creation:    September 1, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//    Add support for curves.
//
//    Walter Herrera, Tue Sep 04 15:08:48 PST 2003
//    Add support for defaultPlots.
//
//    Hank Childs, Sat Sep 20 08:32:38 PDT 2003
//    Delete tensors.
//
// ****************************************************************************

avtDatabaseMetaData::~avtDatabaseMetaData()
{
    std::vector<avtMeshMetaData *>::iterator mit;
    for (mit = meshes.begin() ; mit != meshes.end() ; mit++)
    {
        delete (*mit);
    }

    std::vector<avtScalarMetaData *>::iterator sit;
    for (sit = scalars.begin() ; sit != scalars.end() ; sit++)
    {
        delete (*sit);
    }

    std::vector<avtVectorMetaData *>::iterator vit;
    for (vit = vectors.begin() ; vit != vectors.end() ; vit++)
    {
        delete (*vit);
    }

    std::vector<avtTensorMetaData *>::iterator ten_it;
    for (ten_it = tensors.begin() ; ten_it != tensors.end() ; ten_it++)
    {
        delete (*ten_it);
    }

    std::vector<avtSymmetricTensorMetaData *>::iterator st_it;
    for (st_it = symm_tensors.begin() ; st_it != symm_tensors.end() ; st_it++)
    {
        delete (*st_it);
    }

    std::vector<avtMaterialMetaData *>::iterator mait;
    for (mait = materials.begin() ; mait != materials.end() ; mait++)
    {
        delete (*mait);
    }

    std::vector<avtSpeciesMetaData *>::iterator spit;
    for (spit = species.begin() ; spit != species.end() ; spit++)
    {
        delete (*spit);
    }

    std::vector<avtCurveMetaData *>::iterator cit;
    for (cit = curves.begin() ; cit != curves.end() ; cit++)
    {
        delete (*cit);
    }

    std::vector<avtDefaultPlotMetaData *>::iterator defpltit;
    for (defpltit = defaultPlots.begin() ; defpltit != defaultPlots.end() ;
                                                                   defpltit++)
    {
        delete (*defpltit);
    }

    std::vector<avtSILMetaData *>::iterator silit;
    for (silit = sils.begin() ; silit != sils.end() ; silit++)
    {
        delete (*silit);
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetNumStates
//
//  Arguments:
//      int   the number of states
//
//  Programmer: Jeremy Meredith
//  Creation:   September 12, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 11 10:00:14 PST 2002 
//    Set up boolean vectors dependent on number of states.
//
//    Brad Whitlock, Tue Mar 25 14:32:13 PST 2003
//    I added the time step names.
//
// ****************************************************************************

void
avtDatabaseMetaData::SetNumStates(int n)
{
    numStates = n;
    cyclesAreAccurate.clear();
    timesAreAccurate.clear();
    cycles.clear();
    times.clear();
    timeStepNames.clear();
    for (int i = 0 ; i < numStates ; i++)
    {
        timeStepNames.push_back("");
        cyclesAreAccurate.push_back(0);
        timesAreAccurate.push_back(0);
        cycles.push_back(0);
        times.push_back(0.);
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetTemporalExtents
//
//  Purpose:
//      Sets the minimum and maximum temporal extents.
//
//  Arguments:
//      min    The minimum temporal extents.
//      max    The maximum temporal extents.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 09:57:20 PST 2002
//    Changed type to double.
//
// ****************************************************************************

void
avtDatabaseMetaData::SetTemporalExtents(double min, double max)
{
    hasTemporalExtents = true;
    minTemporalExtents = min;
    maxTemporalExtents = max;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetCycles
//
//  Arguments:
//      c       The cycles in a std::vector.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

void
avtDatabaseMetaData::SetCycles(std::vector<int> &c)
{
    cycles = c;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetTimes
//
//  Arguments:
//      t       The times in a std::vector.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::SetTimes(std::vector<double> &t)
{
    times = t;
}

// ****************************************************************************
// Method: avtDatabaseMetaData::SetTimeStepPath
//
// Purpose: 
//   Sets the timestep path.
//
// Arguments:
//   tsp : The new timestep path.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 2 13:28:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtDatabaseMetaData::SetTimeStepPath(const std::string &tsp)
{
    timeStepPath = tsp;
}

// ****************************************************************************
// Method: avtDatabaseMetaData::SetTimeStepNames
//
// Purpose: 
//   Sets the timestep names in the metadata.
//
// Arguments:
//   tsn : The time step names.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 14:33:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtDatabaseMetaData::SetTimeStepNames(const std::vector<std::string> &tsn)
{
    timeStepNames = tsn;
}

// ****************************************************************************
//  Method: avtDatabaseMetaData::SetCycleIsAccurate
//
//  Purpose:
//      Sets a boolean indicating whether a cycle number is accurate.
//
//  Arguments:
//      b       A boolean indicating whether a cycle is accurate.
//      ts      The timestep b corresponds to.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::SetCycleIsAccurate(bool b, int ts)
{
    cyclesAreAccurate[ts] = (b ? 1 : 0);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetCyclesAreAccurate
//
//  Purpose:
//      Sets a boolean indicating whether the cycle numbers are accurate.
//
//  Arguments:
//      b       A boolean indicating whether the cycle std::vector is accurate.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::SetCyclesAreAccurate(bool b)
{
    for (int i = 0 ; i < cyclesAreAccurate.size() ; i++)
    {
        cyclesAreAccurate[i] = (b ? 1 : 0);
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::IsCycleAccurate
//
//  Purpose:
//      Gets whether a single cycle is accurate.
//
//  Arguments:
//      ts       A timestep index.
//
//  Returns:     true if the cycle is accurate, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    March 11, 2002
//
// ****************************************************************************

bool
avtDatabaseMetaData::IsCycleAccurate(int ts) const
{
    return (cyclesAreAccurate[ts] != 0 ? true : false);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetTimeIsAccurate
//
//  Purpose:
//      Sets a boolean indicating whether a specific timestep is accurate.
//
//  Arguments:
//      b       A boolean indicating whether a time is accurate.
//      ts      The timestep b corresponds to.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::SetTimeIsAccurate(bool b, int ts)
{
    timesAreAccurate[ts] = (b ? 1 : 0);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetTimesAreAccurate
//
//  Purpose:
//      Sets a boolean indicating whether the times are accurate.
//
//  Arguments:
//      b       A boolean indicating whether the times std::vector is accurate.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::SetTimesAreAccurate(bool b)
{
    for (int i = 0 ; i < timesAreAccurate.size() ; i++)
    {
        timesAreAccurate[i] = (b ? 1 : 0);
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::IsTimeAccurate
//
//  Purpose:
//      Gets whether a single time is accurate.
//
//  Arguments:
//      ts       A timestep index.
//
//  Returns:     true if the time is accurate, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    March 11, 2002
//
// ****************************************************************************

bool
avtDatabaseMetaData::IsTimeAccurate(int ts) const
{
    return (timesAreAccurate[ts] != 0 ? true : false);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetCycle
//
//  Purpose:
//      Sets a cycle for a specific timestep.
//
//  Arguments:
//      ts       The timestep.
//      c        The cycle number.
//
//  Programmer:  Hank Childs
//  Creation:    March 11, 2002
// ****************************************************************************

void
avtDatabaseMetaData::SetCycle(int ts, int c)
{
    cycles[ts] = c;
    cyclesAreAccurate[ts] = true;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetTime
//
//  Purpose:
//      Sets a time for a specific timestep.
//
//  Arguments:
//      ts       The timestep.
//      t        The time.
//
//  Programmer:  Hank Childs
//  Creation:    March 11, 2002
// ****************************************************************************

void
avtDatabaseMetaData::SetTime(int ts, double t)
{
    times[ts] = t;
    timesAreAccurate[ts] = true;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      mmd    A mesh meta data object.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtMeshMetaData *mmd)
{
    meshes.push_back(mmd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      smd    A scalar meta data object.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtScalarMetaData *smd)
{
    scalars.push_back(smd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      vmd    A vector meta data object.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtVectorMetaData *vmd)
{
    vectors.push_back(vmd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      tmd    A tensor meta data object.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtTensorMetaData *tmd)
{
    tensors.push_back(tmd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//     stmd   A symmetric tensor meta data object.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtSymmetricTensorMetaData *stmd)
{
    symm_tensors.push_back(stmd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      mmd    A material meta data object.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtMaterialMetaData *mmd)
{
    materials.push_back(mmd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      smd    A species meta data object.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtSpeciesMetaData *smd)
{
    species.push_back(smd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      cmd    A curve meta data object.
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2003
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtCurveMetaData *cmd)
{
    curves.push_back(cmd);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      smd    A SIL meta data object.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 28, 2003
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtSILMetaData *smd)
{
    smd->Validate();
    sils.push_back(smd);
}

// ****************************************************************************
//  Method: avtDatabaseMetaData::Add
//
//  Arguments:
//      pmd    A default plot meta data object.
//
//  Programmer: Walter Herrera
//  Creation:   Septemver 04, 2003
//
// ****************************************************************************

void
avtDatabaseMetaData::Add(avtDefaultPlotMetaData *pmd)
{
    defaultPlots.push_back(pmd);
}

// ****************************************************************************
//  Method: avtDatabaseMetaData::SetExtents
//
//  Purpose:
//      Sets the extents of a variable.
//
//  Arguments:
//      name      The name of the variable to set extents for.
//      extents   The extents for the variable.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug 28 13:42:03 PDT 2003
//    Test for 'name' matching 'blockTitle' or 'groupTitle' in MeshMetaData.
// 
// ****************************************************************************

void
avtDatabaseMetaData::SetExtents(std::string name, const float *extents)
{
    bool   foundVar = false;

    std::vector<avtMeshMetaData *>::iterator mit;
    for (mit = meshes.begin() ; mit != meshes.end() ; mit++)
    {
        if (((*mit)->name == name) ||
            ((*mit)->blockTitle == name) ||
            ((*mit)->groupTitle == name))
        {
            (*mit)->SetExtents(extents);
            foundVar = true;
        }
    }

    std::vector<avtScalarMetaData *>::iterator sit;
    for (sit = scalars.begin() ; sit != scalars.end() ; sit++)
    {
        if ((*sit)->name == name)
        {
            (*sit)->SetExtents(extents);
            foundVar = true;
        }
    }

    std::vector<avtVectorMetaData *>::iterator vit;
    for (vit = vectors.begin() ; vit != vectors.end() ; vit++)
    {
        if ((*vit)->name == name)
        {
            (*vit)->SetExtents(extents);
            foundVar = true;
        }
    }

    if (! foundVar)
    {
        EXCEPTION1(InvalidVariableException, name);
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::GetNDomains
//
//  Purpose:
//      Gets the number of domains for this variable.
//
//  Arguments:
//      var     A variable name.
//
//  Returns:    The number of domains for var.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:50:51 PDT 2003
//    Added support for curves.
//
//    Hank Childs, Fri Sep 12 09:11:26 PDT 2003
//    Re-wrote so this could be designated const.
//
//    Hank Childs, Mon Dec  1 14:06:19 PST 2003
//    Made a more informative error message.
//
// ****************************************************************************

int
avtDatabaseMetaData::GetNDomains(std::string var) const
{
    int  i;

    std::string  meshname = MeshForVar(var);

    int nmeshes = meshes.size();
    for (i = 0 ; i < nmeshes ; i++)
        if (meshes[i]->name == meshname)
            return meshes[i]->numBlocks;

    int ncurves = curves.size();
    for (i = 0 ; i < ncurves ; i++)
    {
        if (curves[i]->name == meshname)
        {
            return 1;
        }
    }

    debug1 << "Unable to find mesh \"" << meshname.c_str() << "\" associated with "
           << "variable \"" << var.c_str() << "\"." << endl;
    EXCEPTION1(InvalidVariableException, var);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::DetermineVarType
//
//  Purpose:
//      Determines the type of the variable argument.
//
//  Arguments:
//      var_in  A variable name.
//
//  Returns:    The type of var.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep  5 13:53:15 PDT 2002 
//    If var is compound, parse it. 
//
//    Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//    Add support for curves.
//
//    Sean Ahern, Wed Feb  5 16:30:36 PST 2003
//    Added support for expressions.
//
//    Kathleen Bonnell, Thu Aug 28 13:42:03 PDT 2003
//    Test for 'var' matching 'blockTitle' or 'groupTitle' in MeshMetaData.
//
//    Hank Childs, Fri Sep 12 09:11:26 PDT 2003
//    Made modification so that routine could be 'const'.
//
//    Hank Childs, Sat Sep 20 08:32:38 PDT 2003
//    Add support for tensors.
//
// ****************************************************************************

avtVarType
avtDatabaseMetaData::DetermineVarType(std::string var_in) const
{
    int  i;

    // If the variable is an expression, we need to find a "real" variable
    // name to work with.
    ExprNode *tree = ParsingExprList::GetExpressionTree(var_in);
    while (tree != NULL)
    {
        var_in = *tree->GetVarLeaves().begin();
        tree = ParsingExprList::GetExpressionTree(var_in);
    }

    std::string var; 
    if (!VarIsCompound(var_in))
    {
        var = var_in;
    }
    else 
    {
        ParseCompoundForVar(var_in, var);
    }

    int nmeshes = meshes.size();
    for (i = 0 ; i < nmeshes ; i++)
    {
        if ((meshes[i]->name == var) || 
            (meshes[i]->blockTitle == var) ||
            (meshes[i]->groupTitle == var))
        {
            return AVT_MESH;
        }
    }

    int nvectors = vectors.size();
    for (i = 0 ; i < nvectors ; i++)
    {
        if (vectors[i]->name == var)
        {
            return AVT_VECTOR_VAR;
        }
    }

    int ntensors = tensors.size();
    for (i = 0 ; i < ntensors ; i++)
    {
        if (tensors[i]->name == var)
        {
            return AVT_TENSOR_VAR;
        }
    }

    int nsymmtensors = symm_tensors.size();
    for (i = 0 ; i < nsymmtensors ; i++)
    {
        if (symm_tensors[i]->name == var)
        {
            return AVT_SYMMETRIC_TENSOR_VAR;
        }
    }

    int nscalars = scalars.size();
    for (i = 0 ; i < nscalars ; i++)
    {
        if (scalars[i]->name == var)
        {
            return AVT_SCALAR_VAR;
        }
    }

    int nmats = materials.size();
    for (i = 0 ; i < nmats ; i++)
    {
        if (materials[i]->name == var)
        {
            return AVT_MATERIAL;
        }
    }

    int nspecies = species.size();
    for (i = 0 ; i < nspecies ; i++)
    {
        if (species[i]->name == var)
        {
            return AVT_MATSPECIES;
        }
    }

    int ncurves = curves.size();
    for (i = 0 ; i < ncurves ; i++)
    {
        if (curves[i]->name == var)
        {
            return AVT_CURVE;
        }
    }

    EXCEPTION1(InvalidVariableException, var);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::MeshForVar
//
//  Purpose:
//      Determines the mesh that the variable argument is defined on.
//
//  Arguments:
//      var     A variable name.
//
//  Returns:    The mesh that var is defined on.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep  5 13:53:15 PDT 2002 
//    If var is compound, parse it.
//
//    Hank Childs, Fri Aug  1 21:35:00 PDT 2003
//    Have curve plots return themselves.
//
//    Sean Ahern, Fri Dec 13 11:04:50 PST 2002
//    Added expression support.
//
//    Kathleen Bonnell, Thu Aug 28 13:42:03 PDT 2003
//    Test for 'var' matching 'blockTitle' or 'groupTitle' in MeshMetaData.
//
//    Hank Childs, Fri Sep 12 09:17:33 PDT 2003
//    Re-coded some sections so this routine could be 'const'.
//
//    Hank Childs, Sat Sep 20 08:49:16 PDT 2003
//    Add support for tensors.
//
// ****************************************************************************

std::string
avtDatabaseMetaData::MeshForVar(std::string var) const
{
    int   i;

    // Check if we even have a variable.
    if (var == "")
    {
        debug1 << "avtDatabaseMetaData::MeshForVar: Null variable passed." 
               << endl;
        EXCEPTION1(InvalidVariableException, var);
    }

    // If the variable is an expression, we need to find a "real" variable
    // name to work with.
    ExprNode *tree = ParsingExprList::GetExpressionTree(var);
    while (tree != NULL)
    {
        var = *tree->GetVarLeaves().begin();
        tree = ParsingExprList::GetExpressionTree(var);
    }

    // If the variable is compound, parse out the variable name.
    if (VarIsCompound(var))
    {
        std::string meshName;
        ParseCompoundForMesh(var, meshName);
        return meshName;
    }

    // Look through the meshes.
    int nmeshes = meshes.size();
    for (i = 0 ; i < nmeshes ; i++)
    {
        if (meshes[i]->name == var)
        {
            //
            // The mesh is defined on itself??  A little weird, but this is
            // convenient for some routines.
            //
            return var;
        }
        else if ((meshes[i]->blockTitle == var) ||
                 (meshes[i]->groupTitle == var))
        {
            return meshes[i]->name;
        }
    }

    // Look through the vectors.
    int nvectors = vectors.size();
    for (i = 0 ; i < nvectors ; i++)
    {
        if (vectors[i]->name == var)
        {
            return vectors[i]->meshName;
        }
    }

    // Look through the tensors.
    int ntensors = tensors.size();
    for (i = 0 ; i < ntensors ; i++)
    {
        if (tensors[i]->name == var)
        {
            return tensors[i]->meshName;
        }
    }

    // Look through the symmteric tensors.
    int nsymmtensors = symm_tensors.size();
    for (i = 0 ; i < nsymmtensors ; i++)
    {
        if (symm_tensors[i]->name == var)
        {
            return symm_tensors[i]->meshName;
        }
    }

    // Look through the scalars.
    int nscalars = scalars.size();
    for (i = 0 ; i < nscalars ; i++)
    {
        if (scalars[i]->name == var)
        {
            return scalars[i]->meshName;
        }
    }

    // Look through the materials.
    int nmats = materials.size();
    for (i = 0 ; i < nmats ; i++)
    {
        if (materials[i]->name == var)
        {
            return materials[i]->meshName;
        }
    }

    // Look through the species.
    int nspecies = species.size();
    for (i = 0 ; i < nspecies ; i++)
    {
        if (species[i]->name == var)
        {
            return species[i]->meshName;
        }
    }

    // Look through the curves.
    int ncurves = curves.size();
    for (i = 0 ; i < ncurves ; i++)
    {
        if (curves[i]->name == var)
        {
            return var;
        }
    }

    // Look through the sils.
    for (i = 0 ; i < sils.size(); i++)
    {
        const std::vector<avtSILCollectionMetaData *> &collections = 
                                                          sils[i]->collections;
        for (int j = 0; j < collections.size(); j++)
        {
           if (collections[j]->classOfCollection == var)
              return sils[i]->meshName;
        }
    }


    EXCEPTION1(InvalidVariableException, var);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::MaterialOnMesh
//
//  Purpose:
//      Finds a material for a mesh.
//
//  Arguments:
//      mesh    A mesh name.
//
//  Returns:    The name of a material defined on that mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  6 14:52:23 PST 2003
//    Issue a warning to the debug files if there are multiple materials on a
//    mesh.
//
//    Hank Childs, Fri Sep 12 09:17:33 PDT 2003
//    Re-coded some sections so this routine could be 'const'.
//
// ****************************************************************************

std::string
avtDatabaseMetaData::MaterialOnMesh(std::string mesh) const
{
    std::string rv = "";
    bool foundValue = false;

    int nmats = materials.size();
    for (int i = 0 ; i < nmats ; i++)
    {
        if (materials[i]->meshName == mesh)
        {
            if (foundValue)
            {
                debug1 << "WARNING: screwy file.  There are multiple materials"
                       << " (" << rv.c_str() << " and " 
                       << materials[i]->name.c_str() << ") defined"
                       << " on the same mesh." << endl;
                debug1 << "There are assumption in the VisIt code that this "
                       << "will never happen." << endl; 
            }

            rv = materials[i]->name;
            foundValue = true;
        }
    }

    if (foundValue)
    {
        return rv;
    }

    EXCEPTION1(InvalidVariableException, mesh);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SpeciesOnMesh
//
//  Purpose:
//      Finds a species for a mesh.
//
//  Arguments:
//      mesh    A mesh name.
//
//  Returns:    The name of a species defined on that mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 17, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 12 09:17:33 PDT 2003
//    Re-coded some sections so this routine could be 'const'.
//
// ****************************************************************************

std::string
avtDatabaseMetaData::SpeciesOnMesh(std::string mesh) const
{
    int nspecies = species.size();
    for (int i = 0 ; i < nspecies ; i++)
    {
        if (species[i]->meshName == mesh)
        {
            return species[i]->name;
        }
    }

    EXCEPTION1(InvalidVariableException, mesh);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::GetMaterialOnMesh
//
//  Purpose:
//      Gets a material for a mesh and returns it.
//
//  Arguments:
//      mesh    A mesh name.
//
//  Returns:    The material defined on that mesh.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Mar  6 14:52:23 PST 2003
//    Issue a warning to the debug files if there are multiple materials on a
//    mesh.
//
//    Hank Childs, Fri Sep 12 09:17:33 PDT 2003
//    Re-coded some sections so this routine could be 'const'.
//
// ****************************************************************************

const avtMaterialMetaData *
avtDatabaseMetaData::GetMaterialOnMesh(std::string mesh) const
{
    const avtMaterialMetaData *rv = NULL;

    int nmaterials = materials.size();
    for (int i = 0 ; i < nmaterials ; i++)
    {
        if (materials[i]->meshName == mesh)
        {
            if (rv != NULL)
            {
                debug1 << "WARNING: screwy file.  There are multiple materials"
                       << " (" << rv << " and " << materials[i]->name.c_str() 
                       << ") defined on the same mesh." << endl;
                debug1 << "There are assumptions in the VisIt code that this "
                       << "will never happen." << endl; 
            }
            rv = materials[i];
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::GetSpeciesOnMesh
//
//  Purpose:
//      Gets a species for a mesh and returns it.
//
//  Arguments:
//      mesh    A mesh name.
//
//  Returns:    The species defined on that mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 14, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 12 09:17:33 PDT 2003
//    Re-coded some sections so this routine could be 'const'.
//
// ****************************************************************************

const avtSpeciesMetaData *
avtDatabaseMetaData::GetSpeciesOnMesh(std::string mesh) const
{
    int nspecies = species.size();
    for (int i = 0 ; i < nspecies ; i++)
    {
        if (species[i]->meshName == mesh)
        {
            return species[i];
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::Print
//
//  Purpose:
//      Prints out all of the meta-data objects in the database meta-data
//      object.  Meant for debugging only.
//
//  Arguments:
//      out     The stream to print out to.
//      indent  The indentation level for each line.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 12 14:58:42 PDT 2000
//    Added code to print the number of time states.
//
//    Hank Childs, Fri Sep 15 19:07:21 PDT 2000
//    Print out Temporal extents and cycles.
//
//    Brad Whitlock, Tue Aug 20 15:23:59 PST 2002
//    I made it so categories that contain no entries are not printed.
//
//    Hank Childs, Wed Sep  4 11:32:48 PDT 2002
//    Print out information related to expressions.
//
//    Sean Ahern, Fri Dec 13 16:50:58 PST 2002
//    Changed to use the ExprssionList state objects.
//
//    Sean Ahern, Mon Mar 17 23:48:43 America/Los_Angeles 2003
//    Changed the names of expression types.
//
//    Brad Whitlock, Wed Apr 2 12:01:10 PDT 2003
//    I made it print out the timestep names if it is a virtual database.
//
//    Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//    Add support for curves.
//
//    Hank Childs, Sat Sep 20 08:49:16 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Fri Mar  5 11:21:06 PST 2004
//    Print file format.
//
// ****************************************************************************

void
avtDatabaseMetaData::Print(ostream &out, int indent) const
{
    Indent(out, indent);
    out << "Database: " << databaseName.c_str() << endl;

    Indent(out, indent);
    out << "File format: " << fileFormat.c_str() << endl;

    Indent(out, indent);
    out << "Num Time States: " << numStates << endl;

    Indent(out, indent);
    out << "MetaData" << (mustRepopulateOnStateChange ? " IS " : " is NOT ") << 
        "repopulated on state changes" << endl;

    Indent(out, indent);
    if (hasTemporalExtents)
    {
        out << "Temporal extents are from " << minTemporalExtents << " to "
            << maxTemporalExtents << "." << endl;
    }
    else
    {
        out << "The temporal extents are not set." << endl;
    }

    Indent(out, indent);
    if (cycles.size() == 0)
    {
        out << "The cycles are not set." << endl;
    }
    else
    {
        out << "Cycles: ";
        for (int i = 0; i < cycles.size(); ++i)
        {
            out << cycles[i];
            if(i < cycles.size() - 1)
                out << ", ";
        }
        out << endl;
    }

    if(isVirtualDatabase)
    {
        out << endl;
        out << "Database is virtual" << endl;
        out << "Timesteps are located in " << timeStepPath.c_str() << endl;
        out << "Timesteps:" << endl;
        for(int i = 0; i < timeStepNames.size(); ++i)
            out << "\t" << timeStepNames[i].c_str() << endl;
        out << endl;
    }

    if(meshes.begin() != meshes.end())
    {
        Indent(out, indent);
        out << "Meshes: " << endl;
    }
    std::vector< avtMeshMetaData * >::const_iterator mesh_it;
    for (mesh_it = meshes.begin() ; mesh_it != meshes.end() ; mesh_it++)
    {
        (*mesh_it)->Print(out, indent+1);
        out << endl;
    }
    
    if(scalars.begin() != scalars.end())
    {
        Indent(out, indent);
        out << "Scalars: " << endl;
    }
    std::vector< avtScalarMetaData * >::const_iterator scalar_it;
    for (scalar_it= scalars.begin() ; scalar_it != scalars.end() ; scalar_it++)
    {
        (*scalar_it)->Print(out, indent+1);
        out << endl;
    }
    
    if(vectors.begin() != vectors.end())
    {
        Indent(out, indent);
        out << "Vectors: " << endl;
    }
    std::vector< avtVectorMetaData * >::const_iterator vector_it;
    for (vector_it= vectors.begin() ; vector_it != vectors.end() ; vector_it++)
    {
        (*vector_it)->Print(out, indent+1);
        out << endl;
    }
    
    if(tensors.begin() != tensors.end())
    {
        Indent(out, indent);
        out << "Tensors: " << endl;
    }
    std::vector< avtTensorMetaData * >::const_iterator tensor_it;
    for (tensor_it= tensors.begin() ; tensor_it != tensors.end() ; tensor_it++)
    {
        (*tensor_it)->Print(out, indent+1);
        out << endl;
    }
    
    if(symm_tensors.begin() != symm_tensors.end())
    {
        Indent(out, indent);
        out << "Symmetric Tensors: " << endl;
    }
    std::vector< avtSymmetricTensorMetaData * >::const_iterator st_it;
    for (st_it= symm_tensors.begin() ; st_it != symm_tensors.end() ; st_it++)
    {
        (*st_it)->Print(out, indent+1);
        out << endl;
    }

    if(materials.begin() != materials.end())
    {
        Indent(out, indent);
        out << "Materials: " << endl;
    }
    std::vector< avtMaterialMetaData * >::const_iterator mat_it;
    for (mat_it = materials.begin() ; mat_it != materials.end() ; mat_it++)
    {
        (*mat_it)->Print(out, indent+1);
        out << endl;
    }
    
    if(species.begin() != species.end())
    {
        Indent(out, indent);
        out << "Material Species: " << endl;
    }
    std::vector< avtSpeciesMetaData * >::const_iterator spec_it;
    for (spec_it = species.begin() ; spec_it != species.end() ; spec_it++)
    {
        (*spec_it)->Print(out, indent+1);
        out << endl;
    }

    if(curves.begin() != curves.end())
    {
        Indent(out, indent);
        out << "Curves: " << endl;
    }
    std::vector< avtCurveMetaData * >::const_iterator cit;
    for (cit = curves.begin() ; cit != curves.end() ; cit++)
    {
        (*cit)->Print(out, indent+1);
        out << endl;
    }

    if(defaultPlots.begin() != defaultPlots.end())
    {
        Indent(out, indent);
        out << "Default Plots: " << endl;
    }
    std::vector< avtDefaultPlotMetaData * >::const_iterator plot_it;
    for (plot_it = defaultPlots.begin() ; plot_it != defaultPlots.end() ; 
         plot_it++)
    {
        (*plot_it)->Print(out, indent+1);
        out << endl;
    }

    if (exprList.GetNumExpressions() > 0)
    {
        Indent(out, indent);
        out << "Expressions:" << endl;
        for (int i = 0 ; i < exprList.GetNumExpressions() ; i++)
        {
            Indent(out, indent+1);
            std::string vartype("unknown var type");
            switch (exprList[i].GetType())
            {
              case Expression::Mesh:
                vartype = "mesh";
                break;
              case Expression::ScalarMeshVar:
                vartype = "scalar";
                break;
              case Expression::VectorMeshVar:
                vartype = "vector";
                break;
              case Expression::TensorMeshVar:
                vartype = "tensor";
                break;
              case Expression::SymmetricTensorMeshVar:
                vartype = "symmetrictensor";
                break;
              case Expression::Material:
                vartype = "material";
                break;
              case Expression::Species:
                vartype = "species";
                break;
              case Expression::Unknown:
                vartype = "unknown!";
                break;
            }
            out << exprList[i].GetName().c_str()
                << " (" << vartype.c_str() << "): \t"
                << exprList[i].GetDefinition().c_str() << endl;
        }
    }
    std::vector< avtSILMetaData * >::const_iterator sit;
    for (sit = sils.begin() ; sit != sils.end() ; sit++)
    {
        (*sit)->Print(out, indent+1);
        out << endl;
    }
}


// *******************************************************************
// Method: avtDatabaseMetaData::SelectAll
//
// Purpose: 
//   This method selects all of the components in the database 
//   metadata object so that they will be communicated.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
//   Hank Childs, Mon Sep 18 12:09:37 PDT 2000
//   Add cycles.
//
//   Hank Childs, Mon Mar 11 09:22:45 PST 2002
//   Add new data members.
//
//   Hank Childs, Wed Sep  4 11:32:48 PDT 2002
//   Add expressions.
//
//   Sean Ahern, Fri Dec 13 16:54:24 PST 2002
//   Changed to use an ExpressionList state object.
//
//   Brad Whitlock, Tue Mar 25 14:43:31 PST 2003
//   I added timestep names and timestep path.
//
//   Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//   Add support for curves.
//
//   Hank Childs, Thu Aug 14 08:16:07 PDT 2003
//   Added database name.
//
//   Walter Herrera, Tue Sep 04 15:405:17 PST 2003
//   Add defaultPlots
//
//   Hank Childs, Sat Sep 20 08:49:16 PDT 2003
//   Added tensors.
//
//   Hank Childs, Fri Mar  5 11:21:06 PST 2004
//   Add file format.
//
// *******************************************************************

void
avtDatabaseMetaData::SelectAll()
{
    Select(0, (void*)&databaseName);
    Select(1, (void*)&fileFormat);
    Select(2, (void*)&hasTemporalExtents);
    Select(3, (void*)&minTemporalExtents);
    Select(4, (void*)&maxTemporalExtents);
    Select(5, (void*)&numStates);
    Select(6, (void*)&isVirtualDatabase);

    Select(7, (void*)&timeStepPath);
    Select(8, (void*)&timeStepNames);
    Select(9, (void*)&cyclesAreAccurate);
    Select(10, (void*)&cycles);
    Select(11, (void*)&timesAreAccurate);
    Select(12, (void*)&times);

    Select(13, (void*)&meshes);
    Select(14, (void*)&scalars);
    Select(15, (void*)&vectors);
    Select(16, (void*)&tensors);
    Select(17, (void*)&symm_tensors);
    Select(18, (void*)&materials);
    Select(19, (void*)&species);
    Select(20, (void*)&curves);
    Select(21, (void*)&defaultPlots);
    Select(22, (void*)&exprList);
    Select(23, (void*)&mustRepopulateOnStateChange);
    Select(24, (void*)&sils);
}

// *******************************************************************
// Method: avtDatabaseMetaData::CreateSubAttributeGroup
//
// Purpose: 
//     This factory method creates one instance of one of the 
//     AttributeGroups contained in the vectors.
//
// Arguments:
//     n  :  the index used by SelectAll
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
//   Hank Childs, Mon Sep 18 12:09:37 PDT 2000
//   Bumped all of the numbers up one to account for cycles.
//
//   Hank Childs, Mon Mar 11 09:22:45 PST 2002
//   Bumped numbers again to account for new data members.
//
//   Brad Whitlock, Tue Mar 25 14:43:47 PST 2003
//   Bumped up the numbers.
//
//   Hank Childs, Fri Aug  1 11:08:21 PDT 2003
//   Add support for curves.
//
//   Hank Childs, Thu Aug 14 08:16:07 PDT 2003
//   Account for database name being added to front of list.
//
//   Hank Childs, Sat Sep 20 08:49:16 PDT 2003
//   Add support for tensors.
//
//   Hank Childs, Fri Mar  5 15:54:24 PST 2004
//   Update for indexing change due to file format types being added.
//
// *******************************************************************

AttributeGroup *
avtDatabaseMetaData::CreateSubAttributeGroup(int n)
{
    switch (n)
    {
      case 13:
        return new avtMeshMetaData;
      case 14:
        return new avtScalarMetaData;
      case 15:
        return new avtVectorMetaData;
      case 16:
        return new avtTensorMetaData;
      case 17:
        return new avtSymmetricTensorMetaData;
      case 18:
        return new avtMaterialMetaData;
      case 19:
        return new avtSpeciesMetaData;
      case 20:
        return new avtCurveMetaData;
      case 21:
        return new avtDefaultPlotMetaData;
      case 24:
        return new avtSILMetaData;
      default:
        return NULL;
    }
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetMesh
//
// Purpose: 
//     This returns the metadata for the nth mesh in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtMeshMetaData *
avtDatabaseMetaData::GetMesh(int n) const
{
    return meshes[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetMesh
//
// Purpose: 
//     This returns the metadata for the mesh in the file whose name is n.
//
// Arguments:
//     n  :  the name of the mesh object
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtMeshMetaData *
avtDatabaseMetaData::GetMesh(const std::string &n) const
{
    for (int i=0; i<meshes.size(); i++)
        if (meshes[i]->name == n)
            return meshes[i];
    return NULL;
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetScalar
//
// Purpose: 
//     This returns the metadata for the nth scalar in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtScalarMetaData *
avtDatabaseMetaData::GetScalar(int n) const
{
    return scalars[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetScalar
//
// Purpose: 
//     This returns the metadata for the scalar in the file whose name is n.
//
// Arguments:
//     n  :  the name of the scalar object
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtScalarMetaData *
avtDatabaseMetaData::GetScalar(const std::string &n) const
{
    for (int i=0; i<scalars.size(); i++)
        if (scalars[i]->name == n)
            return scalars[i];
    return NULL;
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetVector
//
// Purpose: 
//     This returns the metadata for the nth vector in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtVectorMetaData *
avtDatabaseMetaData::GetVector(int n) const
{
    return vectors[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetVector
//
// Purpose: 
//     This returns the metadata for the vector in the file whose name is n.
//
// Arguments:
//     n  :  the name of the vector object
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtVectorMetaData *
avtDatabaseMetaData::GetVector(const std::string &n) const
{
    for (int i=0; i<vectors.size(); i++)
        if (vectors[i]->name == n)
            return vectors[i];
    return NULL;
}

// ****************************************************************************
// Method: avtDatabaseMetaData::GetTensor
//
// Purpose: 
//     This returns the metadata for the nth tensor in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Hank Childs
// Creation:   September 20, 2003
//
// ****************************************************************************

const avtTensorMetaData *
avtDatabaseMetaData::GetTensor(int n) const
{
    return tensors[n];
}

// ****************************************************************************
// Method: avtDatabaseMetaData::GetTensor
//
// Purpose: 
//     This returns the metadata for the tensor in the file whose name is n.
//
// Arguments:
//     n  :  the name of the tensor object
//
// Programmer: Hank Childs
// Creation:   September 20, 2003
//
// ****************************************************************************

const avtTensorMetaData *
avtDatabaseMetaData::GetTensor(const std::string &n) const
{
    for (int i=0; i<tensors.size(); i++)
        if (tensors[i]->name == n)
            return tensors[i];
    return NULL;
}

// ****************************************************************************
// Method: avtDatabaseMetaData::GetSymmTensor
//
// Purpose: 
//     This returns the metadata for the nth symmetric tensor in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Hank Childs
// Creation:   September 20, 2003
//
// ****************************************************************************

const avtSymmetricTensorMetaData *
avtDatabaseMetaData::GetSymmTensor(int n) const
{
    return symm_tensors[n];
}

// ****************************************************************************
// Method: avtDatabaseMetaData::GetSymmTensor
//
// Purpose: 
//     This returns the metadata for the symmetric tensor in the file whose
//     name is n.
//
// Arguments:
//     n  :  the name of the tensor object
//
// Programmer: Hank Childs
// Creation:   September 20, 2003
//
// ****************************************************************************

const avtSymmetricTensorMetaData *
avtDatabaseMetaData::GetSymmTensor(const std::string &n) const
{
    for (int i=0; i<symm_tensors.size(); i++)
        if (symm_tensors[i]->name == n)
            return symm_tensors[i];
    return NULL;
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetMaterial
//
// Purpose: 
//     This returns the metadata for the nth material in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtMaterialMetaData *
avtDatabaseMetaData::GetMaterial(int n) const
{
    return materials[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetMaterial
//
// Purpose: 
//     This returns the metadata for the material in the file whose name is n.
//
// Arguments:
//     n  :  the name of the material object
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Kathleen Bonnell, Thu Sep  5 13:53:15 PDT 2002 
//   Call ParseCompoundForVar, in case the variable is compound. 
// *******************************************************************

const avtMaterialMetaData *
avtDatabaseMetaData::GetMaterial(const std::string &n) const
{
    std::string n2;
    const_cast<avtDatabaseMetaData*>(this)->ParseCompoundForVar(n, n2);
    for (int i=0; i<materials.size(); i++)
        if (materials[i]->name == n2)
            return materials[i];
    return NULL;
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetSpecies
//
// Purpose: 
//     This returns the metadata for the nth species in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtSpeciesMetaData *
avtDatabaseMetaData::GetSpecies(int n) const
{
    return species[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetSpecies
//
// Purpose: 
//     This returns the metadata for the species in the file whose name is n.
//
// Arguments:
//     n  :  the name of the species object
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// *******************************************************************

const avtSpeciesMetaData *
avtDatabaseMetaData::GetSpecies(const std::string &n) const
{
    for (int i=0; i<species.size(); i++)
        if (species[i]->name == n)
            return species[i];
    return NULL;
}
    

// *******************************************************************
// Method: avtDatabaseMetaData::GetCurve
//
// Purpose: 
//     This returns the metadata for the nth curve in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Hank Childs
// Creation:   August 1, 2003
//
// Modifications:
//   
// *******************************************************************

const avtCurveMetaData *
avtDatabaseMetaData::GetCurve(int n) const
{
    return curves[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetCurve
//
// Purpose: 
//     This returns the metadata for the curve in the file whose name
//     is n.
//
// Arguments:
//     n  :  the name of the curve object
//
// Programmer: Hank Childs
// Creation:   August  1, 2003
//
// Modifications:
//   
// *******************************************************************

const avtCurveMetaData *
avtDatabaseMetaData::GetCurve(const std::string &n) const
{
    for (int i=0; i<curves.size(); i++)
        if (curves[i]->name == n)
            return curves[i];
    return NULL;
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetSIL
//
// Purpose: 
//     This returns the metadata for the nth SIL in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Mark C. Miller 
// Creation:   04Sep03 
// *******************************************************************

const avtSILMetaData *
avtDatabaseMetaData::GetSIL(int n) const
{
    return sils[n];
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetSIL
//
// Purpose: 
//     This returns the metadata for the SIL in the file whose name
//     is n.
//
// Arguments:
//     n  :  the name of the SIL object
//
// Programmer: Mark C. Miller
// Creation:   04Sep03 
// *******************************************************************

const avtSILMetaData *
avtDatabaseMetaData::GetSIL(const std::string &n) const
{
    for (int i=0; i<sils.size(); i++)
        if (sils[i]->meshName == n)
            return sils[i];
    return NULL;
}

// *******************************************************************
// Method: avtDatabaseMetaData::GetDefaultPlot
//
// Purpose: 
//     This returns the metadata for the nth default plot in the file.
//
// Arguments:
//     n  :  the index into the array
//
// Programmer: Walter Herrera
// Creation:   September 04, 2003
//
// Modifications:
//   
// *******************************************************************

const avtDefaultPlotMetaData *
avtDatabaseMetaData::GetDefaultPlot(int n) const
{
    return defaultPlots[n];
}

// ****************************************************************************
//  Method: avtDatabaseMetaData::SetBlocksForMesh
//
//  Purpose:
//      Resets the number of blocks a mesh can have without violating
//      encapsulation.
//
//  Arguments:
//      index    The index of the mesh in the std::vector "meshes".
//      nBlocks  The number of blocks the mesh actually has.
//
//  Programmer:  Hank Childs
//  Creation:    October 11, 2001
//
// ****************************************************************************

void
avtDatabaseMetaData::SetBlocksForMesh(int index, int nBlocks)
{
    if (index < 0 || index >= meshes.size())
    {
        EXCEPTION2(BadIndexException, index, meshes.size());
    }

    meshes[index]->numBlocks = nBlocks;
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetContainsGhostZones
//
//  Purpose:
//      Sets whether a particular mesh has ghost zones.
//
//  Arguments:
//      name     The name of a mesh.
//      val      True if it has ghost zones, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    September 30, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:58:01 PDT 2003
//    No longer throw an exception.  This is a valid case for curves.
//
// ****************************************************************************

void
avtDatabaseMetaData::SetContainsGhostZones(std::string name, avtGhostType val)
{
    for (int i = 0 ; i < GetNumMeshes() ; i++)
    {
        if (meshes[i]->name == name)
        {
            meshes[i]->containsGhostZones = val;
            return;
        }
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::SetContainsOriginalCells
//
//  Purpose:
//      Sets whether a particular mesh has original cells array. 
//
//  Arguments:
//      name     The name of a mesh.
//      val      True if it has ghost zones, false otherwise.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 25, 2003 
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:58:01 PDT 2003
//    No longer throw an exception.  This is a valid case for curves.
//
// ****************************************************************************

void
avtDatabaseMetaData::SetContainsOriginalCells(std::string name, bool val)
{
    for (int i = 0 ; i < GetNumMeshes() ; i++)
    {
        if (meshes[i]->name == name)
        {
            meshes[i]->containsOriginalCells = val;
            return;
        }
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::AddGroupInformation
//
//  Purpose:
//      Sets the group information for all applicable meshes in this object.
//      They are applicable if they have the correct number of blocks.
//
//  Arguments:
//      nGroups   The total number of groups.
//      nBlocks   The number of blocks in the mesh.
//      groupIds  The group index for each block.
//
//  Programmer:  Hank Childs
//  Creation:    October 11, 2001
//
// ****************************************************************************

void
avtDatabaseMetaData::AddGroupInformation(int nGroups, int nBlocks,
                                         std::vector<int> &groupIds)
{
    for (int i = 0 ; i < meshes.size() ; i++)
    {
        if (meshes[i]->numBlocks == nBlocks)
        {
            meshes[i]->numGroups = nGroups;
            meshes[i]->groupIds  = groupIds;
        }
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::UnsetExtents
//
//  Purpose:
//      Allows all of the extents for all of the meshes, scalar vars, and
//      std::vector vars to be unset at one time.
//
//  Programmer:  Hank Childs
//  Creation:    March 6, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::UnsetExtents(void)
{
    int  i;

    for (i = 0 ; i < meshes.size() ; i++)
    {
        meshes[i]->UnsetExtents();
    }
    for (i = 0 ; i < scalars.size() ; i++)
    {
        scalars[i]->UnsetExtents();
    }
    for (i = 0 ; i < vectors.size() ; i++)
    {
        vectors[i]->UnsetExtents();
    }
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::AddExpression
//
//  Purpose:
//      Adds a new expression to the database.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2002
//
// ****************************************************************************

void
avtDatabaseMetaData::AddExpression(Expression *expr)
{
    expr->SetFromDB(true);
    expr->SetDbName(databaseName);
    exprList.AddExpression(*expr);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::GetExpression
//
//  Purpose:
//      Get a particular expression.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2002
//
// ****************************************************************************

const Expression *
avtDatabaseMetaData::GetExpression(int expr) const
{
    return &(exprList[expr]);
}


// ****************************************************************************
//  Method: avtDatabaseMetaData::GetNumberOfExpressions
//
//  Purpose:
//      Get the number of expressions defined for this database.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2002
//
// ****************************************************************************

int
avtDatabaseMetaData::GetNumberOfExpressions(void) const
{
    return exprList.GetNumExpressions();
}


// ****************************************************************************
//  Function: avtDatabaseMetaData::VarIsCompound
//
//  Purpose:
//    Determines if a variable is in compound form: 'CategoryName(MeshName)'. 
//
//  Arguments:
//    v       The (possibly) compound variable. 
//
//  Returns:
//    true if the variable is in compound form, false otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2002 
//
// ****************************************************************************

bool
avtDatabaseMetaData::VarIsCompound(const std::string &v) const
{
    int beg = -1, end = -1;

    // find the mesh name enclosed in parentheses
    beg = v.find('(');
    end = v.find(')');

    if (beg == -1 || end == -1)
    {
        return false;
    }
    return true;
}


// ****************************************************************************
//  Function: avtDatabaseMetaData::ParseCompoundForMesh
//
//  Purpose:
//    Parses a compound variable of the form 'CategoryName(MeshName)' for
//    the mesh portion. 
//
//  Arguments:
//    inVar   The (possibly) compound variable to parse. 
//    outVar  A place to store the parsed mesh name. 
//
//  Notes:
//    If inVar is not in the correct compound form, then outVar set to inVar.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2002 
//
// ****************************************************************************

void
avtDatabaseMetaData::ParseCompoundForMesh(const std::string &inVar, 
      std::string &outVar) const
{
    int beg = -1, end = -1;
 
    // find the mesh name enclosed in parentheses
    beg = inVar.find('(');
    end = inVar.find(')');
 
    if (beg == -1 || end == -1)
    {
        // this is not a parseable variable.
        outVar = inVar;
        return;
    }
 
    //move past the first paren
    beg += 1;
    outVar = inVar.substr(beg, end - beg);
}


// ****************************************************************************
//  Function: avtDatabaseMetaData::ParseCompoundForCategory
//
//  Purpose:
//    Parses a compound variable of the form 'CategoryName(MeshName)' for
//    the category portion. 
//
//  Arguments:
//    inVar   The (possibly) compound variable to parse. 
//    outVar  A place to store the parsed category name. 
//
//  Notes:
//    If inVar is not in the correct compound form, then outVar set to inVar.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2002 
//
// ****************************************************************************


void
avtDatabaseMetaData::ParseCompoundForCategory(const std::string &inVar, 
      std::string &outVar) const
{
    int end = -1, beg = -1;
 
    // find the mesh name enclosed in parentheses
    beg = inVar.find('(');
    end = inVar.find(')');

    if (beg == -1 || end == -1)
    {
        // this is not a compound variable.
        outVar = inVar;
        return;
    }
    outVar = inVar.substr(0, beg);
}


// ****************************************************************************
//  Function: avtDatabaseMetaData::DetermineSubsetType
//
//  Purpose:
//    Determines the subset type of the passed  compound variable of the form 
//    'CategoryName(MeshName)'. 
//
//  Arguments:
//    inVar   The (possibly) compound variable. 
//
//  Returns:
//    The subset type (AVT_UNKNOWN_SUBSET) if inVar is not compound.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Aug 22 18:02:15 PDT 2003
//    Subset vars are no longer always 'compound', parse accordingly.
// 
// ****************************************************************************

avtSubsetType
avtDatabaseMetaData::DetermineSubsetType(const std::string &inVar) const
{
    std::string category, mesh;
 
    if (VarIsCompound(inVar))
    {
        ParseCompoundForMesh(inVar, mesh);
        ParseCompoundForCategory(inVar, category);
    }
    else 
    {
        category = inVar;
        mesh = MeshForVar(inVar);
    }

    //
    // determine which part of the var we want to return
    // HACKISH ... only checking domains, and probably incorrectly at that!
    //
    const avtMeshMetaData *mmd = GetMesh(mesh);
    std::string blockTitle;
    std::string groupTitle;
    if (mmd == NULL)
    {
        blockTitle = "domains";   // Not a lot we can do.
        groupTitle = "blocks";
    }
    else
    {
        blockTitle = mmd->blockTitle;
        groupTitle = mmd->groupTitle;
    }
 
    if (category == blockTitle)
    {
        return AVT_DOMAIN_SUBSET;
    }
    else if (category == groupTitle)
    {
        return AVT_GROUP_SUBSET; 
    }
    else
    {
        const avtMaterialMetaData *matmd = GetMaterialOnMesh(mesh); 

        if (matmd != NULL && matmd->name == category)
            return AVT_MATERIAL_SUBSET;
        else
            return AVT_UNKNOWN_SUBSET;
    }
}

// ****************************************************************************
//  Function: avtDatabaseMetaData::ParseCompoundForVar
//
//  Purpose:
//    Parses a compound variable of the form 'CategoryName(MeshName)' for
//    the variable portion. 
//
// Notes:
//    The desired variable is one that can be 'typed', which may be either 
//    CategoryName or MeshName, depending depending upon the subset. 
//    e.g. if the CategoryName matches with blockTile or groupTitle of the 
//    corresponding MeshMetaData, then the MeshName is returned, so that the 
//    variable can be typed as 'AVT_MESH_VAR'. 
//
//    This code pulled from gui. (QvisPlotManagerWidget).
//    If inVar is not in the correct compound form, then outVar set to inVar.
//
//  Arguments:
//    inVar   The (possibly) compound variable to parse. 
//    outVar  A place to store the parsed variable name. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 5, 2002 
//
// ****************************************************************************

void
avtDatabaseMetaData::ParseCompoundForVar(const std::string &inVar, 
    std::string &outVar) const
{
    if (!VarIsCompound(inVar))
    {
        outVar = inVar;
        return; 
    }

    avtSubsetType sT = DetermineSubsetType(inVar);

    switch (sT)
    {
        case AVT_DOMAIN_SUBSET : // fall-through
        case AVT_GROUP_SUBSET  :
            ParseCompoundForMesh(inVar, outVar);
            break; 
        case AVT_MATERIAL_SUBSET :
            ParseCompoundForCategory(inVar, outVar);
            break; 
        default:
            outVar = inVar;
            break; 
    }  
}


// ****************************************************************************
//  Function: Indent
//
//  Purpose:
//      Indents the proper amount as a convenience to routines that frequently
//      need to indent.
//
//  Arguments:
//      out     The stream to output to.
//      indent  The level to indent to.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2000
//
// ****************************************************************************

inline void
Indent(ostream &out, int indent)
{
    for (int i = 0 ; i < indent ; i++)
    {
        out << "\t";
    }
}
