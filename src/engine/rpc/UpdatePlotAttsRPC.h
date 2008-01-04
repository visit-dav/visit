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

#ifndef UPDATE_PLOT_ATTS_RPC_H
#define UPDATE_PLOT_ATTS_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

class UpdatePlotAttsRPC;

// ****************************************************************************
//  Class:  PrepareUpdatePlotAttsRPC
//
//  Purpose:
//    Signals the name of the plot about to be created so that
//    the UpdatePlotAttsRPC has space to store the correct attributes.
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
class ENGINE_RPC_API PrepareUpdatePlotAttsRPC : public BlockingRPC
{
  public:
    PrepareUpdatePlotAttsRPC();
    ~PrepareUpdatePlotAttsRPC();

    virtual const std::string TypeName() const { return "PrepareUpdatePlotAttsRPC"; }

    void SetUpdatePlotAttsRPC(UpdatePlotAttsRPC*);
    UpdatePlotAttsRPC *GetUpdatePlotAttsRPC();

    void operator()(const std::string &n);
    void SelectAll();
    std::string GetID();
  private:
    std::string id;
    UpdatePlotAttsRPC *updatePlotAttsRPC;
};


// ****************************************************************************
//  Class:  UpdatePlotAttsRPC
//
//  Purpose:
//      Update plot attributs.
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 25 09:55:56 PDT 2002
//    Removed SetSocket.
//
// ****************************************************************************

class ENGINE_RPC_API UpdatePlotAttsRPC : public BlockingRPC
{
  public:
    UpdatePlotAttsRPC();
    virtual ~UpdatePlotAttsRPC();

    virtual const std::string TypeName() const { return "UpdatePlotAttsRPC"; }

    void operator()(const std::string&, int, const AttributeSubject*);

    virtual void SelectAll();

    std::string GetID();
    const AttributeSubject *GetAtts();
    int GetPlotIndex() { return index; };
    PrepareUpdatePlotAttsRPC &GetPrepareUpdatePlotAttsRPC();

    void SetAtts(AttributeSubject*);

    virtual void SetXfer(Xfer *x);

  private:
    AttributeSubject *atts;
    PrepareUpdatePlotAttsRPC prepareUpdatePlotAttsRPC;
    int index;
};

#endif
