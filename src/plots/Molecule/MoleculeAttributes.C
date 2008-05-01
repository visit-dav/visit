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

#include <MoleculeAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for MoleculeAttributes::AtomRenderingMode
//

static const char *AtomRenderingMode_strings[] = {
"NoAtoms", "SphereAtoms", "ImposterAtoms"
};

std::string
MoleculeAttributes::AtomRenderingMode_ToString(MoleculeAttributes::AtomRenderingMode t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return AtomRenderingMode_strings[index];
}

std::string
MoleculeAttributes::AtomRenderingMode_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return AtomRenderingMode_strings[index];
}

bool
MoleculeAttributes::AtomRenderingMode_FromString(const std::string &s, MoleculeAttributes::AtomRenderingMode &val)
{
    val = MoleculeAttributes::NoAtoms;
    for(int i = 0; i < 3; ++i)
    {
        if(s == AtomRenderingMode_strings[i])
        {
            val = (AtomRenderingMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for MoleculeAttributes::RadiusType
//

static const char *RadiusType_strings[] = {
"Fixed", "Covalent", "Atomic", 
"Variable"};

std::string
MoleculeAttributes::RadiusType_ToString(MoleculeAttributes::RadiusType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return RadiusType_strings[index];
}

std::string
MoleculeAttributes::RadiusType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return RadiusType_strings[index];
}

bool
MoleculeAttributes::RadiusType_FromString(const std::string &s, MoleculeAttributes::RadiusType &val)
{
    val = MoleculeAttributes::Fixed;
    for(int i = 0; i < 4; ++i)
    {
        if(s == RadiusType_strings[i])
        {
            val = (RadiusType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for MoleculeAttributes::BondColoringMode
//

static const char *BondColoringMode_strings[] = {
"ColorByAtom", "SingleColor"};

std::string
MoleculeAttributes::BondColoringMode_ToString(MoleculeAttributes::BondColoringMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return BondColoringMode_strings[index];
}

std::string
MoleculeAttributes::BondColoringMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return BondColoringMode_strings[index];
}

bool
MoleculeAttributes::BondColoringMode_FromString(const std::string &s, MoleculeAttributes::BondColoringMode &val)
{
    val = MoleculeAttributes::ColorByAtom;
    for(int i = 0; i < 2; ++i)
    {
        if(s == BondColoringMode_strings[i])
        {
            val = (BondColoringMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for MoleculeAttributes::BondRenderingMode
//

static const char *BondRenderingMode_strings[] = {
"NoBonds", "LineBonds", "CylinderBonds"
};

std::string
MoleculeAttributes::BondRenderingMode_ToString(MoleculeAttributes::BondRenderingMode t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return BondRenderingMode_strings[index];
}

std::string
MoleculeAttributes::BondRenderingMode_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return BondRenderingMode_strings[index];
}

bool
MoleculeAttributes::BondRenderingMode_FromString(const std::string &s, MoleculeAttributes::BondRenderingMode &val)
{
    val = MoleculeAttributes::NoBonds;
    for(int i = 0; i < 3; ++i)
    {
        if(s == BondRenderingMode_strings[i])
        {
            val = (BondRenderingMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for MoleculeAttributes::DetailLevel
//

static const char *DetailLevel_strings[] = {
"Low", "Medium", "High", 
"Super"};

std::string
MoleculeAttributes::DetailLevel_ToString(MoleculeAttributes::DetailLevel t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return DetailLevel_strings[index];
}

std::string
MoleculeAttributes::DetailLevel_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return DetailLevel_strings[index];
}

bool
MoleculeAttributes::DetailLevel_FromString(const std::string &s, MoleculeAttributes::DetailLevel &val)
{
    val = MoleculeAttributes::Low;
    for(int i = 0; i < 4; ++i)
    {
        if(s == DetailLevel_strings[i])
        {
            val = (DetailLevel)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *MoleculeAttributes::TypeMapFormatString = "iiiiasffiifiissssbbfbf";

// ****************************************************************************
// Method: MoleculeAttributes::MoleculeAttributes
//
// Purpose: 
//   Constructor for the MoleculeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

MoleculeAttributes::MoleculeAttributes() : 
    AttributeSubject(MoleculeAttributes::TypeMapFormatString),
    bondSingleColor(128, 128, 128), radiusVariable("Default"), 
    elementColorTable("cpk_jmol"), residueTypeColorTable("amino_shapely"), 
    residueSequenceColorTable("Default"), continuousColorTable("Default")
{
    drawAtomsAs = SphereAtoms;
    scaleRadiusBy = Fixed;
    drawBondsAs = CylinderBonds;
    colorBonds = ColorByAtom;
    radiusScaleFactor = 1;
    radiusFixed = 0.3;
    atomSphereQuality = Medium;
    bondCylinderQuality = Medium;
    bondRadius = 0.12;
    bondLineWidth = 0;
    bondLineStyle = 0;
    legendFlag = true;
    minFlag = false;
    scalarMin = 0;
    maxFlag = false;
    scalarMax = 1;
}

// ****************************************************************************
// Method: MoleculeAttributes::MoleculeAttributes
//
// Purpose: 
//   Copy constructor for the MoleculeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

MoleculeAttributes::MoleculeAttributes(const MoleculeAttributes &obj) : 
    AttributeSubject(MoleculeAttributes::TypeMapFormatString)
{
    drawAtomsAs = obj.drawAtomsAs;
    scaleRadiusBy = obj.scaleRadiusBy;
    drawBondsAs = obj.drawBondsAs;
    colorBonds = obj.colorBonds;
    bondSingleColor = obj.bondSingleColor;
    radiusVariable = obj.radiusVariable;
    radiusScaleFactor = obj.radiusScaleFactor;
    radiusFixed = obj.radiusFixed;
    atomSphereQuality = obj.atomSphereQuality;
    bondCylinderQuality = obj.bondCylinderQuality;
    bondRadius = obj.bondRadius;
    bondLineWidth = obj.bondLineWidth;
    bondLineStyle = obj.bondLineStyle;
    elementColorTable = obj.elementColorTable;
    residueTypeColorTable = obj.residueTypeColorTable;
    residueSequenceColorTable = obj.residueSequenceColorTable;
    continuousColorTable = obj.continuousColorTable;
    legendFlag = obj.legendFlag;
    minFlag = obj.minFlag;
    scalarMin = obj.scalarMin;
    maxFlag = obj.maxFlag;
    scalarMax = obj.scalarMax;

    SelectAll();
}

// ****************************************************************************
// Method: MoleculeAttributes::~MoleculeAttributes
//
// Purpose: 
//   Destructor for the MoleculeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

MoleculeAttributes::~MoleculeAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: MoleculeAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the MoleculeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

MoleculeAttributes& 
MoleculeAttributes::operator = (const MoleculeAttributes &obj)
{
    if (this == &obj) return *this;
    drawAtomsAs = obj.drawAtomsAs;
    scaleRadiusBy = obj.scaleRadiusBy;
    drawBondsAs = obj.drawBondsAs;
    colorBonds = obj.colorBonds;
    bondSingleColor = obj.bondSingleColor;
    radiusVariable = obj.radiusVariable;
    radiusScaleFactor = obj.radiusScaleFactor;
    radiusFixed = obj.radiusFixed;
    atomSphereQuality = obj.atomSphereQuality;
    bondCylinderQuality = obj.bondCylinderQuality;
    bondRadius = obj.bondRadius;
    bondLineWidth = obj.bondLineWidth;
    bondLineStyle = obj.bondLineStyle;
    elementColorTable = obj.elementColorTable;
    residueTypeColorTable = obj.residueTypeColorTable;
    residueSequenceColorTable = obj.residueSequenceColorTable;
    continuousColorTable = obj.continuousColorTable;
    legendFlag = obj.legendFlag;
    minFlag = obj.minFlag;
    scalarMin = obj.scalarMin;
    maxFlag = obj.maxFlag;
    scalarMax = obj.scalarMax;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: MoleculeAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the MoleculeAttributes class.
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
MoleculeAttributes::operator == (const MoleculeAttributes &obj) const
{
    // Create the return value
    return ((drawAtomsAs == obj.drawAtomsAs) &&
            (scaleRadiusBy == obj.scaleRadiusBy) &&
            (drawBondsAs == obj.drawBondsAs) &&
            (colorBonds == obj.colorBonds) &&
            (bondSingleColor == obj.bondSingleColor) &&
            (radiusVariable == obj.radiusVariable) &&
            (radiusScaleFactor == obj.radiusScaleFactor) &&
            (radiusFixed == obj.radiusFixed) &&
            (atomSphereQuality == obj.atomSphereQuality) &&
            (bondCylinderQuality == obj.bondCylinderQuality) &&
            (bondRadius == obj.bondRadius) &&
            (bondLineWidth == obj.bondLineWidth) &&
            (bondLineStyle == obj.bondLineStyle) &&
            (elementColorTable == obj.elementColorTable) &&
            (residueTypeColorTable == obj.residueTypeColorTable) &&
            (residueSequenceColorTable == obj.residueSequenceColorTable) &&
            (continuousColorTable == obj.continuousColorTable) &&
            (legendFlag == obj.legendFlag) &&
            (minFlag == obj.minFlag) &&
            (scalarMin == obj.scalarMin) &&
            (maxFlag == obj.maxFlag) &&
            (scalarMax == obj.scalarMax));
}

// ****************************************************************************
// Method: MoleculeAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the MoleculeAttributes class.
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
MoleculeAttributes::operator != (const MoleculeAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: MoleculeAttributes::TypeName
//
// Purpose: 
//   Type name method for the MoleculeAttributes class.
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
MoleculeAttributes::TypeName() const
{
    return "MoleculeAttributes";
}

// ****************************************************************************
// Method: MoleculeAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the MoleculeAttributes class.
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
MoleculeAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const MoleculeAttributes *tmp = (const MoleculeAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: MoleculeAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the MoleculeAttributes class.
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
MoleculeAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new MoleculeAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: MoleculeAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the MoleculeAttributes class.
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
MoleculeAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new MoleculeAttributes(*this);
    else
        retval = new MoleculeAttributes;

    return retval;
}

// ****************************************************************************
// Method: MoleculeAttributes::SelectAll
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
MoleculeAttributes::SelectAll()
{
    Select(ID_drawAtomsAs,               (void *)&drawAtomsAs);
    Select(ID_scaleRadiusBy,             (void *)&scaleRadiusBy);
    Select(ID_drawBondsAs,               (void *)&drawBondsAs);
    Select(ID_colorBonds,                (void *)&colorBonds);
    Select(ID_bondSingleColor,           (void *)&bondSingleColor);
    Select(ID_radiusVariable,            (void *)&radiusVariable);
    Select(ID_radiusScaleFactor,         (void *)&radiusScaleFactor);
    Select(ID_radiusFixed,               (void *)&radiusFixed);
    Select(ID_atomSphereQuality,         (void *)&atomSphereQuality);
    Select(ID_bondCylinderQuality,       (void *)&bondCylinderQuality);
    Select(ID_bondRadius,                (void *)&bondRadius);
    Select(ID_bondLineWidth,             (void *)&bondLineWidth);
    Select(ID_bondLineStyle,             (void *)&bondLineStyle);
    Select(ID_elementColorTable,         (void *)&elementColorTable);
    Select(ID_residueTypeColorTable,     (void *)&residueTypeColorTable);
    Select(ID_residueSequenceColorTable, (void *)&residueSequenceColorTable);
    Select(ID_continuousColorTable,      (void *)&continuousColorTable);
    Select(ID_legendFlag,                (void *)&legendFlag);
    Select(ID_minFlag,                   (void *)&minFlag);
    Select(ID_scalarMin,                 (void *)&scalarMin);
    Select(ID_maxFlag,                   (void *)&maxFlag);
    Select(ID_scalarMax,                 (void *)&scalarMax);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MoleculeAttributes::CreateNode
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
MoleculeAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    MoleculeAttributes defaultObject;
    bool addToParent = false;
    // Create a node for MoleculeAttributes.
    DataNode *node = new DataNode("MoleculeAttributes");

    if(completeSave || !FieldsEqual(ID_drawAtomsAs, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("drawAtomsAs", AtomRenderingMode_ToString(drawAtomsAs)));
    }

    if(completeSave || !FieldsEqual(ID_scaleRadiusBy, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleRadiusBy", RadiusType_ToString(scaleRadiusBy)));
    }

    if(completeSave || !FieldsEqual(ID_drawBondsAs, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("drawBondsAs", BondRenderingMode_ToString(drawBondsAs)));
    }

    if(completeSave || !FieldsEqual(ID_colorBonds, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorBonds", BondColoringMode_ToString(colorBonds)));
    }

        DataNode *bondSingleColorNode = new DataNode("bondSingleColor");
        if(bondSingleColor.CreateNode(bondSingleColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(bondSingleColorNode);
        }
        else
            delete bondSingleColorNode;
    if(completeSave || !FieldsEqual(ID_radiusVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radiusVariable", radiusVariable));
    }

    if(completeSave || !FieldsEqual(ID_radiusScaleFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radiusScaleFactor", radiusScaleFactor));
    }

    if(completeSave || !FieldsEqual(ID_radiusFixed, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radiusFixed", radiusFixed));
    }

    if(completeSave || !FieldsEqual(ID_atomSphereQuality, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("atomSphereQuality", DetailLevel_ToString(atomSphereQuality)));
    }

    if(completeSave || !FieldsEqual(ID_bondCylinderQuality, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("bondCylinderQuality", DetailLevel_ToString(bondCylinderQuality)));
    }

    if(completeSave || !FieldsEqual(ID_bondRadius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("bondRadius", bondRadius));
    }

    if(completeSave || !FieldsEqual(ID_bondLineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("bondLineWidth", bondLineWidth));
    }

    if(completeSave || !FieldsEqual(ID_bondLineStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("bondLineStyle", bondLineStyle));
    }

    if(completeSave || !FieldsEqual(ID_elementColorTable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("elementColorTable", elementColorTable));
    }

    if(completeSave || !FieldsEqual(ID_residueTypeColorTable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("residueTypeColorTable", residueTypeColorTable));
    }

    if(completeSave || !FieldsEqual(ID_residueSequenceColorTable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("residueSequenceColorTable", residueSequenceColorTable));
    }

    if(completeSave || !FieldsEqual(ID_continuousColorTable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("continuousColorTable", continuousColorTable));
    }

    if(completeSave || !FieldsEqual(ID_legendFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }

    if(completeSave || !FieldsEqual(ID_minFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minFlag", minFlag));
    }

    if(completeSave || !FieldsEqual(ID_scalarMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalarMin", scalarMin));
    }

    if(completeSave || !FieldsEqual(ID_maxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxFlag", maxFlag));
    }

    if(completeSave || !FieldsEqual(ID_scalarMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalarMax", scalarMax));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: MoleculeAttributes::SetFromNode
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
MoleculeAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("MoleculeAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("drawAtomsAs")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDrawAtomsAs(AtomRenderingMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            AtomRenderingMode value;
            if(AtomRenderingMode_FromString(node->AsString(), value))
                SetDrawAtomsAs(value);
        }
    }
    if((node = searchNode->GetNode("scaleRadiusBy")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetScaleRadiusBy(RadiusType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            RadiusType value;
            if(RadiusType_FromString(node->AsString(), value))
                SetScaleRadiusBy(value);
        }
    }
    if((node = searchNode->GetNode("drawBondsAs")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDrawBondsAs(BondRenderingMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BondRenderingMode value;
            if(BondRenderingMode_FromString(node->AsString(), value))
                SetDrawBondsAs(value);
        }
    }
    if((node = searchNode->GetNode("colorBonds")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetColorBonds(BondColoringMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BondColoringMode value;
            if(BondColoringMode_FromString(node->AsString(), value))
                SetColorBonds(value);
        }
    }
    if((node = searchNode->GetNode("bondSingleColor")) != 0)
        bondSingleColor.SetFromNode(node);
    if((node = searchNode->GetNode("radiusVariable")) != 0)
        SetRadiusVariable(node->AsString());
    if((node = searchNode->GetNode("radiusScaleFactor")) != 0)
        SetRadiusScaleFactor(node->AsFloat());
    if((node = searchNode->GetNode("radiusFixed")) != 0)
        SetRadiusFixed(node->AsFloat());
    if((node = searchNode->GetNode("atomSphereQuality")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetAtomSphereQuality(DetailLevel(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DetailLevel value;
            if(DetailLevel_FromString(node->AsString(), value))
                SetAtomSphereQuality(value);
        }
    }
    if((node = searchNode->GetNode("bondCylinderQuality")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetBondCylinderQuality(DetailLevel(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DetailLevel value;
            if(DetailLevel_FromString(node->AsString(), value))
                SetBondCylinderQuality(value);
        }
    }
    if((node = searchNode->GetNode("bondRadius")) != 0)
        SetBondRadius(node->AsFloat());
    if((node = searchNode->GetNode("bondLineWidth")) != 0)
        SetBondLineWidth(node->AsInt());
    if((node = searchNode->GetNode("bondLineStyle")) != 0)
        SetBondLineStyle(node->AsInt());
    if((node = searchNode->GetNode("elementColorTable")) != 0)
        SetElementColorTable(node->AsString());
    if((node = searchNode->GetNode("residueTypeColorTable")) != 0)
        SetResidueTypeColorTable(node->AsString());
    if((node = searchNode->GetNode("residueSequenceColorTable")) != 0)
        SetResidueSequenceColorTable(node->AsString());
    if((node = searchNode->GetNode("continuousColorTable")) != 0)
        SetContinuousColorTable(node->AsString());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("minFlag")) != 0)
        SetMinFlag(node->AsBool());
    if((node = searchNode->GetNode("scalarMin")) != 0)
        SetScalarMin(node->AsFloat());
    if((node = searchNode->GetNode("maxFlag")) != 0)
        SetMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("scalarMax")) != 0)
        SetScalarMax(node->AsFloat());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
MoleculeAttributes::SetDrawAtomsAs(MoleculeAttributes::AtomRenderingMode drawAtomsAs_)
{
    drawAtomsAs = drawAtomsAs_;
    Select(ID_drawAtomsAs, (void *)&drawAtomsAs);
}

void
MoleculeAttributes::SetScaleRadiusBy(MoleculeAttributes::RadiusType scaleRadiusBy_)
{
    scaleRadiusBy = scaleRadiusBy_;
    Select(ID_scaleRadiusBy, (void *)&scaleRadiusBy);
}

void
MoleculeAttributes::SetDrawBondsAs(MoleculeAttributes::BondRenderingMode drawBondsAs_)
{
    drawBondsAs = drawBondsAs_;
    Select(ID_drawBondsAs, (void *)&drawBondsAs);
}

void
MoleculeAttributes::SetColorBonds(MoleculeAttributes::BondColoringMode colorBonds_)
{
    colorBonds = colorBonds_;
    Select(ID_colorBonds, (void *)&colorBonds);
}

void
MoleculeAttributes::SetBondSingleColor(const ColorAttribute &bondSingleColor_)
{
    bondSingleColor = bondSingleColor_;
    Select(ID_bondSingleColor, (void *)&bondSingleColor);
}

void
MoleculeAttributes::SetRadiusVariable(const std::string &radiusVariable_)
{
    radiusVariable = radiusVariable_;
    Select(ID_radiusVariable, (void *)&radiusVariable);
}

void
MoleculeAttributes::SetRadiusScaleFactor(float radiusScaleFactor_)
{
    radiusScaleFactor = radiusScaleFactor_;
    Select(ID_radiusScaleFactor, (void *)&radiusScaleFactor);
}

void
MoleculeAttributes::SetRadiusFixed(float radiusFixed_)
{
    radiusFixed = radiusFixed_;
    Select(ID_radiusFixed, (void *)&radiusFixed);
}

void
MoleculeAttributes::SetAtomSphereQuality(MoleculeAttributes::DetailLevel atomSphereQuality_)
{
    atomSphereQuality = atomSphereQuality_;
    Select(ID_atomSphereQuality, (void *)&atomSphereQuality);
}

void
MoleculeAttributes::SetBondCylinderQuality(MoleculeAttributes::DetailLevel bondCylinderQuality_)
{
    bondCylinderQuality = bondCylinderQuality_;
    Select(ID_bondCylinderQuality, (void *)&bondCylinderQuality);
}

void
MoleculeAttributes::SetBondRadius(float bondRadius_)
{
    bondRadius = bondRadius_;
    Select(ID_bondRadius, (void *)&bondRadius);
}

void
MoleculeAttributes::SetBondLineWidth(int bondLineWidth_)
{
    bondLineWidth = bondLineWidth_;
    Select(ID_bondLineWidth, (void *)&bondLineWidth);
}

void
MoleculeAttributes::SetBondLineStyle(int bondLineStyle_)
{
    bondLineStyle = bondLineStyle_;
    Select(ID_bondLineStyle, (void *)&bondLineStyle);
}

void
MoleculeAttributes::SetElementColorTable(const std::string &elementColorTable_)
{
    elementColorTable = elementColorTable_;
    Select(ID_elementColorTable, (void *)&elementColorTable);
}

void
MoleculeAttributes::SetResidueTypeColorTable(const std::string &residueTypeColorTable_)
{
    residueTypeColorTable = residueTypeColorTable_;
    Select(ID_residueTypeColorTable, (void *)&residueTypeColorTable);
}

void
MoleculeAttributes::SetResidueSequenceColorTable(const std::string &residueSequenceColorTable_)
{
    residueSequenceColorTable = residueSequenceColorTable_;
    Select(ID_residueSequenceColorTable, (void *)&residueSequenceColorTable);
}

void
MoleculeAttributes::SetContinuousColorTable(const std::string &continuousColorTable_)
{
    continuousColorTable = continuousColorTable_;
    Select(ID_continuousColorTable, (void *)&continuousColorTable);
}

void
MoleculeAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
MoleculeAttributes::SetMinFlag(bool minFlag_)
{
    minFlag = minFlag_;
    Select(ID_minFlag, (void *)&minFlag);
}

void
MoleculeAttributes::SetScalarMin(float scalarMin_)
{
    scalarMin = scalarMin_;
    Select(ID_scalarMin, (void *)&scalarMin);
}

void
MoleculeAttributes::SetMaxFlag(bool maxFlag_)
{
    maxFlag = maxFlag_;
    Select(ID_maxFlag, (void *)&maxFlag);
}

void
MoleculeAttributes::SetScalarMax(float scalarMax_)
{
    scalarMax = scalarMax_;
    Select(ID_scalarMax, (void *)&scalarMax);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

MoleculeAttributes::AtomRenderingMode
MoleculeAttributes::GetDrawAtomsAs() const
{
    return AtomRenderingMode(drawAtomsAs);
}

MoleculeAttributes::RadiusType
MoleculeAttributes::GetScaleRadiusBy() const
{
    return RadiusType(scaleRadiusBy);
}

MoleculeAttributes::BondRenderingMode
MoleculeAttributes::GetDrawBondsAs() const
{
    return BondRenderingMode(drawBondsAs);
}

MoleculeAttributes::BondColoringMode
MoleculeAttributes::GetColorBonds() const
{
    return BondColoringMode(colorBonds);
}

const ColorAttribute &
MoleculeAttributes::GetBondSingleColor() const
{
    return bondSingleColor;
}

ColorAttribute &
MoleculeAttributes::GetBondSingleColor()
{
    return bondSingleColor;
}

const std::string &
MoleculeAttributes::GetRadiusVariable() const
{
    return radiusVariable;
}

std::string &
MoleculeAttributes::GetRadiusVariable()
{
    return radiusVariable;
}

float
MoleculeAttributes::GetRadiusScaleFactor() const
{
    return radiusScaleFactor;
}

float
MoleculeAttributes::GetRadiusFixed() const
{
    return radiusFixed;
}

MoleculeAttributes::DetailLevel
MoleculeAttributes::GetAtomSphereQuality() const
{
    return DetailLevel(atomSphereQuality);
}

MoleculeAttributes::DetailLevel
MoleculeAttributes::GetBondCylinderQuality() const
{
    return DetailLevel(bondCylinderQuality);
}

float
MoleculeAttributes::GetBondRadius() const
{
    return bondRadius;
}

int
MoleculeAttributes::GetBondLineWidth() const
{
    return bondLineWidth;
}

int
MoleculeAttributes::GetBondLineStyle() const
{
    return bondLineStyle;
}

const std::string &
MoleculeAttributes::GetElementColorTable() const
{
    return elementColorTable;
}

std::string &
MoleculeAttributes::GetElementColorTable()
{
    return elementColorTable;
}

const std::string &
MoleculeAttributes::GetResidueTypeColorTable() const
{
    return residueTypeColorTable;
}

std::string &
MoleculeAttributes::GetResidueTypeColorTable()
{
    return residueTypeColorTable;
}

const std::string &
MoleculeAttributes::GetResidueSequenceColorTable() const
{
    return residueSequenceColorTable;
}

std::string &
MoleculeAttributes::GetResidueSequenceColorTable()
{
    return residueSequenceColorTable;
}

const std::string &
MoleculeAttributes::GetContinuousColorTable() const
{
    return continuousColorTable;
}

std::string &
MoleculeAttributes::GetContinuousColorTable()
{
    return continuousColorTable;
}

bool
MoleculeAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
MoleculeAttributes::GetMinFlag() const
{
    return minFlag;
}

float
MoleculeAttributes::GetScalarMin() const
{
    return scalarMin;
}

bool
MoleculeAttributes::GetMaxFlag() const
{
    return maxFlag;
}

float
MoleculeAttributes::GetScalarMax() const
{
    return scalarMax;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
MoleculeAttributes::SelectBondSingleColor()
{
    Select(ID_bondSingleColor, (void *)&bondSingleColor);
}

void
MoleculeAttributes::SelectRadiusVariable()
{
    Select(ID_radiusVariable, (void *)&radiusVariable);
}

void
MoleculeAttributes::SelectElementColorTable()
{
    Select(ID_elementColorTable, (void *)&elementColorTable);
}

void
MoleculeAttributes::SelectResidueTypeColorTable()
{
    Select(ID_residueTypeColorTable, (void *)&residueTypeColorTable);
}

void
MoleculeAttributes::SelectResidueSequenceColorTable()
{
    Select(ID_residueSequenceColorTable, (void *)&residueSequenceColorTable);
}

void
MoleculeAttributes::SelectContinuousColorTable()
{
    Select(ID_continuousColorTable, (void *)&continuousColorTable);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MoleculeAttributes::GetFieldName
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
MoleculeAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_drawAtomsAs:               return "drawAtomsAs";
    case ID_scaleRadiusBy:             return "scaleRadiusBy";
    case ID_drawBondsAs:               return "drawBondsAs";
    case ID_colorBonds:                return "colorBonds";
    case ID_bondSingleColor:           return "bondSingleColor";
    case ID_radiusVariable:            return "radiusVariable";
    case ID_radiusScaleFactor:         return "radiusScaleFactor";
    case ID_radiusFixed:               return "radiusFixed";
    case ID_atomSphereQuality:         return "atomSphereQuality";
    case ID_bondCylinderQuality:       return "bondCylinderQuality";
    case ID_bondRadius:                return "bondRadius";
    case ID_bondLineWidth:             return "bondLineWidth";
    case ID_bondLineStyle:             return "bondLineStyle";
    case ID_elementColorTable:         return "elementColorTable";
    case ID_residueTypeColorTable:     return "residueTypeColorTable";
    case ID_residueSequenceColorTable: return "residueSequenceColorTable";
    case ID_continuousColorTable:      return "continuousColorTable";
    case ID_legendFlag:                return "legendFlag";
    case ID_minFlag:                   return "minFlag";
    case ID_scalarMin:                 return "scalarMin";
    case ID_maxFlag:                   return "maxFlag";
    case ID_scalarMax:                 return "scalarMax";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: MoleculeAttributes::GetFieldType
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
MoleculeAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_drawAtomsAs:               return FieldType_enum;
    case ID_scaleRadiusBy:             return FieldType_enum;
    case ID_drawBondsAs:               return FieldType_enum;
    case ID_colorBonds:                return FieldType_enum;
    case ID_bondSingleColor:           return FieldType_color;
    case ID_radiusVariable:            return FieldType_variablename;
    case ID_radiusScaleFactor:         return FieldType_float;
    case ID_radiusFixed:               return FieldType_float;
    case ID_atomSphereQuality:         return FieldType_enum;
    case ID_bondCylinderQuality:       return FieldType_enum;
    case ID_bondRadius:                return FieldType_float;
    case ID_bondLineWidth:             return FieldType_linewidth;
    case ID_bondLineStyle:             return FieldType_linestyle;
    case ID_elementColorTable:         return FieldType_colortable;
    case ID_residueTypeColorTable:     return FieldType_colortable;
    case ID_residueSequenceColorTable: return FieldType_colortable;
    case ID_continuousColorTable:      return FieldType_colortable;
    case ID_legendFlag:                return FieldType_bool;
    case ID_minFlag:                   return FieldType_bool;
    case ID_scalarMin:                 return FieldType_float;
    case ID_maxFlag:                   return FieldType_bool;
    case ID_scalarMax:                 return FieldType_float;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: MoleculeAttributes::GetFieldTypeName
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
MoleculeAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_drawAtomsAs:               return "enum";
    case ID_scaleRadiusBy:             return "enum";
    case ID_drawBondsAs:               return "enum";
    case ID_colorBonds:                return "enum";
    case ID_bondSingleColor:           return "color";
    case ID_radiusVariable:            return "variablename";
    case ID_radiusScaleFactor:         return "float";
    case ID_radiusFixed:               return "float";
    case ID_atomSphereQuality:         return "enum";
    case ID_bondCylinderQuality:       return "enum";
    case ID_bondRadius:                return "float";
    case ID_bondLineWidth:             return "linewidth";
    case ID_bondLineStyle:             return "linestyle";
    case ID_elementColorTable:         return "colortable";
    case ID_residueTypeColorTable:     return "colortable";
    case ID_residueSequenceColorTable: return "colortable";
    case ID_continuousColorTable:      return "colortable";
    case ID_legendFlag:                return "bool";
    case ID_minFlag:                   return "bool";
    case ID_scalarMin:                 return "float";
    case ID_maxFlag:                   return "bool";
    case ID_scalarMax:                 return "float";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: MoleculeAttributes::FieldsEqual
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
MoleculeAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const MoleculeAttributes &obj = *((const MoleculeAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_drawAtomsAs:
        {  // new scope
        retval = (drawAtomsAs == obj.drawAtomsAs);
        }
        break;
    case ID_scaleRadiusBy:
        {  // new scope
        retval = (scaleRadiusBy == obj.scaleRadiusBy);
        }
        break;
    case ID_drawBondsAs:
        {  // new scope
        retval = (drawBondsAs == obj.drawBondsAs);
        }
        break;
    case ID_colorBonds:
        {  // new scope
        retval = (colorBonds == obj.colorBonds);
        }
        break;
    case ID_bondSingleColor:
        {  // new scope
        retval = (bondSingleColor == obj.bondSingleColor);
        }
        break;
    case ID_radiusVariable:
        {  // new scope
        retval = (radiusVariable == obj.radiusVariable);
        }
        break;
    case ID_radiusScaleFactor:
        {  // new scope
        retval = (radiusScaleFactor == obj.radiusScaleFactor);
        }
        break;
    case ID_radiusFixed:
        {  // new scope
        retval = (radiusFixed == obj.radiusFixed);
        }
        break;
    case ID_atomSphereQuality:
        {  // new scope
        retval = (atomSphereQuality == obj.atomSphereQuality);
        }
        break;
    case ID_bondCylinderQuality:
        {  // new scope
        retval = (bondCylinderQuality == obj.bondCylinderQuality);
        }
        break;
    case ID_bondRadius:
        {  // new scope
        retval = (bondRadius == obj.bondRadius);
        }
        break;
    case ID_bondLineWidth:
        {  // new scope
        retval = (bondLineWidth == obj.bondLineWidth);
        }
        break;
    case ID_bondLineStyle:
        {  // new scope
        retval = (bondLineStyle == obj.bondLineStyle);
        }
        break;
    case ID_elementColorTable:
        {  // new scope
        retval = (elementColorTable == obj.elementColorTable);
        }
        break;
    case ID_residueTypeColorTable:
        {  // new scope
        retval = (residueTypeColorTable == obj.residueTypeColorTable);
        }
        break;
    case ID_residueSequenceColorTable:
        {  // new scope
        retval = (residueSequenceColorTable == obj.residueSequenceColorTable);
        }
        break;
    case ID_continuousColorTable:
        {  // new scope
        retval = (continuousColorTable == obj.continuousColorTable);
        }
        break;
    case ID_legendFlag:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
        }
        break;
    case ID_minFlag:
        {  // new scope
        retval = (minFlag == obj.minFlag);
        }
        break;
    case ID_scalarMin:
        {  // new scope
        retval = (scalarMin == obj.scalarMin);
        }
        break;
    case ID_maxFlag:
        {  // new scope
        retval = (maxFlag == obj.maxFlag);
        }
        break;
    case ID_scalarMax:
        {  // new scope
        retval = (scalarMax == obj.scalarMax);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

bool
MoleculeAttributes::ChangesRequireRecalculation(const MoleculeAttributes &obj)
{
    // Really, we should make sure that this will cause us to add the element
    // or other array as a secondary variable.  This specific test is
    // a little too ready to cause recalculation.
    if (scaleRadiusBy != obj.scaleRadiusBy)
        return true;

    // Same situation here.
    if (radiusVariable != obj.radiusVariable)
        return true;

    return false;
}

