/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
"Similarity", "Coordinate", "Linear"
};

std::string
TransformAttributes::TransformType_ToString(TransformAttributes::TransformType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return TransformType_strings[index];
}

std::string
TransformAttributes::TransformType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return TransformType_strings[index];
}

bool
TransformAttributes::TransformType_FromString(const std::string &s, TransformAttributes::TransformType &val)
{
    val = TransformAttributes::Similarity;
    for(int i = 0; i < 3; ++i)
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

//
// Enum conversion methods for TransformAttributes::VectorTransformMethod
//

static const char *VectorTransformMethod_strings[] = {
"None", "AsPoint", "AsDisplacement", 
"AsDirection"};

std::string
TransformAttributes::VectorTransformMethod_ToString(TransformAttributes::VectorTransformMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return VectorTransformMethod_strings[index];
}

std::string
TransformAttributes::VectorTransformMethod_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return VectorTransformMethod_strings[index];
}

bool
TransformAttributes::VectorTransformMethod_FromString(const std::string &s, TransformAttributes::VectorTransformMethod &val)
{
    val = TransformAttributes::None;
    for(int i = 0; i < 4; ++i)
    {
        if(s == VectorTransformMethod_strings[i])
        {
            val = (VectorTransformMethod)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Init utility for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void TransformAttributes::Init()
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
    continuousPhi = false;
    m00 = 1;
    m01 = 0;
    m02 = 0;
    m03 = 0;
    m10 = 0;
    m11 = 1;
    m12 = 0;
    m13 = 0;
    m20 = 0;
    m21 = 0;
    m22 = 1;
    m23 = 0;
    m30 = 0;
    m31 = 0;
    m32 = 0;
    m33 = 1;
    invertLinearTransform = false;
    vectorTransformMethod = AsDirection;
    transformVectors = true;

    TransformAttributes::SelectAll();
}

// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Copy utility for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void TransformAttributes::Copy(const TransformAttributes &obj)
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
    continuousPhi = obj.continuousPhi;
    m00 = obj.m00;
    m01 = obj.m01;
    m02 = obj.m02;
    m03 = obj.m03;
    m10 = obj.m10;
    m11 = obj.m11;
    m12 = obj.m12;
    m13 = obj.m13;
    m20 = obj.m20;
    m21 = obj.m21;
    m22 = obj.m22;
    m23 = obj.m23;
    m30 = obj.m30;
    m31 = obj.m31;
    m32 = obj.m32;
    m33 = obj.m33;
    invertLinearTransform = obj.invertLinearTransform;
    vectorTransformMethod = obj.vectorTransformMethod;
    transformVectors = obj.transformVectors;

    TransformAttributes::SelectAll();
}

// Type map format string
const char *TransformAttributes::TypeMapFormatString = TRANSFORMATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t TransformAttributes::TmfsStruct = {TRANSFORMATTRIBUTES_TMFS};


// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Default constructor for the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::TransformAttributes() : 
    AttributeSubject(TransformAttributes::TypeMapFormatString)
{
    TransformAttributes::Init();
}

// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Constructor for the derived classes of TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::TransformAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    TransformAttributes::Init();
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::TransformAttributes(const TransformAttributes &obj) : 
    AttributeSubject(TransformAttributes::TypeMapFormatString)
{
    TransformAttributes::Copy(obj);
}

// ****************************************************************************
// Method: TransformAttributes::TransformAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the TransformAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes::TransformAttributes(const TransformAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    TransformAttributes::Copy(obj);
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
// Creation:   omitted
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransformAttributes& 
TransformAttributes::operator = (const TransformAttributes &obj)
{
    if (this == &obj) return *this;

    TransformAttributes::Copy(obj);

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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
TransformAttributes::operator == (const TransformAttributes &obj) const
{
    // Compare the rotateOrigin arrays.
    bool rotateOrigin_equal = true;
    for(int i = 0; i < 3 && rotateOrigin_equal; ++i)
        rotateOrigin_equal = (rotateOrigin[i] == obj.rotateOrigin[i]);

    // Compare the rotateAxis arrays.
    bool rotateAxis_equal = true;
    for(int i = 0; i < 3 && rotateAxis_equal; ++i)
        rotateAxis_equal = (rotateAxis[i] == obj.rotateAxis[i]);

    // Compare the scaleOrigin arrays.
    bool scaleOrigin_equal = true;
    for(int i = 0; i < 3 && scaleOrigin_equal; ++i)
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
            (outputCoordSys == obj.outputCoordSys) &&
            (continuousPhi == obj.continuousPhi) &&
            (m00 == obj.m00) &&
            (m01 == obj.m01) &&
            (m02 == obj.m02) &&
            (m03 == obj.m03) &&
            (m10 == obj.m10) &&
            (m11 == obj.m11) &&
            (m12 == obj.m12) &&
            (m13 == obj.m13) &&
            (m20 == obj.m20) &&
            (m21 == obj.m21) &&
            (m22 == obj.m22) &&
            (m23 == obj.m23) &&
            (m30 == obj.m30) &&
            (m31 == obj.m31) &&
            (m32 == obj.m32) &&
            (m33 == obj.m33) &&
            (invertLinearTransform == obj.invertLinearTransform) &&
            (vectorTransformMethod == obj.vectorTransformMethod) &&
            (transformVectors == obj.transformVectors));
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
// Creation:   omitted
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
// Creation:   omitted
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
// Creation:   omitted
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
TransformAttributes::SelectAll()
{
    Select(ID_doRotate,              (void *)&doRotate);
    Select(ID_rotateOrigin,          (void *)rotateOrigin, 3);
    Select(ID_rotateAxis,            (void *)rotateAxis, 3);
    Select(ID_rotateAmount,          (void *)&rotateAmount);
    Select(ID_rotateType,            (void *)&rotateType);
    Select(ID_doScale,               (void *)&doScale);
    Select(ID_scaleOrigin,           (void *)scaleOrigin, 3);
    Select(ID_scaleX,                (void *)&scaleX);
    Select(ID_scaleY,                (void *)&scaleY);
    Select(ID_scaleZ,                (void *)&scaleZ);
    Select(ID_doTranslate,           (void *)&doTranslate);
    Select(ID_translateX,            (void *)&translateX);
    Select(ID_translateY,            (void *)&translateY);
    Select(ID_translateZ,            (void *)&translateZ);
    Select(ID_transformType,         (void *)&transformType);
    Select(ID_inputCoordSys,         (void *)&inputCoordSys);
    Select(ID_outputCoordSys,        (void *)&outputCoordSys);
    Select(ID_continuousPhi,         (void *)&continuousPhi);
    Select(ID_m00,                   (void *)&m00);
    Select(ID_m01,                   (void *)&m01);
    Select(ID_m02,                   (void *)&m02);
    Select(ID_m03,                   (void *)&m03);
    Select(ID_m10,                   (void *)&m10);
    Select(ID_m11,                   (void *)&m11);
    Select(ID_m12,                   (void *)&m12);
    Select(ID_m13,                   (void *)&m13);
    Select(ID_m20,                   (void *)&m20);
    Select(ID_m21,                   (void *)&m21);
    Select(ID_m22,                   (void *)&m22);
    Select(ID_m23,                   (void *)&m23);
    Select(ID_m30,                   (void *)&m30);
    Select(ID_m31,                   (void *)&m31);
    Select(ID_m32,                   (void *)&m32);
    Select(ID_m33,                   (void *)&m33);
    Select(ID_invertLinearTransform, (void *)&invertLinearTransform);
    Select(ID_vectorTransformMethod, (void *)&vectorTransformMethod);
    Select(ID_transformVectors,      (void *)&transformVectors);
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
// Creation:   omitted
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

    if(completeSave || !FieldsEqual(ID_doRotate, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doRotate", doRotate));
    }

    if(completeSave || !FieldsEqual(ID_rotateOrigin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateOrigin", rotateOrigin, 3));
    }

    if(completeSave || !FieldsEqual(ID_rotateAxis, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateAxis", rotateAxis, 3));
    }

    if(completeSave || !FieldsEqual(ID_rotateAmount, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateAmount", rotateAmount));
    }

    if(completeSave || !FieldsEqual(ID_rotateType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rotateType", AngleType_ToString(rotateType)));
    }

    if(completeSave || !FieldsEqual(ID_doScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doScale", doScale));
    }

    if(completeSave || !FieldsEqual(ID_scaleOrigin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleOrigin", scaleOrigin, 3));
    }

    if(completeSave || !FieldsEqual(ID_scaleX, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleX", scaleX));
    }

    if(completeSave || !FieldsEqual(ID_scaleY, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleY", scaleY));
    }

    if(completeSave || !FieldsEqual(ID_scaleZ, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaleZ", scaleZ));
    }

    if(completeSave || !FieldsEqual(ID_doTranslate, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doTranslate", doTranslate));
    }

    if(completeSave || !FieldsEqual(ID_translateX, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("translateX", translateX));
    }

    if(completeSave || !FieldsEqual(ID_translateY, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("translateY", translateY));
    }

    if(completeSave || !FieldsEqual(ID_translateZ, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("translateZ", translateZ));
    }

    if(completeSave || !FieldsEqual(ID_transformType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("transformType", TransformType_ToString(transformType)));
    }

    if(completeSave || !FieldsEqual(ID_inputCoordSys, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("inputCoordSys", CoordinateSystem_ToString(inputCoordSys)));
    }

    if(completeSave || !FieldsEqual(ID_outputCoordSys, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("outputCoordSys", CoordinateSystem_ToString(outputCoordSys)));
    }

    if(completeSave || !FieldsEqual(ID_continuousPhi, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("continuousPhi", continuousPhi));
    }

    if(completeSave || !FieldsEqual(ID_m00, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m00", m00));
    }

    if(completeSave || !FieldsEqual(ID_m01, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m01", m01));
    }

    if(completeSave || !FieldsEqual(ID_m02, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m02", m02));
    }

    if(completeSave || !FieldsEqual(ID_m03, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m03", m03));
    }

    if(completeSave || !FieldsEqual(ID_m10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m10", m10));
    }

    if(completeSave || !FieldsEqual(ID_m11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m11", m11));
    }

    if(completeSave || !FieldsEqual(ID_m12, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m12", m12));
    }

    if(completeSave || !FieldsEqual(ID_m13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m13", m13));
    }

    if(completeSave || !FieldsEqual(ID_m20, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m20", m20));
    }

    if(completeSave || !FieldsEqual(ID_m21, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m21", m21));
    }

    if(completeSave || !FieldsEqual(ID_m22, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m22", m22));
    }

    if(completeSave || !FieldsEqual(ID_m23, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m23", m23));
    }

    if(completeSave || !FieldsEqual(ID_m30, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m30", m30));
    }

    if(completeSave || !FieldsEqual(ID_m31, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m31", m31));
    }

    if(completeSave || !FieldsEqual(ID_m32, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m32", m32));
    }

    if(completeSave || !FieldsEqual(ID_m33, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("m33", m33));
    }

    if(completeSave || !FieldsEqual(ID_invertLinearTransform, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("invertLinearTransform", invertLinearTransform));
    }

    if(completeSave || !FieldsEqual(ID_vectorTransformMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("vectorTransformMethod", VectorTransformMethod_ToString(vectorTransformMethod)));
    }

    if(completeSave || !FieldsEqual(ID_transformVectors, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("transformVectors", transformVectors));
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
TransformAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("TransformAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("doRotate")) != 0)
        SetDoRotate(node->AsBool());
    if((node = searchNode->GetNode("rotateOrigin")) != 0)
        SetRotateOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("rotateAxis")) != 0)
        SetRotateAxis(node->AsDoubleArray());
    if((node = searchNode->GetNode("rotateAmount")) != 0)
        SetRotateAmount(node->AsDouble());
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
        SetScaleOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("scaleX")) != 0)
        SetScaleX(node->AsDouble());
    if((node = searchNode->GetNode("scaleY")) != 0)
        SetScaleY(node->AsDouble());
    if((node = searchNode->GetNode("scaleZ")) != 0)
        SetScaleZ(node->AsDouble());
    if((node = searchNode->GetNode("doTranslate")) != 0)
        SetDoTranslate(node->AsBool());
    if((node = searchNode->GetNode("translateX")) != 0)
        SetTranslateX(node->AsDouble());
    if((node = searchNode->GetNode("translateY")) != 0)
        SetTranslateY(node->AsDouble());
    if((node = searchNode->GetNode("translateZ")) != 0)
        SetTranslateZ(node->AsDouble());
    if((node = searchNode->GetNode("transformType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
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
    if((node = searchNode->GetNode("continuousPhi")) != 0)
        SetContinuousPhi(node->AsBool());
    if((node = searchNode->GetNode("m00")) != 0)
        SetM00(node->AsDouble());
    if((node = searchNode->GetNode("m01")) != 0)
        SetM01(node->AsDouble());
    if((node = searchNode->GetNode("m02")) != 0)
        SetM02(node->AsDouble());
    if((node = searchNode->GetNode("m03")) != 0)
        SetM03(node->AsDouble());
    if((node = searchNode->GetNode("m10")) != 0)
        SetM10(node->AsDouble());
    if((node = searchNode->GetNode("m11")) != 0)
        SetM11(node->AsDouble());
    if((node = searchNode->GetNode("m12")) != 0)
        SetM12(node->AsDouble());
    if((node = searchNode->GetNode("m13")) != 0)
        SetM13(node->AsDouble());
    if((node = searchNode->GetNode("m20")) != 0)
        SetM20(node->AsDouble());
    if((node = searchNode->GetNode("m21")) != 0)
        SetM21(node->AsDouble());
    if((node = searchNode->GetNode("m22")) != 0)
        SetM22(node->AsDouble());
    if((node = searchNode->GetNode("m23")) != 0)
        SetM23(node->AsDouble());
    if((node = searchNode->GetNode("m30")) != 0)
        SetM30(node->AsDouble());
    if((node = searchNode->GetNode("m31")) != 0)
        SetM31(node->AsDouble());
    if((node = searchNode->GetNode("m32")) != 0)
        SetM32(node->AsDouble());
    if((node = searchNode->GetNode("m33")) != 0)
        SetM33(node->AsDouble());
    if((node = searchNode->GetNode("invertLinearTransform")) != 0)
        SetInvertLinearTransform(node->AsBool());
    if((node = searchNode->GetNode("vectorTransformMethod")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetVectorTransformMethod(VectorTransformMethod(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VectorTransformMethod value;
            if(VectorTransformMethod_FromString(node->AsString(), value))
                SetVectorTransformMethod(value);
        }
    }
    if((node = searchNode->GetNode("transformVectors")) != 0)
        SetTransformVectors(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
TransformAttributes::SetDoRotate(bool doRotate_)
{
    doRotate = doRotate_;
    Select(ID_doRotate, (void *)&doRotate);
}

void
TransformAttributes::SetRotateOrigin(const double *rotateOrigin_)
{
    rotateOrigin[0] = rotateOrigin_[0];
    rotateOrigin[1] = rotateOrigin_[1];
    rotateOrigin[2] = rotateOrigin_[2];
    Select(ID_rotateOrigin, (void *)rotateOrigin, 3);
}

void
TransformAttributes::SetRotateAxis(const double *rotateAxis_)
{
    rotateAxis[0] = rotateAxis_[0];
    rotateAxis[1] = rotateAxis_[1];
    rotateAxis[2] = rotateAxis_[2];
    Select(ID_rotateAxis, (void *)rotateAxis, 3);
}

void
TransformAttributes::SetRotateAmount(double rotateAmount_)
{
    rotateAmount = rotateAmount_;
    Select(ID_rotateAmount, (void *)&rotateAmount);
}

void
TransformAttributes::SetRotateType(TransformAttributes::AngleType rotateType_)
{
    rotateType = rotateType_;
    Select(ID_rotateType, (void *)&rotateType);
}

void
TransformAttributes::SetDoScale(bool doScale_)
{
    doScale = doScale_;
    Select(ID_doScale, (void *)&doScale);
}

void
TransformAttributes::SetScaleOrigin(const double *scaleOrigin_)
{
    scaleOrigin[0] = scaleOrigin_[0];
    scaleOrigin[1] = scaleOrigin_[1];
    scaleOrigin[2] = scaleOrigin_[2];
    Select(ID_scaleOrigin, (void *)scaleOrigin, 3);
}

void
TransformAttributes::SetScaleX(double scaleX_)
{
    scaleX = scaleX_;
    Select(ID_scaleX, (void *)&scaleX);
}

void
TransformAttributes::SetScaleY(double scaleY_)
{
    scaleY = scaleY_;
    Select(ID_scaleY, (void *)&scaleY);
}

void
TransformAttributes::SetScaleZ(double scaleZ_)
{
    scaleZ = scaleZ_;
    Select(ID_scaleZ, (void *)&scaleZ);
}

void
TransformAttributes::SetDoTranslate(bool doTranslate_)
{
    doTranslate = doTranslate_;
    Select(ID_doTranslate, (void *)&doTranslate);
}

void
TransformAttributes::SetTranslateX(double translateX_)
{
    translateX = translateX_;
    Select(ID_translateX, (void *)&translateX);
}

void
TransformAttributes::SetTranslateY(double translateY_)
{
    translateY = translateY_;
    Select(ID_translateY, (void *)&translateY);
}

void
TransformAttributes::SetTranslateZ(double translateZ_)
{
    translateZ = translateZ_;
    Select(ID_translateZ, (void *)&translateZ);
}

void
TransformAttributes::SetTransformType(TransformAttributes::TransformType transformType_)
{
    transformType = transformType_;
    Select(ID_transformType, (void *)&transformType);
}

void
TransformAttributes::SetInputCoordSys(TransformAttributes::CoordinateSystem inputCoordSys_)
{
    inputCoordSys = inputCoordSys_;
    Select(ID_inputCoordSys, (void *)&inputCoordSys);
}

void
TransformAttributes::SetOutputCoordSys(TransformAttributes::CoordinateSystem outputCoordSys_)
{
    outputCoordSys = outputCoordSys_;
    Select(ID_outputCoordSys, (void *)&outputCoordSys);
}

void
TransformAttributes::SetContinuousPhi(bool continuousPhi_)
{
    continuousPhi = continuousPhi_;
    Select(ID_continuousPhi, (void *)&continuousPhi);
}

void
TransformAttributes::SetM00(double m00_)
{
    m00 = m00_;
    Select(ID_m00, (void *)&m00);
}

void
TransformAttributes::SetM01(double m01_)
{
    m01 = m01_;
    Select(ID_m01, (void *)&m01);
}

void
TransformAttributes::SetM02(double m02_)
{
    m02 = m02_;
    Select(ID_m02, (void *)&m02);
}

void
TransformAttributes::SetM03(double m03_)
{
    m03 = m03_;
    Select(ID_m03, (void *)&m03);
}

void
TransformAttributes::SetM10(double m10_)
{
    m10 = m10_;
    Select(ID_m10, (void *)&m10);
}

void
TransformAttributes::SetM11(double m11_)
{
    m11 = m11_;
    Select(ID_m11, (void *)&m11);
}

void
TransformAttributes::SetM12(double m12_)
{
    m12 = m12_;
    Select(ID_m12, (void *)&m12);
}

void
TransformAttributes::SetM13(double m13_)
{
    m13 = m13_;
    Select(ID_m13, (void *)&m13);
}

void
TransformAttributes::SetM20(double m20_)
{
    m20 = m20_;
    Select(ID_m20, (void *)&m20);
}

void
TransformAttributes::SetM21(double m21_)
{
    m21 = m21_;
    Select(ID_m21, (void *)&m21);
}

void
TransformAttributes::SetM22(double m22_)
{
    m22 = m22_;
    Select(ID_m22, (void *)&m22);
}

void
TransformAttributes::SetM23(double m23_)
{
    m23 = m23_;
    Select(ID_m23, (void *)&m23);
}

void
TransformAttributes::SetM30(double m30_)
{
    m30 = m30_;
    Select(ID_m30, (void *)&m30);
}

void
TransformAttributes::SetM31(double m31_)
{
    m31 = m31_;
    Select(ID_m31, (void *)&m31);
}

void
TransformAttributes::SetM32(double m32_)
{
    m32 = m32_;
    Select(ID_m32, (void *)&m32);
}

void
TransformAttributes::SetM33(double m33_)
{
    m33 = m33_;
    Select(ID_m33, (void *)&m33);
}

void
TransformAttributes::SetInvertLinearTransform(bool invertLinearTransform_)
{
    invertLinearTransform = invertLinearTransform_;
    Select(ID_invertLinearTransform, (void *)&invertLinearTransform);
}

void
TransformAttributes::SetVectorTransformMethod(TransformAttributes::VectorTransformMethod vectorTransformMethod_)
{
    vectorTransformMethod = vectorTransformMethod_;
    Select(ID_vectorTransformMethod, (void *)&vectorTransformMethod);
}

void
TransformAttributes::SetTransformVectors(bool transformVectors_)
{
    transformVectors = transformVectors_;
    Select(ID_transformVectors, (void *)&transformVectors);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
TransformAttributes::GetDoRotate() const
{
    return doRotate;
}

const double *
TransformAttributes::GetRotateOrigin() const
{
    return rotateOrigin;
}

double *
TransformAttributes::GetRotateOrigin()
{
    return rotateOrigin;
}

const double *
TransformAttributes::GetRotateAxis() const
{
    return rotateAxis;
}

double *
TransformAttributes::GetRotateAxis()
{
    return rotateAxis;
}

double
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

const double *
TransformAttributes::GetScaleOrigin() const
{
    return scaleOrigin;
}

double *
TransformAttributes::GetScaleOrigin()
{
    return scaleOrigin;
}

double
TransformAttributes::GetScaleX() const
{
    return scaleX;
}

double
TransformAttributes::GetScaleY() const
{
    return scaleY;
}

double
TransformAttributes::GetScaleZ() const
{
    return scaleZ;
}

bool
TransformAttributes::GetDoTranslate() const
{
    return doTranslate;
}

double
TransformAttributes::GetTranslateX() const
{
    return translateX;
}

double
TransformAttributes::GetTranslateY() const
{
    return translateY;
}

double
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

bool
TransformAttributes::GetContinuousPhi() const
{
    return continuousPhi;
}

double
TransformAttributes::GetM00() const
{
    return m00;
}

double
TransformAttributes::GetM01() const
{
    return m01;
}

double
TransformAttributes::GetM02() const
{
    return m02;
}

double
TransformAttributes::GetM03() const
{
    return m03;
}

double
TransformAttributes::GetM10() const
{
    return m10;
}

double
TransformAttributes::GetM11() const
{
    return m11;
}

double
TransformAttributes::GetM12() const
{
    return m12;
}

double
TransformAttributes::GetM13() const
{
    return m13;
}

double
TransformAttributes::GetM20() const
{
    return m20;
}

double
TransformAttributes::GetM21() const
{
    return m21;
}

double
TransformAttributes::GetM22() const
{
    return m22;
}

double
TransformAttributes::GetM23() const
{
    return m23;
}

double
TransformAttributes::GetM30() const
{
    return m30;
}

double
TransformAttributes::GetM31() const
{
    return m31;
}

double
TransformAttributes::GetM32() const
{
    return m32;
}

double
TransformAttributes::GetM33() const
{
    return m33;
}

bool
TransformAttributes::GetInvertLinearTransform() const
{
    return invertLinearTransform;
}

TransformAttributes::VectorTransformMethod
TransformAttributes::GetVectorTransformMethod() const
{
    return VectorTransformMethod(vectorTransformMethod);
}

bool
TransformAttributes::GetTransformVectors() const
{
    return transformVectors;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
TransformAttributes::SelectRotateOrigin()
{
    Select(ID_rotateOrigin, (void *)rotateOrigin, 3);
}

void
TransformAttributes::SelectRotateAxis()
{
    Select(ID_rotateAxis, (void *)rotateAxis, 3);
}

void
TransformAttributes::SelectScaleOrigin()
{
    Select(ID_scaleOrigin, (void *)scaleOrigin, 3);
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
TransformAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_doRotate:              return "doRotate";
    case ID_rotateOrigin:          return "rotateOrigin";
    case ID_rotateAxis:            return "rotateAxis";
    case ID_rotateAmount:          return "rotateAmount";
    case ID_rotateType:            return "rotateType";
    case ID_doScale:               return "doScale";
    case ID_scaleOrigin:           return "scaleOrigin";
    case ID_scaleX:                return "scaleX";
    case ID_scaleY:                return "scaleY";
    case ID_scaleZ:                return "scaleZ";
    case ID_doTranslate:           return "doTranslate";
    case ID_translateX:            return "translateX";
    case ID_translateY:            return "translateY";
    case ID_translateZ:            return "translateZ";
    case ID_transformType:         return "transformType";
    case ID_inputCoordSys:         return "inputCoordSys";
    case ID_outputCoordSys:        return "outputCoordSys";
    case ID_continuousPhi:         return "continuousPhi";
    case ID_m00:                   return "m00";
    case ID_m01:                   return "m01";
    case ID_m02:                   return "m02";
    case ID_m03:                   return "m03";
    case ID_m10:                   return "m10";
    case ID_m11:                   return "m11";
    case ID_m12:                   return "m12";
    case ID_m13:                   return "m13";
    case ID_m20:                   return "m20";
    case ID_m21:                   return "m21";
    case ID_m22:                   return "m22";
    case ID_m23:                   return "m23";
    case ID_m30:                   return "m30";
    case ID_m31:                   return "m31";
    case ID_m32:                   return "m32";
    case ID_m33:                   return "m33";
    case ID_invertLinearTransform: return "invertLinearTransform";
    case ID_vectorTransformMethod: return "vectorTransformMethod";
    case ID_transformVectors:      return "transformVectors";
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
TransformAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_doRotate:              return FieldType_bool;
    case ID_rotateOrigin:          return FieldType_doubleArray;
    case ID_rotateAxis:            return FieldType_doubleArray;
    case ID_rotateAmount:          return FieldType_double;
    case ID_rotateType:            return FieldType_enum;
    case ID_doScale:               return FieldType_bool;
    case ID_scaleOrigin:           return FieldType_doubleArray;
    case ID_scaleX:                return FieldType_double;
    case ID_scaleY:                return FieldType_double;
    case ID_scaleZ:                return FieldType_double;
    case ID_doTranslate:           return FieldType_bool;
    case ID_translateX:            return FieldType_double;
    case ID_translateY:            return FieldType_double;
    case ID_translateZ:            return FieldType_double;
    case ID_transformType:         return FieldType_enum;
    case ID_inputCoordSys:         return FieldType_enum;
    case ID_outputCoordSys:        return FieldType_enum;
    case ID_continuousPhi:         return FieldType_bool;
    case ID_m00:                   return FieldType_double;
    case ID_m01:                   return FieldType_double;
    case ID_m02:                   return FieldType_double;
    case ID_m03:                   return FieldType_double;
    case ID_m10:                   return FieldType_double;
    case ID_m11:                   return FieldType_double;
    case ID_m12:                   return FieldType_double;
    case ID_m13:                   return FieldType_double;
    case ID_m20:                   return FieldType_double;
    case ID_m21:                   return FieldType_double;
    case ID_m22:                   return FieldType_double;
    case ID_m23:                   return FieldType_double;
    case ID_m30:                   return FieldType_double;
    case ID_m31:                   return FieldType_double;
    case ID_m32:                   return FieldType_double;
    case ID_m33:                   return FieldType_double;
    case ID_invertLinearTransform: return FieldType_bool;
    case ID_vectorTransformMethod: return FieldType_enum;
    case ID_transformVectors:      return FieldType_bool;
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
TransformAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_doRotate:              return "bool";
    case ID_rotateOrigin:          return "doubleArray";
    case ID_rotateAxis:            return "doubleArray";
    case ID_rotateAmount:          return "double";
    case ID_rotateType:            return "enum";
    case ID_doScale:               return "bool";
    case ID_scaleOrigin:           return "doubleArray";
    case ID_scaleX:                return "double";
    case ID_scaleY:                return "double";
    case ID_scaleZ:                return "double";
    case ID_doTranslate:           return "bool";
    case ID_translateX:            return "double";
    case ID_translateY:            return "double";
    case ID_translateZ:            return "double";
    case ID_transformType:         return "enum";
    case ID_inputCoordSys:         return "enum";
    case ID_outputCoordSys:        return "enum";
    case ID_continuousPhi:         return "bool";
    case ID_m00:                   return "double";
    case ID_m01:                   return "double";
    case ID_m02:                   return "double";
    case ID_m03:                   return "double";
    case ID_m10:                   return "double";
    case ID_m11:                   return "double";
    case ID_m12:                   return "double";
    case ID_m13:                   return "double";
    case ID_m20:                   return "double";
    case ID_m21:                   return "double";
    case ID_m22:                   return "double";
    case ID_m23:                   return "double";
    case ID_m30:                   return "double";
    case ID_m31:                   return "double";
    case ID_m32:                   return "double";
    case ID_m33:                   return "double";
    case ID_invertLinearTransform: return "bool";
    case ID_vectorTransformMethod: return "enum";
    case ID_transformVectors:      return "bool";
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
TransformAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const TransformAttributes &obj = *((const TransformAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_doRotate:
        {  // new scope
        retval = (doRotate == obj.doRotate);
        }
        break;
    case ID_rotateOrigin:
        {  // new scope
        // Compare the rotateOrigin arrays.
        bool rotateOrigin_equal = true;
        for(int i = 0; i < 3 && rotateOrigin_equal; ++i)
            rotateOrigin_equal = (rotateOrigin[i] == obj.rotateOrigin[i]);

        retval = rotateOrigin_equal;
        }
        break;
    case ID_rotateAxis:
        {  // new scope
        // Compare the rotateAxis arrays.
        bool rotateAxis_equal = true;
        for(int i = 0; i < 3 && rotateAxis_equal; ++i)
            rotateAxis_equal = (rotateAxis[i] == obj.rotateAxis[i]);

        retval = rotateAxis_equal;
        }
        break;
    case ID_rotateAmount:
        {  // new scope
        retval = (rotateAmount == obj.rotateAmount);
        }
        break;
    case ID_rotateType:
        {  // new scope
        retval = (rotateType == obj.rotateType);
        }
        break;
    case ID_doScale:
        {  // new scope
        retval = (doScale == obj.doScale);
        }
        break;
    case ID_scaleOrigin:
        {  // new scope
        // Compare the scaleOrigin arrays.
        bool scaleOrigin_equal = true;
        for(int i = 0; i < 3 && scaleOrigin_equal; ++i)
            scaleOrigin_equal = (scaleOrigin[i] == obj.scaleOrigin[i]);

        retval = scaleOrigin_equal;
        }
        break;
    case ID_scaleX:
        {  // new scope
        retval = (scaleX == obj.scaleX);
        }
        break;
    case ID_scaleY:
        {  // new scope
        retval = (scaleY == obj.scaleY);
        }
        break;
    case ID_scaleZ:
        {  // new scope
        retval = (scaleZ == obj.scaleZ);
        }
        break;
    case ID_doTranslate:
        {  // new scope
        retval = (doTranslate == obj.doTranslate);
        }
        break;
    case ID_translateX:
        {  // new scope
        retval = (translateX == obj.translateX);
        }
        break;
    case ID_translateY:
        {  // new scope
        retval = (translateY == obj.translateY);
        }
        break;
    case ID_translateZ:
        {  // new scope
        retval = (translateZ == obj.translateZ);
        }
        break;
    case ID_transformType:
        {  // new scope
        retval = (transformType == obj.transformType);
        }
        break;
    case ID_inputCoordSys:
        {  // new scope
        retval = (inputCoordSys == obj.inputCoordSys);
        }
        break;
    case ID_outputCoordSys:
        {  // new scope
        retval = (outputCoordSys == obj.outputCoordSys);
        }
        break;
    case ID_continuousPhi:
        {  // new scope
        retval = (continuousPhi == obj.continuousPhi);
        }
        break;
    case ID_m00:
        {  // new scope
        retval = (m00 == obj.m00);
        }
        break;
    case ID_m01:
        {  // new scope
        retval = (m01 == obj.m01);
        }
        break;
    case ID_m02:
        {  // new scope
        retval = (m02 == obj.m02);
        }
        break;
    case ID_m03:
        {  // new scope
        retval = (m03 == obj.m03);
        }
        break;
    case ID_m10:
        {  // new scope
        retval = (m10 == obj.m10);
        }
        break;
    case ID_m11:
        {  // new scope
        retval = (m11 == obj.m11);
        }
        break;
    case ID_m12:
        {  // new scope
        retval = (m12 == obj.m12);
        }
        break;
    case ID_m13:
        {  // new scope
        retval = (m13 == obj.m13);
        }
        break;
    case ID_m20:
        {  // new scope
        retval = (m20 == obj.m20);
        }
        break;
    case ID_m21:
        {  // new scope
        retval = (m21 == obj.m21);
        }
        break;
    case ID_m22:
        {  // new scope
        retval = (m22 == obj.m22);
        }
        break;
    case ID_m23:
        {  // new scope
        retval = (m23 == obj.m23);
        }
        break;
    case ID_m30:
        {  // new scope
        retval = (m30 == obj.m30);
        }
        break;
    case ID_m31:
        {  // new scope
        retval = (m31 == obj.m31);
        }
        break;
    case ID_m32:
        {  // new scope
        retval = (m32 == obj.m32);
        }
        break;
    case ID_m33:
        {  // new scope
        retval = (m33 == obj.m33);
        }
        break;
    case ID_invertLinearTransform:
        {  // new scope
        retval = (invertLinearTransform == obj.invertLinearTransform);
        }
        break;
    case ID_vectorTransformMethod:
        {  // new scope
        retval = (vectorTransformMethod == obj.vectorTransformMethod);
        }
        break;
    case ID_transformVectors:
        {  // new scope
        retval = (transformVectors == obj.transformVectors);
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
// Method: TransformAttributes::ProcessOldVersions
//
// Purpose:
//   This method handles some old fields by converting them to new fields.
//
// Programmer: Burlen Loring
// Creation:   Fri Jul 18 15:20:59 PDT 2014
//
// Modifications:
//
// ****************************************************************************
void
TransformAttributes::ProcessOldVersions(DataNode *parentNode,
    const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("TransformAttributes");
    if(searchNode == 0)
        return;

    // deal with the changes in r19090 where type for a number of attributes
    // was changed from float/floatArray to double/doubleArray.
    if(VersionLessThan(configVersion, "2.6.0"))
    {
        const char *floatArrayAtts[] = {
            "rotateOrigin",
            "rotateAxis",
            "scaleOrigin",
            };
        for (size_t i=0; i<3; ++i)
        {
            DataNode *floatArrayAtt = searchNode->GetNode(floatArrayAtts[i]);
            if (floatArrayAtt)
            {
                const float *f = floatArrayAtt->AsFloatArray();
                double d[3] = {f[0], f[1], f[2]};
                searchNode->RemoveNode(floatArrayAtts[i], true);
                searchNode->AddNode(new DataNode(floatArrayAtts[i], d, 3));
            }
        }
        const char *floatAtts[] = {
            "rotateAmount",
            "scaleX",
            "scaleY",
            "scaleZ",
            "translateX",
            "translateY",
            "translateZ"
            };
        for (size_t i=0; i<7; ++i)
        {
            DataNode *floatAtt = searchNode->GetNode(floatAtts[i]);
            if (floatAtt)
            {
                float f = floatAtt->AsFloat();
                searchNode->RemoveNode(floatAtts[i], true);
                searchNode->AddNode(new DataNode(floatAtts[i], double(f)));
            }
        }
    }
}

