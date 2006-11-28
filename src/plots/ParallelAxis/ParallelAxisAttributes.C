#include <ParallelAxisAttributes.h>
#include <DataNode.h>

#include <DebugStream.h>


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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

ParallelAxisAttributes::ParallelAxisAttributes() :
    AttributeSubject("s*id*d*d*d*i*i*bs*i*d*")
{
    orderedAxisNames.push_back(std::string("default"));
    shownVarAxisPosition = 0;
    axisMinima.push_back(-1e+37);
    axisMaxima.push_back(+1e+37);
    extentMinima.push_back(0.0);
    extentMaxima.push_back(1.0);
    extMinTimeOrds.push_back(0);
    extMaxTimeOrds.push_back(0);
    plotDrawsAxisLabels = true;
    axisGroupNames.push_back(std::string("(not_in_a_group)"));
    axisLabelStates.push_back(EA_DRAW_ALL_LABELS | EA_LABELS_NOW_VISIBLE);
    axisXIntervals.push_back(-1.0);
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

ParallelAxisAttributes::ParallelAxisAttributes(
    const ParallelAxisAttributes &obj) : AttributeSubject("s*id*d*d*d*i*i*bs*i*d*")
{
    orderedAxisNames = obj.orderedAxisNames;
    shownVarAxisPosition = obj.shownVarAxisPosition;
    axisMinima = obj.axisMinima;
    axisMaxima = obj.axisMaxima;
    extentMinima = obj.extentMinima;
    extentMaxima = obj.extentMaxima;
    extMinTimeOrds = obj.extMinTimeOrds;
    extMaxTimeOrds = obj.extMaxTimeOrds;
    plotDrawsAxisLabels = obj.plotDrawsAxisLabels;
    axisGroupNames = obj.axisGroupNames;
    axisLabelStates = obj.axisLabelStates;
    axisXIntervals = obj.axisXIntervals;

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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
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
    extMinTimeOrds = obj.extMinTimeOrds;
    extMaxTimeOrds = obj.extMaxTimeOrds;
    plotDrawsAxisLabels = obj.plotDrawsAxisLabels;
    axisGroupNames = obj.axisGroupNames;
    axisLabelStates = obj.axisLabelStates;
    axisXIntervals = obj.axisXIntervals;

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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
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
            (extentMaxima == obj.extentMaxima) &&
            (extMinTimeOrds == obj.extMinTimeOrds) &&
            (extMaxTimeOrds == obj.extMaxTimeOrds) &&
            (plotDrawsAxisLabels == obj.plotDrawsAxisLabels) &&
            (axisGroupNames == obj.axisGroupNames) &&
            (axisLabelStates == obj.axisLabelStates) &&
            (axisXIntervals == obj.axisXIntervals));
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Added support for non-uniform axis spacing and axis labeling by Extents
//    tool when enabled.
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
    else if (atts->TypeName() == "ExtentsAttributes")
    {
        const ExtentsAttributes *extAtts = (const ExtentsAttributes *)atts;
        
        stringVector toolVarNames     = extAtts->GetScalarNames();
        doubleVector toolVarMinima    = extAtts->GetScalarMinima();
        doubleVector toolVarMaxima    = extAtts->GetScalarMaxima();
        doubleVector toolSliderMinima = extAtts->GetMinima();
        doubleVector toolSliderMaxima = extAtts->GetMaxima();
        intVector    toolMinTimeOrds  = extAtts->GetMinTimeOrdinals();
        intVector    toolMaxTimeOrds  = extAtts->GetMaxTimeOrdinals();
        stringVector toolGroupNames   = extAtts->GetAxisGroupNames();
        intVector    toolLabelStates  = extAtts->GetAxisLabelStates();
        doubleVector toolXIntervals   = extAtts->GetAxisXIntervals();

        int toolVarCount = toolVarNames.size();
        int axisCount = orderedAxisNames.size();
        int toolVarNum, axisNum;
        double axisMargin;
        std::string toolVarName;

        for (toolVarNum = 0; toolVarNum < toolVarCount; toolVarNum++)
        {
            toolVarName = toolVarNames[toolVarNum];

            for (axisNum = 0; axisNum < axisCount; axisNum++ )
            {
                if (orderedAxisNames[axisNum] == toolVarName)
                {
                    axisMargin =
                        (toolVarMaxima[toolVarNum]-toolVarMinima[toolVarNum]) / 22.0;

                    axisMinima[axisNum] = toolVarMinima[toolVarNum] + axisMargin;
                    axisMaxima[axisNum] = toolVarMaxima[toolVarNum] - axisMargin;

                    extentMinima[axisNum] = toolSliderMinima[toolVarNum];
                    extentMaxima[axisNum] = toolSliderMaxima[toolVarNum];
                    
                    extMinTimeOrds[axisNum] = toolMinTimeOrds[toolVarNum];
                    extMaxTimeOrds[axisNum] = toolMaxTimeOrds[toolVarNum];
                    
                    axisGroupNames[axisNum]  = toolGroupNames[toolVarNum];
                    axisLabelStates[axisNum] = toolLabelStates[toolVarNum];
                    axisXIntervals[axisNum]  = toolXIntervals[toolVarNum];

                    break;
                }
            }
        }
        
        plotDrawsAxisLabels = !extAtts->GetToolDrawsAxisLabels();
        
        retval = true;
    }

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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Removed ExtentsAttributes case, which no longer occurs.
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
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

    if (completeSave || !FieldsEqual(6, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("extMinTimeOrds", extMinTimeOrds));
    }

    if (completeSave || !FieldsEqual(7, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("extMaxTimeOrds", extMaxTimeOrds));
    }

    if (completeSave || !FieldsEqual(8, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("plotDrawsAxisLabels", plotDrawsAxisLabels));
    }

    if (completeSave || !FieldsEqual(9, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("axisGroupNames", axisGroupNames));
    }

    if (completeSave || !FieldsEqual(10, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("axisLabelStates", axisLabelStates));
    }

    if (completeSave || !FieldsEqual(11, &defaultObject)) {
        addToParent = true;
        node->AddNode(new DataNode("axisXIntervals", axisXIntervals));
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
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

    if ((node = searchNode->GetNode("extMinTimeOrds")) != 0) {
        SetExtMinTimeOrds(node->AsIntVector());
    }

    if ((node = searchNode->GetNode("extMaxTimeOrds")) != 0) {
        SetExtMaxTimeOrds(node->AsIntVector());
    }

    if ((node = searchNode->GetNode("plotDrawsAxisLabels")) != 0) {
        SetPlotDrawsAxisLabels(node->AsBool());
    }

    if ((node = searchNode->GetNode("axisGroupNames")) != 0) {
        SetAxisGroupNames(node->AsStringVector());
    }

    if ((node = searchNode->GetNode("axisLabelStates")) != 0) {
        SetAxisLabelStates(node->AsIntVector());
    }

    if ((node = searchNode->GetNode("axisXIntervals")) != 0) {
        SetAxisXIntervals(node->AsDoubleVector());
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


void
ParallelAxisAttributes::SetExtMinTimeOrds(const intVector &extMinTimeOrds_)
{
    extMinTimeOrds = extMinTimeOrds_;
    Select(6, (void *)&extMinTimeOrds);
}


void
ParallelAxisAttributes::SetExtMaxTimeOrds(const intVector &extMaxTimeOrds_)
{
    extMaxTimeOrds = extMaxTimeOrds_;
    Select(7, (void *)&extMaxTimeOrds);
}


void
ParallelAxisAttributes::SetPlotDrawsAxisLabels(bool plotDrawsAxisLabels_)
{
    plotDrawsAxisLabels = plotDrawsAxisLabels_;
    Select(8, (void *)&plotDrawsAxisLabels);
}


void
ParallelAxisAttributes::SetAxisGroupNames(const stringVector &axisGroupNames_)
{
    axisGroupNames = axisGroupNames_;
    Select(9, (void *)&axisGroupNames);
}


void
ParallelAxisAttributes::SetAxisLabelStates(const intVector &axisLabelStates_)
{
    axisLabelStates = axisLabelStates_;
    Select(10, (void *)&axisLabelStates);
}


void
ParallelAxisAttributes::SetAxisXIntervals(const doubleVector &axisXIntervals_)
{
    axisXIntervals = axisXIntervals_;
    Select(11, (void *)&axisXIntervals);
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
    int saveMinTimeOrd, saveMaxTimeOrd, saveLabelState;
    double saveAxisMin, saveAxisMax, saveExtentMin, saveExtentMax, saveXInterval;
    std::string saveGroupName;

    stringVector::iterator axisNamesIt;
    doubleVector::iterator axisMinIt;
    doubleVector::iterator axisMaxIt;
    doubleVector::iterator extentMinIt;
    doubleVector::iterator extentMaxIt;
    intVector::iterator minTimeOrdIt;
    intVector::iterator maxTimeOrdIt;
    stringVector::iterator groupNamesIt;
    intVector::iterator labelStateIt;
    doubleVector::iterator xIntervalIt;

    for (axisPosition = 0; axisPosition < curAxisCount; axisPosition++)
    {
        if (orderedAxisNames[axisPosition] == newAxisName) break;
    }

    if (axisPosition < curAxisCount)
    {
        if (axisPosition == shownVarAxisPosition) return;

        saveAxisMin    = axisMinima[axisPosition];
        saveAxisMax    = axisMaxima[axisPosition];
        saveExtentMin  = extentMinima[axisPosition];
        saveExtentMax  = extentMaxima[axisPosition];
        saveMinTimeOrd = extMinTimeOrds[axisPosition];
        saveMaxTimeOrd = extMaxTimeOrds[axisPosition];
        saveGroupName  = axisGroupNames[axisPosition];
        saveLabelState = axisLabelStates[axisPosition];
        saveXInterval  = axisXIntervals[axisPosition];

        axisNamesIt  = orderedAxisNames.begin() + axisPosition;
        axisMinIt    = axisMinima.begin()       + axisPosition;
        axisMaxIt    = axisMaxima.begin()       + axisPosition;
        extentMinIt  = extentMinima.begin()     + axisPosition;
        extentMaxIt  = extentMaxima.begin()     + axisPosition;
        minTimeOrdIt = extMinTimeOrds.begin()   + axisPosition;
        maxTimeOrdIt = extMaxTimeOrds.begin()   + axisPosition;
        groupNamesIt = axisGroupNames.begin()   + axisPosition;
        labelStateIt = axisLabelStates.begin()  + axisPosition;
        xIntervalIt  = axisXIntervals.begin()   + axisPosition;

        orderedAxisNames.erase(axisNamesIt);
        axisMinima.erase(axisMinIt);
        axisMaxima.erase(axisMaxIt);
        extentMinima.erase(extentMinIt);
        extentMaxima.erase(extentMaxIt);
        extMinTimeOrds.erase(minTimeOrdIt);
        extMaxTimeOrds.erase(maxTimeOrdIt);
        axisGroupNames.erase(groupNamesIt);
        axisLabelStates.erase(labelStateIt);
        axisXIntervals.erase(xIntervalIt);

        if (axisPosition < shownVarAxisPosition)
            insertPosition = shownVarAxisPosition;
        else
            insertPosition = ++shownVarAxisPosition;
    }
    else
    {
        saveAxisMin    = -1e+37;
        saveAxisMax    = +1e+37;
        saveExtentMin  = 0.0;
        saveExtentMax  = 1.0;
        saveMinTimeOrd = 0;
        saveMaxTimeOrd = 0;
        saveGroupName  = std::string("(not_in_a_group)");
        saveLabelState = EA_DRAW_ALL_LABELS | EA_LABELS_NOW_VISIBLE;
        saveXInterval  = -1.0;

        insertPosition = ++shownVarAxisPosition;
    }

    axisNamesIt  = orderedAxisNames.begin() + insertPosition;
    axisMinIt    = axisMinima.begin()       + insertPosition;
    axisMaxIt    = axisMaxima.begin()       + insertPosition;
    extentMinIt  = extentMinima.begin()     + insertPosition;
    extentMaxIt  = extentMaxima.begin()     + insertPosition;
    minTimeOrdIt = extMinTimeOrds.begin()   + insertPosition;
    maxTimeOrdIt = extMaxTimeOrds.begin()   + insertPosition;
    groupNamesIt = axisGroupNames.begin()   + insertPosition;
    labelStateIt = axisLabelStates.begin()  + insertPosition;
    xIntervalIt  = axisXIntervals.begin()   + insertPosition;

    orderedAxisNames.insert(axisNamesIt, newAxisName);
    axisMinima.insert(axisMinIt, saveAxisMin);
    axisMaxima.insert(axisMaxIt, saveAxisMax);
    extentMinima.insert(extentMinIt, saveExtentMin);
    extentMaxima.insert(extentMaxIt, saveExtentMax);
    extMinTimeOrds.insert(minTimeOrdIt, saveMinTimeOrd);
    extMaxTimeOrds.insert(maxTimeOrdIt, saveMaxTimeOrd);
    axisGroupNames.insert(groupNamesIt, saveGroupName);
    axisLabelStates.insert(labelStateIt, saveLabelState);
    axisXIntervals.insert(xIntervalIt, saveXInterval);

    SelectAll();
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
        extMinTimeOrds.erase(extMinTimeOrds.begin() + axisPosition);
        extMaxTimeOrds.erase(extMaxTimeOrds.begin() + axisPosition);
        axisGroupNames.erase(axisGroupNames.begin() + axisPosition);
        axisLabelStates.erase(axisLabelStates.begin() + axisPosition);
        axisXIntervals.erase(axisXIntervals.begin() + axisPosition);

        if (axisPosition <= shownVarAxisPosition)
        {
            shownVarAxisPosition =
            (shownVarAxisPosition + curAxisCount - 2) % (curAxisCount - 1);
        }
        
        SelectAll();
    }
}


void
ParallelAxisAttributes::SwitchToLeftAxis(const std::string &axisName_)
{
    std::string newAxisName = axisName_;

    int curAxisCount = orderedAxisNames.size();
    int axisPosition, saveMinTimeOrd, saveMaxTimeOrd, saveLabelState;
    double saveAxisMin, saveAxisMax, saveExtentMin, saveExtentMax, saveXInterval;
    std::string orderedAxisName;
    std::string saveGroupName;

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

        saveAxisMin    = axisMinima[axisPosition];
        saveAxisMax    = axisMaxima[axisPosition];
        saveExtentMin  = extentMinima[axisPosition];
        saveExtentMax  = extentMaxima[axisPosition];
        saveMinTimeOrd = extMinTimeOrds[axisPosition];
        saveMaxTimeOrd = extMaxTimeOrds[axisPosition];
        saveGroupName  = axisGroupNames[axisPosition];
        saveLabelState = axisLabelStates[axisPosition];
        saveXInterval  = axisXIntervals[axisPosition];

        orderedAxisNames.erase(orderedAxisNames.begin() + axisPosition);
        axisMinima.erase(axisMinima.begin() + axisPosition);
        axisMaxima.erase(axisMaxima.begin() + axisPosition);
        extentMinima.erase(extentMinima.begin() + axisPosition);
        extentMaxima.erase(extentMaxima.begin() + axisPosition);
        extMinTimeOrds.erase(extMinTimeOrds.begin() + axisPosition);
        extMaxTimeOrds.erase(extMaxTimeOrds.begin() + axisPosition);
        axisGroupNames.erase(axisGroupNames.begin() + axisPosition);
        axisLabelStates.erase(axisLabelStates.begin() + axisPosition);
        axisXIntervals.erase(axisXIntervals.begin() + axisPosition);
    }
    else
    {
        saveAxisMin    = -1e+37;
        saveAxisMax    = +1e+37;
        saveExtentMin  = 0.0;
        saveExtentMax  = 1.0;
        saveMinTimeOrd = 0;
        saveMaxTimeOrd = 0;
        saveGroupName  = std::string("(not_in_a_group)");
        saveLabelState = EA_DRAW_ALL_LABELS | EA_LABELS_NOW_VISIBLE;
        saveXInterval  = -1.0;
    }

    orderedAxisNames.insert(orderedAxisNames.begin(), newAxisName);
    axisMinima.insert(axisMinima.begin(), saveAxisMin);
    axisMaxima.insert(axisMaxima.begin(), saveAxisMax);
    extentMinima.insert(extentMinima.begin(), saveExtentMin);
    extentMaxima.insert(extentMaxima.begin(), saveExtentMax);
    extMinTimeOrds.insert(extMinTimeOrds.begin(), saveMinTimeOrd);
    extMaxTimeOrds.insert(extMaxTimeOrds.begin(), saveMaxTimeOrd);
    axisGroupNames.insert(axisGroupNames.begin(), saveGroupName);
    axisLabelStates.insert(axisLabelStates.begin(), saveLabelState);
    axisXIntervals.insert(axisXIntervals.begin(), saveXInterval);
    
    SelectAll();
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
    return shownVarAxisPosition;
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


const intVector &
ParallelAxisAttributes::GetExtMinTimeOrds() const
{
    return extMinTimeOrds;
}


const intVector &
ParallelAxisAttributes::GetExtMaxTimeOrds() const
{
    return extMaxTimeOrds;
}


bool
ParallelAxisAttributes::GetPlotDrawsAxisLabels() const
{
    return plotDrawsAxisLabels;
}


const stringVector &
ParallelAxisAttributes::GetAxisGroupNames() const
{
    return axisGroupNames;
}


const intVector &
ParallelAxisAttributes::GetAxisLabelStates() const
{
    return axisLabelStates;
}


const doubleVector &
ParallelAxisAttributes::GetAxisXIntervals() const
{
    return axisXIntervals;
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

void
ParallelAxisAttributes::SelectAll()
{
    Select( 0, (void *)&orderedAxisNames);
    Select( 1, (void *)&shownVarAxisPosition);
    Select( 2, (void *)&axisMinima);
    Select( 3, (void *)&axisMaxima);
    Select( 4, (void *)&extentMinima);
    Select( 5, (void *)&extentMaxima);
    Select( 6, (void *)&extMinTimeOrds);
    Select( 7, (void *)&extMaxTimeOrds);
    Select( 8, (void *)&plotDrawsAxisLabels);
    Select( 9, (void *)&axisGroupNames);
    Select(10, (void *)&axisLabelStates);
    Select(11, (void *)&axisXIntervals);
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


void
ParallelAxisAttributes::SelectExtMinTimeOrds()
{
    Select(6, (void *)&extMinTimeOrds);
}


void
ParallelAxisAttributes::SelectExtMaxTimeOrds()
{
    Select(7, (void *)&extMaxTimeOrds);
}


void
ParallelAxisAttributes::SelectPlotDrawsAxisLabels()
{
    Select(8, (void *)&plotDrawsAxisLabels);
}


void
ParallelAxisAttributes::SelectAxisGroupNames()
{
    Select(9, (void *)&axisGroupNames);
}


void
ParallelAxisAttributes::SelectAxisLabelStates()
{
    Select(10, (void *)&axisLabelStates);
}


void
ParallelAxisAttributes::SelectAxisXIntervals()
{
    Select(11, (void *)&axisXIntervals);
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
    return shownVarAxisPosition;
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


intVector &
ParallelAxisAttributes::GetExtMinTimeOrds()
{
    return extMinTimeOrds;
}


intVector &
ParallelAxisAttributes::GetExtMaxTimeOrds()
{
    return extMaxTimeOrds;
}


stringVector &
ParallelAxisAttributes::GetAxisGroupNames()
{
    return axisGroupNames;
}


intVector &
ParallelAxisAttributes::GetAxisLabelStates()
{
    return axisLabelStates;
}


doubleVector &
ParallelAxisAttributes::GetAxisXIntervals()
{
    return axisXIntervals;
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

std::string
ParallelAxisAttributes::GetFieldName(int index) const
{
    switch (index)
    {
        case  0: return "orderedAxisNames";
        case  1: return "shownVarAxisPosition";
        case  2: return "axisMinima";
        case  3: return "axisMaxima";
        case  4: return "extentMinima";
        case  5: return "extentMaxima";
        case  6: return "extMinTimeOrds";
        case  7: return "extMaxTimeOrds";
        case  8: return "plotDrawsAxisLabels";
        case  9: return "axisGroupNames";
        case 10: return "axisLabelStates";
        case 11: return "axisXIntervals";
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

AttributeGroup::FieldType
ParallelAxisAttributes::GetFieldType(int index) const
{
    switch (index)
    {
        case  0: return FieldType_stringVector;
        case  1: return FieldType_int;
        case  2: return FieldType_doubleVector;
        case  3: return FieldType_doubleVector;
        case  4: return FieldType_doubleVector;
        case  5: return FieldType_doubleVector;
        case  6: return FieldType_intVector;
        case  7: return FieldType_intVector;
        case  8: return FieldType_bool;
        case  9: return FieldType_stringVector;
        case 10: return FieldType_intVector;
        case 11: return FieldType_doubleVector;
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

std::string
ParallelAxisAttributes::GetFieldTypeName(int index) const
{
    switch (index)
   {
        case  0: return "stringVector";
        case  1: return "int";
        case  2: return "doubleVector";
        case  3: return "doubleVector";
        case  4: return "doubleVector";
        case  5: return "doubleVector";
        case  6: return "intVector";
        case  7: return "intVector";
        case  8: return "bool";
        case  9: return "stringVector";
        case 10: return "intVector";
        case 11: return "doubleVector";
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
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
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
    case 6:
        {  // new scope
        retval = (extMinTimeOrds == obj.extMinTimeOrds);
        }
        break;
    case 7:
        {  // new scope
        retval = (extMaxTimeOrds == obj.extMaxTimeOrds);
        }
        break;
    case 8:
        {  // new scope
        retval = (plotDrawsAxisLabels == obj.plotDrawsAxisLabels);
        }
        break;
    case 9:
        {  // new scope
        retval = (axisGroupNames == obj.axisGroupNames);
        }
        break;
    case 10:
        {  // new scope
        retval = (axisLabelStates == obj.axisLabelStates);
        }
        break;
    case 11:
        {  // new scope
        retval = (axisXIntervals == obj.axisXIntervals);
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


// ****************************************************************************
// Method: ParallelAxisAttributes::AttributesAreConsistent
//
// Purpose: Returns true only if (1) all vector attributes are the same length,
//          (2) all axis names are unique, and (3) the index of the currently
//          displayable axis information in the ParallelAxis GUI is in range.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added attributes to support non-uniform axis spacing.
//   
// ****************************************************************************

bool ParallelAxisAttributes::AttributesAreConsistent() const
{
    int axisNamesSize = orderedAxisNames.size();
    int axisNum, axis2Num;
    std::string axisName;
    
    if (axisNamesSize < 2)
        return false;
    
    if ((axisMinima.size()      != axisNamesSize) ||
        (axisMaxima.size()      != axisNamesSize) ||
        (extentMinima.size()    != axisNamesSize) ||
        (extentMaxima.size()    != axisNamesSize) ||
        (extMinTimeOrds.size()  != axisNamesSize) ||
        (extMaxTimeOrds.size()  != axisNamesSize) ||
        (axisGroupNames.size()  != axisNamesSize) ||
        (axisLabelStates.size() != axisNamesSize) ||
        (axisXIntervals.size()  != axisNamesSize))
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
