#include <LabelAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for LabelAttributes::LabelIndexDisplay
//

static const char *LabelIndexDisplay_strings[] = {
"Natural", "LogicalIndex", "Index"
};

std::string
LabelAttributes::LabelIndexDisplay_ToString(LabelAttributes::LabelIndexDisplay t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return LabelIndexDisplay_strings[index];
}

std::string
LabelAttributes::LabelIndexDisplay_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return LabelIndexDisplay_strings[index];
}

bool
LabelAttributes::LabelIndexDisplay_FromString(const std::string &s, LabelAttributes::LabelIndexDisplay &val)
{
    val = LabelAttributes::Natural;
    for(int i = 0; i < 3; ++i)
    {
        if(s == LabelIndexDisplay_strings[i])
        {
            val = (LabelIndexDisplay)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LabelAttributes::LabelHorizontalAlignment
//

static const char *LabelHorizontalAlignment_strings[] = {
"HCenter", "Left", "Right"
};

std::string
LabelAttributes::LabelHorizontalAlignment_ToString(LabelAttributes::LabelHorizontalAlignment t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return LabelHorizontalAlignment_strings[index];
}

std::string
LabelAttributes::LabelHorizontalAlignment_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return LabelHorizontalAlignment_strings[index];
}

bool
LabelAttributes::LabelHorizontalAlignment_FromString(const std::string &s, LabelAttributes::LabelHorizontalAlignment &val)
{
    val = LabelAttributes::HCenter;
    for(int i = 0; i < 3; ++i)
    {
        if(s == LabelHorizontalAlignment_strings[i])
        {
            val = (LabelHorizontalAlignment)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LabelAttributes::LabelVerticalAlignment
//

static const char *LabelVerticalAlignment_strings[] = {
"VCenter", "Top", "Bottom"
};

std::string
LabelAttributes::LabelVerticalAlignment_ToString(LabelAttributes::LabelVerticalAlignment t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return LabelVerticalAlignment_strings[index];
}

std::string
LabelAttributes::LabelVerticalAlignment_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return LabelVerticalAlignment_strings[index];
}

bool
LabelAttributes::LabelVerticalAlignment_FromString(const std::string &s, LabelAttributes::LabelVerticalAlignment &val)
{
    val = LabelAttributes::VCenter;
    for(int i = 0; i < 3; ++i)
    {
        if(s == LabelVerticalAlignment_strings[i])
        {
            val = (LabelVerticalAlignment)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LabelAttributes::LabelDrawFacing
//

static const char *LabelDrawFacing_strings[] = {
"Front", "Back", "FrontAndBack"
};

std::string
LabelAttributes::LabelDrawFacing_ToString(LabelAttributes::LabelDrawFacing t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return LabelDrawFacing_strings[index];
}

std::string
LabelAttributes::LabelDrawFacing_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return LabelDrawFacing_strings[index];
}

bool
LabelAttributes::LabelDrawFacing_FromString(const std::string &s, LabelAttributes::LabelDrawFacing &val)
{
    val = LabelAttributes::Front;
    for(int i = 0; i < 3; ++i)
    {
        if(s == LabelDrawFacing_strings[i])
        {
            val = (LabelDrawFacing)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LabelAttributes::VariableType
//

static const char *VariableType_strings[] = {
"LABEL_VT_MESH", "LABEL_VT_SCALAR_VAR", "LABEL_VT_VECTOR_VAR", 
"LABEL_VT_TENSOR_VAR", "LABEL_VT_SYMMETRIC_TENSOR_VAR", "LABEL_VT_ARRAY_VAR", 
"LABEL_VT_LABEL_VAR", "LABEL_VT_MATERIAL", "LABEL_VT_SUBSET", 
"LABEL_VT_UNKNOWN_TYPE"};

std::string
LabelAttributes::VariableType_ToString(LabelAttributes::VariableType t)
{
    int index = int(t);
    if(index < 0 || index >= 10) index = 0;
    return VariableType_strings[index];
}

std::string
LabelAttributes::VariableType_ToString(int t)
{
    int index = (t < 0 || t >= 10) ? 0 : t;
    return VariableType_strings[index];
}

bool
LabelAttributes::VariableType_FromString(const std::string &s, LabelAttributes::VariableType &val)
{
    val = LabelAttributes::LABEL_VT_MESH;
    for(int i = 0; i < 10; ++i)
    {
        if(s == VariableType_strings[i])
        {
            val = (VariableType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LabelAttributes::DepthTestMode
//

static const char *DepthTestMode_strings[] = {
"LABEL_DT_AUTO", "LABEL_DT_ALWAYS", "LABEL_DT_NEVER"
};

std::string
LabelAttributes::DepthTestMode_ToString(LabelAttributes::DepthTestMode t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return DepthTestMode_strings[index];
}

std::string
LabelAttributes::DepthTestMode_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return DepthTestMode_strings[index];
}

bool
LabelAttributes::DepthTestMode_FromString(const std::string &s, LabelAttributes::DepthTestMode &val)
{
    val = LabelAttributes::LABEL_DT_AUTO;
    for(int i = 0; i < 3; ++i)
    {
        if(s == DepthTestMode_strings[i])
        {
            val = (DepthTestMode)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: LabelAttributes::LabelAttributes
//
// Purpose: 
//   Constructor for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

LabelAttributes::LabelAttributes() : AttributeSubject("ibbbbiiibafbafiii"),
    textColor1(255, 0, 0, 0), textColor2(0, 0, 255, 0)
{
    varType = LABEL_VT_UNKNOWN_TYPE;
    legendFlag = true;
    showNodes = false;
    showCells = true;
    restrictNumberOfLabels = true;
    drawLabelsFacing = Front;
    labelDisplayFormat = Natural;
    numberOfLabels = 200;
    specifyTextColor1 = false;
    textHeight1 = 0.02;
    specifyTextColor2 = false;
    textHeight2 = 0.02;
    horizontalJustification = HCenter;
    verticalJustification = VCenter;
    depthTestMode = LABEL_DT_AUTO;
}

// ****************************************************************************
// Method: LabelAttributes::LabelAttributes
//
// Purpose: 
//   Copy constructor for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

LabelAttributes::LabelAttributes(const LabelAttributes &obj) : AttributeSubject("ibbbbiiibafbafiii")
{
    varType = obj.varType;
    legendFlag = obj.legendFlag;
    showNodes = obj.showNodes;
    showCells = obj.showCells;
    restrictNumberOfLabels = obj.restrictNumberOfLabels;
    drawLabelsFacing = obj.drawLabelsFacing;
    labelDisplayFormat = obj.labelDisplayFormat;
    numberOfLabels = obj.numberOfLabels;
    specifyTextColor1 = obj.specifyTextColor1;
    textColor1 = obj.textColor1;
    textHeight1 = obj.textHeight1;
    specifyTextColor2 = obj.specifyTextColor2;
    textColor2 = obj.textColor2;
    textHeight2 = obj.textHeight2;
    horizontalJustification = obj.horizontalJustification;
    verticalJustification = obj.verticalJustification;
    depthTestMode = obj.depthTestMode;

    SelectAll();
}

// ****************************************************************************
// Method: LabelAttributes::~LabelAttributes
//
// Purpose: 
//   Destructor for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

LabelAttributes::~LabelAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: LabelAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

LabelAttributes& 
LabelAttributes::operator = (const LabelAttributes &obj)
{
    if (this == &obj) return *this;
    varType = obj.varType;
    legendFlag = obj.legendFlag;
    showNodes = obj.showNodes;
    showCells = obj.showCells;
    restrictNumberOfLabels = obj.restrictNumberOfLabels;
    drawLabelsFacing = obj.drawLabelsFacing;
    labelDisplayFormat = obj.labelDisplayFormat;
    numberOfLabels = obj.numberOfLabels;
    specifyTextColor1 = obj.specifyTextColor1;
    textColor1 = obj.textColor1;
    textHeight1 = obj.textHeight1;
    specifyTextColor2 = obj.specifyTextColor2;
    textColor2 = obj.textColor2;
    textHeight2 = obj.textHeight2;
    horizontalJustification = obj.horizontalJustification;
    verticalJustification = obj.verticalJustification;
    depthTestMode = obj.depthTestMode;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: LabelAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
LabelAttributes::operator == (const LabelAttributes &obj) const
{
    // Create the return value
    return (true /* can ignore varType */ &&
            (legendFlag == obj.legendFlag) &&
            (showNodes == obj.showNodes) &&
            (showCells == obj.showCells) &&
            (restrictNumberOfLabels == obj.restrictNumberOfLabels) &&
            (drawLabelsFacing == obj.drawLabelsFacing) &&
            (labelDisplayFormat == obj.labelDisplayFormat) &&
            (numberOfLabels == obj.numberOfLabels) &&
            (specifyTextColor1 == obj.specifyTextColor1) &&
            (textColor1 == obj.textColor1) &&
            (textHeight1 == obj.textHeight1) &&
            (specifyTextColor2 == obj.specifyTextColor2) &&
            (textColor2 == obj.textColor2) &&
            (textHeight2 == obj.textHeight2) &&
            (horizontalJustification == obj.horizontalJustification) &&
            (verticalJustification == obj.verticalJustification) &&
            (depthTestMode == obj.depthTestMode));
}

// ****************************************************************************
// Method: LabelAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
LabelAttributes::operator != (const LabelAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: LabelAttributes::TypeName
//
// Purpose: 
//   Type name method for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

const std::string
LabelAttributes::TypeName() const
{
    return "LabelAttributes";
}

// ****************************************************************************
// Method: LabelAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
LabelAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const LabelAttributes *tmp = (const LabelAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: LabelAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
LabelAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new LabelAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: LabelAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the LabelAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
LabelAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new LabelAttributes(*this);
    else
        retval = new LabelAttributes;

    return retval;
}

// ****************************************************************************
// Method: LabelAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
LabelAttributes::SelectAll()
{
    Select(0, (void *)&varType);
    Select(1, (void *)&legendFlag);
    Select(2, (void *)&showNodes);
    Select(3, (void *)&showCells);
    Select(4, (void *)&restrictNumberOfLabels);
    Select(5, (void *)&drawLabelsFacing);
    Select(6, (void *)&labelDisplayFormat);
    Select(7, (void *)&numberOfLabels);
    Select(8, (void *)&specifyTextColor1);
    Select(9, (void *)&textColor1);
    Select(10, (void *)&textHeight1);
    Select(11, (void *)&specifyTextColor2);
    Select(12, (void *)&textColor2);
    Select(13, (void *)&textHeight2);
    Select(14, (void *)&horizontalJustification);
    Select(15, (void *)&verticalJustification);
    Select(16, (void *)&depthTestMode);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LabelAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
LabelAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    LabelAttributes defaultObject;
    bool addToParent = false;
    // Create a node for LabelAttributes.
    DataNode *node = new DataNode("LabelAttributes");

    if(completeSave || !FieldsEqual(0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("varType", VariableType_ToString(varType)));
    }

    if(completeSave || !FieldsEqual(1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }

    if(completeSave || !FieldsEqual(2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showNodes", showNodes));
    }

    if(completeSave || !FieldsEqual(3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showCells", showCells));
    }

    if(completeSave || !FieldsEqual(4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("restrictNumberOfLabels", restrictNumberOfLabels));
    }

    if(completeSave || !FieldsEqual(5, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("drawLabelsFacing", LabelDrawFacing_ToString(drawLabelsFacing)));
    }

    if(completeSave || !FieldsEqual(6, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("labelDisplayFormat", LabelIndexDisplay_ToString(labelDisplayFormat)));
    }

    if(completeSave || !FieldsEqual(7, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numberOfLabels", numberOfLabels));
    }

    if(completeSave || !FieldsEqual(8, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specifyTextColor1", specifyTextColor1));
    }

        DataNode *textColor1Node = new DataNode("textColor1");
        if(textColor1.CreateNode(textColor1Node, completeSave, true))
        {
            addToParent = true;
            node->AddNode(textColor1Node);
        }
        else
            delete textColor1Node;
    if(completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("textHeight1", textHeight1));
    }

    if(completeSave || !FieldsEqual(11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specifyTextColor2", specifyTextColor2));
    }

        DataNode *textColor2Node = new DataNode("textColor2");
        if(textColor2.CreateNode(textColor2Node, completeSave, true))
        {
            addToParent = true;
            node->AddNode(textColor2Node);
        }
        else
            delete textColor2Node;
    if(completeSave || !FieldsEqual(13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("textHeight2", textHeight2));
    }

    if(completeSave || !FieldsEqual(14, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("horizontalJustification", LabelHorizontalAlignment_ToString(horizontalJustification)));
    }

    if(completeSave || !FieldsEqual(15, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("verticalJustification", LabelVerticalAlignment_ToString(verticalJustification)));
    }

    if(completeSave || !FieldsEqual(16, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("depthTestMode", DepthTestMode_ToString(depthTestMode)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: LabelAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
LabelAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("LabelAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("varType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 10)
                SetVarType(VariableType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VariableType value;
            if(VariableType_FromString(node->AsString(), value))
                SetVarType(value);
        }
    }
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("showNodes")) != 0)
        SetShowNodes(node->AsBool());
    if((node = searchNode->GetNode("showCells")) != 0)
        SetShowCells(node->AsBool());
    if((node = searchNode->GetNode("restrictNumberOfLabels")) != 0)
        SetRestrictNumberOfLabels(node->AsBool());
    if((node = searchNode->GetNode("drawLabelsFacing")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDrawLabelsFacing(LabelDrawFacing(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LabelDrawFacing value;
            if(LabelDrawFacing_FromString(node->AsString(), value))
                SetDrawLabelsFacing(value);
        }
    }
    if((node = searchNode->GetNode("labelDisplayFormat")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetLabelDisplayFormat(LabelIndexDisplay(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LabelIndexDisplay value;
            if(LabelIndexDisplay_FromString(node->AsString(), value))
                SetLabelDisplayFormat(value);
        }
    }
    if((node = searchNode->GetNode("numberOfLabels")) != 0)
        SetNumberOfLabels(node->AsInt());
    if((node = searchNode->GetNode("specifyTextColor1")) != 0)
        SetSpecifyTextColor1(node->AsBool());
    if((node = searchNode->GetNode("textColor1")) != 0)
        textColor1.SetFromNode(node);
    if((node = searchNode->GetNode("textHeight1")) != 0)
        SetTextHeight1(node->AsFloat());
    if((node = searchNode->GetNode("specifyTextColor2")) != 0)
        SetSpecifyTextColor2(node->AsBool());
    if((node = searchNode->GetNode("textColor2")) != 0)
        textColor2.SetFromNode(node);
    if((node = searchNode->GetNode("textHeight2")) != 0)
        SetTextHeight2(node->AsFloat());
    if((node = searchNode->GetNode("horizontalJustification")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetHorizontalJustification(LabelHorizontalAlignment(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LabelHorizontalAlignment value;
            if(LabelHorizontalAlignment_FromString(node->AsString(), value))
                SetHorizontalJustification(value);
        }
    }
    if((node = searchNode->GetNode("verticalJustification")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetVerticalJustification(LabelVerticalAlignment(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LabelVerticalAlignment value;
            if(LabelVerticalAlignment_FromString(node->AsString(), value))
                SetVerticalJustification(value);
        }
    }
    if((node = searchNode->GetNode("depthTestMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDepthTestMode(DepthTestMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DepthTestMode value;
            if(DepthTestMode_FromString(node->AsString(), value))
                SetDepthTestMode(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
LabelAttributes::SetVarType(LabelAttributes::VariableType varType_)
{
    varType = varType_;
    Select(0, (void *)&varType);
}

void
LabelAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(1, (void *)&legendFlag);
}

void
LabelAttributes::SetShowNodes(bool showNodes_)
{
    showNodes = showNodes_;
    Select(2, (void *)&showNodes);
}

void
LabelAttributes::SetShowCells(bool showCells_)
{
    showCells = showCells_;
    Select(3, (void *)&showCells);
}

void
LabelAttributes::SetRestrictNumberOfLabels(bool restrictNumberOfLabels_)
{
    restrictNumberOfLabels = restrictNumberOfLabels_;
    Select(4, (void *)&restrictNumberOfLabels);
}

void
LabelAttributes::SetDrawLabelsFacing(LabelAttributes::LabelDrawFacing drawLabelsFacing_)
{
    drawLabelsFacing = drawLabelsFacing_;
    Select(5, (void *)&drawLabelsFacing);
}

void
LabelAttributes::SetLabelDisplayFormat(LabelAttributes::LabelIndexDisplay labelDisplayFormat_)
{
    labelDisplayFormat = labelDisplayFormat_;
    Select(6, (void *)&labelDisplayFormat);
}

void
LabelAttributes::SetNumberOfLabels(int numberOfLabels_)
{
    numberOfLabels = numberOfLabels_;
    Select(7, (void *)&numberOfLabels);
}

void
LabelAttributes::SetSpecifyTextColor1(bool specifyTextColor1_)
{
    specifyTextColor1 = specifyTextColor1_;
    Select(8, (void *)&specifyTextColor1);
}

void
LabelAttributes::SetTextColor1(const ColorAttribute &textColor1_)
{
    textColor1 = textColor1_;
    Select(9, (void *)&textColor1);
}

void
LabelAttributes::SetTextHeight1(float textHeight1_)
{
    textHeight1 = textHeight1_;
    Select(10, (void *)&textHeight1);
}

void
LabelAttributes::SetSpecifyTextColor2(bool specifyTextColor2_)
{
    specifyTextColor2 = specifyTextColor2_;
    Select(11, (void *)&specifyTextColor2);
}

void
LabelAttributes::SetTextColor2(const ColorAttribute &textColor2_)
{
    textColor2 = textColor2_;
    Select(12, (void *)&textColor2);
}

void
LabelAttributes::SetTextHeight2(float textHeight2_)
{
    textHeight2 = textHeight2_;
    Select(13, (void *)&textHeight2);
}

void
LabelAttributes::SetHorizontalJustification(LabelAttributes::LabelHorizontalAlignment horizontalJustification_)
{
    horizontalJustification = horizontalJustification_;
    Select(14, (void *)&horizontalJustification);
}

void
LabelAttributes::SetVerticalJustification(LabelAttributes::LabelVerticalAlignment verticalJustification_)
{
    verticalJustification = verticalJustification_;
    Select(15, (void *)&verticalJustification);
}

void
LabelAttributes::SetDepthTestMode(LabelAttributes::DepthTestMode depthTestMode_)
{
    depthTestMode = depthTestMode_;
    Select(16, (void *)&depthTestMode);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

LabelAttributes::VariableType
LabelAttributes::GetVarType() const
{
    return VariableType(varType);
}

bool
LabelAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
LabelAttributes::GetShowNodes() const
{
    return showNodes;
}

bool
LabelAttributes::GetShowCells() const
{
    return showCells;
}

bool
LabelAttributes::GetRestrictNumberOfLabels() const
{
    return restrictNumberOfLabels;
}

LabelAttributes::LabelDrawFacing
LabelAttributes::GetDrawLabelsFacing() const
{
    return LabelDrawFacing(drawLabelsFacing);
}

LabelAttributes::LabelIndexDisplay
LabelAttributes::GetLabelDisplayFormat() const
{
    return LabelIndexDisplay(labelDisplayFormat);
}

int
LabelAttributes::GetNumberOfLabels() const
{
    return numberOfLabels;
}

bool
LabelAttributes::GetSpecifyTextColor1() const
{
    return specifyTextColor1;
}

const ColorAttribute &
LabelAttributes::GetTextColor1() const
{
    return textColor1;
}

ColorAttribute &
LabelAttributes::GetTextColor1()
{
    return textColor1;
}

float
LabelAttributes::GetTextHeight1() const
{
    return textHeight1;
}

bool
LabelAttributes::GetSpecifyTextColor2() const
{
    return specifyTextColor2;
}

const ColorAttribute &
LabelAttributes::GetTextColor2() const
{
    return textColor2;
}

ColorAttribute &
LabelAttributes::GetTextColor2()
{
    return textColor2;
}

float
LabelAttributes::GetTextHeight2() const
{
    return textHeight2;
}

LabelAttributes::LabelHorizontalAlignment
LabelAttributes::GetHorizontalJustification() const
{
    return LabelHorizontalAlignment(horizontalJustification);
}

LabelAttributes::LabelVerticalAlignment
LabelAttributes::GetVerticalJustification() const
{
    return LabelVerticalAlignment(verticalJustification);
}

LabelAttributes::DepthTestMode
LabelAttributes::GetDepthTestMode() const
{
    return DepthTestMode(depthTestMode);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
LabelAttributes::SelectTextColor1()
{
    Select(9, (void *)&textColor1);
}

void
LabelAttributes::SelectTextColor2()
{
    Select(12, (void *)&textColor2);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LabelAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
LabelAttributes::GetFieldName(int index) const
{
    switch (index)
    {
        case 0:  return "Variable type";
        case 1:  return "Legend";
        case 2:  return "Show nodes";
        case 3:  return "Show cells";
        case 4:  return "Restrict number of labels";
        case 5:  return "Draw labels facing";
        case 6:  return "Label display";
        case 7:  return "numberOfLabels";
        case 8:  return "Specify text color1";
        case 9:  return "Text color1";
        case 10:  return "textHeight1";
        case 11:  return "Specify text color2";
        case 12:  return "Text color2";
        case 13:  return "textHeight2";
        case 14:  return "Horizontal justification";
        case 15:  return "Vertical Justification";
        case 16:  return "Depth test mode";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LabelAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
LabelAttributes::GetFieldType(int index) const
{
    switch (index)
    {
        case 0:  return FieldType_enum;
        case 1:  return FieldType_bool;
        case 2:  return FieldType_bool;
        case 3:  return FieldType_bool;
        case 4:  return FieldType_bool;
        case 5:  return FieldType_enum;
        case 6:  return FieldType_enum;
        case 7:  return FieldType_int;
        case 8:  return FieldType_bool;
        case 9:  return FieldType_color;
        case 10:  return FieldType_float;
        case 11:  return FieldType_bool;
        case 12:  return FieldType_color;
        case 13:  return FieldType_float;
        case 14:  return FieldType_enum;
        case 15:  return FieldType_enum;
        case 16:  return FieldType_enum;
        default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: LabelAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
LabelAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
        case 0:  return "enum";
        case 1:  return "bool";
        case 2:  return "bool";
        case 3:  return "bool";
        case 4:  return "bool";
        case 5:  return "enum";
        case 6:  return "enum";
        case 7:  return "int";
        case 8:  return "bool";
        case 9:  return "color";
        case 10:  return "float";
        case 11:  return "bool";
        case 12:  return "color";
        case 13:  return "float";
        case 14:  return "enum";
        case 15:  return "enum";
        case 16:  return "enum";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LabelAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Aug 9 14:00:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
LabelAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const LabelAttributes &obj = *((const LabelAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case 0:
        {  // new scope
        retval = (varType == obj.varType);
        }
        break;
    case 1:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
        }
        break;
    case 2:
        {  // new scope
        retval = (showNodes == obj.showNodes);
        }
        break;
    case 3:
        {  // new scope
        retval = (showCells == obj.showCells);
        }
        break;
    case 4:
        {  // new scope
        retval = (restrictNumberOfLabels == obj.restrictNumberOfLabels);
        }
        break;
    case 5:
        {  // new scope
        retval = (drawLabelsFacing == obj.drawLabelsFacing);
        }
        break;
    case 6:
        {  // new scope
        retval = (labelDisplayFormat == obj.labelDisplayFormat);
        }
        break;
    case 7:
        {  // new scope
        retval = (numberOfLabels == obj.numberOfLabels);
        }
        break;
    case 8:
        {  // new scope
        retval = (specifyTextColor1 == obj.specifyTextColor1);
        }
        break;
    case 9:
        {  // new scope
        retval = (textColor1 == obj.textColor1);
        }
        break;
    case 10:
        {  // new scope
        retval = (textHeight1 == obj.textHeight1);
        }
        break;
    case 11:
        {  // new scope
        retval = (specifyTextColor2 == obj.specifyTextColor2);
        }
        break;
    case 12:
        {  // new scope
        retval = (textColor2 == obj.textColor2);
        }
        break;
    case 13:
        {  // new scope
        retval = (textHeight2 == obj.textHeight2);
        }
        break;
    case 14:
        {  // new scope
        retval = (horizontalJustification == obj.horizontalJustification);
        }
        break;
    case 15:
        {  // new scope
        retval = (verticalJustification == obj.verticalJustification);
        }
        break;
    case 16:
        {  // new scope
        retval = (depthTestMode == obj.depthTestMode);
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
LabelAttributes::ChangesRequireRecalculation(const LabelAttributes &) const
{
    return false;
}

bool
LabelAttributes::VarChangeRequiresReset()
{
    return true;
}

