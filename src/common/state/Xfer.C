#include <Xfer.h>
#include <AttributeSubject.h>
#include <VisItRPC.h>
#include <Connection.h>
#include <visitstream.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: Xfer::Xfer
//
// Purpose: 
//   Constructor for the Xfer class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:16:23 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Feb 27 11:07:00 PDT 2003
//   I initialized some new members related to special opcodes.
//
// ****************************************************************************

Xfer::Xfer() : SimpleObserver(), subjectList()
{
    input = output = NULL;

    opcode = 0; length = -1;
    haveStoredHeader = false;

    nextGuido = 0;
    nextSpecialOpcode = -2;
    specialOpcodeCallback = 0;
    specialOpcodeCallbackData = 0;
}

// ****************************************************************************
// Method: Xfer::~Xfer
//
// Purpose: 
//   Destructor for the Xfer class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:17:24 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

Xfer::~Xfer()
{
    std::vector<AttributeSubject*>::iterator pos;

    // Detach this observer from all of the subjects it's observing.
    for(pos = subjectList.begin(); pos != subjectList.end(); ++pos)
    {
        (*pos)->Detach(this);
    }
}

// ****************************************************************************
// Method: Xfer::Add
//
// Purpose: 
//   Adds a new AttributeSubject to the list of subjects it is
//   observing.
//
// Arguments:
//   subject: The subject that will be added to the list.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:19:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::Add(AttributeSubject *subject)
{
    // Subscribe to the subject
    subject->Attach(this);
    subjectList.push_back(subject);

    // Set the subject's Guido
    subject->SetGuido(GetNextGuido());
}

// ****************************************************************************
// Method: Xfer::Add
//
// Purpose: 
//   Adds a new RPC to the list of subjects it is observing.
//
// Arguments:
//   rpc: The rpc that will be added to the list.
//
// Returns:    
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   Fri Aug 11 13:47:56 PDT 2000
//
// Modifications:
//
// ****************************************************************************

void
Xfer::Add(VisItRPC *rpc)
{
    Add((AttributeSubject *)rpc);
    rpc->SetXfer(this);

    Add(rpc->GetReply());
}

// ****************************************************************************
// Method: Xfer::Remove
//
// Purpose: 
//   Removes a subject from the list of subjects the Xfer object is
//   observing.
//
// Arguments:
//   subject : The subject that is being removed from the list.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:20:12 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::Remove(AttributeSubject *subject)
{
    std::vector<AttributeSubject*>::iterator pos;

    // Erase all references to the subject.
    for(pos = subjectList.begin(); pos != subjectList.end(); )
    {
       if(*pos == subject)
           pos = subjectList.erase(pos);
       else
           ++pos;
    }

    // Detach from the subject
    subject->Detach(this);
}

// ****************************************************************************
// Method: Xfer::Update
//
// Purpose: 
//   Writes a subject to the Xfer object's output connection.
//   Depending on the type of output connection, this is mainly
//   used to send subjects to other processes.
//
// Arguments:
//    TheChangedSubject : The subject to write on the output connection.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:21:40 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Jul 25 12:16:17 PDT 2003
//   Added debug coding.
//
// ****************************************************************************

void
Xfer::Update(Subject *TheChangedSubject)
{
    if (output == NULL)
        return;

    AttributeSubject *subject = (AttributeSubject *)TheChangedSubject;

    // Write out the subject's guido and message size.
    output->WriteInt(subject->GetGuido());
    int sz = subject->CalculateMessageSize(*output);
    output->WriteInt(sz);

    debug5 << "Xfer::Update: Sending: opcode=" << subject->GetGuido()
           << ", len=" << sz
           << ", name=" << subject->TypeName().c_str() << endl;

    // Write the things about the subject that have changed onto the
    // output connection and flush it out to make sure it's sent.
    subject->Write(*output);
    output->Flush();
}

// ****************************************************************************
//  Method:  Xfer::SendInterruption
//
//  Purpose:
//    Send an interruption message to the remote connection.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  2, 2001
//
// ****************************************************************************

void
Xfer::SendInterruption()
{
    output->WriteInt(-1);
    output->WriteInt(0);
    output->Flush();
}

// ****************************************************************************
// Method: Xfer::SubjectRemoved
//
// Purpose: 
//   Removes a subject from the list of subjects that the Xfer object
//   is watching. This is called when the subject is destroyed. It
//   is a precaution against the Xfer trying to detach from a subject
//   that has been destroyed.
//
// Arguments:
//   TheRemovedSubject : The subject that was destroyed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 15:28:41 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::SubjectRemoved(Subject *TheRemovedSubject)
{
    std::vector<AttributeSubject*>::iterator pos;

    // Erase all references to the subject that was removed.
    for(pos = subjectList.begin(); pos != subjectList.end(); )
    {
       if(*pos == TheRemovedSubject)
           pos = subjectList.erase(pos);
       else
           ++pos;
    }
}

// ****************************************************************************
// Method: Xfer::ReadPendingMessages
//
// Purpose: 
//   This method reads messages encoded on the Xfer object's
//   input connection and copies them to another buffer.  It also
//   checks for pending Abort commands.
//
// Notes:      This function returns whether we've processed an interruption.
//             If we processed an interruption, we cleared pending commands
//             from the buffered input and returned true. Otherwise we return
//             false.
//
// Programmer: Jeremy Meredith
// Creation:   July  3, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 27 11:00:34 PDT 2003
//   I added support for special opcodes.
//
//   Jeremy Meredith, Tue Mar  4 13:10:25 PST 2003
//   Added length to the new buffer because MPIXfer needs it.
//
//   Brad Whitlock, Thu May 5 16:59:28 PST 2005
//   I changed the code so interrupt can be handled as a special opcode.
//
// ****************************************************************************

bool
Xfer::ReadPendingMessages()
{
    // While there are complete messages, read them.
    while(ReadHeader())
    {
        if (opcode > nextSpecialOpcode && opcode < 0)
        {
            if (opcode == -1)
                bufferedInput.Flush();

            // If the callback and the data were provided, call the callback
            // so it can process the user-defined opcode.
            if(specialOpcodeCallback != 0)
                specialOpcodeCallback(opcode, specialOpcodeCallbackData);

            if (opcode == -1)
                return true;

            continue;
        }

        bufferedInput.WriteInt(opcode);
        bufferedInput.WriteInt(length);
        for (int i=0; i<length; i++)
        {
            unsigned char tmp;
            input->Read(&tmp);
            bufferedInput.Write(tmp);
        }
    }

    return false;
}

// ****************************************************************************
// Method: Xfer::Process
//
// Purpose: 
//   This method processes messages encoded on the Xfer object's
//   input connection. It keeps going while there are complete
//   messages.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:23:57 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Tue Jul  3 15:02:12 PDT 2001
//    Made it read from the new buffer instead of the input connection.
//   
//    Jeremy Meredith, Tue Mar  4 13:10:25 PST 2003
//    Added length to the new buffer because MPIXfer needs it.
//
//    Brad Whitlock, Fri Jul 25 12:14:56 PDT 2003
//    Added debug messages and added code to skip unknown opcodes so we
//    don't hang as easily if the protocol gets messed up somehow.
//
// ****************************************************************************

void
Xfer::Process()
{
    ReadPendingMessages();

    // While there are complete messages, read and process them.
    while (bufferedInput.Size() > 0)
    {
        int     curOpcode;
        int     curLength;
        bufferedInput.ReadInt(&curOpcode);
        bufferedInput.ReadInt(&curLength);

        bool    bytesNeedToBeSkipped = true;
        if (curOpcode < subjectList.size())
        {
            if (subjectList[curOpcode])
            {
                debug5 << "Xfer::Process: Opcode=" << curOpcode
                    << ", len=" << curLength
                    << ", type="
                    << subjectList[curOpcode]->TypeName().c_str() << endl;

                // Read the object into its local copy.
                subjectList[curOpcode]->Read(bufferedInput);

                // Indicate that we want Xfer to ignore update messages if
                // it gets them while processing the Notify.
                SetUpdate(false);
                subjectList[curOpcode]->Notify();
                bytesNeedToBeSkipped = false;
            }
        }

        if (bytesNeedToBeSkipped)
        {
            debug1 << "Xfer::Process: Opcode " << curOpcode
                   << " is unknown! Skipping " << curLength << " bytes." << endl;
            unsigned char uchar;
            for (int i = 0; i < curLength; ++i)
                bufferedInput.Read(&uchar);
        }
    }
}

// *************************************************************************************
// Method: Xfer::ListObjects
//
// Purpose: 
//   Lists the names of the objects that are hooked up.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 22 16:48:28 PST 2002
//
// Modifications:
//   
// *************************************************************************************

void
Xfer::ListObjects()
{
    debug2 << "Xfer: Number of objects: " << subjectList.size() << endl;
    for(int i = 0; i < subjectList.size(); ++i)
        debug2 << "Object[" << i << "] = " << subjectList[i]->TypeName().c_str()
               << endl;
}

// ****************************************************************************
// Method: Xfer::ReadHeader
//
// Purpose: 
//   Checks the input connection to see if there is an entire message
//   waiting to be read. If there is, it strips the header and returns
//   true. Otherwise, don't strip the header and return false.
//
// Arguments:
//
// Returns:    
//   true if the message can be read
//   false if the message cannot be read.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 5 15:11:49 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Jul  3 15:17:24 PDT 2001
//    Changed check to allow empty messages.
//   
// ****************************************************************************

bool
Xfer::ReadHeader()
{
    if(input == NULL)
        return false;

    bool retval = false;

    // If the connection has more than the header in its buffer, try
    // reading the header. Also try if there is a stored header. That
    // means that we've tried to read for this operation before. Maybe
    // the message is all there this time.

    if((input->Size() >= (2 * sizeof(int))) || haveStoredHeader)
    {
        if(!haveStoredHeader)
        {
            input->ReadInt(&opcode);
            input->ReadInt(&length);
        }

        // Check the message size against the size of what's actually
        // arrived in the buffer.
        retval = (length <= input->Size());
        haveStoredHeader = !retval;
    }

    return retval;
}

// ****************************************************************************
// Method: Xfer::SetInputConnection
//
// Purpose: 
//   Sets the object's input connection. This is the connection used
//   to read messages for the Process method.
//
// Arguments:
//   conn : The connection to use for the input connection.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:26:01 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::SetInputConnection(Connection *conn)
{
    input = conn;
}

// ****************************************************************************
// Method: Xfer::SetOutputConnection
//
// Purpose: 
//   Sets the object's output connection. This is the connection used
//   by the Update method that writes an object onto the connection.
//
// Arguments:
//   conn : The connection to use for the output connection.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:26:53 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::SetOutputConnection(Connection *conn)
{
    output = conn;
}

// ****************************************************************************
// Method: Xfer::GetInputConnection
//
// Purpose: 
//   Gets the object's input connection. This is the connection used
//   to read messages for the Process method.
//
// Arguments:
//
// Returns:    Connection*
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   Fri Aug 11 14:55:38 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
Xfer::GetInputConnection() const
{
    return input;
}

// ****************************************************************************
// Method: Xfer::GetOutputConnection
//
// Purpose: 
//   Gets the object's output connection. This is the connection used
//   by the Update method that writes an object onto the connection.
//
// Arguments:
//
// Returns:    Connection*
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   Fri Aug 11 14:55:26 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
Xfer::GetOutputConnection() const
{
    return output;
}

// ****************************************************************************
// Method: Xfer::GetBufferedInputConnection
//
// Purpose: 
//   Returns a pointer to the buffered input connection.
//
// Returns:    Connection*
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 14 16:39:09 PST 2005
//
// Modifications:
//   
// ****************************************************************************

Connection *
Xfer::GetBufferedInputConnection()
{
    return &bufferedInput;
}

// ****************************************************************************
// Method: Xfer::GetNextGuido
//
// Purpose: 
//   Returns the next unique object identifier.
//
// Returns:    The index of the next object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:16:23 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
Xfer::GetNextGuido()
{
    return nextGuido++;
}

// ****************************************************************************
// Method: Xfer::CreateNewSpecialOpcode
//
// Purpose: 
//   Returns a new special opcode.
//
// Returns:    A new special opcode.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 11:13:28 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
Xfer::CreateNewSpecialOpcode()
{
    return nextSpecialOpcode--;
}

// ****************************************************************************
// Method: Xfer::SendSpecialOpcode
//
// Purpose: 
//   Sends a special opcode to the remote process.
//
// Arguments:
//   opcode : The opcode that will be sent.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 11:14:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::SendSpecialOpcode(int opcode)
{
    if(opcode < -1 && opcode > nextSpecialOpcode)
    {
        output->WriteInt(opcode);
        output->WriteInt(0);
        output->Flush();
    }
    else
    {
        debug1 << "Special opcode is out of range!";
    }
}

// ****************************************************************************
// Method: Xfer::SetupSpecialOpcodeHandler
//
// Purpose: 
//   Sets up a callback function to handle special opcodes.
//
// Arguments:
//   cb   : The address of the callback function.
//   data : user-defined data that will be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 11:14:54 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
Xfer::SetupSpecialOpcodeHandler(void (*cb)(int, void *), void *data)
{
    specialOpcodeCallback = cb;
    specialOpcodeCallbackData = data;
}
