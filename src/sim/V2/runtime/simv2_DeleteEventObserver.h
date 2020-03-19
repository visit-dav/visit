// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef simV2_DeleteEventObserver_h
#define simV2_DeleteEventObserver_h

#include <vtkCommand.h>
#include <vtkDataArray.h>

// ****************************************************************************
// Class: smV2_DeleteEventObserver
//
// Purpose:
//  helper that invokes callback passed through the simV2
//  api when a VTK data array is deleted. This allows us
//  to hold a reference to the data while VTK needs it,
//  and release that reference when VTK is finished. Note:
//  after the callback is invoked, this class un-register's
//  itself so one should not explicitly Delete it.
//
// Programmer: Burlen Loring
// Creation:   2014-02-22 09:26:41
//
// Modifications:
//
// ****************************************************************************

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
