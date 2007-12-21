/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ScatterAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for ScatterAttributes::Scaling
//

static const char *Scaling_strings[] = {
"Linear", "Log", "Skew"
};

std::string
ScatterAttributes::Scaling_ToString(ScatterAttributes::Scaling t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Scaling_strings[index];
}

std::string
ScatterAttributes::Scaling_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Scaling_strings[index];
}

bool
ScatterAttributes::Scaling_FromString(const std::string &s, ScatterAttributes::Scaling &val)
{
    val = ScatterAttributes::Linear;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Scaling_strings[i])
        {
            val = (Scaling)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ScatterAttributes::PointType
//

static const char *PointType_strings[] = {
"Box", "Axis", "Icosahedron", 
"Point", "Sphere"};

std::string
ScatterAttributes::PointType_ToString(ScatterAttributes::PointType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PointType_strings[index];
}

std::string
ScatterAttributes::PointType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PointType_strings[index];
}

bool
ScatterAttributes::PointType_FromString(const std::string &s, ScatterAttributes::PointType &val)
{
    val = ScatterAttributes::Box;
    for(int i = 0; i < 5; ++i)
    {
        if(s == PointType_strings[i])
        {
            val = (PointType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ScatterAttributes::VariableRole
//

static const char *VariableRole_strings[] = {
"Coordinate0", "Coordinate1", "Coordinate2", 
"Color", "None"};

std::string
ScatterAttributes::VariableRole_ToString(ScatterAttributes::VariableRole t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return VariableRole_strings[index];
}

std::string
ScatterAttributes::VariableRole_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return VariableRole_strings[index];
}

bool
ScatterAttributes::VariableRole_FromString(const std::string &s, ScatterAttributes::VariableRole &val)
{
    val = ScatterAttributes::Coordinate0;
    for(int i = 0; i < 5; ++i)
    {
        if(s == VariableRole_strings[i])
        {
            val = (VariableRole)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *ScatterAttributes::TypeMapFormatString = "ibbddidisbbddidisbbddidisbbddiddiibsabb";

// ****************************************************************************
// Method: ScatterAttributes::ScatterAttributes
//
// Purpose: 
//   Constructor for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ScatterAttributes::ScatterAttributes() : 
    AttributeSubject(ScatterAttributes::TypeMapFormatString),
    colorTableName("hot"), singleColor(255, 0, 0)
{
    var1Role = Coordinate0;
    var1MinFlag = false;
    var1MaxFlag = false;
    var1Min = 0;
    var1Max = 1;
    var1Scaling = Linear;
    var1SkewFactor = 1;
    var2Role = Coordinate1;
    var2 = "default";
    var2MinFlag = false;
    var2MaxFlag = false;
    var2Min = 0;
    var2Max = 1;
    var2Scaling = Linear;
    var2SkewFactor = 1;
    var3Role = None;
    var3 = "default";
    var3MinFlag = false;
    var3MaxFlag = false;
    var3Min = 0;
    var3Max = 1;
    var3Scaling = Linear;
    var3SkewFactor = 1;
    var4Role = None;
    var4 = "default";
    var4MinFlag = false;
    var4MaxFlag = false;
    var4Min = 0;
    var4Max = 1;
    var4Scaling = Linear;
    var4SkewFactor = 1;
    pointSize = 0.05;
    pointSizePixels = 1;
    pointType = Point;
    scaleCube = true;
    foregroundFlag = true;
    legendFlag = true;
}

// ****************************************************************************
// Method: ScatterAttributes::ScatterAttributes
//
// Purpose: 
//   Copy constructor for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ScatterAttributes::ScatterAttributes(const ScatterAttributes &obj) : 
    AttributeSubject(ScatterAttributes::TypeMapFormatString)
{
    var1Role = obj.var1Role;
    var1MinFlag = obj.var1MinFlag;
    var1MaxFlag = obj.var1MaxFlag;
    var1Min = obj.var1Min;
    var1Max = obj.var1Max;
    var1Scaling = obj.var1Scaling;
    var1SkewFactor = obj.var1SkewFactor;
    var2Role = obj.var2Role;
    var2 = obj.var2;
    var2MinFlag = obj.var2MinFlag;
    var2MaxFlag = obj.var2MaxFlag;
    var2Min = obj.var2Min;
    var2Max = obj.var2Max;
    var2Scaling = obj.var2Scaling;
    var2SkewFactor = obj.var2SkewFactor;
    var3Role = obj.var3Role;
    var3 = obj.var3;
    var3MinFlag = obj.var3MinFlag;
    var3MaxFlag = obj.var3MaxFlag;
    var3Min = obj.var3Min;
    var3Max = obj.var3Max;
    var3Scaling = obj.var3Scaling;
    var3SkewFactor = obj.var3SkewFactor;
    var4Role = obj.var4Role;
    var4 = obj.var4;
    var4MinFlag = obj.var4MinFlag;
    var4MaxFlag = obj.var4MaxFlag;
    var4Min = obj.var4Min;
    var4Max = obj.var4Max;
    var4Scaling = obj.var4Scaling;
    var4SkewFactor = obj.var4SkewFactor;
    pointSize = obj.pointSize;
    pointSizePixels = obj.pointSizePixels;
    pointType = obj.pointType;
    scaleCube = obj.scaleCube;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    foregroundFlag = obj.foregroundFlag;
    legendFlag = obj.legendFlag;

    SelectAll();
}

// ****************************************************************************
// Method: ScatterAttributes::~ScatterAttributes
//
// Purpose: 
//   Destructor for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ScatterAttributes::~ScatterAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: ScatterAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ScatterAttributes& 
ScatterAttributes::operator = (const ScatterAttributes &obj)
{
    if (this == &obj) return *this;
    var1Role = obj.var1Role;
    var1MinFlag = obj.var1MinFlag;
    var1MaxFlag = obj.var1MaxFlag;
    var1Min = obj.var1Min;
    var1Max = obj.var1Max;
    var1Scaling = obj.var1Scaling;
    var1SkewFactor = obj.var1SkewFactor;
    var2Role = obj.var2Role;
    var2 = obj.var2;
    var2MinFlag = obj.var2MinFlag;
    var2MaxFlag = obj.var2MaxFlag;
    var2Min = obj.var2Min;
    var2Max = obj.var2Max;
    var2Scaling = obj.var2Scaling;
    var2SkewFactor = obj.var2SkewFactor;
    var3Role = obj.var3Role;
    var3 = obj.var3;
    var3MinFlag = obj.var3MinFlag;
    var3MaxFlag = obj.var3MaxFlag;
    var3Min = obj.var3Min;
    var3Max = obj.var3Max;
    var3Scaling = obj.var3Scaling;
    var3SkewFactor = obj.var3SkewFactor;
    var4Role = obj.var4Role;
    var4 = obj.var4;
    var4MinFlag = obj.var4MinFlag;
    var4MaxFlag = obj.var4MaxFlag;
    var4Min = obj.var4Min;
    var4Max = obj.var4Max;
    var4Scaling = obj.var4Scaling;
    var4SkewFactor = obj.var4SkewFactor;
    pointSize = obj.pointSize;
    pointSizePixels = obj.pointSizePixels;
    pointType = obj.pointType;
    scaleCube = obj.scaleCube;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    foregroundFlag = obj.foregroundFlag;
    legendFlag = obj.legendFlag;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: ScatterAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ScatterAttributes::operator == (const ScatterAttributes &obj) const
{
    // Create the return value
    return ((var1Role == obj.var1Role) &&
            (var1MinFlag == obj.var1MinFlag) &&
            (var1MaxFlag == obj.var1MaxFlag) &&
            (var1Min == obj.var1Min) &&
            (var1Max == obj.var1Max) &&
            (var1Scaling == obj.var1Scaling) &&
            (var1SkewFactor == obj.var1SkewFactor) &&
            (var2Role == obj.var2Role) &&
            (var2 == obj.var2) &&
            (var2MinFlag == obj.var2MinFlag) &&
            (var2MaxFlag == obj.var2MaxFlag) &&
            (var2Min == obj.var2Min) &&
            (var2Max == obj.var2Max) &&
            (var2Scaling == obj.var2Scaling) &&
            (var2SkewFactor == obj.var2SkewFactor) &&
            (var3Role == obj.var3Role) &&
            (var3 == obj.var3) &&
            (var3MinFlag == obj.var3MinFlag) &&
            (var3MaxFlag == obj.var3MaxFlag) &&
            (var3Min == obj.var3Min) &&
            (var3Max == obj.var3Max) &&
            (var3Scaling == obj.var3Scaling) &&
            (var3SkewFactor == obj.var3SkewFactor) &&
            (var4Role == obj.var4Role) &&
            (var4 == obj.var4) &&
            (var4MinFlag == obj.var4MinFlag) &&
            (var4MaxFlag == obj.var4MaxFlag) &&
            (var4Min == obj.var4Min) &&
            (var4Max == obj.var4Max) &&
            (var4Scaling == obj.var4Scaling) &&
            (var4SkewFactor == obj.var4SkewFactor) &&
            (pointSize == obj.pointSize) &&
            (pointSizePixels == obj.pointSizePixels) &&
            (pointType == obj.pointType) &&
            (scaleCube == obj.scaleCube) &&
            (colorTableName == obj.colorTableName) &&
            (singleColor == obj.singleColor) &&
            (foregroundFlag == obj.foregroundFlag) &&
            (legendFlag == obj.legendFlag));
}

// ****************************************************************************
// Method: ScatterAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ScatterAttributes::operator != (const ScatterAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ScatterAttributes::TypeName
//
// Purpose: 
//   Type name method for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
ScatterAttributes::TypeName() const
{
    return "ScatterAttributes";
}

// ****************************************************************************
// Method: ScatterAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ScatterAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ScatterAttributes *tmp = (const ScatterAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ScatterAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ScatterAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ScatterAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: ScatterAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the ScatterAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ScatterAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ScatterAttributes(*this);
    else
        retval = new ScatterAttributes;

    return retval;
}

// ****************************************************************************
// Method: ScatterAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
ScatterAttributes::SelectAll()
{
    Select(ID_var1Role,        (void *)&var1Role);
    Select(ID_var1MinFlag,     (void *)&var1MinFlag);
    Select(ID_var1MaxFlag,     (void *)&var1MaxFlag);
    Select(ID_var1Min,         (void *)&var1Min);
    Select(ID_var1Max,         (void *)&var1Max);
    Select(ID_var1Scaling,     (void *)&var1Scaling);
    Select(ID_var1SkewFactor,  (void *)&var1SkewFactor);
    Select(ID_var2Role,        (void *)&var2Role);
    Select(ID_var2,            (void *)&var2);
    Select(ID_var2MinFlag,     (void *)&var2MinFlag);
    Select(ID_var2MaxFlag,     (void *)&var2MaxFlag);
    Select(ID_var2Min,         (void *)&var2Min);
    Select(ID_var2Max,         (void *)&var2Max);
    Select(ID_var2Scaling,     (void *)&var2Scaling);
    Select(ID_var2SkewFactor,  (void *)&var2SkewFactor);
    Select(ID_var3Role,        (void *)&var3Role);
    Select(ID_var3,            (void *)&var3);
    Select(ID_var3MinFlag,     (void *)&var3MinFlag);
    Select(ID_var3MaxFlag,     (void *)&var3MaxFlag);
    Select(ID_var3Min,         (void *)&var3Min);
    Select(ID_var3Max,         (void *)&var3Max);
    Select(ID_var3Scaling,     (void *)&var3Scaling);
    Select(ID_var3SkewFactor,  (void *)&var3SkewFactor);
    Select(ID_var4Role,        (void *)&var4Role);
    Select(ID_var4,            (void *)&var4);
    Select(ID_var4MinFlag,     (void *)&var4MinFlag);
    Select(ID_var4MaxFlag,     (void *)&var4MaxFlag);
    Select(ID_var4Min,         (void *)&var4Min);
    Select(ID_var4Max,         (void *)&var4Max);
    Select(ID_var4Scaling,     (void *)&var4Scaling);
    Select(ID_var4SkewFactor,  (void *)&var4SkewFactor);
    Select(ID_pointSize,       (void *)&pointSize);
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
    Select(ID_pointType,       (void *)&pointType);
    Select(ID_scaleCube,       (void *)&scaleCube);
    Select(ID_colorTableName,  (void *)&colorTableName);
    Select(ID_singleColor,     (void *)&singleColor);
    Select(ID_foregroundFlag,  (void *)&foregroundFlag);
    Select(ID_legendFlag,      (void *)&legendFlag);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ScatterAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ScatterAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    ScatterAttributes defaultObject;
    bool addToParent = false;
    // Create a node for ScatterAttributes.
    DataNode *node = new DataNode("ScatterAttributes");

    if(completeSave || !FieldsEqual(ID_var1Role, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1Role", VariableRole_ToString(var1Role)));
    }

    if(completeSave || !FieldsEqual(ID_var1MinFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1MinFlag", var1MinFlag));
    }

    if(completeSave || !FieldsEqual(ID_var1MaxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1MaxFlag", var1MaxFlag));
    }

    if(completeSave || !FieldsEqual(ID_var1Min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1Min", var1Min));
    }

    if(completeSave || !FieldsEqual(ID_var1Max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1Max", var1Max));
    }

    if(completeSave || !FieldsEqual(ID_var1Scaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1Scaling", Scaling_ToString(var1Scaling)));
    }

    if(completeSave || !FieldsEqual(ID_var1SkewFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var1SkewFactor", var1SkewFactor));
    }

    if(completeSave || !FieldsEqual(ID_var2Role, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2Role", VariableRole_ToString(var2Role)));
    }

    if(completeSave || !FieldsEqual(ID_var2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2", var2));
    }

    if(completeSave || !FieldsEqual(ID_var2MinFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2MinFlag", var2MinFlag));
    }

    if(completeSave || !FieldsEqual(ID_var2MaxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2MaxFlag", var2MaxFlag));
    }

    if(completeSave || !FieldsEqual(ID_var2Min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2Min", var2Min));
    }

    if(completeSave || !FieldsEqual(ID_var2Max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2Max", var2Max));
    }

    if(completeSave || !FieldsEqual(ID_var2Scaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2Scaling", Scaling_ToString(var2Scaling)));
    }

    if(completeSave || !FieldsEqual(ID_var2SkewFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var2SkewFactor", var2SkewFactor));
    }

    if(completeSave || !FieldsEqual(ID_var3Role, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3Role", VariableRole_ToString(var3Role)));
    }

    if(completeSave || !FieldsEqual(ID_var3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3", var3));
    }

    if(completeSave || !FieldsEqual(ID_var3MinFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3MinFlag", var3MinFlag));
    }

    if(completeSave || !FieldsEqual(ID_var3MaxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3MaxFlag", var3MaxFlag));
    }

    if(completeSave || !FieldsEqual(ID_var3Min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3Min", var3Min));
    }

    if(completeSave || !FieldsEqual(ID_var3Max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3Max", var3Max));
    }

    if(completeSave || !FieldsEqual(ID_var3Scaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3Scaling", Scaling_ToString(var3Scaling)));
    }

    if(completeSave || !FieldsEqual(ID_var3SkewFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var3SkewFactor", var3SkewFactor));
    }

    if(completeSave || !FieldsEqual(ID_var4Role, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4Role", VariableRole_ToString(var4Role)));
    }

    if(completeSave || !FieldsEqual(ID_var4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4", var4));
    }

    if(completeSave || !FieldsEqual(ID_var4MinFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4MinFlag", var4MinFlag));
    }

    if(completeSave || !FieldsEqual(ID_var4MaxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4MaxFlag", var4MaxFlag));
    }

    if(completeSave || !FieldsEqual(ID_var4Min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4Min", var4Min));
    }

    if(completeSave || !FieldsEqual(ID_var4Max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4Max", var4Max));
    }

    if(completeSave || !FieldsEqual(ID_var4Scaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4Scaling", Scaling_ToString(var4Scaling)));
    }

    if(completeSave || !FieldsEqual(ID_var4SkewFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("var4SkewFactor", var4SkewFactor));
    }

    if(completeSave || !FieldsEqual(ID_pointSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSize", pointSize));
    }

    if(completeSave || !FieldsEqual(ID_pointSizePixels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizePixels", pointSizePixels));
    }

    if(completeSave || !FieldsEqual(ID_pointType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointType", PointType_ToString(pointType)));
    }

    if(completeSave || !FieldsEqual(ID_scaleCube, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleCube", scaleCube));
    }

    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

        DataNode *singleColorNode = new DataNode("singleColor");
        if(singleColor.CreateNode(singleColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(singleColorNode);
        }
        else
            delete singleColorNode;
    if(completeSave || !FieldsEqual(ID_foregroundFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("foregroundFlag", foregroundFlag));
    }

    if(completeSave || !FieldsEqual(ID_legendFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: ScatterAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
ScatterAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ScatterAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("var1Role")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetVar1Role(VariableRole(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VariableRole value;
            if(VariableRole_FromString(node->AsString(), value))
                SetVar1Role(value);
        }
    }
    if((node = searchNode->GetNode("var1MinFlag")) != 0)
        SetVar1MinFlag(node->AsBool());
    if((node = searchNode->GetNode("var1MaxFlag")) != 0)
        SetVar1MaxFlag(node->AsBool());
    if((node = searchNode->GetNode("var1Min")) != 0)
        SetVar1Min(node->AsDouble());
    if((node = searchNode->GetNode("var1Max")) != 0)
        SetVar1Max(node->AsDouble());
    if((node = searchNode->GetNode("var1Scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetVar1Scaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetVar1Scaling(value);
        }
    }
    if((node = searchNode->GetNode("var1SkewFactor")) != 0)
        SetVar1SkewFactor(node->AsDouble());
    if((node = searchNode->GetNode("var2Role")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetVar2Role(VariableRole(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VariableRole value;
            if(VariableRole_FromString(node->AsString(), value))
                SetVar2Role(value);
        }
    }
    if((node = searchNode->GetNode("var2")) != 0)
        SetVar2(node->AsString());
    if((node = searchNode->GetNode("var2MinFlag")) != 0)
        SetVar2MinFlag(node->AsBool());
    if((node = searchNode->GetNode("var2MaxFlag")) != 0)
        SetVar2MaxFlag(node->AsBool());
    if((node = searchNode->GetNode("var2Min")) != 0)
        SetVar2Min(node->AsDouble());
    if((node = searchNode->GetNode("var2Max")) != 0)
        SetVar2Max(node->AsDouble());
    if((node = searchNode->GetNode("var2Scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetVar2Scaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetVar2Scaling(value);
        }
    }
    if((node = searchNode->GetNode("var2SkewFactor")) != 0)
        SetVar2SkewFactor(node->AsDouble());
    if((node = searchNode->GetNode("var3Role")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetVar3Role(VariableRole(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VariableRole value;
            if(VariableRole_FromString(node->AsString(), value))
                SetVar3Role(value);
        }
    }
    if((node = searchNode->GetNode("var3")) != 0)
        SetVar3(node->AsString());
    if((node = searchNode->GetNode("var3MinFlag")) != 0)
        SetVar3MinFlag(node->AsBool());
    if((node = searchNode->GetNode("var3MaxFlag")) != 0)
        SetVar3MaxFlag(node->AsBool());
    if((node = searchNode->GetNode("var3Min")) != 0)
        SetVar3Min(node->AsDouble());
    if((node = searchNode->GetNode("var3Max")) != 0)
        SetVar3Max(node->AsDouble());
    if((node = searchNode->GetNode("var3Scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetVar3Scaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetVar3Scaling(value);
        }
    }
    if((node = searchNode->GetNode("var3SkewFactor")) != 0)
        SetVar3SkewFactor(node->AsDouble());
    if((node = searchNode->GetNode("var4Role")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetVar4Role(VariableRole(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VariableRole value;
            if(VariableRole_FromString(node->AsString(), value))
                SetVar4Role(value);
        }
    }
    if((node = searchNode->GetNode("var4")) != 0)
        SetVar4(node->AsString());
    if((node = searchNode->GetNode("var4MinFlag")) != 0)
        SetVar4MinFlag(node->AsBool());
    if((node = searchNode->GetNode("var4MaxFlag")) != 0)
        SetVar4MaxFlag(node->AsBool());
    if((node = searchNode->GetNode("var4Min")) != 0)
        SetVar4Min(node->AsDouble());
    if((node = searchNode->GetNode("var4Max")) != 0)
        SetVar4Max(node->AsDouble());
    if((node = searchNode->GetNode("var4Scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetVar4Scaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetVar4Scaling(value);
        }
    }
    if((node = searchNode->GetNode("var4SkewFactor")) != 0)
        SetVar4SkewFactor(node->AsDouble());
    if((node = searchNode->GetNode("pointSize")) != 0)
        SetPointSize(node->AsDouble());
    if((node = searchNode->GetNode("pointSizePixels")) != 0)
        SetPointSizePixels(node->AsInt());
    if((node = searchNode->GetNode("pointType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetPointType(PointType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PointType value;
            if(PointType_FromString(node->AsString(), value))
                SetPointType(value);
        }
    }
    if((node = searchNode->GetNode("scaleCube")) != 0)
        SetScaleCube(node->AsBool());
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("foregroundFlag")) != 0)
        SetForegroundFlag(node->AsBool());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ScatterAttributes::SetVar1Role(ScatterAttributes::VariableRole var1Role_)
{
    var1Role = var1Role_;
    Select(ID_var1Role, (void *)&var1Role);
}

void
ScatterAttributes::SetVar1MinFlag(bool var1MinFlag_)
{
    var1MinFlag = var1MinFlag_;
    Select(ID_var1MinFlag, (void *)&var1MinFlag);
}

void
ScatterAttributes::SetVar1MaxFlag(bool var1MaxFlag_)
{
    var1MaxFlag = var1MaxFlag_;
    Select(ID_var1MaxFlag, (void *)&var1MaxFlag);
}

void
ScatterAttributes::SetVar1Min(double var1Min_)
{
    var1Min = var1Min_;
    Select(ID_var1Min, (void *)&var1Min);
}

void
ScatterAttributes::SetVar1Max(double var1Max_)
{
    var1Max = var1Max_;
    Select(ID_var1Max, (void *)&var1Max);
}

void
ScatterAttributes::SetVar1Scaling(ScatterAttributes::Scaling var1Scaling_)
{
    var1Scaling = var1Scaling_;
    Select(ID_var1Scaling, (void *)&var1Scaling);
}

void
ScatterAttributes::SetVar1SkewFactor(double var1SkewFactor_)
{
    var1SkewFactor = var1SkewFactor_;
    Select(ID_var1SkewFactor, (void *)&var1SkewFactor);
}

void
ScatterAttributes::SetVar2Role(ScatterAttributes::VariableRole var2Role_)
{
    var2Role = var2Role_;
    Select(ID_var2Role, (void *)&var2Role);
}

void
ScatterAttributes::SetVar2(const std::string &var2_)
{
    var2 = var2_;
    Select(ID_var2, (void *)&var2);
}

void
ScatterAttributes::SetVar2MinFlag(bool var2MinFlag_)
{
    var2MinFlag = var2MinFlag_;
    Select(ID_var2MinFlag, (void *)&var2MinFlag);
}

void
ScatterAttributes::SetVar2MaxFlag(bool var2MaxFlag_)
{
    var2MaxFlag = var2MaxFlag_;
    Select(ID_var2MaxFlag, (void *)&var2MaxFlag);
}

void
ScatterAttributes::SetVar2Min(double var2Min_)
{
    var2Min = var2Min_;
    Select(ID_var2Min, (void *)&var2Min);
}

void
ScatterAttributes::SetVar2Max(double var2Max_)
{
    var2Max = var2Max_;
    Select(ID_var2Max, (void *)&var2Max);
}

void
ScatterAttributes::SetVar2Scaling(ScatterAttributes::Scaling var2Scaling_)
{
    var2Scaling = var2Scaling_;
    Select(ID_var2Scaling, (void *)&var2Scaling);
}

void
ScatterAttributes::SetVar2SkewFactor(double var2SkewFactor_)
{
    var2SkewFactor = var2SkewFactor_;
    Select(ID_var2SkewFactor, (void *)&var2SkewFactor);
}

void
ScatterAttributes::SetVar3Role(ScatterAttributes::VariableRole var3Role_)
{
    var3Role = var3Role_;
    Select(ID_var3Role, (void *)&var3Role);
}

void
ScatterAttributes::SetVar3(const std::string &var3_)
{
    var3 = var3_;
    Select(ID_var3, (void *)&var3);
}

void
ScatterAttributes::SetVar3MinFlag(bool var3MinFlag_)
{
    var3MinFlag = var3MinFlag_;
    Select(ID_var3MinFlag, (void *)&var3MinFlag);
}

void
ScatterAttributes::SetVar3MaxFlag(bool var3MaxFlag_)
{
    var3MaxFlag = var3MaxFlag_;
    Select(ID_var3MaxFlag, (void *)&var3MaxFlag);
}

void
ScatterAttributes::SetVar3Min(double var3Min_)
{
    var3Min = var3Min_;
    Select(ID_var3Min, (void *)&var3Min);
}

void
ScatterAttributes::SetVar3Max(double var3Max_)
{
    var3Max = var3Max_;
    Select(ID_var3Max, (void *)&var3Max);
}

void
ScatterAttributes::SetVar3Scaling(ScatterAttributes::Scaling var3Scaling_)
{
    var3Scaling = var3Scaling_;
    Select(ID_var3Scaling, (void *)&var3Scaling);
}

void
ScatterAttributes::SetVar3SkewFactor(double var3SkewFactor_)
{
    var3SkewFactor = var3SkewFactor_;
    Select(ID_var3SkewFactor, (void *)&var3SkewFactor);
}

void
ScatterAttributes::SetVar4Role(ScatterAttributes::VariableRole var4Role_)
{
    var4Role = var4Role_;
    Select(ID_var4Role, (void *)&var4Role);
}

void
ScatterAttributes::SetVar4(const std::string &var4_)
{
    var4 = var4_;
    Select(ID_var4, (void *)&var4);
}

void
ScatterAttributes::SetVar4MinFlag(bool var4MinFlag_)
{
    var4MinFlag = var4MinFlag_;
    Select(ID_var4MinFlag, (void *)&var4MinFlag);
}

void
ScatterAttributes::SetVar4MaxFlag(bool var4MaxFlag_)
{
    var4MaxFlag = var4MaxFlag_;
    Select(ID_var4MaxFlag, (void *)&var4MaxFlag);
}

void
ScatterAttributes::SetVar4Min(double var4Min_)
{
    var4Min = var4Min_;
    Select(ID_var4Min, (void *)&var4Min);
}

void
ScatterAttributes::SetVar4Max(double var4Max_)
{
    var4Max = var4Max_;
    Select(ID_var4Max, (void *)&var4Max);
}

void
ScatterAttributes::SetVar4Scaling(ScatterAttributes::Scaling var4Scaling_)
{
    var4Scaling = var4Scaling_;
    Select(ID_var4Scaling, (void *)&var4Scaling);
}

void
ScatterAttributes::SetVar4SkewFactor(double var4SkewFactor_)
{
    var4SkewFactor = var4SkewFactor_;
    Select(ID_var4SkewFactor, (void *)&var4SkewFactor);
}

void
ScatterAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
ScatterAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
}

void
ScatterAttributes::SetPointType(ScatterAttributes::PointType pointType_)
{
    pointType = pointType_;
    Select(ID_pointType, (void *)&pointType);
}

void
ScatterAttributes::SetScaleCube(bool scaleCube_)
{
    scaleCube = scaleCube_;
    Select(ID_scaleCube, (void *)&scaleCube);
}

void
ScatterAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
ScatterAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
ScatterAttributes::SetForegroundFlag(bool foregroundFlag_)
{
    foregroundFlag = foregroundFlag_;
    Select(ID_foregroundFlag, (void *)&foregroundFlag);
}

void
ScatterAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

ScatterAttributes::VariableRole
ScatterAttributes::GetVar1Role() const
{
    return VariableRole(var1Role);
}

bool
ScatterAttributes::GetVar1MinFlag() const
{
    return var1MinFlag;
}

bool
ScatterAttributes::GetVar1MaxFlag() const
{
    return var1MaxFlag;
}

double
ScatterAttributes::GetVar1Min() const
{
    return var1Min;
}

double
ScatterAttributes::GetVar1Max() const
{
    return var1Max;
}

ScatterAttributes::Scaling
ScatterAttributes::GetVar1Scaling() const
{
    return Scaling(var1Scaling);
}

double
ScatterAttributes::GetVar1SkewFactor() const
{
    return var1SkewFactor;
}

ScatterAttributes::VariableRole
ScatterAttributes::GetVar2Role() const
{
    return VariableRole(var2Role);
}

const std::string &
ScatterAttributes::GetVar2() const
{
    return var2;
}

std::string &
ScatterAttributes::GetVar2()
{
    return var2;
}

bool
ScatterAttributes::GetVar2MinFlag() const
{
    return var2MinFlag;
}

bool
ScatterAttributes::GetVar2MaxFlag() const
{
    return var2MaxFlag;
}

double
ScatterAttributes::GetVar2Min() const
{
    return var2Min;
}

double
ScatterAttributes::GetVar2Max() const
{
    return var2Max;
}

ScatterAttributes::Scaling
ScatterAttributes::GetVar2Scaling() const
{
    return Scaling(var2Scaling);
}

double
ScatterAttributes::GetVar2SkewFactor() const
{
    return var2SkewFactor;
}

ScatterAttributes::VariableRole
ScatterAttributes::GetVar3Role() const
{
    return VariableRole(var3Role);
}

const std::string &
ScatterAttributes::GetVar3() const
{
    return var3;
}

std::string &
ScatterAttributes::GetVar3()
{
    return var3;
}

bool
ScatterAttributes::GetVar3MinFlag() const
{
    return var3MinFlag;
}

bool
ScatterAttributes::GetVar3MaxFlag() const
{
    return var3MaxFlag;
}

double
ScatterAttributes::GetVar3Min() const
{
    return var3Min;
}

double
ScatterAttributes::GetVar3Max() const
{
    return var3Max;
}

ScatterAttributes::Scaling
ScatterAttributes::GetVar3Scaling() const
{
    return Scaling(var3Scaling);
}

double
ScatterAttributes::GetVar3SkewFactor() const
{
    return var3SkewFactor;
}

ScatterAttributes::VariableRole
ScatterAttributes::GetVar4Role() const
{
    return VariableRole(var4Role);
}

const std::string &
ScatterAttributes::GetVar4() const
{
    return var4;
}

std::string &
ScatterAttributes::GetVar4()
{
    return var4;
}

bool
ScatterAttributes::GetVar4MinFlag() const
{
    return var4MinFlag;
}

bool
ScatterAttributes::GetVar4MaxFlag() const
{
    return var4MaxFlag;
}

double
ScatterAttributes::GetVar4Min() const
{
    return var4Min;
}

double
ScatterAttributes::GetVar4Max() const
{
    return var4Max;
}

ScatterAttributes::Scaling
ScatterAttributes::GetVar4Scaling() const
{
    return Scaling(var4Scaling);
}

double
ScatterAttributes::GetVar4SkewFactor() const
{
    return var4SkewFactor;
}

double
ScatterAttributes::GetPointSize() const
{
    return pointSize;
}

int
ScatterAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

ScatterAttributes::PointType
ScatterAttributes::GetPointType() const
{
    return PointType(pointType);
}

bool
ScatterAttributes::GetScaleCube() const
{
    return scaleCube;
}

const std::string &
ScatterAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
ScatterAttributes::GetColorTableName()
{
    return colorTableName;
}

const ColorAttribute &
ScatterAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
ScatterAttributes::GetSingleColor()
{
    return singleColor;
}

bool
ScatterAttributes::GetForegroundFlag() const
{
    return foregroundFlag;
}

bool
ScatterAttributes::GetLegendFlag() const
{
    return legendFlag;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ScatterAttributes::SelectVar2()
{
    Select(ID_var2, (void *)&var2);
}

void
ScatterAttributes::SelectVar3()
{
    Select(ID_var3, (void *)&var3);
}

void
ScatterAttributes::SelectVar4()
{
    Select(ID_var4, (void *)&var4);
}

void
ScatterAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
ScatterAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ScatterAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
ScatterAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_var1Role:        return "var1Role";
    case ID_var1MinFlag:     return "var1MinFlag";
    case ID_var1MaxFlag:     return "var1MaxFlag";
    case ID_var1Min:         return "var1Min";
    case ID_var1Max:         return "var1Max";
    case ID_var1Scaling:     return "var1Scaling";
    case ID_var1SkewFactor:  return "var1SkewFactor";
    case ID_var2Role:        return "var2Role";
    case ID_var2:            return "var2";
    case ID_var2MinFlag:     return "var2MinFlag";
    case ID_var2MaxFlag:     return "var2MaxFlag";
    case ID_var2Min:         return "var2Min";
    case ID_var2Max:         return "var2Max";
    case ID_var2Scaling:     return "var2Scaling";
    case ID_var2SkewFactor:  return "var2SkewFactor";
    case ID_var3Role:        return "var3Role";
    case ID_var3:            return "var3";
    case ID_var3MinFlag:     return "var3MinFlag";
    case ID_var3MaxFlag:     return "var3MaxFlag";
    case ID_var3Min:         return "var3Min";
    case ID_var3Max:         return "var3Max";
    case ID_var3Scaling:     return "var3Scaling";
    case ID_var3SkewFactor:  return "var3SkewFactor";
    case ID_var4Role:        return "var4Role";
    case ID_var4:            return "var4";
    case ID_var4MinFlag:     return "var4MinFlag";
    case ID_var4MaxFlag:     return "var4MaxFlag";
    case ID_var4Min:         return "var4Min";
    case ID_var4Max:         return "var4Max";
    case ID_var4Scaling:     return "var4Scaling";
    case ID_var4SkewFactor:  return "var4SkewFactor";
    case ID_pointSize:       return "pointSize";
    case ID_pointSizePixels: return "pointSizePixels";
    case ID_pointType:       return "pointType";
    case ID_scaleCube:       return "scaleCube";
    case ID_colorTableName:  return "colorTableName";
    case ID_singleColor:     return "singleColor";
    case ID_foregroundFlag:  return "foregroundFlag";
    case ID_legendFlag:      return "legendFlag";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ScatterAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
ScatterAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_var1Role:        return FieldType_enum;
    case ID_var1MinFlag:     return FieldType_bool;
    case ID_var1MaxFlag:     return FieldType_bool;
    case ID_var1Min:         return FieldType_double;
    case ID_var1Max:         return FieldType_double;
    case ID_var1Scaling:     return FieldType_enum;
    case ID_var1SkewFactor:  return FieldType_double;
    case ID_var2Role:        return FieldType_enum;
    case ID_var2:            return FieldType_string;
    case ID_var2MinFlag:     return FieldType_bool;
    case ID_var2MaxFlag:     return FieldType_bool;
    case ID_var2Min:         return FieldType_double;
    case ID_var2Max:         return FieldType_double;
    case ID_var2Scaling:     return FieldType_enum;
    case ID_var2SkewFactor:  return FieldType_double;
    case ID_var3Role:        return FieldType_enum;
    case ID_var3:            return FieldType_string;
    case ID_var3MinFlag:     return FieldType_bool;
    case ID_var3MaxFlag:     return FieldType_bool;
    case ID_var3Min:         return FieldType_double;
    case ID_var3Max:         return FieldType_double;
    case ID_var3Scaling:     return FieldType_enum;
    case ID_var3SkewFactor:  return FieldType_double;
    case ID_var4Role:        return FieldType_enum;
    case ID_var4:            return FieldType_string;
    case ID_var4MinFlag:     return FieldType_bool;
    case ID_var4MaxFlag:     return FieldType_bool;
    case ID_var4Min:         return FieldType_double;
    case ID_var4Max:         return FieldType_double;
    case ID_var4Scaling:     return FieldType_enum;
    case ID_var4SkewFactor:  return FieldType_double;
    case ID_pointSize:       return FieldType_double;
    case ID_pointSizePixels: return FieldType_int;
    case ID_pointType:       return FieldType_enum;
    case ID_scaleCube:       return FieldType_bool;
    case ID_colorTableName:  return FieldType_colortable;
    case ID_singleColor:     return FieldType_color;
    case ID_foregroundFlag:  return FieldType_bool;
    case ID_legendFlag:      return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ScatterAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
ScatterAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_var1Role:        return "enum";
    case ID_var1MinFlag:     return "bool";
    case ID_var1MaxFlag:     return "bool";
    case ID_var1Min:         return "double";
    case ID_var1Max:         return "double";
    case ID_var1Scaling:     return "enum";
    case ID_var1SkewFactor:  return "double";
    case ID_var2Role:        return "enum";
    case ID_var2:            return "string";
    case ID_var2MinFlag:     return "bool";
    case ID_var2MaxFlag:     return "bool";
    case ID_var2Min:         return "double";
    case ID_var2Max:         return "double";
    case ID_var2Scaling:     return "enum";
    case ID_var2SkewFactor:  return "double";
    case ID_var3Role:        return "enum";
    case ID_var3:            return "string";
    case ID_var3MinFlag:     return "bool";
    case ID_var3MaxFlag:     return "bool";
    case ID_var3Min:         return "double";
    case ID_var3Max:         return "double";
    case ID_var3Scaling:     return "enum";
    case ID_var3SkewFactor:  return "double";
    case ID_var4Role:        return "enum";
    case ID_var4:            return "string";
    case ID_var4MinFlag:     return "bool";
    case ID_var4MaxFlag:     return "bool";
    case ID_var4Min:         return "double";
    case ID_var4Max:         return "double";
    case ID_var4Scaling:     return "enum";
    case ID_var4SkewFactor:  return "double";
    case ID_pointSize:       return "double";
    case ID_pointSizePixels: return "int";
    case ID_pointType:       return "enum";
    case ID_scaleCube:       return "bool";
    case ID_colorTableName:  return "colortable";
    case ID_singleColor:     return "color";
    case ID_foregroundFlag:  return "bool";
    case ID_legendFlag:      return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ScatterAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Mon Dec 17 12:11:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ScatterAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ScatterAttributes &obj = *((const ScatterAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_var1Role:
        {  // new scope
        retval = (var1Role == obj.var1Role);
        }
        break;
    case ID_var1MinFlag:
        {  // new scope
        retval = (var1MinFlag == obj.var1MinFlag);
        }
        break;
    case ID_var1MaxFlag:
        {  // new scope
        retval = (var1MaxFlag == obj.var1MaxFlag);
        }
        break;
    case ID_var1Min:
        {  // new scope
        retval = (var1Min == obj.var1Min);
        }
        break;
    case ID_var1Max:
        {  // new scope
        retval = (var1Max == obj.var1Max);
        }
        break;
    case ID_var1Scaling:
        {  // new scope
        retval = (var1Scaling == obj.var1Scaling);
        }
        break;
    case ID_var1SkewFactor:
        {  // new scope
        retval = (var1SkewFactor == obj.var1SkewFactor);
        }
        break;
    case ID_var2Role:
        {  // new scope
        retval = (var2Role == obj.var2Role);
        }
        break;
    case ID_var2:
        {  // new scope
        retval = (var2 == obj.var2);
        }
        break;
    case ID_var2MinFlag:
        {  // new scope
        retval = (var2MinFlag == obj.var2MinFlag);
        }
        break;
    case ID_var2MaxFlag:
        {  // new scope
        retval = (var2MaxFlag == obj.var2MaxFlag);
        }
        break;
    case ID_var2Min:
        {  // new scope
        retval = (var2Min == obj.var2Min);
        }
        break;
    case ID_var2Max:
        {  // new scope
        retval = (var2Max == obj.var2Max);
        }
        break;
    case ID_var2Scaling:
        {  // new scope
        retval = (var2Scaling == obj.var2Scaling);
        }
        break;
    case ID_var2SkewFactor:
        {  // new scope
        retval = (var2SkewFactor == obj.var2SkewFactor);
        }
        break;
    case ID_var3Role:
        {  // new scope
        retval = (var3Role == obj.var3Role);
        }
        break;
    case ID_var3:
        {  // new scope
        retval = (var3 == obj.var3);
        }
        break;
    case ID_var3MinFlag:
        {  // new scope
        retval = (var3MinFlag == obj.var3MinFlag);
        }
        break;
    case ID_var3MaxFlag:
        {  // new scope
        retval = (var3MaxFlag == obj.var3MaxFlag);
        }
        break;
    case ID_var3Min:
        {  // new scope
        retval = (var3Min == obj.var3Min);
        }
        break;
    case ID_var3Max:
        {  // new scope
        retval = (var3Max == obj.var3Max);
        }
        break;
    case ID_var3Scaling:
        {  // new scope
        retval = (var3Scaling == obj.var3Scaling);
        }
        break;
    case ID_var3SkewFactor:
        {  // new scope
        retval = (var3SkewFactor == obj.var3SkewFactor);
        }
        break;
    case ID_var4Role:
        {  // new scope
        retval = (var4Role == obj.var4Role);
        }
        break;
    case ID_var4:
        {  // new scope
        retval = (var4 == obj.var4);
        }
        break;
    case ID_var4MinFlag:
        {  // new scope
        retval = (var4MinFlag == obj.var4MinFlag);
        }
        break;
    case ID_var4MaxFlag:
        {  // new scope
        retval = (var4MaxFlag == obj.var4MaxFlag);
        }
        break;
    case ID_var4Min:
        {  // new scope
        retval = (var4Min == obj.var4Min);
        }
        break;
    case ID_var4Max:
        {  // new scope
        retval = (var4Max == obj.var4Max);
        }
        break;
    case ID_var4Scaling:
        {  // new scope
        retval = (var4Scaling == obj.var4Scaling);
        }
        break;
    case ID_var4SkewFactor:
        {  // new scope
        retval = (var4SkewFactor == obj.var4SkewFactor);
        }
        break;
    case ID_pointSize:
        {  // new scope
        retval = (pointSize == obj.pointSize);
        }
        break;
    case ID_pointSizePixels:
        {  // new scope
        retval = (pointSizePixels == obj.pointSizePixels);
        }
        break;
    case ID_pointType:
        {  // new scope
        retval = (pointType == obj.pointType);
        }
        break;
    case ID_scaleCube:
        {  // new scope
        retval = (scaleCube == obj.scaleCube);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_singleColor:
        {  // new scope
        retval = (singleColor == obj.singleColor);
        }
        break;
    case ID_foregroundFlag:
        {  // new scope
        retval = (foregroundFlag == obj.foregroundFlag);
        }
        break;
    case ID_legendFlag:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Modifications:
//
// ****************************************************************************
bool
ScatterAttributes::ChangesRequireRecalculation(const ScatterAttributes &obj) const
{
    bool var1Changed = false;
    if(var1Role != None && var1Role != Color)
    {
        bool minMaxChanged =
            var1MinFlag != obj.var1MinFlag ||
            var1MaxFlag != obj.var1MaxFlag ||
            var1Min != obj.var1Min ||
            var1Max != obj.var1Max;

        bool scalingChanged = 
            var1Scaling != obj.var1Scaling ||
            (var1Scaling == Skew && var1SkewFactor != obj.var1SkewFactor);

        var1Changed = minMaxChanged || scalingChanged;
    }

    bool var2Changed = false;
    if(var2Role != None)
    {
        if(var2Role != Color)
        {
            bool minMaxChanged =
                var2MinFlag != obj.var2MinFlag ||
                var2MaxFlag != obj.var2MaxFlag ||
                var2Min != obj.var2Min ||
                var2Max != obj.var2Max;

            bool scalingChanged = 
                var2Scaling != obj.var2Scaling ||
                (var2Scaling == Skew && var2SkewFactor != obj.var2SkewFactor);

            var2Changed = var2 != obj.var2 || minMaxChanged || scalingChanged;
        }
        else
            var2Changed = var2 != obj.var2;
    }

    bool var3Changed = false;
    if(var3Role != None)
    {
        if(var3Role != Color)
        {
            bool minMaxChanged =
                var3MinFlag != obj.var3MinFlag ||
                var3MaxFlag != obj.var3MaxFlag ||
                var3Min != obj.var3Min ||
                var3Max != obj.var3Max;

            bool scalingChanged = 
                var3Scaling != obj.var3Scaling ||
                (var3Scaling == Skew && var3SkewFactor != obj.var3SkewFactor);

            var3Changed = var3 != obj.var3 || minMaxChanged || scalingChanged;
        }
        else
            var3Changed = var3 != obj.var3;
    }

    bool var4Changed = false;
    if(var4Role != None)
    {
        if(var4Role != Color)
        {
            bool minMaxChanged =
                var4MinFlag != obj.var4MinFlag ||
                var4MaxFlag != obj.var4MaxFlag ||
                var4Min != obj.var4Min ||
                var4Max != obj.var4Max;

            bool scalingChanged = 
                var4Scaling != obj.var4Scaling ||
                (var4Scaling == Skew && var4SkewFactor != obj.var4SkewFactor);

            var4Changed = var4 != obj.var4 || minMaxChanged || scalingChanged;
        }
        else
            var4Changed = var4 != obj.var4;
    }

    return var1Role != obj.var1Role ||
           var2Role != obj.var2Role ||
           var3Role != obj.var3Role ||
           var4Role != obj.var4Role ||
           scaleCube != obj.scaleCube ||
           var1Changed ||
           var2Changed ||
           var3Changed ||
           var4Changed;
}


