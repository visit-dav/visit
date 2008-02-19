/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                              ViewerOperator.h                             //
// ************************************************************************* //

#ifndef VIEWER_OPERATOR_H
#define VIEWER_OPERATOR_H
#include <viewer_exports.h>
#include <ViewerBase.h>

// Forward declarations
class AttributeSubject;
class DataNode;
class ViewerPlot;
class ViewerOperatorPluginInfo;
class avtPluginFilter;
class ExpressionList;

#include <vector>
#include <string>

// ****************************************************************************
//  Class: ViewerOperator
//
//  Purpose:
//      ViewerOperator is an abstract base class from which the concrete
//      operators are derived.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 13 11:23:19 PST 2000
//    Made it also use a plot when creating an operator and store it for later.
//
//    Jeremy Meredith, Wed Jul 25 13:02:06 PDT 2001
//    Now a real class, not just an abstract base.  Needed for operator
//    plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Brad Whitlock, Tue Oct 9 15:21:56 PST 2001
//    Added a method for handling tools.
//
//    Brad Whitlock, Mon Feb 11 14:15:53 PST 2002
//    Added a method for accessing operator attributes.
//
//    Brad Whitlock, Fri Feb 22 17:25:21 PST 2002
//    I removed the engine proxy argument from ExecuteEngineRPC.
//
//    Kathleen Bonnell, Wed Jun 5 13:34:53 PDT 2002 
//    Added method to retrieve the name of the operator. 
//
//    Eric Brugger, Wed Jan 15 15:44:16 PST 2003
//    Modify the plot data member so that it is not const.
//
//    Brad Whitlock, Thu Apr 10 10:58:58 PDT 2003
//    I added Removeable, Moveable, and AllowsSubsequentOperators.
//
//    Jeremy Meredith, Wed May 21 13:08:31 PDT 2003
//    Added NeedsRecalculation.
//
//    Brad Whitlock, Wed Jul 16 17:08:10 PST 2003
//    Added GetPluginID, CreateNode, SetFromNode.
//
//    Kathleen Bonnell, Thu Sep 11 11:37:51 PDT 2003
//    Added fromDefault arg to constructor.
//
//    Brad Whitlock, Fri Apr 2 11:04:16 PDT 2004
//    I added a copy constructor and a method to set the plot pointer
//    for the operator.
//
//    Brad Whitlock, Mon Feb 12 17:49:49 PST 2007
//    Added ViewerBase base class.
//
//    Jeremy Meredith, Tue Feb 19 14:23:39 EST 2008
//    Allow operators to create new variables.
//
//    Jeremy Meredith, Tue Feb 19 15:39:42 EST 2008
//    Allow operators to construct full expressions for their new variables.
//
// ****************************************************************************

class VIEWER_API ViewerOperator : public ViewerBase
{
  public:
    ViewerOperator(const int type_, 
                   ViewerOperatorPluginInfo *viewerPluginInfo_,
                   ViewerPlot *plot_, const bool fromDefault);
    ViewerOperator(const ViewerOperator &);
    virtual ~ViewerOperator();

    int  GetType() const;
    const char *GetPluginID() const;
    const char *GetName() const;

    void SetPlot(ViewerPlot *p);
    void SetClientAttsFromOperator();
    void SetOperatorAttsFromClient();
    bool SetOperatorAtts(const AttributeSubject *toolAtts);
    const AttributeSubject *GetOperatorAtts() const;

    bool Removeable() const;
    bool Moveable() const;
    bool AllowsSubsequentOperators() const;

    bool NeedsRecalculation() const;
    bool ExecuteEngineRPC() const;

    ExpressionList *GetCreatedVariables(const char *mesh);

    void CreateNode(DataNode *);
    void SetFromNode(DataNode *, const std::string &);

  protected:
    ViewerPlot                   *plot;
    
  private:
    bool                          needsRecalculation;
    int                           type;
    ViewerOperatorPluginInfo     *viewerPluginInfo;
    AttributeSubject             *operatorAtts;
    avtPluginFilter              *avtfilter;

};

#endif
