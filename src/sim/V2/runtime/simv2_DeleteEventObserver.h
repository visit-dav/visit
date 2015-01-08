/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#ifndef simV2_DeleteEventObserver_h
#define simV2_DeleteEventObserver_h

#include <vtkCommand.h>
#include <vtkDataArray.h>

// help that invokes callback passed through the simV2
// api when a VTK data array is deleted. This allows us
// to hold a reference to the data while VTK needs it,
// and release that reference when VTK is finished. Note:
// after the callback is invoked, this class un-register's
// itself so one should not explicitly Delete it.
class simV2_DeleteEventObserver : public vtkCommand
{
public:
    static simV2_DeleteEventObserver *New()
        { return new simV2_DeleteEventObserver; }

    // Install the observer on the DeleteEvent.
    void Observe(vtkDataArray *object, void(*callback)(void*), void*callbackData)
    {
        this->Id = object->AddObserver(vtkCommand::DeleteEvent, this);
        this->Callback = callback;
        this->CallbackData = callbackData;
    }
 
    // invoke the callback, ignore the event, delete the observer
    virtual void Execute(vtkObject *object, unsigned long, void *)
    {
        this->Callback(this->CallbackData);
        object->RemoveObserver(this->Id);
        this->UnRegister();
    }

protected:
  simV2_DeleteEventObserver() : Id(0), Callback(NULL), CallbackData(NULL) {}
  virtual ~simV2_DeleteEventObserver() {}

  unsigned long Id;
  void(* Callback)(void*);
  void *CallbackData;

private:
  void operator=(const simV2_DeleteEventObserver &); // not implemented
  simV2_DeleteEventObserver(const simV2_DeleteEventObserver &); // not implemented
};

#endif
