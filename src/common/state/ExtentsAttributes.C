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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added attributes to support selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Added attributes to support  all variable axis spacing and axis group
//     conventions.
//
// ****************************************************************************

ExtentsAttributes::ExtentsAttributes() : AttributeSubject("s*d*d*d*d*i*i*is*i*d*dddd")
{
    int allAxesInfoFlags = EA_THRESHOLD_BY_EXTENT_FLAG |
                           EA_SHOW_ALL_AXIS_INFO_FLAGS | EA_AXIS_INFO_SHOWN_FLAG;
    int axisInfoFlags, extentNum;
    
    scalarNames.clear();
    scalarMinima.clear(); scalarMaxima.clear();
    minima.clear(); maxima.clear();
    minTimeOrdinals.clear(); maxTimeOrdinals.clear();
    axisGroupNames.clear();
    axisInfoFlagSets.clear();
    axisXPositions.clear();
    
    for (extentNum = 0; extentNum < EA_DEFAULT_NUMBER_OF_EXTENTS; extentNum++)
    {
        axisInfoFlags = allAxesInfoFlags;
        
        if (extentNum == 0)
            axisInfoFlags |= EA_LEFT_SHOWN_AXIS_FLAG | EA_LEFT_SELECTED_AXIS_FLAG;
        else if (extentNum == EA_DEFAULT_NUMBER_OF_EXTENTS-1)
            axisInfoFlags |= EA_RIGHT_SHOWN_AXIS_FLAG | EA_RIGHT_SELECTED_AXIS_FLAG;

        scalarNames.push_back("default");
        scalarMinima.push_back(-1e+37); scalarMaxima.push_back(1e+37);
        minima.push_back(0.0); maxima.push_back(1.0);
        minTimeOrdinals.push_back(0); maxTimeOrdinals.push_back(0);
        axisGroupNames.push_back(std::string("(not_in_a_group)"));
        axisInfoFlagSets.push_back(axisInfoFlags);
        axisXPositions.push_back(
            (double)extentNum / (double)(EA_DEFAULT_NUMBER_OF_EXTENTS-1));
    }

    plotToolModeFlags = EA_AXIS_INFO_AUTO_LAYOUT_FLAG;

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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added attributes to support selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Added attributes to support  all variable axis spacing and axis group
//     conventions.
//
// ****************************************************************************

ExtentsAttributes::ExtentsAttributes(const ExtentsAttributes &obj) :
    AttributeSubject("s*d*d*d*d*i*i*is*i*d*dddd")
{
    scalarNames = obj.scalarNames;

    scalarMinima = obj.scalarMinima;
    scalarMaxima = obj.scalarMaxima;

    minima = obj.minima;
    maxima = obj.maxima;
    
    minTimeOrdinals = obj.minTimeOrdinals;
    maxTimeOrdinals = obj.maxTimeOrdinals;
    
    plotToolModeFlags = obj.plotToolModeFlags;
    
    axisGroupNames   = obj.axisGroupNames;
    axisInfoFlagSets = obj.axisInfoFlagSets;
    axisXPositions   = obj.axisXPositions;

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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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

    plotToolModeFlags = obj.plotToolModeFlags;
    
    axisGroupNames   = obj.axisGroupNames;
    axisInfoFlagSets = obj.axisInfoFlagSets;
    axisXPositions   = obj.axisXPositions;

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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
// ****************************************************************************

bool ExtentsAttributes::operator == (const ExtentsAttributes &obj) const
{
    return ((scalarNames       == obj.scalarNames) &&
            (scalarMinima      == obj.scalarMinima) &&
            (scalarMaxima      == obj.scalarMaxima) &&
            (minima            == obj.minima) &&
            (maxima            == obj.maxima) &&
            (minTimeOrdinals   == obj.minTimeOrdinals) &&
            (maxTimeOrdinals   == obj.maxTimeOrdinals) &&
            (plotToolModeFlags == obj.plotToolModeFlags) &&
            (axisGroupNames    == obj.axisGroupNames) &&
            (axisInfoFlagSets  == obj.axisInfoFlagSets) &&
            (axisXPositions    == obj.axisXPositions) &&
            (leftSliderX       == obj.leftSliderX) &&
            (rightSliderX      == obj.rightSliderX) &&
            (slidersBottomY    == obj.slidersBottomY) &&
            (slidersTopY       == obj.slidersTopY));
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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
    Select( 7, (void *)&plotToolModeFlags);
    Select( 8, (void *)&axisGroupNames);
    Select( 9, (void *)&axisInfoFlagSets);
    Select(10, (void *)&axisXPositions);
    Select(11, (void *)&leftSliderX);
    Select(12, (void *)&rightSliderX);
    Select(13, (void *)&slidersBottomY);
    Select(14, (void *)&slidersTopY);
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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
        node->AddNode(new DataNode("plotToolModeFlags", plotToolModeFlags));
    }

    if (completeSave || !FieldsEqual(8, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axisGroupNames", axisGroupNames));
    }

    if (completeSave || !FieldsEqual(9, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axisInfoFlagSets", axisInfoFlagSets));
    }

    if (completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axisXPositions", axisXPositions));
    }

    if (completeSave || !FieldsEqual(11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("leftSliderX", leftSliderX));
    }

    if (completeSave || !FieldsEqual(12, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rightSliderX", rightSliderX));
    }

    if (completeSave || !FieldsEqual(13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("slidersBottomY", slidersBottomY));
    }

    if (completeSave || !FieldsEqual(14, &defaultObject))
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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

    if ((node = searchNode->GetNode("plotToolModeFlags")) != NULL)
        SetPlotToolModeFlags(node->AsInt());

    if ((node = searchNode->GetNode("axisGroupNames")) != NULL)
        SetAxisGroupNames(node->AsStringVector());

    if ((node = searchNode->GetNode("axisInfoFlagSets")) != NULL)
        SetAxisInfoFlagSets(node->AsIntVector());

    if ((node = searchNode->GetNode("axisXPositions")) != NULL)
        SetAxisXPositions(node->AsDoubleVector());

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


void ExtentsAttributes::SetPlotToolModeFlags(int plotToolModeFlags_)
{
    plotToolModeFlags = plotToolModeFlags_;
    Select(7, (void *)&plotToolModeFlags);
}


void ExtentsAttributes::SetAxisGroupNames(const stringVector &axisGroupNames_)
{
    axisGroupNames = axisGroupNames_;
    Select(8, (void *)&axisGroupNames);
}


void ExtentsAttributes::SetAxisInfoFlagSets(const intVector &axisInfoFlagSets_)
{
    axisInfoFlagSets = axisInfoFlagSets_;
    Select(9, (void *)&axisInfoFlagSets);
}


void ExtentsAttributes::SetAxisXPositions(const doubleVector &axisXPositions_)
{
    axisXPositions = axisXPositions_;
    Select(10, (void *)&axisXPositions);
}


void ExtentsAttributes::SetLeftSliderX(double leftSliderX_)
{
    leftSliderX = leftSliderX_;
    Select(11, (void *)&leftSliderX);
}


void ExtentsAttributes::SetRightSliderX(double rightSliderX_)
{
    rightSliderX = rightSliderX_;
    Select(12, (void *)&rightSliderX);
}


void ExtentsAttributes::SetSlidersBottomY(double slidersBottomY_)
{
    slidersBottomY = slidersBottomY_;
    Select(13, (void *)&slidersBottomY);
}


void ExtentsAttributes::SetSlidersTopY(double slidersTopY_)
{
    slidersTopY = slidersTopY_;
    Select(14, (void *)&slidersTopY);
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


int ExtentsAttributes::GetPlotToolModeFlags() const
{
    return plotToolModeFlags;
}


const stringVector &ExtentsAttributes::GetAxisGroupNames() const
{
    return axisGroupNames;
}


const intVector &ExtentsAttributes::GetAxisInfoFlagSets() const
{
    return axisInfoFlagSets;
}


const doubleVector &ExtentsAttributes::GetAxisXPositions() const
{
    return axisXPositions;
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


void ExtentsAttributes::SelectPlotToolModeFlags()
{
    Select (7, (void *)&plotToolModeFlags);
}


void ExtentsAttributes::SelectAxisGroupNames()
{
    Select (8, (void *)&axisGroupNames);
}


void ExtentsAttributes::SelectAxisInfoFlagSets()
{
    Select (9, (void *)&axisInfoFlagSets);
}


void ExtentsAttributes::SelectAxisXPositions()
{
    Select (10, (void *)&axisXPositions);
}


void ExtentsAttributes::SelectLeftSliderX()
{
    Select (11, (void *)&leftSliderX);
}


void ExtentsAttributes::SelectRightSliderX()
{
    Select (12, (void *)&rightSliderX);
}


void ExtentsAttributes::SelectSlidersBottomY()
{
    Select (13, (void *)&slidersBottomY);
}


void ExtentsAttributes::SelectSlidersTopY()
{
    Select (14, (void *)&slidersTopY);
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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
        case  7:  return "plotToolModeFlags";
        case  8:  return "axisGroupNames";
        case  9:  return "axisInfoFlagSets";
        case 10:  return "axisXPositions";
        case 11:  return "leftSliderX";
        case 12:  return "rightSliderX";
        case 13:  return "slidersBottomY";
        case 14:  return "slidersTopY";
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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
        case  7:  return FieldType_int;
        case  8:  return FieldType_stringVector;
        case  9:  return FieldType_intVector;
        case 10:  return FieldType_doubleVector;
        case 11:  return FieldType_double;
        case 12:  return FieldType_double;
        case 13:  return FieldType_double;
        case 14:  return FieldType_double;
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
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
        case  7:  return "int";
        case  8:  return "stringVector";
        case  9:  return "intVector";
        case 10:  return "doubleVector";
        case 11:  return "double";
        case 12:  return "double";
        case 13:  return "double";
        case 14:  return "double";
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
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added support for selective axis information in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
// ****************************************************************************

bool ExtentsAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ExtentsAttributes &obj = *((const ExtentsAttributes*)rhs);

    switch (index_) {
        case 0:
            if (obj.scalarNames != scalarNames) return false;
            break;

        case 1:
            if (obj.scalarMinima != scalarMinima) return false;
            break;

        case 2:
            if (obj.scalarMaxima != scalarMaxima) return false;
            break;

        case 3:
            if (obj.minima != minima) return false;
            break;

        case 4:
            if (obj.maxima != maxima) return false;
            break;

        case 5:
            if (obj.minTimeOrdinals != minTimeOrdinals) return false;
            break;

        case 6:
            if (obj.maxTimeOrdinals != maxTimeOrdinals) return false;
            break;

        case 7:
            if (obj.plotToolModeFlags != plotToolModeFlags) return false;
            break;

        case 8:
            if (obj.axisGroupNames != axisGroupNames) return false;
            break;

        case 9:
            if (obj.axisInfoFlagSets != axisInfoFlagSets) return false;
            break;

        case 10:
            if (obj.axisXPositions != axisXPositions) return false;
            break;

        case 11:
            if (obj.leftSliderX != leftSliderX) return false;
            break;

        case 12:
            if (obj.rightSliderX != rightSliderX) return false;
            break;

        case 13:
            if (obj.slidersBottomY != slidersBottomY) return false;
            break;

        case 14:
            if (obj.slidersTopY != slidersTopY) return false;
            break;

        default:
            return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////
