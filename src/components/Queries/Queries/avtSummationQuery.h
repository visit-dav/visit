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
//                            avtSummationQuery.h                            //
// ************************************************************************* //

#ifndef AVT_SUMMATION_QUERY_H
#define AVT_SUMMATION_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <string>

class vtkDataSet;

// ****************************************************************************
//  Class: avtSummationQuery
//
//  Purpose:
//      This query sums all of the values for a variable.
//
//  Notes:
//    Taken mostly from Hank Childs' avtSummationFilter and reworked to
//    fit into the Query hierarchy.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//    Add domain to Execute arguments.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//    Add unitsAppend. 
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Thu Jan 12 14:58:07 PST 2006
//    Added qualifier.
//
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Added sumFromOriginalElement.
//
//    Hank Childs, Tue May 16 09:14:41 PDT 2006
//    Add support for averaging.
//
// ****************************************************************************

class QUERY_API avtSummationQuery : public avtDatasetQuery
{
  public:
                                    avtSummationQuery();
    virtual                        ~avtSummationQuery();

    virtual void                    SetVariableName(std::string &);
    void                            SetSumType(std::string &);
    void                            SetUnitsAppend(std::string &);

    virtual const char             *GetType(void)
                                             { return "avtSummationQuery"; };
    virtual const char             *GetDescription(void)
                                             { return descriptionBuffer; };

    void                            SumGhostValues(bool);
    void                            SumOnlyPositiveValues(bool);
    void                            SumFromOriginalElement(bool);

  protected:
    double                          sum;
    double                          denomSum;
    std::string                     variableName;
    std::string                     denomVariableName;
    std::string                     sumType;
    std::string                     unitsAppend;
    std::string                     qualifier;
    bool                            sumGhostValues;
    bool                            sumOnlyPositiveValues;
    bool                            sumFromOriginalElement;
    char                            descriptionBuffer[1024];

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual bool                    CalculateAverage() { return false; };
};


#endif


