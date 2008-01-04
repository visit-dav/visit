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
//                                 Pick.h                                    //
// ************************************************************************* //

#ifndef PICK__H
#define PICK__H
#include <viswindow_exports.h>
#include <queue>


class VisWindowInteractorProxy;

#include <VisitInteractor.h>


// ****************************************************************************
//  Class: Pick
//
//  Purpose:
//      Defines what Visit's  Pick interactions should look like.  
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:47:00 PST 2002
//    Comply with new interface from base class for better buttonpress control.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments from StartLeftButtonAction, in order to match
//    vtk's new interactor api.
//
//    Kathleen Bonnell, Thu Jan 12 14:03:39 PST 2006 
//    Renamed from Pick3D.  Added private members picking, handlingCache,
//    pickCache and methods HandlePickCache and DoPick.
//
//    Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006 
//    Added EndLeftButtonAction. 
// 
// ****************************************************************************

class VISWINDOW_API Pick : public VisitInteractor
{
  public:
                        Pick(VisWindowInteractorProxy &);
 
    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();

  private:
    bool                picking;
    bool                handlingCache;
    std::queue<int>     pickCache;
    void                HandlePickCache();
    void                DoPick(int x, int y);
};


#endif


