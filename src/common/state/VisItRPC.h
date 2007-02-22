/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
    private:
        int               status;
        std::string       message;
        std::string       type ;
        AttributeSubject *data;
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
        CompletionData() : AttributeSubject("iiiisa"), percent(0),
                           curStageNum(0), maxStageNum(0), replyData(NULL){ };
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
protected:
    virtual void Execute();
};

#endif
