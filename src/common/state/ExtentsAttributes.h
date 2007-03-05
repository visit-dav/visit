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

#ifndef EXTENTSATTRIBUTES_H
#define EXTENTSATTRIBUTES_H

#include <state_exports.h>
#include <AttributeSubject.h>

#define EA_DEFAULT_NUMBER_OF_EXTENTS     2

#define EA_DEFAULT_LEFT_SLIDER_X         0.04
#define EA_DEFAULT_RIGHT_SLIDER_X        0.96
#define EA_DEFAULT_SLIDERS_BOTTOM_Y      0.12
#define EA_DEFAULT_SLIDERS_TOP_Y         0.88

#define EA_TOOL_BUTTON_MARGIN_FRACTION   0.08
#define EA_TOOL_MARK_MARGIN_FRACTION     0.02

#define EA_TOOL_DRAWS_AXIS_INFO_FLAG     0x00000001
#define EA_SHOW_MARKED_AXES_ONLY_FLAG    0x00000002
#define EA_SHOW_LIMITED_AXIS_INFO_FLAG   0x00000004
#define EA_THRESHOLD_SELECTED_ONLY_FLAG  0x00000008

#define EA_AXIS_INFO_AUTO_LAYOUT_FLAG    0x00000100
#define EA_VERTICAL_TEXT_AXIS_INFO_FLAG  0x00000200
#define EA_PLOT_AXES_WERE_MODIFIED_FLAG  0x00000400

#define EA_LEFT_SHOWN_AXIS_FLAG          0x00000001
#define EA_LEFT_SELECTED_AXIS_FLAG       0x00000002
#define EA_RIGHT_SHOWN_AXIS_FLAG         0x00000100
#define EA_RIGHT_SELECTED_AXIS_FLAG      0x00000200
#define EA_SHOWN_AXIS_FLAGS             (EA_LEFT_SHOWN_AXIS_FLAG |             \
                                         EA_RIGHT_SHOWN_AXIS_FLAG )
#define EA_SELECTED_AXIS_FLAGS          (EA_LEFT_SELECTED_AXIS_FLAG |          \
                                         EA_RIGHT_SELECTED_AXIS_FLAG )
#define EA_AXIS_SEQUENCE_FLAGS          (EA_SHOWN_AXIS_FLAGS |                 \
                                         EA_SELECTED_AXIS_FLAGS )

#define EA_SHOW_AXIS_TITLE_ONLY_FLAG     0x00010000
#define EA_SHOW_AXIS_LIMITS_ONLY_FLAG    0x00020000
#define EA_SHOW_SLIDER_LIMITS_ONLY_FLAG  0x00040000
#define EA_SHOW_AXIS_SLIDERS_ONLY_FLAG   0x00080000
#define EA_SHOW_ALL_AXIS_INFO_FLAGS     (EA_SHOW_AXIS_TITLE_ONLY_FLAG |        \
                                         EA_SHOW_AXIS_LIMITS_ONLY_FLAG |       \
                                         EA_SHOW_SLIDER_LIMITS_ONLY_FLAG |     \
                                         EA_SHOW_AXIS_SLIDERS_ONLY_FLAG )
                                         
#define EA_AXIS_INFO_SHOWN_FLAG          0x01000000
#define EA_THRESHOLD_BY_EXTENT_FLAG      0x02000000

#define EA_PREFERRED_VIEWPORT_LEFT_X     0.02
#define EA_PREFERRED_VIEWPORT_RIGHT_X    0.98
#define EA_PREFERRED_VIEWPORT_BOTTOM_Y   0.06
#define EA_PREFERRED_VIEWPORT_TOP_Y      0.98

// ****************************************************************************
// Class: ExtentsAttributes
//
// Purpose: Attributes for a list of extents set by sliders
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added methods and members related to slider change time ordinals.
//
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added methods and members for selective axis info in associated plot.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
// ****************************************************************************

class STATE_API ExtentsAttributes : public AttributeSubject
{
public:
    ExtentsAttributes();
    ExtentsAttributes(const ExtentsAttributes &obj);
    virtual ~ExtentsAttributes();

    virtual void operator = (const ExtentsAttributes &obj);
    virtual bool operator == (const ExtentsAttributes &obj) const;
    virtual bool operator != (const ExtentsAttributes &obj) const;

    virtual const std::string TypeName() const;
    virtual bool              CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property setting methods
    void SetScalarNames(const stringVector &scalarNames_);
    void SetScalarMinima(const doubleVector &scalarMinima_);
    void SetScalarMaxima(const doubleVector &scalarMaxima_);
    void SetMinima(const doubleVector &minima_);
    void SetMaxima(const doubleVector &maxima_);
    void SetMinTimeOrdinals(const intVector &minTimeOrdinals_);
    void SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_);
    void SetPlotToolModeFlags(int plotToolModeFlags_);
    void SetAxisGroupNames(const stringVector &axisGroupNames_);
    void SetAxisInfoFlagSets(const intVector &axisInfoFlagSets_);
    void SetAxisXPositions(const doubleVector &axisXPositions_);
    void SetLeftSliderX(double leftSliderX_);
    void SetRightSliderX(double rightSliderX_);
    void SetSlidersBottomY(double slidersBottomY_);
    void SetSlidersTopY(double slidersTopY_);

    // Property getting methods
    const stringVector &GetScalarNames() const;
    const doubleVector &GetScalarMinima() const;
    const doubleVector &GetScalarMaxima() const;
    const doubleVector &GetMinima() const;
    const doubleVector &GetMaxima() const;
    const intVector    &GetMinTimeOrdinals() const;
    const intVector    &GetMaxTimeOrdinals() const;
    int                 GetPlotToolModeFlags() const;
    const stringVector &GetAxisGroupNames() const;
    const intVector    &GetAxisInfoFlagSets() const;
    const doubleVector &GetAxisXPositions() const;
    double              GetLeftSliderX() const;
    double              GetRightSliderX() const;
    double              GetSlidersBottomY() const;
    double              GetSlidersTopY() const;

    int                 GetNumberOfExtents() const;

    // Property selection methods
    virtual void SelectAll();
    void SelectScalarNames();
    void SelectScalarMinima();
    void SelectScalarMaxima();
    void SelectMinima();
    void SelectMaxima();
    void SelectMinTimeOrdinals();
    void SelectMaxTimeOrdinals();
    void SelectPlotToolModeFlags();
    void SelectAxisGroupNames();
    void SelectAxisInfoFlagSets();
    void SelectAxisXPositions();
    void SelectLeftSliderX();
    void SelectRightSliderX();
    void SelectSlidersBottomY();
    void SelectSlidersTopY();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

private:
    stringVector scalarNames;
    doubleVector scalarMinima;
    doubleVector scalarMaxima;
    doubleVector minima;
    doubleVector maxima;
    intVector    minTimeOrdinals;
    intVector    maxTimeOrdinals;
    int          plotToolModeFlags;
    stringVector axisGroupNames;
    intVector    axisInfoFlagSets;
    doubleVector axisXPositions;

    double       leftSliderX;
    double       rightSliderX;
    double       slidersBottomY;
    double       slidersTopY;
};

#endif
