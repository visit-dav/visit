/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <LineSamplerAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for LineSamplerAttributes::BeamType
//

static const char *BeamType_strings[] = {
"Parallel", "Fan"};

std::string
LineSamplerAttributes::BeamType_ToString(LineSamplerAttributes::BeamType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return BeamType_strings[index];
}

std::string
LineSamplerAttributes::BeamType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return BeamType_strings[index];
}

bool
LineSamplerAttributes::BeamType_FromString(const std::string &s, LineSamplerAttributes::BeamType &val)
{
    val = LineSamplerAttributes::Parallel;
    for(int i = 0; i < 2; ++i)
    {
        if(s == BeamType_strings[i])
        {
            val = (BeamType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::BeamAxis
//

static const char *BeamAxis_strings[] = {
"R", "Z"};

std::string
LineSamplerAttributes::BeamAxis_ToString(LineSamplerAttributes::BeamAxis t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return BeamAxis_strings[index];
}

std::string
LineSamplerAttributes::BeamAxis_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return BeamAxis_strings[index];
}

bool
LineSamplerAttributes::BeamAxis_FromString(const std::string &s, LineSamplerAttributes::BeamAxis &val)
{
    val = LineSamplerAttributes::R;
    for(int i = 0; i < 2; ++i)
    {
        if(s == BeamAxis_strings[i])
        {
            val = (BeamAxis)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::BeamShape
//

static const char *BeamShape_strings[] = {
"Line", "Cylinder", "Cone"
};

std::string
LineSamplerAttributes::BeamShape_ToString(LineSamplerAttributes::BeamShape t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return BeamShape_strings[index];
}

std::string
LineSamplerAttributes::BeamShape_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return BeamShape_strings[index];
}

bool
LineSamplerAttributes::BeamShape_FromString(const std::string &s, LineSamplerAttributes::BeamShape &val)
{
    val = LineSamplerAttributes::Line;
    for(int i = 0; i < 3; ++i)
    {
        if(s == BeamShape_strings[i])
        {
            val = (BeamShape)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ViewDimension
//

static const char *ViewDimension_strings[] = {
"One", "Two", "Three"
};

std::string
LineSamplerAttributes::ViewDimension_ToString(LineSamplerAttributes::ViewDimension t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ViewDimension_strings[index];
}

std::string
LineSamplerAttributes::ViewDimension_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ViewDimension_strings[index];
}

bool
LineSamplerAttributes::ViewDimension_FromString(const std::string &s, LineSamplerAttributes::ViewDimension &val)
{
    val = LineSamplerAttributes::One;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ViewDimension_strings[i])
        {
            val = (ViewDimension)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Init utility for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void LineSamplerAttributes::Init()
{
    beamType = Parallel;
    beamShape = Line;
    radius = 5;
    divergence = 5;
    nBeams = 5;
    nLinearSamples = 100;
    nRadialSamples = 36;
    spacing = 5;
    angle = 45;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;
    beamAxis = Z;
    poloialAngle = 0;
    poloialTilt = 0;
    toroialAngle = 0;
    viewDimension = Three;

    LineSamplerAttributes::SelectAll();
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Copy utility for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void LineSamplerAttributes::Copy(const LineSamplerAttributes &obj)
{
    beamType = obj.beamType;
    beamShape = obj.beamShape;
    radius = obj.radius;
    divergence = obj.divergence;
    nBeams = obj.nBeams;
    nLinearSamples = obj.nLinearSamples;
    nRadialSamples = obj.nRadialSamples;
    spacing = obj.spacing;
    angle = obj.angle;
    origin[0] = obj.origin[0];
    origin[1] = obj.origin[1];
    origin[2] = obj.origin[2];

    beamAxis = obj.beamAxis;
    poloialAngle = obj.poloialAngle;
    poloialTilt = obj.poloialTilt;
    toroialAngle = obj.toroialAngle;
    viewDimension = obj.viewDimension;

    LineSamplerAttributes::SelectAll();
}

// Type map format string
const char *LineSamplerAttributes::TypeMapFormatString = LINESAMPLERATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t LineSamplerAttributes::TmfsStruct = {LINESAMPLERATTRIBUTES_TMFS};


// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Default constructor for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes() : 
    AttributeSubject(LineSamplerAttributes::TypeMapFormatString)
{
    LineSamplerAttributes::Init();
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Constructor for the derived classes of LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    LineSamplerAttributes::Init();
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Copy constructor for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes(const LineSamplerAttributes &obj) : 
    AttributeSubject(LineSamplerAttributes::TypeMapFormatString)
{
    LineSamplerAttributes::Copy(obj);
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes(const LineSamplerAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    LineSamplerAttributes::Copy(obj);
}

// ****************************************************************************
// Method: LineSamplerAttributes::~LineSamplerAttributes
//
// Purpose: 
//   Destructor for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::~LineSamplerAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: LineSamplerAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes& 
LineSamplerAttributes::operator = (const LineSamplerAttributes &obj)
{
    if (this == &obj) return *this;

    LineSamplerAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: LineSamplerAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LineSamplerAttributes::operator == (const LineSamplerAttributes &obj) const
{
    // Compare the origin arrays.
    bool origin_equal = true;
    for(int i = 0; i < 3 && origin_equal; ++i)
        origin_equal = (origin[i] == obj.origin[i]);

    // Create the return value
    return ((beamType == obj.beamType) &&
            (beamShape == obj.beamShape) &&
            (radius == obj.radius) &&
            (divergence == obj.divergence) &&
            (nBeams == obj.nBeams) &&
            (nLinearSamples == obj.nLinearSamples) &&
            (nRadialSamples == obj.nRadialSamples) &&
            (spacing == obj.spacing) &&
            (angle == obj.angle) &&
            origin_equal &&
            (beamAxis == obj.beamAxis) &&
            (poloialAngle == obj.poloialAngle) &&
            (poloialTilt == obj.poloialTilt) &&
            (toroialAngle == obj.toroialAngle) &&
            (viewDimension == obj.viewDimension));
}

// ****************************************************************************
// Method: LineSamplerAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LineSamplerAttributes::operator != (const LineSamplerAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: LineSamplerAttributes::TypeName
//
// Purpose: 
//   Type name method for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const std::string
LineSamplerAttributes::TypeName() const
{
    return "LineSamplerAttributes";
}

// ****************************************************************************
// Method: LineSamplerAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LineSamplerAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const LineSamplerAttributes *tmp = (const LineSamplerAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: LineSamplerAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
LineSamplerAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new LineSamplerAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: LineSamplerAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
LineSamplerAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new LineSamplerAttributes(*this);
    else
        retval = new LineSamplerAttributes;

    return retval;
}

// ****************************************************************************
// Method: LineSamplerAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
LineSamplerAttributes::SelectAll()
{
    Select(ID_beamType,       (void *)&beamType);
    Select(ID_beamShape,      (void *)&beamShape);
    Select(ID_radius,         (void *)&radius);
    Select(ID_divergence,     (void *)&divergence);
    Select(ID_nBeams,         (void *)&nBeams);
    Select(ID_nLinearSamples, (void *)&nLinearSamples);
    Select(ID_nRadialSamples, (void *)&nRadialSamples);
    Select(ID_spacing,        (void *)&spacing);
    Select(ID_angle,          (void *)&angle);
    Select(ID_origin,         (void *)origin, 3);
    Select(ID_beamAxis,       (void *)&beamAxis);
    Select(ID_poloialAngle,   (void *)&poloialAngle);
    Select(ID_poloialTilt,    (void *)&poloialTilt);
    Select(ID_toroialAngle,   (void *)&toroialAngle);
    Select(ID_viewDimension,  (void *)&viewDimension);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LineSamplerAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LineSamplerAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    LineSamplerAttributes defaultObject;
    bool addToParent = false;
    // Create a node for LineSamplerAttributes.
    DataNode *node = new DataNode("LineSamplerAttributes");

    if(completeSave || !FieldsEqual(ID_beamType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("beamType", BeamType_ToString(beamType)));
    }

    if(completeSave || !FieldsEqual(ID_beamShape, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("beamShape", BeamShape_ToString(beamShape)));
    }

    if(completeSave || !FieldsEqual(ID_radius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radius", radius));
    }

    if(completeSave || !FieldsEqual(ID_divergence, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("divergence", divergence));
    }

    if(completeSave || !FieldsEqual(ID_nBeams, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nBeams", nBeams));
    }

    if(completeSave || !FieldsEqual(ID_nLinearSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nLinearSamples", nLinearSamples));
    }

    if(completeSave || !FieldsEqual(ID_nRadialSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nRadialSamples", nRadialSamples));
    }

    if(completeSave || !FieldsEqual(ID_spacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("spacing", spacing));
    }

    if(completeSave || !FieldsEqual(ID_angle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("angle", angle));
    }

    if(completeSave || !FieldsEqual(ID_origin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("origin", origin, 3));
    }

    if(completeSave || !FieldsEqual(ID_beamAxis, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("beamAxis", BeamAxis_ToString(beamAxis)));
    }

    if(completeSave || !FieldsEqual(ID_poloialAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloialAngle", poloialAngle));
    }

    if(completeSave || !FieldsEqual(ID_poloialTilt, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloialTilt", poloialTilt));
    }

    if(completeSave || !FieldsEqual(ID_toroialAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroialAngle", toroialAngle));
    }

    if(completeSave || !FieldsEqual(ID_viewDimension, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("viewDimension", ViewDimension_ToString(viewDimension)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: LineSamplerAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
LineSamplerAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("LineSamplerAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("beamType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetBeamType(BeamType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BeamType value;
            if(BeamType_FromString(node->AsString(), value))
                SetBeamType(value);
        }
    }
    if((node = searchNode->GetNode("beamShape")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetBeamShape(BeamShape(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BeamShape value;
            if(BeamShape_FromString(node->AsString(), value))
                SetBeamShape(value);
        }
    }
    if((node = searchNode->GetNode("radius")) != 0)
        SetRadius(node->AsDouble());
    if((node = searchNode->GetNode("divergence")) != 0)
        SetDivergence(node->AsDouble());
    if((node = searchNode->GetNode("nBeams")) != 0)
        SetNBeams(node->AsInt());
    if((node = searchNode->GetNode("nLinearSamples")) != 0)
        SetNLinearSamples(node->AsInt());
    if((node = searchNode->GetNode("nRadialSamples")) != 0)
        SetNRadialSamples(node->AsInt());
    if((node = searchNode->GetNode("spacing")) != 0)
        SetSpacing(node->AsDouble());
    if((node = searchNode->GetNode("angle")) != 0)
        SetAngle(node->AsDouble());
    if((node = searchNode->GetNode("origin")) != 0)
        SetOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("beamAxis")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetBeamAxis(BeamAxis(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BeamAxis value;
            if(BeamAxis_FromString(node->AsString(), value))
                SetBeamAxis(value);
        }
    }
    if((node = searchNode->GetNode("poloialAngle")) != 0)
        SetPoloialAngle(node->AsDouble());
    if((node = searchNode->GetNode("poloialTilt")) != 0)
        SetPoloialTilt(node->AsDouble());
    if((node = searchNode->GetNode("toroialAngle")) != 0)
        SetToroialAngle(node->AsDouble());
    if((node = searchNode->GetNode("viewDimension")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetViewDimension(ViewDimension(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ViewDimension value;
            if(ViewDimension_FromString(node->AsString(), value))
                SetViewDimension(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
LineSamplerAttributes::SetBeamType(LineSamplerAttributes::BeamType beamType_)
{
    beamType = beamType_;
    Select(ID_beamType, (void *)&beamType);
}

void
LineSamplerAttributes::SetBeamShape(LineSamplerAttributes::BeamShape beamShape_)
{
    beamShape = beamShape_;
    Select(ID_beamShape, (void *)&beamShape);
}

void
LineSamplerAttributes::SetRadius(double radius_)
{
    radius = radius_;
    Select(ID_radius, (void *)&radius);
}

void
LineSamplerAttributes::SetDivergence(double divergence_)
{
    divergence = divergence_;
    Select(ID_divergence, (void *)&divergence);
}

void
LineSamplerAttributes::SetNBeams(int nBeams_)
{
    nBeams = nBeams_;
    Select(ID_nBeams, (void *)&nBeams);
}

void
LineSamplerAttributes::SetNLinearSamples(int nLinearSamples_)
{
    nLinearSamples = nLinearSamples_;
    Select(ID_nLinearSamples, (void *)&nLinearSamples);
}

void
LineSamplerAttributes::SetNRadialSamples(int nRadialSamples_)
{
    nRadialSamples = nRadialSamples_;
    Select(ID_nRadialSamples, (void *)&nRadialSamples);
}

void
LineSamplerAttributes::SetSpacing(double spacing_)
{
    spacing = spacing_;
    Select(ID_spacing, (void *)&spacing);
}

void
LineSamplerAttributes::SetAngle(double angle_)
{
    angle = angle_;
    Select(ID_angle, (void *)&angle);
}

void
LineSamplerAttributes::SetOrigin(const double *origin_)
{
    origin[0] = origin_[0];
    origin[1] = origin_[1];
    origin[2] = origin_[2];
    Select(ID_origin, (void *)origin, 3);
}

void
LineSamplerAttributes::SetBeamAxis(LineSamplerAttributes::BeamAxis beamAxis_)
{
    beamAxis = beamAxis_;
    Select(ID_beamAxis, (void *)&beamAxis);
}

void
LineSamplerAttributes::SetPoloialAngle(double poloialAngle_)
{
    poloialAngle = poloialAngle_;
    Select(ID_poloialAngle, (void *)&poloialAngle);
}

void
LineSamplerAttributes::SetPoloialTilt(double poloialTilt_)
{
    poloialTilt = poloialTilt_;
    Select(ID_poloialTilt, (void *)&poloialTilt);
}

void
LineSamplerAttributes::SetToroialAngle(double toroialAngle_)
{
    toroialAngle = toroialAngle_;
    Select(ID_toroialAngle, (void *)&toroialAngle);
}

void
LineSamplerAttributes::SetViewDimension(LineSamplerAttributes::ViewDimension viewDimension_)
{
    viewDimension = viewDimension_;
    Select(ID_viewDimension, (void *)&viewDimension);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

LineSamplerAttributes::BeamType
LineSamplerAttributes::GetBeamType() const
{
    return BeamType(beamType);
}

LineSamplerAttributes::BeamShape
LineSamplerAttributes::GetBeamShape() const
{
    return BeamShape(beamShape);
}

double
LineSamplerAttributes::GetRadius() const
{
    return radius;
}

double
LineSamplerAttributes::GetDivergence() const
{
    return divergence;
}

int
LineSamplerAttributes::GetNBeams() const
{
    return nBeams;
}

int
LineSamplerAttributes::GetNLinearSamples() const
{
    return nLinearSamples;
}

int
LineSamplerAttributes::GetNRadialSamples() const
{
    return nRadialSamples;
}

double
LineSamplerAttributes::GetSpacing() const
{
    return spacing;
}

double
LineSamplerAttributes::GetAngle() const
{
    return angle;
}

const double *
LineSamplerAttributes::GetOrigin() const
{
    return origin;
}

double *
LineSamplerAttributes::GetOrigin()
{
    return origin;
}

LineSamplerAttributes::BeamAxis
LineSamplerAttributes::GetBeamAxis() const
{
    return BeamAxis(beamAxis);
}

double
LineSamplerAttributes::GetPoloialAngle() const
{
    return poloialAngle;
}

double
LineSamplerAttributes::GetPoloialTilt() const
{
    return poloialTilt;
}

double
LineSamplerAttributes::GetToroialAngle() const
{
    return toroialAngle;
}

LineSamplerAttributes::ViewDimension
LineSamplerAttributes::GetViewDimension() const
{
    return ViewDimension(viewDimension);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
LineSamplerAttributes::SelectOrigin()
{
    Select(ID_origin, (void *)origin, 3);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LineSamplerAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
LineSamplerAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_beamType:       return "beamType";
    case ID_beamShape:      return "beamShape";
    case ID_radius:         return "radius";
    case ID_divergence:     return "divergence";
    case ID_nBeams:         return "nBeams";
    case ID_nLinearSamples: return "nLinearSamples";
    case ID_nRadialSamples: return "nRadialSamples";
    case ID_spacing:        return "spacing";
    case ID_angle:          return "angle";
    case ID_origin:         return "origin";
    case ID_beamAxis:       return "beamAxis";
    case ID_poloialAngle:   return "poloialAngle";
    case ID_poloialTilt:    return "poloialTilt";
    case ID_toroialAngle:   return "toroialAngle";
    case ID_viewDimension:  return "viewDimension";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LineSamplerAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
LineSamplerAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_beamType:       return FieldType_enum;
    case ID_beamShape:      return FieldType_enum;
    case ID_radius:         return FieldType_double;
    case ID_divergence:     return FieldType_double;
    case ID_nBeams:         return FieldType_int;
    case ID_nLinearSamples: return FieldType_int;
    case ID_nRadialSamples: return FieldType_int;
    case ID_spacing:        return FieldType_double;
    case ID_angle:          return FieldType_double;
    case ID_origin:         return FieldType_doubleArray;
    case ID_beamAxis:       return FieldType_enum;
    case ID_poloialAngle:   return FieldType_double;
    case ID_poloialTilt:    return FieldType_double;
    case ID_toroialAngle:   return FieldType_double;
    case ID_viewDimension:  return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: LineSamplerAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
LineSamplerAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_beamType:       return "enum";
    case ID_beamShape:      return "enum";
    case ID_radius:         return "double";
    case ID_divergence:     return "double";
    case ID_nBeams:         return "int";
    case ID_nLinearSamples: return "int";
    case ID_nRadialSamples: return "int";
    case ID_spacing:        return "double";
    case ID_angle:          return "double";
    case ID_origin:         return "doubleArray";
    case ID_beamAxis:       return "enum";
    case ID_poloialAngle:   return "double";
    case ID_poloialTilt:    return "double";
    case ID_toroialAngle:   return "double";
    case ID_viewDimension:  return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LineSamplerAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LineSamplerAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const LineSamplerAttributes &obj = *((const LineSamplerAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_beamType:
        {  // new scope
        retval = (beamType == obj.beamType);
        }
        break;
    case ID_beamShape:
        {  // new scope
        retval = (beamShape == obj.beamShape);
        }
        break;
    case ID_radius:
        {  // new scope
        retval = (radius == obj.radius);
        }
        break;
    case ID_divergence:
        {  // new scope
        retval = (divergence == obj.divergence);
        }
        break;
    case ID_nBeams:
        {  // new scope
        retval = (nBeams == obj.nBeams);
        }
        break;
    case ID_nLinearSamples:
        {  // new scope
        retval = (nLinearSamples == obj.nLinearSamples);
        }
        break;
    case ID_nRadialSamples:
        {  // new scope
        retval = (nRadialSamples == obj.nRadialSamples);
        }
        break;
    case ID_spacing:
        {  // new scope
        retval = (spacing == obj.spacing);
        }
        break;
    case ID_angle:
        {  // new scope
        retval = (angle == obj.angle);
        }
        break;
    case ID_origin:
        {  // new scope
        // Compare the origin arrays.
        bool origin_equal = true;
        for(int i = 0; i < 3 && origin_equal; ++i)
            origin_equal = (origin[i] == obj.origin[i]);

        retval = origin_equal;
        }
        break;
    case ID_beamAxis:
        {  // new scope
        retval = (beamAxis == obj.beamAxis);
        }
        break;
    case ID_poloialAngle:
        {  // new scope
        retval = (poloialAngle == obj.poloialAngle);
        }
        break;
    case ID_poloialTilt:
        {  // new scope
        retval = (poloialTilt == obj.poloialTilt);
        }
        break;
    case ID_toroialAngle:
        {  // new scope
        retval = (toroialAngle == obj.toroialAngle);
        }
        break;
    case ID_viewDimension:
        {  // new scope
        retval = (viewDimension == obj.viewDimension);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

