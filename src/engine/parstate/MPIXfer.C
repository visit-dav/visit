/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifdef PARALLEL
#include <MPIXfer.h>
#include <avtParallel.h>
#include <parallel.h>
#include <BufferConnection.h>
#include <AttributeSubject.h>
#include <DebugStream.h>

#include <visitstream.h>

void (*MPIXfer::slaveProcessInstruction)() = NULL;

// ****************************************************************************
// Method: MPIXfer::MPIXfer
//
// Purpose: 
//   Constructor for the MPIXfer class.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:06:54 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

MPIXfer::MPIXfer() : Xfer()
{
    enableReadHeader = true;
    readsSinceReadHeaderDisabled = 0;
}

// ****************************************************************************
// Method: MPIXfer::~MPIXfer
//
// Purpose: 
//   Destructor for the MPIXfer class.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:07:16 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

MPIXfer::~MPIXfer()
{
}

// ****************************************************************************
// Method: MPIXfer::SetEnableReadHeader
//
// Purpose: 
//   Sets an internal flag that helps to determine the value returned by the
//   ReadHeader method.
//
// Arguments:
//   val : The value set into the enableReadHeader flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 16 10:56:19 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
MPIXfer::SetEnableReadHeader(bool val)
{
    enableReadHeader = val;
    if(!enableReadHeader)
        readsSinceReadHeaderDisabled = 0;
}

// ****************************************************************************
// Method: MPIXfer::ReadHeader
//
// Purpose: 
//   Reads the header information for a message.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 16 10:54:07 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

bool
MPIXfer::ReadHeader()
{
    bool retval;

    if(enableReadHeader)
        retval = Xfer::ReadHeader();
    else
    {
        retval = (readsSinceReadHeaderDisabled == 0);
        ++readsSinceReadHeaderDisabled;            
    }

    return retval;
}


// ****************************************************************************
//  Method:  Xfer::SendInterruption
//
//  Purpose:
//    Send an interruption message to the remote connection.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2001
//
//  Modifications:
//
//    Mark C. Miller, Thu Jun 10 09:08:18 PDT 2004
//    Added arg for interrupt message tag
//
// ****************************************************************************

void
MPIXfer::SendInterruption(int mpiInterruptTag)
{
    if (PAR_UIProcess())
    {
        // Do a nonblocking send to all processes to do it quickly
        int size = PAR_Size();
        unsigned char buf[1] = {255};
        MPI_Request *request = new MPI_Request[size];
        for (int i=1; i<size; i++)
        {
            MPI_Isend(buf, 1, MPI_CHAR, i, mpiInterruptTag, MPI_COMM_WORLD, &request[i]);
        }

        // Then wait for them all to read the command
        int ncomplete = 0;
        while (ncomplete < size-1)
        {
            for (int i=1; i<size; i++)
            {
                int flag;
                MPI_Status status;
                MPI_Test(&request[i], &flag, &status);
                if (flag)
                    ncomplete++;
            }
        }
    }
}


// ****************************************************************************
// Method: MPIXfer::Process
//
// Purpose: 
//   Reads the MPIXfer object's input connection and makes objects
//   update themselves when there are complete messages. In addition,
//   complete messages are broadcast to other processes on the
//   MPI world communicator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 10:20:48 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Fri Sep 21 14:31:22 PDT 2001
//    Added use of buffered input.
//
//    Jeremy Meredith, Tue Mar  4 13:10:25 PST 2003
//    Used the length from the new buffer.  Multiple non-blocking messages
//    mess up MPIXfer if we don't keep track of the length for each message.
//
//    Jeremy Meredith, Thu Oct  7 14:09:10 PDT 2004
//    Added callback so the master process could tell the slaves they
//    are about to receive data.  This was needed for running inside a
//    parallel simulation because slave processes need some way to know
//    that the next command coming is visit-specific.
//
// ****************************************************************************

void
MPIXfer::Process()
{
    ReadPendingMessages();

    // While there are complete messages, read and process them.
    while(bufferedInput.Size() > 0)
    {
        int curOpcode;
        int curLength;
        bufferedInput.ReadInt(&curOpcode);
        bufferedInput.ReadInt(&curLength);

        if(subjectList[curOpcode])
        {
            if(PAR_UIProcess())
            {
                int              i;
                unsigned char    c;
                char             *cptr;
                PAR_StateBuffer  buf;
                BufferConnection newInput;

                // Add the message's opcode and length into the buffer
                // that we'll broadcast to other processes so their
                // Xfer objects know what's up.
                buf.nbytes = 0;
                cptr = (char *)&curOpcode;
                for(i = 0; i < sizeof(int); ++i)
                    buf.buffer[buf.nbytes++] = *cptr++;
                cptr = (char *)&curLength;
                for(i = 0; i < sizeof(int); ++i)
                    buf.buffer[buf.nbytes++] = *cptr++;

                // Transcribe the message into a buffer that we'll
                // communicate to other processes and also a new
                // temporary connection that we'll feed to the interested
                // object.
                for(i = curLength; i > 0; --i)
                {
                    // Read a character from the input connection.
                    bufferedInput.ReadChar(&c);

                    buf.buffer[buf.nbytes++] = c;
                    newInput.WriteChar(c);

                    // Buffer or send it to other processes.
                    if(buf.nbytes >= INPUT_BUFFER_SIZE)
                    {
                        if (slaveProcessInstruction)
                            slaveProcessInstruction();
                        MPI_Bcast((void *)&buf, 1, PAR_STATEBUFFER,
                                  0, MPI_COMM_WORLD);

                        buf.nbytes = 0;
                    }
                }

                // Write last part of message if it exists.
                if(buf.nbytes > 0)
                {
                    if (slaveProcessInstruction)
                        slaveProcessInstruction();
                    MPI_Bcast((void *)&buf, 1, PAR_STATEBUFFER,
                              0, MPI_COMM_WORLD);
                }

                // Read the object from the newInput into its local copy.
                subjectList[curOpcode]->Read(newInput);
            }
            else
            {
                // Non-UI Processes can read the object from *input.
                subjectList[curOpcode]->Read(bufferedInput);
            }

            // Indicate that we want Xfer to ignore update messages if
            // it gets them while processing the Notify.
            SetUpdate(false);
            subjectList[curOpcode]->Notify();
        }
    }
}


// ****************************************************************************
// Method: MPIXfer::Update
//
// Purpose: 
//   Action performed when a subject changes
//
// Arguments:
//
// Returns:    
//
// Note:       This is the same as MPI::Update except that
//             only the UI process actually sends anything.
//
// Programmer: Jeremy Meredith
// Creation:   September 21, 2000
//
// Modifications:
//
// ****************************************************************************

void
MPIXfer::Update(Subject *TheChangedSubject)
{
    // We only do this until we know how to merge replies....
    if (!PAR_UIProcess())
        return;

    if (output == NULL)
        return;

    AttributeSubject *subject = (AttributeSubject *)TheChangedSubject;

    // Write out the subject's guido and message size.
    output->WriteInt(subject->GetGuido());
    output->WriteInt(subject->CalculateMessageSize(*output));

    // Write the things about the subject that have changed onto the
    // output connection and flush it out to make sure it's sent.
    subject->Write(*output);
    output->Flush();
}

// ****************************************************************************
//  Method:  MPIXfer::SetSlaveProcessInstructionCallback
//
//  Purpose:
//    Sets the callback for the master process to tell the slaves
//    they are about to receive data to process.
//
//  Arguments:
//    spi        the callback function
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  7, 2004
//
// ****************************************************************************
void
MPIXfer::SetSlaveProcessInstructionCallback(void (*spi)())
{
    slaveProcessInstruction = spi;
}


#endif
