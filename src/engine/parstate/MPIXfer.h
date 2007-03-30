#ifndef MPI_XFER_H
#define MPI_XFER_H
#include <engine_parstate_exports.h>
#include <Xfer.h>

// *******************************************************************
// Class: MPIXfer
//
// Purpose:
//   Observes multiple AttributeSubject objects. This class is
//   responsible for writing AttributeSubject objects onto a Connection.
//   The difference between this and Xfer is that before an object is
//   told to read its connection in the Process method, the message is
//   broadcast to other processes on the MPI world communicator.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 10:15:20 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Thu Sep 21 22:19:48 PDT 2000
//    Added override for Xfer::Update.
//
//    Brad Whitlock, Fri Mar 16 10:52:06 PDT 2001
//    Added override of Xfer::ReadHeader.
//
//    Jeremy Meredith, Fri Sep 21 14:43:21 PDT 2001
//    Added override of SendInterruption.
//
// *******************************************************************

class ENGINE_PARSTATE_API MPIXfer : public Xfer
{
public:
    MPIXfer();
    virtual ~MPIXfer();
    virtual void Process();
    virtual void Update(Subject*);
    virtual void SendInterruption();

    void SetEnableReadHeader(bool val);
protected:
    bool ReadHeader();
private:
    bool enableReadHeader;
    int  readsSinceReadHeaderDisabled;
};

#endif
