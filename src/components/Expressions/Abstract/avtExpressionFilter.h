/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                            avtExpressionFilter.h                          //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_FILTER_H
#define AVT_EXPRESSION_FILTER_H

#include <expression_exports.h>

#include <string>

#include <avtStreamer.h>
#include <avtTypes.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;


// ****************************************************************************
//  Class: avtExpressionFilter
//
//  Purpose:
//      This is a base class that lets derived types not worry about how to set
//      up a derived variable.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//
//    Sean Ahern, Fri Jun 13 11:18:07 PDT 2003
//    Added the virtual function NumVariableArguments that lets an
//    expression declare how many of its arguments are variables.
//
//    Hank Childs, Tue Aug 12 10:34:02 PDT 2003
//    Store the domain label and index when deriving a variable.  Certain
//    filters need this information.
//  
//    Hank Childs, Fri Oct 24 14:46:20 PDT 2003
//    Made Evaluator be a friend class so that it could call perform
//    restriction on the expressions it contains.
//
//    Hank Childs, Wed Dec 10 09:40:18 PST 2003
//    Add support for recentering a variable.  It is at this level so all
//    derived types can use the same routine.
// 
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004 
//    Add currentTimeState, ExamineSpecification.
//
//    Hank Childs, Mon Dec 27 10:13:44 PST 2004
//    Separate out parts that are related to streaming.
//
//    Hank Childs, Fri Aug  5 16:40:12 PDT 2005
//    Added GetVariableType.
//
//    Hank Childs, Mon Aug 29 14:44:20 PDT 2005
//    Added SetExpressionAtts.
//
//    Hank Childs, Sun Jan 22 12:38:57 PST 2006
//    Made Recenter be a static, public function.  Also improved implementation
//    of GetVariableDimension.
//
//    Hank Childs, Mon Jan  8 10:17:15 PST 2007
//    Added method to determine variable type of variable.
//
// ****************************************************************************

class EXPRESSION_API avtExpressionFilter : virtual public 
                                                     avtDatasetToDatasetFilter
{
    friend class             avtExpressionEvaluatorFilter;

  public:
                             avtExpressionFilter();
    virtual                 ~avtExpressionFilter();

    void                     SetOutputVariableName(const char *);
    virtual void             AddInputVariableName(const char *var)
                                {SetActiveVariable(var);}

    virtual void             ProcessArguments(ArgsExpr *, ExprPipelineState *);
    virtual int              NumVariableArguments() = 0;
    static vtkDataArray     *Recenter(vtkDataSet*, vtkDataArray*,avtCentering);

  protected:
    char                    *outputVariableName;
    int                      currentTimeState;

    virtual bool             IsPointVariable();

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual void             RefashionDataObjectInfo(void);
    void                     SetExpressionAttributes(const avtDataAttributes &,
                                                     avtDataAttributes &);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual void             ExamineSpecification(avtPipelineSpecification_p);

    virtual int              GetVariableDimension();
    virtual avtVarType       GetVariableType() { return AVT_UNKNOWN_TYPE; };

    //
    // The method "GetVariableType" declares what the variable type of the
    // output is.  This method will determine the variable type of an
    // existing variable.
    //
    avtVarType               DetermineVariableType(std::string &);

    void                     UpdateExtents(avtDataTree_p);
};


#endif


