#include <ExtentsAttributes.h>
#include <DataNode.h>

#include <math.h>


// ****************************************************************************
// Method: ExtentsAttributes::ExtentsAttributes
//
// Purpose: Constructor for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

ExtentsAttributes::ExtentsAttributes() : AttributeSubject("s*d*d*d*d*i*i*dddd")
{
    scalarNames.clear();
    scalarMinima.clear(); scalarMaxima.clear();
    minima.clear(); maxima.clear();
    minTimeOrdinals.clear(); maxTimeOrdinals.clear();

    for (int extentNum = 0; extentNum < EA_DEFAULT_NUMBER_OF_EXTENTS; extentNum++)
    {
        scalarNames.push_back("default");
        scalarMinima.push_back(-1e+37); scalarMaxima.push_back(1e+37);
        minima.push_back(0.0); maxima.push_back(1.0);
        minTimeOrdinals.push_back(0); maxTimeOrdinals.push_back(0);
    }

    leftSliderX    = EA_DEFAULT_LEFT_SLIDER_X;
    rightSliderX   = EA_DEFAULT_RIGHT_SLIDER_X;
    slidersBottomY = EA_DEFAULT_SLIDERS_BOTTOM_Y;
    slidersTopY    = EA_DEFAULT_SLIDERS_TOP_Y;
}


// ****************************************************************************
// Method: ExtentsAttributes::ExtentsAttributes
//
// Purpose: Copy constructor for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

ExtentsAttributes::ExtentsAttributes(const ExtentsAttributes &obj) :
    AttributeSubject("s*d*d*d*d*i*i*dddd")
{
    scalarNames = obj.scalarNames;

    scalarMinima = obj.scalarMinima;
    scalarMaxima = obj.scalarMaxima;

    minima = obj.minima;
    maxima = obj.maxima;
    
    minTimeOrdinals = obj.minTimeOrdinals;
    maxTimeOrdinals = obj.maxTimeOrdinals;

    leftSliderX    = obj.leftSliderX;
    rightSliderX   = obj.rightSliderX;
    slidersBottomY = obj.slidersBottomY;
    slidersTopY    = obj.slidersTopY;

    SelectAll();
}


// ****************************************************************************
// Method: ExtentsAttributes::~ExtentsAttributes
//
// Purpose: Destructor for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

ExtentsAttributes::~ExtentsAttributes()
{
    // nothing here
}


// ****************************************************************************
// Method: ExtentsAttributes::operator = 
//
// Purpose: Assignment operator for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

void ExtentsAttributes::operator = (const ExtentsAttributes &obj)
{
    scalarNames = obj.scalarNames;

    scalarMinima = obj.scalarMinima;
    scalarMaxima = obj.scalarMaxima;

    minima = obj.minima;
    maxima = obj.maxima;

    minTimeOrdinals = obj.minTimeOrdinals;
    maxTimeOrdinals = obj.maxTimeOrdinals;

    leftSliderX    = obj.leftSliderX;
    rightSliderX   = obj.rightSliderX;
    slidersBottomY = obj.slidersBottomY;
    slidersTopY    = obj.slidersTopY;

    SelectAll();
}


// ****************************************************************************
// Method: ExtentsAttributes::operator == 
//
// Purpose: Comparison operator == for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

bool ExtentsAttributes::operator == (const ExtentsAttributes &obj) const
{
    int extentCount, extentNum;
    
    if ((extentCount = scalarNames.size()) != obj.scalarNames.size()) return false;
    if (obj.scalarMinima.size()    != extentCount) return false; // Shouldn't happen
    if (obj.scalarMaxima.size()    != extentCount) return false; // Shouldn't happen
    if (obj.minima.size()          != extentCount) return false; // Shouldn't happen
    if (obj.maxima.size()          != extentCount) return false; // Shouldn't happen
    if (obj.minTimeOrdinals.size() != extentCount) return false; // Shouldn't happen
    if (obj.maxTimeOrdinals.size() != extentCount) return false; // Shouldn't happen
    
    if (obj.minTimeOrdinals != minTimeOrdinals) return false;
    if (obj.maxTimeOrdinals != maxTimeOrdinals) return false;

    double coordEpsilon = (slidersTopY - slidersBottomY) * 0.000001;

    if (fabs(leftSliderX    - obj.leftSliderX   ) > coordEpsilon) return false;
    if (fabs(rightSliderX   - obj.rightSliderX  ) > coordEpsilon) return false;
    if (fabs(slidersBottomY - obj.slidersBottomY) > coordEpsilon) return false;
    if (fabs(slidersTopY    - obj.slidersTopY   ) > coordEpsilon) return false;

    if (extentCount == 0) return true;

    double scalarEpsilon;

    for (extentNum = 0; extentNum < extentCount; extentNum++)
    {
        if (obj.scalarNames[extentNum] != scalarNames[extentNum]) return false;
        
        scalarEpsilon = (scalarMaxima[extentNum] - scalarMinima[extentNum]) * 0.000001;

        if (fabs(scalarMinima[extentNum]-obj.scalarMinima[extentNum]) > scalarEpsilon)
            return false;
        if (fabs(scalarMaxima[extentNum]-obj.scalarMaxima[extentNum]) > scalarEpsilon)
            return false;

        if (fabs(minima[extentNum] - obj.minima[extentNum]) > 0.000001)
            return false;
        if (fabs(maxima[extentNum] - obj.maxima[extentNum]) > 0.000001)
            return false;
    }

    return true;
}


// ****************************************************************************
// Method: ExtentsAttributes::operator != 
//
// Purpose: Comparison operator != for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool ExtentsAttributes::operator != (const ExtentsAttributes &obj) const
{
    return !(this->operator == (obj));
}


// ****************************************************************************
// Method: ExtentsAttributes::TypeName
//
// Purpose: Type name method for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

const std::string ExtentsAttributes::TypeName() const
{
    return "ExtentsAttributes";
}


// ****************************************************************************
// Method: ExtentsAttributes::CopyAttributes
//
// Purpose: CopyAttributes method for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool ExtentsAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if (TypeName() != atts->TypeName()) return false;

    // Call assignment operator.
    const ExtentsAttributes *tmp = (const ExtentsAttributes *)atts;
    *this = *tmp;

    return true;
}


// ****************************************************************************
// Method: ExtentsAttributes::CreateCompatible
//
// Purpose: CreateCompatible method for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *ExtentsAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = NULL;

    if (TypeName() == tname) retval = new ExtentsAttributes(*this);

    // Other cases could go here too. 

    return retval;
}


// ****************************************************************************
// Method: ExtentsAttributes::NewInstance
//
// Purpose: NewInstance method for the ExtentsAttributes class.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *ExtentsAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = NULL;

    if (copy)
        retval = new ExtentsAttributes(*this);
    else
        retval = new ExtentsAttributes;

    return retval;
}


// ****************************************************************************
// Method: ExtentsAttributes::SelectAll
//
// Purpose: Selects all attributes.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

void ExtentsAttributes::SelectAll()
{
    Select( 0, (void *)&scalarNames);
    Select( 1, (void *)&scalarMinima);
    Select( 2, (void *)&scalarMaxima);
    Select( 3, (void *)&minima);
    Select( 4, (void *)&maxima);
    Select( 5, (void *)&minTimeOrdinals);
    Select( 6, (void *)&maxTimeOrdinals);
    Select( 7, (void *)&leftSliderX);
    Select( 8, (void *)&rightSliderX);
    Select( 9, (void *)&slidersBottomY);
    Select(10, (void *)&slidersTopY);
}


///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: ExtentsAttributes::CreateNode
//
// Purpose: This method creates a DataNode representation of the object so it
//          can be saved to a config file.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

bool ExtentsAttributes::CreateNode(
    DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if (parentNode == NULL) return false;

    ExtentsAttributes defaultObject;
    bool addToParent = false;

    // Create a node for ExtentsAttributes.

    DataNode *node = new DataNode("ExtentsAttributes");

    if (completeSave || !FieldsEqual(0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalarNames", scalarNames));
    }

    if (completeSave || !FieldsEqual(1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalarMinima", scalarMinima));
    }

    if (completeSave || !FieldsEqual(2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalarMaxima", scalarMaxima));
    }

    if (completeSave || !FieldsEqual(3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minima", minima));
    }

    if (completeSave || !FieldsEqual(4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxima", maxima));
    }

    if (completeSave || !FieldsEqual(5, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minTimeOrdinals", minTimeOrdinals));
    }

    if (completeSave || !FieldsEqual(6, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxTimeOrdinals", maxTimeOrdinals));
    }

    if (completeSave || !FieldsEqual(7, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("leftSliderX", leftSliderX));
    }

    if (completeSave || !FieldsEqual(8, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rightSliderX", rightSliderX));
    }

    if (completeSave || !FieldsEqual(9, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("slidersBottomY", slidersBottomY));
    }

    if (completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("slidersTopY", slidersTopY));
    }


    // Add the node to the parent node.

    if (addToParent || forceAdd) parentNode->AddNode(node);
    else delete node;

    return (addToParent || forceAdd);
}


// ****************************************************************************
// Method: ExtentsAttributes::SetFromNode
//
// Purpose: This method sets attributes in this object from values in a
//          DataNode representation of the object.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

void ExtentsAttributes::SetFromNode(DataNode *parentNode)
{
    if (parentNode == NULL) return;

    DataNode *searchNode = parentNode->GetNode("ExtentsAttributes");
    if (searchNode == NULL) return;

    DataNode *node;

    if ((node = searchNode->GetNode("scalarNames")) != NULL)
        SetScalarNames(node->AsStringVector());

    if ((node = searchNode->GetNode("scalarMinima")) != NULL)
        SetScalarMinima(node->AsDoubleVector());

    if ((node = searchNode->GetNode("scalarMaxima")) != NULL)
        SetScalarMaxima(node->AsDoubleVector());

    if ((node = searchNode->GetNode("minima")) != NULL)
        SetMinima(node->AsDoubleVector());

    if ((node = searchNode->GetNode("maxima")) != NULL)
        SetMaxima(node->AsDoubleVector());

    if ((node = searchNode->GetNode("minTimeOrdinals")) != NULL)
        SetMinTimeOrdinals(node->AsIntVector());

    if ((node = searchNode->GetNode("maxTimeOrdinals")) != NULL)
        SetMaxTimeOrdinals(node->AsIntVector());

    if ((node = searchNode->GetNode("leftSliderX")) != NULL)
        SetLeftSliderX(node->AsDouble());

    if ((node = searchNode->GetNode("rightSliderX")) != NULL)
        SetRightSliderX(node->AsDouble());

    if ((node = searchNode->GetNode("slidersBottomY")) != NULL)
        SetSlidersBottomY(node->AsDouble());

    if ((node = searchNode->GetNode("slidersTopY")) != NULL)
        SetSlidersTopY(node->AsDouble());
}


///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////


void ExtentsAttributes::SetScalarNames(const stringVector &scalarNames_)
{
    scalarNames = scalarNames_;
    Select(0, (void *)&scalarNames);
}


void ExtentsAttributes::SetScalarMinima(const doubleVector &scalarMinima_)
{
    scalarMinima = scalarMinima_;
    Select(1, (void *)&scalarMinima);
}


void ExtentsAttributes::SetScalarMaxima(const doubleVector &scalarMaxima_)
{
    scalarMaxima = scalarMaxima_;
    Select(2, (void *)&scalarMaxima);
}


void ExtentsAttributes::SetMinima(const doubleVector &minima_)
{
    minima = minima_;
    Select(3, (void *)&minima);
}


void ExtentsAttributes::SetMaxima(const doubleVector &maxima_)
{
    maxima = maxima_;
    Select(4, (void *)&maxima);
}


void ExtentsAttributes::SetMinTimeOrdinals(const intVector &minTimeOrdinals_)
{
    minTimeOrdinals = minTimeOrdinals_;
    Select(5, (void *)&minTimeOrdinals);
}


void ExtentsAttributes::SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_)
{
    maxTimeOrdinals = maxTimeOrdinals_;
    Select(6, (void *)&maxTimeOrdinals);
}


void ExtentsAttributes::SetLeftSliderX(double leftSliderX_)
{
    leftSliderX = leftSliderX_;
    Select(7, (void *)&leftSliderX);
}


void ExtentsAttributes::SetRightSliderX(double rightSliderX_)
{
    rightSliderX = rightSliderX_;
    Select(8, (void *)&rightSliderX);
}


void ExtentsAttributes::SetSlidersBottomY(double slidersBottomY_)
{
    slidersBottomY = slidersBottomY_;
    Select(9, (void *)&slidersBottomY);
}


void ExtentsAttributes::SetSlidersTopY(double slidersTopY_)
{
    slidersTopY = slidersTopY_;
    Select(10, (void *)&slidersTopY);
}


///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////


const stringVector &ExtentsAttributes::GetScalarNames() const
{
    return scalarNames;
}


const doubleVector &ExtentsAttributes::GetScalarMinima() const
{
    return scalarMinima;
}


const doubleVector &ExtentsAttributes::GetScalarMaxima() const
{
    return scalarMaxima;
}


const doubleVector &ExtentsAttributes::GetMinima() const
{
    return minima;
}


const doubleVector &ExtentsAttributes::GetMaxima() const
{
    return maxima;
}


const intVector &ExtentsAttributes::GetMinTimeOrdinals() const
{
    return minTimeOrdinals;
}


const intVector &ExtentsAttributes::GetMaxTimeOrdinals() const
{
    return maxTimeOrdinals;
}


double ExtentsAttributes::GetLeftSliderX() const
{
    return leftSliderX;
}


double ExtentsAttributes::GetRightSliderX() const
{
    return rightSliderX;
}


double ExtentsAttributes::GetSlidersBottomY() const
{
    return slidersBottomY;
}


double ExtentsAttributes::GetSlidersTopY() const
{
    return slidersTopY;
}


int ExtentsAttributes::GetNumberOfExtents() const
{
    return scalarNames.size();
}


///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////


void ExtentsAttributes::SelectScalarNames()
{
    Select (0, (void *)&scalarNames);
}


void ExtentsAttributes::SelectScalarMinima()
{
    Select (1, (void *)&scalarMinima);
}


void ExtentsAttributes::SelectScalarMaxima()
{
    Select (2, (void *)&scalarMaxima);
}


void ExtentsAttributes::SelectMinima()
{
    Select (3, (void *)&minima);
}


void ExtentsAttributes::SelectMaxima()
{
    Select (4, (void *)&maxima);
}


void ExtentsAttributes::SelectMinTimeOrdinals()
{
    Select (5, (void *)&minTimeOrdinals);
}


void ExtentsAttributes::SelectMaxTimeOrdinals()
{
    Select (6, (void *)&maxTimeOrdinals);
}


void ExtentsAttributes::SelectLeftSliderX()
{
    Select (7, (void *)&leftSliderX);
}


void ExtentsAttributes::SelectRightSliderX()
{
    Select (8, (void *)&rightSliderX);
}


void ExtentsAttributes::SelectSlidersBottomY()
{
    Select (9, (void *)&slidersBottomY);
}


void ExtentsAttributes::SelectSlidersTopY()
{
    Select (10, (void *)&slidersTopY);
}


///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: ExtentsAttributes::GetFieldName
//
// Purpose: This method returns the name of a field given its index.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

std::string ExtentsAttributes::GetFieldName(int index) const
{
    switch (index) {
        case  0:  return "scalarNames";
        case  1:  return "scalarMinima";
        case  2:  return "scalarMaxima";
        case  3:  return "minima";
        case  4:  return "maxima";
        case  5:  return "minTimeOrdinals";
        case  6:  return "maxTimeOrdinals";
        case  7:  return "leftSliderX";
        case  8:  return "rightSliderX";
        case  9:  return "slidersBottomY";
        case 10:  return "slidersTopY";
        default:  return "invalid index";
    }
}


// ****************************************************************************
// Method: ExtentsAttributes::GetFieldType
//
// Purpose: This method returns the type of a field given its index.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

AttributeGroup::FieldType ExtentsAttributes::GetFieldType(int index) const
{
    switch (index) {
        case  0:  return FieldType_stringVector;
        case  1:  return FieldType_doubleVector;
        case  2:  return FieldType_doubleVector;
        case  3:  return FieldType_doubleVector;
        case  4:  return FieldType_doubleVector;
        case  5:  return FieldType_intVector;
        case  6:  return FieldType_intVector;
        case  7:  return FieldType_double;
        case  8:  return FieldType_double;
        case  9:  return FieldType_double;
        case 10:  return FieldType_double;
        default:  return FieldType_unknown;
    }
}


// ****************************************************************************
// Method: ExtentsAttributes::GetFieldTypeName
//
// Purpose: This method returns the name of a field type given its index.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

std::string ExtentsAttributes::GetFieldTypeName(int index) const
{
    switch (index) {
        case  0:  return "stringVector";
        case  1:  return "doubleVector";
        case  2:  return "doubleVector";
        case  3:  return "doubleVector";
        case  4:  return "doubleVector";
        case  5:  return "intVector";
        case  6:  return "intVector";
        case  7:  return "double";
        case  8:  return "double";
        case  9:  return "double";
        case 10:  return "double";
        default:  return "invalid index";
    }
}


// ****************************************************************************
// Method: ExtentsAttributes::FieldsEqual
//
// Purpose: This method compares two fields and return true if they are equal.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added slider change time ordinal lists.
//   
// ****************************************************************************

bool ExtentsAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ExtentsAttributes &obj = *((const ExtentsAttributes*)rhs);

    int extentNum;
    int extentCount = scalarNames.size();
    double coordEpsilon = (slidersTopY - slidersBottomY) * 0.000001;
    double scalarEpsilon;

    switch (index_) {
        case 0:
            if (obj.scalarNames != scalarNames) return false;
            break;

        case 1:
            if (obj.scalarMinima.size() != extentCount) return false;

            for (extentNum = 0; extentNum < extentCount; extentNum++)
            {
                scalarEpsilon =
                    (scalarMaxima[extentNum] - scalarMinima[extentNum]) * 0.000001;

                if (fabs(scalarMinima[extentNum]-obj.scalarMinima[extentNum]) > scalarEpsilon)
                    return false;
            }

            break;

        case 2:
            if (obj.scalarMaxima.size() != extentCount) return false;

            for (extentNum = 0; extentNum < extentCount; extentNum++)
            {
                scalarEpsilon =
                    (scalarMaxima[extentNum] - scalarMinima[extentNum]) * 0.000001;

                if (fabs(scalarMaxima[extentNum]-obj.scalarMaxima[extentNum]) > scalarEpsilon)
                    return false;
            }

            break;

        case 3:
            if (obj.minima.size() != extentCount) return false;

            for (extentNum = 0; extentNum < extentCount; extentNum++)
            {
                if (fabs(minima[extentNum] - obj.minima[extentNum]) > 0.000001)
                    return false;
            }

            break;

        case 4:
            if (obj.maxima.size() != extentCount) return false;

            for (extentNum = 0; extentNum < extentCount; extentNum++)
            {
                if (fabs(maxima[extentNum] - obj.maxima[extentNum]) > 0.000001)
                    return false;
            }

            break;

        case 5:
            if (obj.minTimeOrdinals != minTimeOrdinals) return false;
            break;

        case 6:
            if (obj.maxTimeOrdinals != maxTimeOrdinals) return false;
            break;

        case 7:
            if (fabs(leftSliderX - obj.leftSliderX) > coordEpsilon) return false;
            break;

        case 8:
            if (fabs(rightSliderX - obj.rightSliderX) > coordEpsilon) return false;
            break;

        case 9:
            if (fabs(slidersBottomY - obj.slidersBottomY) > coordEpsilon) return false;
            break;

        case 10:
            if (fabs(slidersTopY - obj.slidersTopY) > coordEpsilon) return false;
            break;

        default:
            return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////
