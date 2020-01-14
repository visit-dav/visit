// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_RPC_H
#define VISIT_RPC_H
#include <state_exports.h>
#include <AttributeSubject.h>
#include <string>


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Feb  8 10:22:45 PST 2007
//    Make sure that MaxStageNum is never less than CurStageNum.
//
//    Brad Whitlock, Fri Dec  7 11:57:01 PST 2007
//    Added TypeName overrides.
//
// ****************************************************************************

class Xfer;

//-----------------------------------------------------------------------------
//  Class:  VisItRPC
//-----------------------------------------------------------------------------
class STATE_API VisItRPC : public AttributeSubject
{
public:
    // enum: status
    enum status
    {
        none,
        incomplete,
        complete,
        error,
        warning,
        abort
    };

    // Class:  RPCReply
    class STATE_API RPCReply : public AttributeSubject
    {
    public:
        RPCReply(int s, AttributeSubject *d);
        ~RPCReply();

        virtual void      SelectAll();

        void              SetStatus(int s);
        int               GetStatus() const;

        void              SetMessage(const std::string &msg);
        const std::string  &Message() const;

        void              SetExceptionType(const std::string &type);
        const std::string  &GetExceptionType() const;

        void              SetData(AttributeSubject *d);
        AttributeSubject *GetData();

        void              SetRPCName(const std::string &);
        virtual const std::string TypeName() const;

    private:
        int               status;
        std::string       message;
        std::string       type;
        AttributeSubject *data;

        // non-transmissible
        std::string       rpcName;
    };

public:
    VisItRPC(const char *, int s=none, AttributeSubject *d=NULL);
    virtual ~VisItRPC();

    int       GetStatus() const;
    const std::string &Message() const;
    const std::string &GetExceptionType() const;
    RPCReply *GetReply();

    virtual void      SetXfer(Xfer *x);

    void SendAbort();
    void SendError();
    void SendError(const std::string &msg);
    void SendError(const std::string &msg, const std::string &etype);
    void SendWarning(const std::string &msg);

    virtual const std::string TypeName() const
    {
        return "VisItRPC";
    }
protected:
    virtual void Execute() = 0;
    void SendReply(int s, AttributeSubject *d);
    void RecvReply();

    RPCReply  reply;
    Xfer     *xfer;
};

//-----------------------------------------------------------------------------
//  Class:  NonBlockingRPC
//-----------------------------------------------------------------------------
class STATE_API NonBlockingRPC : public VisItRPC
{
public:
    // Class:  CompletionData
    class STATE_API CompletionData : public AttributeSubject
    {
    private:
        int         percent;
        int         replyLen;
        int         curStageNum;
        int         maxStageNum;
        std::string curStageName;
        AttributeSubject *replyData;
    public:
        CompletionData() : AttributeSubject("iiiisa"), percent(0), replyLen(0),
                           curStageNum(0), maxStageNum(0), curStageName(),
                           replyData(NULL){ };
        virtual ~CompletionData()                               { };
        void SetPercent(int p)      { percent=p;      Select(0,&percent); };
        int  GetPercent() const     { return percent;        };
        void SetReplyLen(int l)     { replyLen=l;     Select(1,&replyLen); };
        int  GetReplyLen() const    { return replyLen;       };
        void SetCurStageNum(int n)  { curStageNum=n;  Select(2,&curStageNum); };
        void SetCurStageNumWithoutSending(int n)  { curStageNum=n; };
        int  GetCurStageNum() const { return curStageNum;        };
        void SetMaxStageNum(int n)  { maxStageNum=n;  Select(3,&maxStageNum); };
        int  GetMaxStageNum() const { return (maxStageNum > curStageNum ?
                                              maxStageNum : curStageNum+1);   };
        void SetCurStageName(const std::string &s)
                                    { curStageName=s; Select(4,&curStageName);};
        std::string GetCurStageName() const
                                    { return curStageName;        };

        void SetReplyData(AttributeSubject *rd)
                   { replyData = rd; if(replyData) Select(5,replyData); };

        AttributeSubject *GetReplyData() const { return replyData; };

        void SelectAll()
        {
            Select(0,&percent);
            Select(1,&replyLen);
            Select(2,&curStageNum);
            Select(3,&maxStageNum);
            Select(4,&curStageName);
            Select(5,replyData);
        };

        virtual const std::string TypeName() const
        {
            return "NonBlockingRPC::CompletionData";
        }
    };
public:
    NonBlockingRPC(const char *fmt, AttributeSubject *d = NULL);
    virtual ~NonBlockingRPC();
    int         GetPercent() const;
    int         GetReplyLen() const;
    int         GetCurStageNum() const;
    int         GetMaxStageNum() const;
    std::string GetCurStageName() const;
    AttributeSubject *GetReplyData() const;

    void SendMaxStages(int n);
    void SendStage(int n, const std::string &s);
    void SendPercent(int p);
    void SendStatus(int percent, int curStage, const std::string &curStageName,
                    int maxStages);
    void SendReply(int l);
    void SendReply(AttributeSubject *rd=NULL);
    void SetCurStageNum(int n) { completion.SetCurStageNumWithoutSending(n); };

    void RecvReply();

    virtual const std::string TypeName() const
    {
        return "NonBlockingRPC";
    }
protected:
    virtual void Execute();
private:
    CompletionData completion;
};

//-----------------------------------------------------------------------------
//  Class:  BlockingRPC
//-----------------------------------------------------------------------------
class STATE_API BlockingRPC : public VisItRPC
{
public:
    BlockingRPC(const char *fmt, AttributeSubject *d=NULL);
    virtual ~BlockingRPC();
    void SendReply(AttributeSubject *d=NULL);

    virtual const std::string TypeName() const
    {
        return "BlockingRPC";
    }
protected:
    virtual void Execute();
};

#endif
