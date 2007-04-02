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

#include <TransformAttributes.h>
#include <DataNode.h>
#include <BoxExtents.h>

//
// Enum conversion methods for TransformAttributes::AngleType
//

static const char *AngleType_strings[] = {
"Deg", "Rad"};

std::string
TransformAttributes::AngleType_ToString(TransformAttributes::AngleType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return AngleType_strings[index];
}

std::string
TransformAttributes::AngleType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return AngleType_strings[index];
}

bool
TransformAttributes::AngleType_FromString(const std::string &s, TransformAttributes::AngleType &val)
{
    val = TransformAttributes::Deg;
    for(int i = 0; i < 2; ++i)
    {
        if(s == AngleType_strings[i])
        {
            val = (AngleType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for TransformAttributes::TransformType
//

static const char *TransformType_strings[] = {
"Similarity", "Coordinate"};

std::string
TransformAttributes::TransformType_ToString(TransformAttributes::TransformType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return TransformType_strings[index];
}

std::string
TransformAttributes::TransformType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return TransformType_strings[index];
}

bool
TransformAttributes::TransformType_FromString(const std::string &s, TransformAttributes::TransformType &val)
{
    val = TransformAttributes::Similarity;
    for(int i = 0; i < 2; ++i)
    {
        if(s == TransformType_strings[i])
        {
            val = (TransformType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for TransformAttributes::CoordinateSystem
//

static const char *CoordinateSystem_strings[] = {
"Cartesian", "Cylindrical", "Spherical"
};

std::string
TransformAttributes::CoordinateSystem_ToString(TransformAttributes::CoordinateSystem t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return CoordinateSystem_strings[index];
}

std::string
TransformAttributes::CoordinateSystem_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return CoordinateSystem_strings[index];
}

bool
TransformAttributes::CoordinateSystem_FromString(const std::string &s, TransformAttributes::CoordinateSystem &val)
{
    val = TransformAttributes::Cartesian;
    for(int i = 0; i < 3; ++i)
    {
        if(s == CoordinateSystem_strings[i])
        {
            val = (CoordinateSystem)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Constructor for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::TransformAttributes() : AttributeSubject("bFFfibFfffbfffiii")
{
    doRotate = false;
    rotateOrigin[0] = 0;
    rotateOrigin[1] = 0;
    rotateOrigin[2] = 0;
    rotateAxis[0] = 0;
    rotateAxis[1] = 0;
    rotateAxis[2] = 1;
    rotateAmount = 0;
    rotateType = Deg;
    doScale = false;
    scaleOrigin[0] = 0;
    scaleOrigin[1] = 0;
    scaleOrigin[2] = 0;
    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;
    doTranslate = false;
    translateX = 0;
    translateY = 0;
    translateZ = 0;
    transformType = Similarity;
    inputCoordSys = Cartesian;
    outputCoordSys = Spherical;
}

// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Copy constructor for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::TransformAttributes(const TransformAttributes &obj) : AttributeSubject("bFFfibFfffbfffiii")
{
    doRotate = obj.doRotate;
    rotateOrigin[0] = obj.rotateOrigin[0];
    rotateOrigin[1] = obj.rotateOrigin[1];
    rotateOrigin[2] = obj.rotateOrigin[2];

    rotateAxis[0] = obj.rotateAxis[0];
    rotateAxis[1] = obj.rotateAxis[1];
    rotateAxis[2] = obj.rotateAxis[2];

    rotateAmount = obj.rotateAmount;
    rotateType = obj.rotateType;
    doScale = obj.doScale;
    scaleOrigin[0] = obj.scaleOrigin[0];
    scaleOrigin[1] = obj.scaleOrigin[1];
    scaleOrigin[2] = obj.scaleOrigin[2];

    scaleX = obj.scaleX;
    scaleY = obj.scaleY;
    scaleZ = obj.scaleZ;
    doTranslate = obj.doTranslate;
    translateX = obj.translateX;
    translateY = obj.translateY;
    translateZ = obj.translateZ;
    transformType = obj.transformType;
    inputCoordSys = obj.inputCoordSys;
    outputCoordSys = obj.outputCoordSys;

    SelectAll();
}

// ****************************************************************************
// Method: TransformAttributes::~TransformAttributes
//
// Purpose: 
//   Destructor for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::~TransformAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: TransformAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes& 
TransformAttributes::operator = (const TransformAttributes &obj)
{
    if (this == &obj) return *this;
    doRotate = obj.doRotate;
    rotateOrigin[0] = obj.rotateOrigin[0];
    rotateOrigin[1] = obj.rotateOrigin[1];
    rotateOrigin[2] = obj.rotateOrigin[2];

    rotateAxis[0] = obj.rotateAxis[0];
    rotateAxis[1] = obj.rotateAxis[1];
    rotateAxis[2] = obj.rotateAxis[2];

    rotateAmount = obj.rotateAmount;
    rotateType = obj.rotateType;
    doScale = obj.doScale;
    scaleOrigin[0] = obj.scaleOrigin[0];
    scaleOrigin[1] = obj.scaleOrigin[1];
    scaleOrigin[2] = obj.scaleOrigin[2];

    scaleX = obj.scaleX;
    scaleY = obj.scaleY;
    scaleZ = obj.scaleZ;
    doTranslate = obj.doTranslate;
    translateX = obj.translateX;
    translateY = obj.translateY;
    translateZ = obj.translateZ;
    transformType = obj.transformType;
    inputCoordSys = obj.inputCoordSys;
    outputCoordSys = obj.outputCoordSys;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: TransformAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
TransformAttributes::operator == (const TransformAttributes &obj) const
{
    int i;

    // Compare the rotateOrigin arrays.
    bool rotateOrigin_equal = true;
    for(i = 0; i < 3 && rotateOrigin_equal; ++i)
        rotateOrigin_equal = (rotateOrigin[i] == obj.rotateOrigin[i]);

    // Compare the rotateAxis arrays.
    bool rotateAxis_equal = true;
    for(i = 0; i < 3 && rotateAxis_equal; ++i)
        rotateAxis_equal = (rotateAxis[i] == obj.rotateAxis[i]);

    // Compare the scaleOrigin arrays.
    bool scaleOrigin_equal = true;
    for(i = 0; i < 3 && scaleOrigin_equal; ++i)
        scaleOrigin_equal = (scaleOrigin[i] == obj.scaleOrigin[i]);

    // Create the return value
    return ((doRotate == obj.doRotate) &&
            rotateOrigin_equal &&
            rotateAxis_equal &&
            (rotateAmount == obj.rotateAmount) &&
            (rotateType == obj.rotateType) &&
            (doScale == obj.doScale) &&
            scaleOrigin_equal &&
            (scaleX == obj.scaleX) &&
            (scaleY == obj.scaleY) &&
            (scaleZ == obj.scaleZ) &&
            (doTranslate == obj.doTranslate) &&
            (translateX == obj.translateX) &&
            (translateY == obj.translateY) &&
            (translateZ == obj.translateZ) &&
            (transformType == obj.transformType) &&
            (inputCoordSys == obj.inputCoordSys) &&
            (outputCoordSys == obj.outputCoordSys));
}

// ****************************************************************************
// Method: TransformAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
TransformAttributes::operator != (const TransformAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: TransformAttributes::TypeName
//
// Purpose: 
//   Type name method for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

const std::string
TransformAttributes::TypeName() const
{
    return "TransformAttributes";
}

// ****************************************************************************
// Method: TransformAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the TransformAttributes class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 29 08:57:18 PDT 2002
//
// Modifications:
//
// ****************************************************************************

bool
TransformAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const TransformAttributes *tmp = (const TransformAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "BoxExtents")
    {
        const BoxExtents *tmp = (const BoxExtents *)atts;
        const double *extents = tmp->GetExtents();
        if(doTranslate)
        {
            SetTranslateX(extents[0]);
            SetTranslateY(extents[2]);
            SetTranslateZ(extents[4]);
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: TransformAttributes::CreateCompatible
//
// Purpose: 
//   Creates an object of the specified type initialized with the attributes
//   from this object.
//
// Arguments:
//   tname : The typename of the object that we want to create.
//
// Returns:    A new object of the type specified by tname or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 29 08:57:46 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
TransformAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;

    if(TypeName() == tname)
    {
        retval = new TransformAttributes(*this);
    }
    else if(tname == "BoxExtents")
    {
        BoxExtents *b = new BoxExtents;
        double extents[6];
        extents[0] = translateX;
        extents[1] = translateX + 1.;
        extents[2] = translateY;
        extents[3] = translateY + 1.;
        extents[4] = translateZ;
        extents[5] = translateZ + 1.;
        b->SetExtents(extents);

        retval = b;
    }

    return retval;
}

// ****************************************************************************
// Method: TransformAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
TransformAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new TransformAttributes(*this);
    else
        retval = new TransformAttributes;

    return retval;
}

// ****************************************************************************
// Method: TransformAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
TransformAttributes::SelectAll()
{
    Select(0, (void *)&doRotate);
    Select(1, (void *)rotateOrigin, 3);
    Select(2, (void *)rotateAxis, 3);
    Select(3, (void *)&rotateAmount);
    Select(4, (void *)&rotateType);
    Select(5, (void *)&doScale);
    Select(6, (void *)scaleOrigin, 3);
    Select(7, (void *)&scaleX);
    Select(8, (void *)&scaleY);
    Select(9, (void *)&scaleZ);
    Select(10, (void *)&doTranslate);
    Select(11, (void *)&translateX);
    Select(12, (void *)&translateY);
    Select(13, (void *)&translateZ);
    Select(14, (void *)&transformType);
    Select(15, (void *)&inputCoordSys);
    Select(16, (void *)&outputCoordSys);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TransformAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
TransformAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    TransformAttributes defaultObject;
    bool addToParent = false;
    // Create a node for TransformAttributes.
    DataNode *node = new DataNode("TransformAttributes");

    if(completeSave || !FieldsEqual(0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doRotate", doRotate));
    }

    if(completeSave || !FieldsEqual(1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateOrigin", rotateOrigin, 3));
    }

    if(completeSave || !FieldsEqual(2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateAxis", rotateAxis, 3));
    }

    if(completeSave || !FieldsEqual(3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateAmount", rotateAmount));
    }

    if(completeSave || !FieldsEqual(4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateType", AngleType_ToString(rotateType)));
    }

    if(completeSave || !FieldsEqual(5, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doScale", doScale));
    }

    if(completeSave || !FieldsEqual(6, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleOrigin", scaleOrigin, 3));
    }

    if(completeSave || !FieldsEqual(7, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleX", scaleX));
    }

    if(completeSave || !FieldsEqual(8, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleY", scaleY));
    }

    if(completeSave || !FieldsEqual(9, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleZ", scaleZ));
    }

    if(completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doTranslate", doTranslate));
    }

    if(completeSave || !FieldsEqual(11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("translateX", translateX));
    }

    if(completeSave || !FieldsEqual(12, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("translateY", translateY));
    }

    if(completeSave || !FieldsEqual(13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("translateZ", translateZ));
    }

    if(completeSave || !FieldsEqual(14, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("transformType", TransformType_ToString(transformType)));
    }

    if(completeSave || !FieldsEqual(15, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("inputCoordSys", CoordinateSystem_ToString(inputCoordSys)));
    }

    if(completeSave || !FieldsEqual(16, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("outputCoordSys", CoordinateSystem_ToString(outputCoordSys)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: TransformAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
TransformAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("TransformAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("doRotate")) != 0)
        SetDoRotate(node->AsBool());
    if((node = searchNode->GetNode("rotateOrigin")) != 0)
        SetRotateOrigin(node->AsFloatArray());
    if((node = searchNode->GetNode("rotateAxis")) != 0)
        SetRotateAxis(node->AsFloatArray());
    if((node = searchNode->GetNode("rotateAmount")) != 0)
        SetRotateAmount(node->AsFloat());
    if((node = searchNode->GetNode("rotateType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetRotateType(AngleType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            AngleType value;
            if(AngleType_FromString(node->AsString(), value))
                SetRotateType(value);
        }
    }
    if((node = searchNode->GetNode("doScale")) != 0)
        SetDoScale(node->AsBool());
    if((node = searchNode->GetNode("scaleOrigin")) != 0)
        SetScaleOrigin(node->AsFloatArray());
    if((node = searchNode->GetNode("scaleX")) != 0)
        SetScaleX(node->AsFloat());
    if((node = searchNode->GetNode("scaleY")) != 0)
        SetScaleY(node->AsFloat());
    if((node = searchNode->GetNode("scaleZ")) != 0)
        SetScaleZ(node->AsFloat());
    if((node = searchNode->GetNode("doTranslate")) != 0)
        SetDoTranslate(node->AsBool());
    if((node = searchNode->GetNode("translateX")) != 0)
        SetTranslateX(node->AsFloat());
    if((node = searchNode->GetNode("translateY")) != 0)
        SetTranslateY(node->AsFloat());
    if((node = searchNode->GetNode("translateZ")) != 0)
        SetTranslateZ(node->AsFloat());
    if((node = searchNode->GetNode("transformType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetTransformType(TransformType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TransformType value;
            if(TransformType_FromString(node->AsString(), value))
                SetTransformType(value);
        }
    }
    if((node = searchNode->GetNode("inputCoordSys")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetInputCoordSys(CoordinateSystem(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CoordinateSystem value;
            if(CoordinateSystem_FromString(node->AsString(), value))
                SetInputCoordSys(value);
        }
    }
    if((node = searchNode->GetNode("outputCoordSys")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetOutputCoordSys(CoordinateSystem(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CoordinateSystem value;
            if(CoordinateSystem_FromString(node->AsString(), value))
                SetOutputCoordSys(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
TransformAttributes::SetDoRotate(bool doRotate_)
{
    doRotate = doRotate_;
    Select(0, (void *)&doRotate);
}

void
TransformAttributes::SetRotateOrigin(const float *rotateOrigin_)
{
    rotateOrigin[0] = rotateOrigin_[0];
    rotateOrigin[1] = rotateOrigin_[1];
    rotateOrigin[2] = rotateOrigin_[2];
    Select(1, (void *)rotateOrigin, 3);
}

void
TransformAttributes::SetRotateAxis(const float *rotateAxis_)
{
    rotateAxis[0] = rotateAxis_[0];
    rotateAxis[1] = rotateAxis_[1];
    rotateAxis[2] = rotateAxis_[2];
    Select(2, (void *)rotateAxis, 3);
}

void
TransformAttributes::SetRotateAmount(float rotateAmount_)
{
    rotateAmount = rotateAmount_;
    Select(3, (void *)&rotateAmount);
}

void
TransformAttributes::SetRotateType(TransformAttributes::AngleType rotateType_)
{
    rotateType = rotateType_;
    Select(4, (void *)&rotateType);
}

void
TransformAttributes::SetDoScale(bool doScale_)
{
    doScale = doScale_;
    Select(5, (void *)&doScale);
}

void
TransformAttributes::SetScaleOrigin(const float *scaleOrigin_)
{
    scaleOrigin[0] = scaleOrigin_[0];
    scaleOrigin[1] = scaleOrigin_[1];
    scaleOrigin[2] = scaleOrigin_[2];
    Select(6, (void *)scaleOrigin, 3);
}

void
TransformAttributes::SetScaleX(float scaleX_)
{
    scaleX = scaleX_;
    Select(7, (void *)&scaleX);
}

void
TransformAttributes::SetScaleY(float scaleY_)
{
    scaleY = scaleY_;
    Select(8, (void *)&scaleY);
}

void
TransformAttributes::SetScaleZ(float scaleZ_)
{
    scaleZ = scaleZ_;
    Select(9, (void *)&scaleZ);
}

void
TransformAttributes::SetDoTranslate(bool doTranslate_)
{
    doTranslate = doTranslate_;
    Select(10, (void *)&doTranslate);
}

void
TransformAttributes::SetTranslateX(float translateX_)
{
    translateX = translateX_;
    Select(11, (void *)&translateX);
}

void
TransformAttributes::SetTranslateY(float translateY_)
{
    translateY = translateY_;
    Select(12, (void *)&translateY);
}

void
TransformAttributes::SetTranslateZ(float translateZ_)
{
    translateZ = translateZ_;
    Select(13, (void *)&translateZ);
}

void
TransformAttributes::SetTransformType(TransformAttributes::TransformType transformType_)
{
    transformType = transformType_;
    Select(14, (void *)&transformType);
}

void
TransformAttributes::SetInputCoordSys(TransformAttributes::CoordinateSystem inputCoordSys_)
{
    inputCoordSys = inputCoordSys_;
    Select(15, (void *)&inputCoordSys);
}

void
TransformAttributes::SetOutputCoordSys(TransformAttributes::CoordinateSystem outputCoordSys_)
{
    outputCoordSys = outputCoordSys_;
    Select(16, (void *)&outputCoordSys);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
TransformAttributes::GetDoRotate() const
{
    return doRotate;
}

const float *
TransformAttributes::GetRotateOrigin() const
{
    return rotateOrigin;
}

float *
TransformAttributes::GetRotateOrigin()
{
    return rotateOrigin;
}

const float *
TransformAttributes::GetRotateAxis() const
{
    return rotateAxis;
}

float *
TransformAttributes::GetRotateAxis()
{
    return rotateAxis;
}

float
TransformAttributes::GetRotateAmount() const
{
    return rotateAmount;
}

TransformAttributes::AngleType
TransformAttributes::GetRotateType() const
{
    return AngleType(rotateType);
}

bool
TransformAttributes::GetDoScale() const
{
    return doScale;
}

const float *
TransformAttributes::GetScaleOrigin() const
{
    return scaleOrigin;
}

float *
TransformAttributes::GetScaleOrigin()
{
    return scaleOrigin;
}

float
TransformAttributes::GetScaleX() const
{
    return scaleX;
}

float
TransformAttributes::GetScaleY() const
{
    return scaleY;
}

float
TransformAttributes::GetScaleZ() const
{
    return scaleZ;
}

bool
TransformAttributes::GetDoTranslate() const
{
    return doTranslate;
}

float
TransformAttributes::GetTranslateX() const
{
    return translateX;
}

float
TransformAttributes::GetTranslateY() const
{
    return translateY;
}

float
TransformAttributes::GetTranslateZ() const
{
    return translateZ;
}

TransformAttributes::TransformType
TransformAttributes::GetTransformType() const
{
    return TransformType(transformType);
}

TransformAttributes::CoordinateSystem
TransformAttributes::GetInputCoordSys() const
{
    return CoordinateSystem(inputCoordSys);
}

TransformAttributes::CoordinateSystem
TransformAttributes::GetOutputCoordSys() const
{
    return CoordinateSystem(outputCoordSys);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
TransformAttributes::SelectRotateOrigin()
{
    Select(1, (void *)rotateOrigin, 3);
}

void
TransformAttributes::SelectRotateAxis()
{
    Select(2, (void *)rotateAxis, 3);
}

void
TransformAttributes::SelectScaleOrigin()
{
    Select(6, (void *)scaleOrigin, 3);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TransformAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
TransformAttributes::GetFieldName(int index) const
{
    switch (index)
    {
        case 0:  return "Rotate";
        case 1:  return "Origin";
        case 2:  return "Axis";
        case 3:  return "Amount";
        case 4:  return "Rotate";
        case 5:  return "Scale";
        case 6:  return "Origin";
        case 7:  return "X";
        case 8:  return "Y";
        case 9:  return "Z";
        case 10:  return "Translate";
        case 11:  return "X";
        case 12:  return "Y";
        case 13:  return "Z";
        case 14:  return "transformType";
        case 15:  return "Input Coordinate System";
        case 16:  return "Desired Coordinate System";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: TransformAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
TransformAttributes::GetFieldType(int index) const
{
    switch (index)
    {
        case 0:  return FieldType_bool;
        case 1:  return FieldType_floatArray;
        case 2:  return FieldType_floatArray;
        case 3:  return FieldType_float;
        case 4:  return FieldType_enum;
        case 5:  return FieldType_bool;
        case 6:  return FieldType_floatArray;
        case 7:  return FieldType_float;
        case 8:  return FieldType_float;
        case 9:  return FieldType_float;
        case 10:  return FieldType_bool;
        case 11:  return FieldType_float;
        case 12:  return FieldType_float;
        case 13:  return FieldType_float;
        case 14:  return FieldType_enum;
        case 15:  return FieldType_enum;
        case 16:  return FieldType_enum;
        default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: TransformAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
TransformAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
        case 0:  return "bool";
        case 1:  return "floatArray";
        case 2:  return "floatArray";
        case 3:  return "float";
        case 4:  return "enum";
        case 5:  return "bool";
        case 6:  return "floatArray";
        case 7:  return "float";
        case 8:  return "float";
        case 9:  return "float";
        case 10:  return "bool";
        case 11:  return "float";
        case 12:  return "float";
        case 13:  return "float";
        case 14:  return "enum";
        case 15:  return "enum";
        case 16:  return "enum";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: TransformAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Fri Feb 4 15:26:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
TransformAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    int i;

    const TransformAttributes &obj = *((const TransformAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case 0:
        {  // new scope
        retval = (doRotate == obj.doRotate);
        }
        break;
    case 1:
        {  // new scope
        // Compare the rotateOrigin arrays.
        bool rotateOrigin_equal = true;
        for(i = 0; i < 3 && rotateOrigin_equal; ++i)
            rotateOrigin_equal = (rotateOrigin[i] == obj.rotateOrigin[i]);

        retval = rotateOrigin_equal;
        }
        break;
    case 2:
        {  // new scope
        // Compare the rotateAxis arrays.
        bool rotateAxis_equal = true;
        for(i = 0; i < 3 && rotateAxis_equal; ++i)
            rotateAxis_equal = (rotateAxis[i] == obj.rotateAxis[i]);

        retval = rotateAxis_equal;
        }
        break;
    case 3:
        {  // new scope
        retval = (rotateAmount == obj.rotateAmount);
        }
        break;
    case 4:
        {  // new scope
        retval = (rotateType == obj.rotateType);
        }
        break;
    case 5:
        {  // new scope
        retval = (doScale == obj.doScale);
        }
        break;
    case 6:
        {  // new scope
        // Compare the scaleOrigin arrays.
        bool scaleOrigin_equal = true;
        for(i = 0; i < 3 && scaleOrigin_equal; ++i)
            scaleOrigin_equal = (scaleOrigin[i] == obj.scaleOrigin[i]);

        retval = scaleOrigin_equal;
        }
        break;
    case 7:
        {  // new scope
        retval = (scaleX == obj.scaleX);
        }
        break;
    case 8:
        {  // new scope
        retval = (scaleY == obj.scaleY);
        }
        break;
    case 9:
        {  // new scope
        retval = (scaleZ == obj.scaleZ);
        }
        break;
    case 10:
        {  // new scope
        retval = (doTranslate == obj.doTranslate);
        }
        break;
    case 11:
        {  // new scope
        retval = (translateX == obj.translateX);
        }
        break;
    case 12:
        {  // new scope
        retval = (translateY == obj.translateY);
        }
        break;
    case 13:
        {  // new scope
        retval = (translateZ == obj.translateZ);
        }
        break;
    case 14:
        {  // new scope
        retval = (transformType == obj.transformType);
        }
        break;
    case 15:
        {  // new scope
        retval = (inputCoordSys == obj.inputCoordSys);
        }
        break;
    case 16:
        {  // new scope
        retval = (outputCoordSys == obj.outputCoordSys);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

