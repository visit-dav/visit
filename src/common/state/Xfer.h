#ifndef XFER_H
#define XFER_H
#include <state_exports.h>
#include <SimpleObserver.h>
#include <BufferConnection.h>
#include <vector>

class AttributeSubject;
class Connection;
class VisItRPC;

// ****************************************************************************
//  Class: Xfer
//
//  Purpose:
//    Observes multiple AttributeSubject objects. This class is
//    responsible for writing AttributeSubject objects onto a Connection.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jun 5 14:43:41 PST 2000
//
//  Modifications:
//    Brad Whitlock, Thu Aug 31 15:26:28 PST 2000
//    Added SubjectRemoved.
//
//    Jeremy Meredith, Tue Jul  3 15:02:57 PDT 2001
//    Added ReadPendingMessages and SendInterruption. 
//    Added buffered input.
//
//    Brad Whitlock, Fri Mar 22 16:47:23 PST 2002
//    Added a debugging function called ListObjects.
//
//    Brad Whitlock, Thu Feb 27 10:57:26 PDT 2003
//    Added support for special opcodes, which are opcodes that the user
//    can inject into the command stream to cause immediate effects.
//
//    Brad Whitlock, Thu Apr 14 16:37:24 PST 2005
//    Added a method to give access to the buffered input so we can store
//    objects directly in it.
//
// ****************************************************************************

class STATE_API Xfer : public SimpleObserver
{
public:
    Xfer();
    virtual ~Xfer();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void Add(AttributeSubject *);
    void Add(VisItRPC *);
    void Remove(AttributeSubject *);
    void SetInputConnection(Connection *);
    void SetOutputConnection(Connection *);
    Connection *GetInputConnection() const;
    Connection *GetOutputConnection() const;
    Connection *GetBufferedInputConnection();

    virtual void Process();
    virtual bool ReadPendingMessages();
    virtual void SendInterruption();

    int  CreateNewSpecialOpcode();
    void SendSpecialOpcode(int opcode);
    void SetupSpecialOpcodeHandler(void (*cb)(int, void *), void *data);

    void ListObjects();
protected:
    bool ReadHeader();
    int  GetNextGuido();
protected:
    std::vector<AttributeSubject *> subjectList;
    Connection *input, *output;
    BufferConnection bufferedInput;

    void (*specialOpcodeCallback)(int, void *);
    void *specialOpcodeCallbackData;

    // These deal with the input connection.
    int opcode, length;
    bool haveStoredHeader;

    // global unique identifier for object (guido)
    int nextGuido;
    int nextSpecialOpcode;
};

#endif
