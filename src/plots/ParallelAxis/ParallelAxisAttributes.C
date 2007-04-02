#include <ParallelAxisAttributes.h>
#include <DataNode.h>
/* Temporarily remove all references to Extents tool until tool is added.
#include <ExtentsAttributes.h>
*/


// ****************************************************************************
// Method: ParallelAxisAttributes::ParallelAxisAttributes
//
// Purpose: Constructor for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ParallelAxisAttributes::ParallelAxisAttributes() :
    AttributeSubject("s*id*d*d*d*")
{
    orderedAxisNames.push_back(std::string("default"));
    shownVarAxisPosition = 0;
    axisMinima.push_back(-1e+37);
    axisMaxima.push_back(+1e+37);
    extentMinima.push_back(0.0);
    extentMaxima.push_back(1.0);
}


// ****************************************************************************
// Method: ParallelAxisAttributes::ParallelAxisAttributes
//
// Purpose: Copy constructor for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ParallelAxisAttributes::ParallelAxisAttributes(
    const ParallelAxisAttributes &obj) : AttributeSubject("s*id*d*d*d*")
{
    orderedAxisNames = obj.orderedAxisNames;
    shownVarAxisPosition = obj.shownVarAxisPosition;
    axisMinima = obj.axisMinima;
    axisMaxima = obj.axisMaxima;
    extentMinima = obj.extentMinima;
    extentMaxima = obj.extentMaxima;

    SelectAll();
}


// ****************************************************************************
// Method: ParallelAxisAttributes::~ParallelAxisAttributes
//
// Purpose: Destructor for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ParallelAxisAttributes::~ParallelAxisAttributes()
{
    // nothing here
}


// ****************************************************************************
// Method: ParallelAxisAttributes::operator = 
//
// Purpose: Assignment operator for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ParallelAxisAttributes& 
ParallelAxisAttributes::operator = (const ParallelAxisAttributes &obj)
{
    if (this == &obj) return *this;

    orderedAxisNames = obj.orderedAxisNames;
    shownVarAxisPosition = obj.shownVarAxisPosition;
    axisMinima = obj.axisMinima;
    axisMaxima = obj.axisMaxima;
    extentMinima = obj.extentMinima;
    extentMaxima = obj.extentMaxima;

    SelectAll();

    return *this;
}


// ****************************************************************************
// Method: ParallelAxisAttributes::operator == 
//
// Purpose: Comparison operator == for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ParallelAxisAttributes::operator == (const ParallelAxisAttributes &obj) const
{
    // Create the return value

    return ((orderedAxisNames == obj.orderedAxisNames) &&
            (shownVarAxisPosition == obj.shownVarAxisPosition) &&
            (axisMinima == obj.axisMinima) &&
            (axisMaxima == obj.axisMaxima) &&
            (extentMinima == obj.extentMinima) &&
            (extentMaxima == obj.extentMaxima));
}


// ****************************************************************************
// Method: ParallelAxisAttributes::operator != 
//
// Purpose: Comparison operator != for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ParallelAxisAttributes::operator != (const ParallelAxisAttributes &obj) const
{
    return !(this->operator == (obj));
}


// ****************************************************************************
// Method: ParallelAxisAttributes::TypeName
//
// Purpose: Type name method for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

const std::string
ParallelAxisAttributes::TypeName() const
{
    return "ParallelAxisAttributes";
}


// ****************************************************************************
// Method: ParallelAxisAttributes::CopyAttributes
//
// Purpose: CopyAttributes method for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
// ****************************************************************************

bool
ParallelAxisAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if (TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const ParallelAxisAttributes *tmp =
            (const ParallelAxisAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
/* Temporarily remove all references to Extents tool until tool is added.
    else if (atts->TypeName() == "ExtentsAttributes")
    {
        const ExtentsAttributes *extAtts = (const ExtentsAttributes *)atts;

        stringVector copiedScalarNames  = extAtts->GetScalarNames();
        doubleVector copiedScalarMinima = extAtts->GetScalarMinima();
        doubleVector copiedScalarMaxima = extAtts->GetScalarMaxima();
        doubleVector copiedMinima       = extAtts->GetMinima();
        doubleVector copiedMaxima       = extAtts->GetMaxima();

        int scalarNum, axisNum;
        double axisMargin;
        std::string scalarName;

        for (scalarNum = 0; scalarNum < copiedScalarNames.size(); scalarNum++)
        {
            scalarName = copiedScalarNames[scalarNum];

            for (axisNum = 0; axisNum < orderedAxisNames.size(); axisNum++ )
            {
                if (orderedAxisNames[axisNum] == scalarName)
                {
                    axisMargin = (copiedScalarMaxima[scalarNum] -
                                  copiedScalarMinima[scalarNum]) / 22.0;

                    axisMinima[axisNum] =
                        copiedScalarMinima[scalarNum] + axisMargin;
                    axisMaxima[axisNum] =
                        copiedScalarMaxima[scalarNum] - axisMargin;

                    extentMinima[axisNum] = copiedMinima[scalarNum];
                    extentMaxima[axisNum] = copiedMaxima[scalarNum];

                    break;
                }
            }
        }

        retval = true;
    }
*/

    return retval;
}


// ****************************************************************************
// Method: ParallelAxisAttributes::CreateCompatible
//
// Purpose: Creates an object of the specified type initialized with the
//          attributes from this object.
//
// Arguments:
//   tname : The typename of the object that we want to create.
//
// Returns: A new object of the type specified by tname or NULL.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
// ****************************************************************************

AttributeSubject *
ParallelAxisAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = NULL;

    if (TypeName() == tname)
    {
        retval = new ParallelAxisAttributes(*this);
    }
/* Temporarily remove all references to Extents tool until tool is added.
    else if (tname == "ExtentsAttributes")
    {
        int axisNum;
        stringVector curScalarNames = orderedAxisNames;
        doubleVector curScalarMinima;
        doubleVector curScalarMaxima;
        doubleVector curExtentMinima = extentMinima;
        doubleVector curExtentMaxima = extentMaxima;
        double trueAxisMin, trueAxisMax, axisMargin;

        ParallelAxisAttributes highDimAtts = *this;

        highDimAtts.UpdateAxisBoundsIfPossible();

        for (axisNum = 0; axisNum < highDimAtts.orderedAxisNames.size(); axisNum++)
        {
            if ((trueAxisMin = highDimAtts.axisMinima[axisNum]) < -9e+36)
            {
                curScalarMinima.push_back(-1e+37);
                curScalarMaxima.push_back(+1e+37);
            }
            else
            {
                trueAxisMax = highDimAtts.axisMaxima[axisNum];
                axisMargin = (trueAxisMax - trueAxisMin) * 0.05;

                curScalarMinima.push_back(trueAxisMin - axisMargin);
                curScalarMaxima.push_back(trueAxisMax + axisMargin);
            }
        }

        ExtentsAttributes *extAtts = new ExtentsAttributes;

        extAtts->SetScalarNames(curScalarNames);

        extAtts->SetScalarMinima(curScalarMinima);
        extAtts->SetScalarMaxima(curScalarMaxima);

        extAtts->SetMinima(curExtentMinima);
        extAtts->SetMaxima(curExtentMaxima);

        if (curScalarNames.size() > PCP_MAX_HORIZONTAL_TITLE_AXES)
        {
            extAtts->SetSlidersBottomY(PCP_V_BOTTOM_AXIS_Y_FRACTION);
            extAtts->SetSlidersTopY(PCP_V_TOP_AXIS_Y_FRACTION);
        }
        else
        {
            extAtts->SetSlidersBottomY(PCP_H_BOTTOM_AXIS_Y_FRACTION);
            extAtts->SetSlidersTopY(PCP_H_TOP_AXIS_Y_FRACTION);
        }

        extAtts->SetLeftSliderX(PCP_LEFT_AXIS_X_FRACTION);
        extAtts->SetRightSliderX(PCP_RIGHT_AXIS_X_FRACTION);

        retval = extAtts;
    }
*/

    return retval;
}


// ****************************************************************************
// Method: ParallelAxisAttributes::NewInstance
//
// Purpose: NewInstance method for the ParallelAxisAttributes class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ParallelAxisAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = NULL;

    if (copy)
        retval = new ParallelAxisAttributes(*this);
    else
        retval = new ParallelAxisAttributes;

    return retval;
}


///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: ParallelAxisAttributes::CreateNode
//
// Purpose: This method creates a DataNode representation of the object so it
//          can be saved to a config file.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
// ****************************************************************************

bool
ParallelAxisAttributes::CreateNode(
    DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if (parentNode == NULL) return false;

    ParallelAxisAttributes defaultObject;
    bool addToParent = false;

    // Create a node for ParallelAxisAttributes.
    DataNode *node = new DataNode("ParallelAxisAttributes");

    if (completeSave || !FieldsEqual(0, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("orderedAxisNames", orderedAxisNames));
    }

    if (completeSave || !FieldsEqual(1, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("shownVarAxisPosition", shownVarAxisPosition));
    }

    if (completeSave || !FieldsEqual(2, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("axisMinima", axisMinima));
    }

    if (completeSave || !FieldsEqual(3, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("axisMaxima", axisMaxima));
    }

    if (completeSave || !FieldsEqual(4, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("extentMinima", extentMinima));
    }

    if (completeSave || !FieldsEqual(5, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("extentMaxima", extentMaxima));
    }

    // Add the node to the parent node.
    if (addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}


// ****************************************************************************
// Method: ParallelAxisAttributes::SetFromNode
//
// Purpose: This method sets attributes in this object from values in a DataNode
//          representation of the object.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ParallelAxisAttributes::SetFromNode(DataNode *parentNode)
{
    if (parentNode == NULL) return;

    DataNode *searchNode = parentNode->GetNode("ParallelAxisAttributes");
    if (searchNode == NULL) return;

    DataNode *node;

    if ((node = searchNode->GetNode("orderedAxisNames")) != 0) {
        SetOrderedAxisNames(node->AsStringVector());
    }

    if ((node = searchNode->GetNode("shownVarAxisPosition")) != 0) {
        SetShownVariableAxisPosition(node->AsInt());
    }

    if ((node = searchNode->GetNode("axisMinima")) != 0) {
        SetAxisMinima(node->AsDoubleVector());
    }

    if ((node = searchNode->GetNode("axisMaxima")) != 0) {
        SetAxisMaxima(node->AsDoubleVector());
    }

    if ((node = searchNode->GetNode("extentMinima")) != 0) {
        SetExtentMinima(node->AsDoubleVector());
    }

    if ((node = searchNode->GetNode("extentMaxima")) != 0) {
        SetExtentMaxima(node->AsDoubleVector());
    }
}


///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////


void
ParallelAxisAttributes::SetOrderedAxisNames(const stringVector &orderedAxisNames_)
{
    orderedAxisNames = orderedAxisNames_;
    Select(0, (void *)&orderedAxisNames);
}


void
ParallelAxisAttributes::SetShownVariableAxisPosition(int shownVarAxisPosition_)
{
    shownVarAxisPosition = shownVarAxisPosition_;
    Select(1, (void *)&shownVarAxisPosition);
}


void
ParallelAxisAttributes::SetAxisMinima(const doubleVector &axisMinima_)
{
    axisMinima = axisMinima_;
    Select(2, (void *)&axisMinima);
}


void
ParallelAxisAttributes::SetAxisMaxima(const doubleVector &axisMaxima_)
{
    axisMaxima = axisMaxima_;
    Select(3, (void *)&axisMaxima);
}


void
ParallelAxisAttributes::SetExtentMinima(const doubleVector &extentMinima_)
{
    extentMinima = extentMinima_;
    Select(4, (void *)&extentMinima);
}


void
ParallelAxisAttributes::SetExtentMaxima(const doubleVector &extentMaxima_)
{
    extentMaxima = extentMaxima_;
    Select(5, (void *)&extentMaxima);
}


////////////////////////////////////////////////////////////////////////////////
// Methods to change a given attribute.  When an attribute is "changed", its
// value is not only set to the new value, but the values of one or more other
// attributes may be changed as well.
//
// DO NOT USE THESE METHODS TO SET ATTRIBUTES FROM A SERIALIZED DATA STRUCTURE.
//
////////////////////////////////////////////////////////////////////////////////


void
ParallelAxisAttributes::InsertAxis(const std::string &axisName_)
{
    std::string newAxisName = axisName_;

    int curAxisCount = orderedAxisNames.size();
    int axisPosition, insertPosition;
    double saveAxisMin, saveAxisMax, saveExtentMin, saveExtentMax;

    stringVector::iterator axisNamesIt;
    doubleVector::iterator axisMinIt;
    doubleVector::iterator axisMaxIt;
    doubleVector::iterator extentMinIt;
    doubleVector::iterator extentMaxIt;

    for (axisPosition = 0; axisPosition < curAxisCount; axisPosition++)
    {
        if (orderedAxisNames[axisPosition] == newAxisName) break;
    }

    if (axisPosition < curAxisCount)
    {
        if (axisPosition == shownVarAxisPosition) return;

        saveAxisMin   = axisMinima[axisPosition];
        saveAxisMax   = axisMaxima[axisPosition];
        saveExtentMin = extentMinima[axisPosition];
        saveExtentMax = extentMaxima[axisPosition];

        axisNamesIt = orderedAxisNames.begin() + axisPosition;
        axisMinIt   = axisMinima.begin()       + axisPosition;
        axisMaxIt   = axisMaxima.begin()       + axisPosition;
        extentMinIt = extentMinima.begin()     + axisPosition;
        extentMaxIt = extentMaxima.begin()     + axisPosition;

        orderedAxisNames.erase(axisNamesIt);
        axisMinima.erase(axisMinIt);
        axisMaxima.erase(axisMaxIt);
        extentMinima.erase(extentMinIt);
        extentMaxima.erase(extentMaxIt);

        if (axisPosition < shownVarAxisPosition)
            insertPosition = shownVarAxisPosition;
        else
            insertPosition = ++shownVarAxisPosition;
    }
    else
    {
        saveAxisMin   = -1e+37;
        saveAxisMax   = +1e+37;
        saveExtentMin = 0.0;
        saveExtentMax = 1.0;

        insertPosition = ++shownVarAxisPosition;
    }

    axisNamesIt = orderedAxisNames.begin() + insertPosition;
    axisMinIt   = axisMinima.begin()       + insertPosition;
    axisMaxIt   = axisMaxima.begin()       + insertPosition;
    extentMinIt = extentMinima.begin()     + insertPosition;
    extentMaxIt = extentMaxima.begin()     + insertPosition;

    orderedAxisNames.insert(axisNamesIt, newAxisName);
    axisMinima.insert(axisMinIt, saveAxisMin);
    axisMaxima.insert(axisMaxIt, saveAxisMax);
    extentMinima.insert(extentMinIt, saveExtentMin);
    extentMaxima.insert(extentMaxIt, saveExtentMax);

    Select(0, (void *)&orderedAxisNames);
    Select(1, (void *)&shownVarAxisPosition);
    Select(2, (void *)&axisMinima);
    Select(3, (void *)&axisMaxima);
    Select(4, (void *)&extentMinima);
    Select(5, (void *)&extentMaxima);
}


void
ParallelAxisAttributes::DeleteAxis(const std::string &axisName_, int minAxisCount)
{
    if (orderedAxisNames.size() <= minAxisCount) return;

    std::string newAxisName = axisName_;

    int curAxisCount = orderedAxisNames.size();
    int axisPosition;

    for (axisPosition = 0; axisPosition < curAxisCount; axisPosition++)
    {
        if (orderedAxisNames[axisPosition] == newAxisName) break;
    }

    if (axisPosition < curAxisCount)
    {
        orderedAxisNames.erase(orderedAxisNames.begin() + axisPosition);
        axisMinima.erase(axisMinima.begin() + axisPosition);
        axisMaxima.erase(axisMaxima.begin() + axisPosition);
        extentMinima.erase(extentMinima.begin() + axisPosition);
        extentMaxima.erase(extentMaxima.begin() + axisPosition);

        if (axisPosition <= shownVarAxisPosition)
        {
            shownVarAxisPosition =
            (shownVarAxisPosition + curAxisCount - 2) % (curAxisCount - 1);
        }

        Select(0, (void *)&orderedAxisNames);
        Select(1, (void *)&shownVarAxisPosition);
        Select(2, (void *)&axisMinima);
        Select(3, (void *)&axisMaxima);
        Select(4, (void *)&extentMinima);
        Select(5, (void *)&extentMaxima);
    }
}


void
ParallelAxisAttributes::SwitchToLeftAxis(const std::string &axisName_)
{
    std::string newAxisName = axisName_;

    int curAxisCount = orderedAxisNames.size();
    int axisPosition, insertPosition;
    double saveAxisMin, saveAxisMax, saveExtentMin, saveExtentMax;
    std::string orderedAxisName;

    shownVarAxisPosition = 0;

    for (axisPosition = 0; axisPosition < curAxisCount; axisPosition++)
    {
        if (orderedAxisNames[axisPosition] == newAxisName) break;
    }

    if (axisPosition < curAxisCount) {
        if (axisPosition == 0)
        {
            Select(1, (void *)&shownVarAxisPosition);
            return;
        }

        saveAxisMin   = axisMinima[axisPosition];
        saveAxisMax   = axisMaxima[axisPosition];
        saveExtentMin = extentMinima[axisPosition];
        saveExtentMax = extentMaxima[axisPosition];

        orderedAxisNames.erase(orderedAxisNames.begin() + axisPosition);
        axisMinima.erase(axisMinima.begin() + axisPosition);
        axisMaxima.erase(axisMaxima.begin() + axisPosition);
        extentMinima.erase(extentMinima.begin() + axisPosition);
        extentMaxima.erase(extentMaxima.begin() + axisPosition);
    }
    else
    {
        saveAxisMin   = -1e+37;
        saveAxisMax   = +1e+37;
        saveExtentMin = 0.0;
        saveExtentMax = 1.0;
    }

    orderedAxisNames.insert(orderedAxisNames.begin(), newAxisName);
    axisMinima.insert(axisMinima.begin(), saveAxisMin);
    axisMaxima.insert(axisMaxima.begin(), saveAxisMax);
    extentMinima.insert(extentMinima.begin(), saveExtentMin);
    extentMaxima.insert(extentMaxima.begin(), saveExtentMax);

    Select(0, (void *)&orderedAxisNames);
    Select(1, (void *)&shownVarAxisPosition);
    Select(2, (void *)&axisMinima);
    Select(3, (void *)&axisMaxima);
    Select(4, (void *)&extentMinima);
    Select(5, (void *)&extentMaxima);
}


void
ParallelAxisAttributes::ShowPreviousAxisVariableData()
{
    if (orderedAxisNames.size() < 2) return;

    shownVarAxisPosition = (shownVarAxisPosition + orderedAxisNames.size() - 1) %
                           orderedAxisNames.size();
    Select(1, (void *)&shownVarAxisPosition);
}


void
ParallelAxisAttributes::ShowNextAxisVariableData()
{
    if (orderedAxisNames.size() < 2) return;

    shownVarAxisPosition = (shownVarAxisPosition + 1) % orderedAxisNames.size();
    Select(1, (void *)&shownVarAxisPosition);
}


///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////


const stringVector &
ParallelAxisAttributes::GetOrderedAxisNames() const
{
    return orderedAxisNames;
}


const std::string &
ParallelAxisAttributes::GetShownVariableAxisName() const
{
    return orderedAxisNames[shownVarAxisPosition];
}


std::string &
ParallelAxisAttributes::GetShownVariableAxisName()
{
    return orderedAxisNames[shownVarAxisPosition];
}


int
ParallelAxisAttributes::GetShownVariableAxisPosition() const
{
    return (shownVarAxisPosition);
}


int
ParallelAxisAttributes::GetShownVariableAxisNormalHumanPosition() const
{
    return (shownVarAxisPosition + 1);  // 1-origin for normal human beings
}


double
ParallelAxisAttributes::GetShownVariableAxisMinimum() const
{
    return axisMinima[shownVarAxisPosition];
}


double
ParallelAxisAttributes::GetShownVariableAxisMaximum() const
{
    return axisMaxima[shownVarAxisPosition];
}


double
ParallelAxisAttributes::GetShownVariableExtentMinimum() const
{
    return extentMinima[shownVarAxisPosition];
}


double
ParallelAxisAttributes::GetShownVariableExtentMaximum() const
{
    return extentMaxima[shownVarAxisPosition];
}


const doubleVector &
ParallelAxisAttributes::GetAxisMinima() const
{
    return axisMinima;
}


const doubleVector &
ParallelAxisAttributes::GetAxisMaxima() const
{
    return axisMaxima;
}


const doubleVector &
ParallelAxisAttributes::GetExtentMinima() const
{
    return extentMinima;
}


const doubleVector &
ParallelAxisAttributes::GetExtentMaxima() const
{
    return extentMaxima;
}


///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: ParallelAxisAttributes::SelectAll
//
// Purpose: Selects all attributes.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ParallelAxisAttributes::SelectAll()
{
    Select(0, (void *)&orderedAxisNames);
    Select(1, (void *)&shownVarAxisPosition);
    Select(2, (void *)&axisMinima);
    Select(3, (void *)&axisMaxima);
    Select(4, (void *)&extentMinima);
    Select(5, (void *)&extentMaxima);
}


///////////////////////////////////////////////////////////////////////////////
// Python compatibility methods
///////////////////////////////////////////////////////////////////////////////


void
ParallelAxisAttributes::SetShownVarAxisPosition(int shownVarAxisPosition_)
{
    shownVarAxisPosition = shownVarAxisPosition_;
    Select(1, (void *)&shownVarAxisPosition);
}


stringVector &
ParallelAxisAttributes::GetOrderedAxisNames()
{
    return orderedAxisNames;
}


int
ParallelAxisAttributes::GetShownVarAxisPosition() const
{
    return (shownVarAxisPosition);
}


doubleVector &
ParallelAxisAttributes::GetAxisMinima()
{
    return axisMinima;
}


doubleVector &
ParallelAxisAttributes::GetAxisMaxima()
{
    return axisMaxima;
}


doubleVector &
ParallelAxisAttributes::GetExtentMinima()
{
    return extentMinima;
}


doubleVector &
ParallelAxisAttributes::GetExtentMaxima()
{
    return extentMaxima;
}


void
ParallelAxisAttributes::SelectOrderedAxisNames()
{
    Select(0, (void *)&orderedAxisNames);
}


void
ParallelAxisAttributes::SelectShownVarAxisPosition()
{
    Select(1, (void *)&shownVarAxisPosition);
}


void
ParallelAxisAttributes::SelectAxisMinima()
{
    Select(2, (void *)&axisMinima);
}


void
ParallelAxisAttributes::SelectAxisMaxima()
{
    Select(3, (void *)&axisMaxima);
}


void
ParallelAxisAttributes::SelectExtentMinima()
{
    Select(4, (void *)&extentMinima);
}


void
ParallelAxisAttributes::SelectExtentMaxima()
{
    Select(5, (void *)&extentMaxima);
}


///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: ParallelAxisAttributes::GetFieldName
//
// Purpose: This method returns the name of a field given its index.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

std::string
ParallelAxisAttributes::GetFieldName(int index) const
{
    switch (index)
    {
        case 0: return "orderedAxisNames";
        case 1: return "shownVarAxisPosition";
        case 2: return "axisMinima";
        case 3: return "axisMaxima";
        case 4: return "extentMinima";
        case 5: return "extentMaxima";
        default: return "invalid index";
    }
}


// ****************************************************************************
// Method: ParallelAxisAttributes::GetFieldType
//
// Purpose: This method returns the type of a field given its index.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
ParallelAxisAttributes::GetFieldType(int index) const
{
    switch (index)
    {
        case 0: return FieldType_stringVector;
        case 1: return FieldType_int;
        case 2: return FieldType_doubleVector;
        case 3: return FieldType_doubleVector;
        case 4: return FieldType_doubleVector;
        case 5: return FieldType_doubleVector;
        default: return FieldType_unknown;
    }
}


// ****************************************************************************
// Method: ParallelAxisAttributes::GetFieldTypeName
//
// Purpose: This method returns the name of a field type given its index.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

std::string
ParallelAxisAttributes::GetFieldTypeName(int index) const
{
    switch (index)
   {
        case 0: return "stringVector";
        case 1: return "int";
        case 2: return "doubleVector";
        case 3: return "doubleVector";
        case 4: return "doubleVector";
        case 5: return "doubleVector";
        default: return "invalid index";
    }
}


// ****************************************************************************
// Method: ParallelAxisAttributes::FieldsEqual
//
// Purpose: This method compares two fields and returns true if they are equal.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ParallelAxisAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ParallelAxisAttributes &obj = *((const ParallelAxisAttributes*)rhs);
    bool retval = false;

    switch (index_) {
    case 0:
        {  // new scope
        retval = (orderedAxisNames == obj.orderedAxisNames);
        }
        break;
    case 1:
        {  // new scope
        retval = (shownVarAxisPosition == obj.shownVarAxisPosition);
        }
        break;
    case 2:
        {  // new scope
        retval = (axisMinima == obj.axisMinima);
        }
        break;
    case 3:
        {  // new scope
        retval = (axisMaxima == obj.axisMaxima);
        }
        break;
    case 4:
        {  // new scope
        retval = (extentMinima == obj.extentMinima);
        }
        break;
    case 5:
        {  // new scope
        retval = (extentMaxima == obj.extentMaxima);
        }
        break;
    default:
        retval = false;
    }

    return retval;
}


///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////


bool
ParallelAxisAttributes::ChangesRequireRecalculation(
    const ParallelAxisAttributes &obj)
{
    return true;
}


bool ParallelAxisAttributes::AttributesAreConsistent() const
{
    int axisNamesSize = orderedAxisNames.size();
    int axisNum, axis2Num;
    std::string axisName;
    
    if (axisNamesSize < 2)
        return false;
    
    if ((axisMinima.size()   != axisNamesSize) ||
        (axisMaxima.size()   != axisNamesSize) ||
        (extentMinima.size() != axisNamesSize) ||
        (extentMaxima.size() != axisNamesSize))
    {
        return false;
    }
            
    for (axisNum = 0; axisNum < axisNamesSize - 1; axisNum++)
    {
        axisName = orderedAxisNames[axisNum];

        for (axis2Num = axisNum + 1; axis2Num < axisNamesSize; axis2Num++ )
        {
            if (orderedAxisNames[axis2Num] == axisName)
                return false;
        }
    }
    
    return (shownVarAxisPosition < axisNamesSize);
}

