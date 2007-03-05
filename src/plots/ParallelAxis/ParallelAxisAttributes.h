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

// ************************************************************************* //
//                             ParallelAxisAttributes.h                      //
// ************************************************************************* //

#ifndef PARALLEL_AXIS_ATTRIBUTES_H
#define PARALLEL_AXIS_ATTRIBUTES_H

#define PCP_LEFT_AXIS_X_FRACTION          0.04
#define PCP_RIGHT_AXIS_X_FRACTION         0.96
#define PCP_H_BOTTOM_AXIS_Y_FRACTION      0.09
#define PCP_V_BOTTOM_AXIS_Y_FRACTION      0.12
#define PCP_H_TOP_AXIS_Y_FRACTION         0.90
#define PCP_V_TOP_AXIS_Y_FRACTION         0.87

#define PCP_MAX_HORIZONTAL_TEXT_INFO_AXES 7
#define PCP_MIN_H_INFO_AXIS_INTERVAL_FRAC 0.15
#define PCP_MIN_V_INFO_AXIS_INTERVAL_FRAC 0.05

#define PCP_NO_VALUE_TOKEN                0
#define PCP_VARIABLE_NAME_TOKEN           1
#define PCP_INTEGER_TOKEN                 2
#define PCP_FLOATING_POINT_NUMBER_TOKEN   3
#define PCP_LEFT_PARENTHESIS_TOKEN        100
#define PCP_RIGHT_PARENTHESIS_TOKEN       101
#define PCP_COMMA_TOKEN                   102
#define PCP_END_OF_EXPRESSION_TOKEN       -1

#define PCP_ATTRIBUTES_PER_AXIS           4

#define PCP_GROUP_ID_ATTRIBUTE_OFFSET     0
#define PCP_AXIS_SPACING_ATTRIBUTE_OFFSET 1
#define PCP_LOWER_BOUND_ATTRIBUTE_OFFSET  2
#define PCP_UPPER_BOUND_ATTRIBUTE_OFFSET  3

#define PCP_GROUP_ID_ATTRIBUTE_FLAG      (1 << PCP_GROUP_ID_ATTRIBUTE_OFFSET)
#define PCP_AXIS_SPACING_ATTRIBUTE_FLAG  (1 << PCP_AXIS_SPACING_ATTRIBUTE_OFFSET)
#define PCP_LOWER_BOUND_ATTRIBUTE_FLAG   (1 << PCP_LOWER_BOUND_ATTRIBUTE_OFFSET)
#define PCP_UPPER_BOUND_ATTRIBUTE_FLAG   (1 << PCP_UPPER_BOUND_ATTRIBUTE_OFFSET)

#define PCP_IMPLICIT_AXIS_GROUP_ID       -1000000001
#define PCP_FIRST_DUMMY_AXIS_GROUP_ID    -1000000000

#include <ExtentsAttributes.h>

#include <vector>
#include <string>


// ****************************************************************************
// Class: ParallelAxisAttributes
//
// Purpose: Maintains plot attributes for the ParallelAxis plot.
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
//    Added attributes to support non-uniform axis spacing.  Also removed
//    UpdateAxisBoundsIfPossible, since scratch file is no longer used.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//   
// ****************************************************************************

class ParallelAxisAttributes : public AttributeSubject
{
public:
    ParallelAxisAttributes();
    ParallelAxisAttributes(const ParallelAxisAttributes &obj);
    virtual ~ParallelAxisAttributes();

    virtual ParallelAxisAttributes& operator = (const ParallelAxisAttributes &obj);
    virtual bool operator == (const ParallelAxisAttributes &obj) const;
    virtual bool operator != (const ParallelAxisAttributes &obj) const;

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property setting methods
    void SetOrderedAxisNames(const stringVector &orderedAxisNames_);
    void SetShownVariableAxisOrdinal(int shownVarAxisOrdinal_);
    void SetAxisMinima(const doubleVector &axisMinima_);
    void SetAxisMaxima(const doubleVector &axisMaxima_);
    void SetExtentMinima(const doubleVector &extentMinima_);
    void SetExtentMaxima(const doubleVector &extentMaxima_);
    void SetExtMinTimeOrds(const intVector &extMinTimeOrds_);
    void SetExtMaxTimeOrds(const intVector &extMaxTimeOrds_);
    void SetPlotToolModeFlags(int plotToolModeFlags_);
    void SetAxisGroupNames(const stringVector &axisGroupNames_);
    void SetAxisInfoFlagSets(const intVector &axisInfoFlagSets_);
    void SetAxisXPositions(const doubleVector &axisXPositions_);
    void SetAxisAttributeVariables(const stringVector &axisAttributeVariables_);
    void SetAttributesPerAxis(int attributesPerAxis_);
    void SetAxisAttributeData(const doubleVector &axisAttributeData_);

    // Property changing methods
    void InsertAxis(const std::string &axisName_);
    void DeleteAxis(const std::string &axisName_, int minAxisCount);
    void SwitchToLeftAxis(const std::string &axisName_);
    void DetermineShownAndSelectedAxisSequences(int &leftShownAxisID,
         int &rightShownAxisID, int &leftSelectedAxisID, int &rightSelectedAxisID);
    void AdjustAxisSequencesAfterInsertingNewAxis(
        int &leftShownAxisID, int &rightShownAxisID,
        int &leftSelectedAxisID, int &rightSelectedAxisID, int insertedAxisID);
    void AdjustAxisSequencesAfterDeletingAxis(int &leftShownAxisID,
        int &rightShownAxisID, int &leftSelectedAxisID, int &rightSelectedAxisID,
        int deletedAxisID, int axisCountBeforeDelete);
    void MarkShownAndSelectedAxisSequences(int leftShownAxisID,
        int rightShownAxisID, int leftSelectedAxisID, int rightSelectedAxisID);
    void RecalculateAxisXPositions(int leftShownAxisID, int rightShownAxisID);
    void IdentifyReasonableAxesToLabel();
    void ShowPreviousAxisVariableData();
    void ShowNextAxisVariableData();

    // Property getting methods
    const stringVector          &GetOrderedAxisNames() const;
    const std::string           &GetShownVariableAxisName() const;
    std::string                 &GetShownVariableAxisName();
    int                          GetShownVariableAxisOrdinal() const;
    int                          GetShownVariableAxisNormalHumanOrdinal() const;
    double                       GetShownVariableAxisMinimum() const;
    double                       GetShownVariableAxisMaximum() const;
    double                       GetShownVariableExtentMinimum() const;
    double                       GetShownVariableExtentMaximum() const;
    const doubleVector          &GetAxisMinima() const;
    const doubleVector          &GetAxisMaxima() const;
    const doubleVector          &GetExtentMinima() const;
    const doubleVector          &GetExtentMaxima() const;
    const intVector             &GetExtMinTimeOrds() const;
    const intVector             &GetExtMaxTimeOrds() const;
    int                          GetPlotToolModeFlags() const;
    const stringVector          &GetAxisGroupNames() const;
    const intVector             &GetAxisInfoFlagSets() const;
    const doubleVector          &GetAxisXPositions() const;
    const stringVector          &GetAxisAttributeVariables() const;
    int                          GetAttributesPerAxis() const;
    const doubleVector          &GetAxisAttributeData() const;

    // Property selection methods
    virtual void        SelectAll();

    void                SelectOrderedAxisNames();
    void                SelectShownVarAxisOrdinal();
    void                SelectAxisMinima();
    void                SelectAxisMaxima();
    void                SelectExtentMinima();
    void                SelectExtentMaxima();
    void                SelectExtMinTimeOrds();
    void                SelectExtMaxTimeOrds();
    void                SelectPlotToolModeFlags();
    void                SelectAxisGroupNames();
    void                SelectAxisInfoFlagSets();
    void                SelectAxisXPositions();
    void                SelectAxisAttributeVariables();
    void                SelectAttributesPerAxis();
    void                SelectAxisAttributeData();

    // Python compatibility methods
    void                SetShownVarAxisOrdinal(int);

    stringVector       &GetOrderedAxisNames();
    int                 GetShownVarAxisOrdinal() const;
    doubleVector       &GetAxisMinima();
    doubleVector       &GetAxisMaxima();
    doubleVector       &GetExtentMinima();
    doubleVector       &GetExtentMaxima();
    intVector          &GetExtMinTimeOrds();
    intVector          &GetExtMaxTimeOrds();
    stringVector       &GetAxisGroupNames();
    intVector          &GetAxisInfoFlagSets();
    doubleVector       &GetAxisXPositions();
    stringVector       &GetAxisAttributeVariables();
    doubleVector       &GetAxisAttributeData();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index,
                                          const AttributeGroup *rhs) const;

    // User-defined methods
    bool ChangesRequireRecalculation(const ParallelAxisAttributes &obj);
    bool AttributesAreConsistent() const;

private:
    stringVector        orderedAxisNames;
    int                 shownVarAxisOrdinal;
    doubleVector        axisMinima;
    doubleVector        axisMaxima;
    doubleVector        extentMinima;
    doubleVector        extentMaxima;
    intVector           extMinTimeOrds;
    intVector           extMaxTimeOrds;
    int                 plotToolModeFlags;
    stringVector        axisGroupNames;
    intVector           axisInfoFlagSets;
    doubleVector        axisXPositions;
    stringVector        axisAttributeVariables;
    int                 attributesPerAxis;
    doubleVector        axisAttributeData;
};

#endif
