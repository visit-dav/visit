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
#include <AttributeSubjectSerialize.h>
#include <Connection.h>
#include <SocketConnection.h>
#include <MapNode.h>
#include <JSONNode.h>
#include <AttributeSubject.h>

#include <stdlib.h>
#if defined(_WIN32)
#include <win32commhelpers.h>
#else
#include <sys/socket.h>
#endif

AttributeSubjectSerialize::AttributeSubjectSerialize() : conn(NULL)
{
}

AttributeSubjectSerialize::~AttributeSubjectSerialize()
{
}

void
AttributeSubjectSerialize::SetConnection(Connection *c)
{
    conn = c;
}

// ****************************************************************************
// Method: AttributeSubjectSerialize::Fill
//
// Purpose:
//   Read an AttributeSubject from a connection
//
// Arguments:
// 
//
// Returns:    
//
// Note:       
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:22:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AttributeSubjectSerialize::Fill()
{
    if(conn != NULL)
    {
        if(conn->GetDestinationFormat().Format == TypeRepresentation::ASCIIFORMAT)
        {
            std::string xmlString = "";

            char tmp[1001]; //leave 1 for null termination//

            int amountRead = 0;
            do
            {

    #if defined(_WIN32)
                int amountRead = recv(conn->GetDescriptor(), (char FAR *)tmp, 1000, 0);
                if(amountRead == SOCKET_ERROR)
                {
                    LogWindowsSocketError("SocketConnection", "Fill");
                    if(WSAGetLastError() == WSAEWOULDBLOCK)
                        return -1;
                }
    #else
                amountRead = recv(conn->GetDescriptor(), (void *)tmp, 1000, 0);
    #endif

                if(amountRead > 0)
                {
                    //zeroesRead = 0;
                    tmp[amountRead] = 0;
                    xmlString += tmp;
                }

                //++zeroesRead;

                // If we have had a certain number of zero length reads in a row,
                // assume the connection died.
//                if(zeroesRead > 100)
//                {
//                     EXCEPTION0(LostConnectionException);
//                }
            }while(amountRead == 1000); //if it gets entire list..

            //std::cout << "raw string: " << xmlString << std::endl;

            //buffer.clear();

            size_t bytes = 0;
            while(xmlString.size() > 0)
            {
                JSONNode node;
                size_t amt = node.Parse(xmlString);

                //std::cout << "message processing: " << node.ToString() << " " << amt << " " << xmlString.size() << std::endl;

                int guido = node["id"].GetInt();

                JSONNode contents = node["contents"];
                JSONNode metadata = node["metadata"];

                bytes += Write(guido, contents, metadata); //Write(guido,&mapnode); //,&metadata["data"]

                if(amt >= xmlString.size())
                    break;

                xmlString = xmlString.substr(amt);
            }
            return bytes;
        }
        else
        {
            //return conn->Fill();
        }
    }

    return 0;
}

// ****************************************************************************
// Method: AttributeSubjectSerialize::Flush
//
// Purpose:
//   Write an AttributeSubject to a connection.
//
// Arguments:
//   subject : The subject to write.
//
// Returns:    
//
// Note:       
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:22:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
AttributeSubjectSerialize::Flush(AttributeSubject *subject)
{
    if(conn == NULL)
        return;

    if(conn->GetDestinationFormat().Format == TypeRepresentation::BINARYFORMAT)
    {
        // Write out the subject's guido and message size.
        conn->WriteInt(subject->GetGuido());
        int sz = subject->CalculateMessageSize(*conn);
        conn->WriteInt(sz);

        subject->Write(*conn);
        conn->Flush();
    }
    else
    {
//        std::cout << subject->TypeName() << " "
//                  << subject->CalculateMessageSize(*this)
//                  << std::endl;

        AttributeSubjectSocketConnection* ascomm = dynamic_cast<AttributeSubjectSocketConnection*>(conn);

        if(ascomm) {
            ascomm->FlushAttr(subject);
        }
        else {

            SocketConnection* sconn = NULL;

            sconn = dynamic_cast<SocketConnection*>(conn);
            bool fixBufferMode = false;
            if(sconn) {
                fixBufferMode = sconn->GetFixedBufferMode();
                sconn->SetFixedBufferMode(false);
            }

            conn->Reset();

            JSONNode startTag;
            startTag[0] = "startTag";
            const std::string& start = startTag.ToString().c_str();

            JSONNode endTag;
            endTag[0] = "endTag";
            const std::string& end = endTag.ToString().c_str();

            if(subject->GetSendMetaInformation())
            {
                conn->Append((const unsigned char *)start.c_str(), start.size());

                const std::string& api = AttributeSubjectSocketConnection::serializeMetaData(subject);
                conn->Append((const unsigned char *)api.c_str(), api.size());

                conn->Append((const unsigned char *)end.c_str(), end.size());

            }

            conn->Append((const unsigned char *)start.c_str(), start.size());

            const std::string& output = AttributeSubjectSocketConnection::serializeAttributeSubject(subject);
            conn->Append((const unsigned char *)output.c_str(), output.size());

            conn->Append((const unsigned char *)end.c_str(), end.size());

            conn->Flush();

            if(sconn) {
                sconn->SetFixedBufferMode(fixBufferMode);
            }
        }
    }
}

// ****************************************************************************
// Method: AttributeSubjectSerialize::Write
//
// Purpose:
//   
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:22:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AttributeSubjectSerialize::Write(int id, MapNode *mapnode)
{
    size_t fillSize = 0;

    //std::cout << key << " " << id << std::endl;
    int totalSize = 0;
    int totalLen = 0;

    WriteToBuffer(mapnode,false,id,totalLen,totalSize);

    int attrSize = (totalLen < 256) ?   conn->GetSourceFormat().CharSize():
                                        conn->GetSourceFormat().IntSize();

    totalSize += attrSize;
    totalSize += (totalLen*attrSize);

    //std::cout << "TotalSize: " << id << " " << totalLen << " " << totalSize << std::endl;

    /// actual write
    conn->WriteInt(id);
    conn->WriteInt(totalSize);
    totalLen < 256 ? conn->WriteUnsignedChar((unsigned char)totalLen) : conn->WriteInt(totalLen);

    WriteToBuffer(mapnode,true,id,totalLen,totalSize);

    /// fileSize is Size of message + 2 ints
    fillSize += totalSize + (conn->GetSourceFormat().IntSize()*2);

    return fillSize;
}

// ****************************************************************************
// Method: AttributeSubjectSerialize::WriteToBuffer
//
// Purpose:
//   
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:22:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
AttributeSubjectSerialize::WriteToBuffer(MapNode *mapnode,
                                bool write,
                                int id,
                                int& totalLen,
                                int &totalSize)
{
    /// loop through mapnode
    /// don't do anything for pure mapnode structures..
    if(mapnode->Type() != 0)
    {
        if(write)
            (totalLen < 256)? conn->WriteUnsignedChar((unsigned char)id) : conn->WriteInt(id);
        else
             ++totalLen;

        if(mapnode->Type() == MapNode::BOOL_TYPE)
        {
            if(write)
                conn->WriteChar( mapnode->AsBool()? 1 : 0);
            else
                totalSize += conn->GetSourceFormat().CharSize();
        }
        else if(mapnode->Type() == MapNode::CHAR_TYPE)
        {
            if(write)
                conn->WriteChar( mapnode->AsChar() );
            else
                totalSize += conn->GetSourceFormat().CharSize();
        }
        else if(mapnode->Type() == MapNode::UNSIGNED_CHAR_TYPE)
        {
            if(write)
                conn->WriteUnsignedChar( mapnode->AsUnsignedChar() );
            else
                totalSize += conn->GetSourceFormat().CharSize();
        }
        else if(mapnode->Type() == MapNode::INT_TYPE)
        {
            if(write)
                conn->WriteInt( mapnode->AsInt() );
            else
                totalSize += conn->GetSourceFormat().IntSize();
        }
        else if(mapnode->Type() == MapNode::FLOAT_TYPE)
        {
            if(write)
                conn->WriteFloat( mapnode->AsFloat() );
            else
                totalSize += conn->GetSourceFormat().FloatSize();
        }
        else if(mapnode->Type() == MapNode::DOUBLE_TYPE)
        {
            if(write)
                conn->WriteDouble( mapnode->AsDouble() );
            else
                totalSize += conn->GetSourceFormat().DoubleSize();
        }
        else if(mapnode->Type() == MapNode::LONG_TYPE)
        {
            if(write)
                conn->WriteLong( mapnode->AsLong() );
            else
                totalSize += conn->GetSourceFormat().LongSize();
        }
        else if(mapnode->Type() == MapNode::STRING_TYPE)
        {
            if(write)
                conn->WriteString( mapnode->AsString() );
            else
                totalSize += conn->GetSourceFormat().CharSize()*(mapnode->AsString().size() + 1);
        }
        else if(mapnode->Type() == MapNode::BOOL_VECTOR_TYPE)
        {
            const boolVector& vec = mapnode->AsBoolVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteChar( vec[i]? 1 : 0);
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().CharSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::CHAR_VECTOR_TYPE)
        {
            const charVector& vec = mapnode->AsCharVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteChar( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().CharSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::UNSIGNED_CHAR_VECTOR_TYPE)
        {
            const unsignedCharVector& vec = mapnode->AsUnsignedCharVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteUnsignedChar( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().CharSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::INT_VECTOR_TYPE)
        {
            const intVector& vec = mapnode->AsIntVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteInt( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().IntSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::FLOAT_VECTOR_TYPE)
        {
            const floatVector& vec = mapnode->AsFloatVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteFloat( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().FloatSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::DOUBLE_VECTOR_TYPE)
        {
            const doubleVector& vec = mapnode->AsDoubleVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteDouble( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().DoubleSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::LONG_VECTOR_TYPE)
        {
            const longVector& vec = mapnode->AsLongVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteLong( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().LongSize());
                }
            }
        }
        else if(mapnode->Type() == MapNode::STRING_VECTOR_TYPE)
        {
            const stringVector& vec = mapnode->AsStringVector();
            //std::cout << "String vector type : " << vec.size() << std::endl;
            //if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteString( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    for(size_t i = 0; i < vec.size(); ++i)
                        totalSize += (vec[i].size() + 1)*conn->GetSourceFormat().CharSize();
                }
            }
        }
        else
        {
            std::cout << "not handled: "
                      << mapnode->TypeName()
                      << std::endl;
        }
    }

    stringVector names;
    mapnode->GetEntryNames(names);
    for(size_t i = 0; i < names.size(); ++i)
    {
        //std::cout << "** " << names[i] << std::endl;
        std::string& name = names[i];
        MapNode* mc = mapnode->GetEntry(names[i]);

        if(mc->Type() == 0)
        {
            WriteToBuffer(mc,write,0,totalLen,totalSize);
        }
        else
        {
            int id = atoi(name.c_str());//data->operator [](name).GetInt();
            WriteToBuffer(mc,write,id,totalLen,totalSize);
        }
    }
}

// ****************************************************************************
// Method: AttributeSubjectSerialize::Write
//
// Purpose:
//   
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:22:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
AttributeSubjectSerialize::Write(int id, JSONNode& node, JSONNode& metadata)
{
    size_t fillSize = 0;

    //std::cout << key << " " << id << std::endl;
    int totalSize = 0;
    int totalLen = 0;

    WriteToBuffer(node,metadata,false,id,totalLen,totalSize);

    int attrSize = (totalLen < 256) ?   conn->GetSourceFormat().CharSize():
                                        conn->GetSourceFormat().IntSize();

    totalSize += attrSize;
    totalSize += (totalLen*attrSize);

    //std::cout << "TotalSize: " << id << " " << totalLen << " " << totalSize << std::endl;

    /// actual write
    conn->WriteInt(id);
    conn->WriteInt(totalSize);
    totalLen < 256 ? conn->WriteUnsignedChar((unsigned char)totalLen) : conn->WriteInt(totalLen);

    WriteToBuffer(node,metadata,true,id,totalLen,totalSize);

    /// fileSize is Size of message + 2 ints
    fillSize += totalSize + (conn->GetSourceFormat().IntSize()*2);

    return fillSize;
}

// ****************************************************************************
// Method: AttributeSubjectSerialize::WriteToBuffer
//
// Purpose:
//   
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:22:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
AttributeSubjectSerialize::WriteToBuffer(const JSONNode &node,
                                const JSONNode &metadata,
                                bool write,
                                int id,
                                int& totalLen,
                                int &totalSize)
{
    int data_type = 0;

    if(metadata.GetType() == JSONNode::JSONINTEGER){
        data_type = metadata.GetInt();
    }
//    else
//    {
////        const std::string name = metadata.GetString();

////        data_type = isdigit(name[0]) ? atoi(name.c_str()) :
////                                       Variant::NameToTypeID(name);
//        //std::cerr << "entry not handled" << std::endl;
//        //return;
//    }

    /// loop through mapnode
    /// don't do anything for pure mapnode structures..
    if(data_type != 0)
    {
        if(data_type == MapNode::MapNodeType)
            return; /// Does not handle mapnode yet..

        if(write)
            (totalLen < 256)? conn->WriteUnsignedChar((unsigned char)id) : conn->WriteInt(id);
        else
             ++totalLen;

        if(data_type == MapNode::BOOL_TYPE)
        {
            if(write)
                conn->WriteChar( node.GetBool()? 1 : 0);
            else
                totalSize += conn->GetSourceFormat().CharSize();
        }
        else if(data_type == MapNode::CHAR_TYPE)
        {
            if(write)
                conn->WriteChar( node.AsChar() );
            else
                totalSize += conn->GetSourceFormat().CharSize();
        }
        else if(data_type == MapNode::UNSIGNED_CHAR_TYPE)
        {
            if(write)
                conn->WriteUnsignedChar( node.AsUnsignedChar() );
            else
                totalSize += conn->GetSourceFormat().CharSize();
        }
        else if(data_type == MapNode::INT_TYPE)
        {
            if(write)
                conn->WriteInt( node.GetInt() );
            else
                totalSize += conn->GetSourceFormat().IntSize();
        }
        else if(data_type == MapNode::FLOAT_TYPE)
        {
            if(write)
                conn->WriteFloat( node.GetFloat() );
            else
                totalSize += conn->GetSourceFormat().FloatSize();
        }
        else if(data_type == MapNode::DOUBLE_TYPE)
        {
            if(write)
                conn->WriteDouble( node.GetDouble() );
            else
                totalSize += conn->GetSourceFormat().DoubleSize();
        }
        else if(data_type == MapNode::LONG_TYPE)
        {
            if(write)
                conn->WriteLong( node.GetLong() );
            else
                totalSize += conn->GetSourceFormat().LongSize();
        }
        else if(data_type == MapNode::STRING_TYPE)
        {
            if(write)
                conn->WriteString( node.GetString() );
            else
                totalSize += conn->GetSourceFormat().CharSize()*(node.GetString().size() + 1);
        }
        else if(data_type == MapNode::BOOL_VECTOR_TYPE)
        {
            const boolVector& vec = node.AsBoolVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteChar( vec[i]? 1 : 0);
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().CharSize());
                }
            }
        }
        else if(data_type == MapNode::CHAR_VECTOR_TYPE)
        {
            const charVector& vec = node.AsCharVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteChar( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().CharSize());
                }
            }
        }
        else if(data_type == MapNode::UNSIGNED_CHAR_VECTOR_TYPE)
        {
            const unsignedCharVector& vec = node.AsUnsignedCharVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteUnsignedChar( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().CharSize());
                }
            }
        }
        else if(data_type == MapNode::INT_VECTOR_TYPE)
        {
            const intVector& vec = node.AsIntVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteInt( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().IntSize());
                }
            }
        }
        else if(data_type == MapNode::FLOAT_VECTOR_TYPE)
        {
            const floatVector& vec = node.AsFloatVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteFloat( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().FloatSize());
                }
            }
        }
        else if(data_type == MapNode::DOUBLE_VECTOR_TYPE)
        {
            const doubleVector& vec = node.AsDoubleVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteDouble( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().DoubleSize());
                }
            }
        }
        else if(data_type == MapNode::LONG_VECTOR_TYPE)
        {
            const longVector& vec = node.AsLongVector();

//            if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteLong( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    totalSize += (vec.size()*conn->GetSourceFormat().LongSize());
                }
            }
        }
        else if(data_type == MapNode::STRING_VECTOR_TYPE)
        {
            const stringVector& vec = node.AsStringVector();
            //std::cout << "String vector type : " << vec.size() << std::endl;
            //if(vec.size() > 0)
            {
                if(write)
                {
                    conn->WriteInt((int)vec.size());
                    for(size_t i = 0; i < vec.size(); ++i)
                        conn->WriteString( vec[i] );
                }
                else
                {
                    //std::cout << mapnode->TypeName() << " " << str << std::endl;
                    totalSize += conn->GetSourceFormat().IntSize();
                    for(size_t i = 0; i < vec.size(); ++i)
                        totalSize += (vec[i].size() + 1)*conn->GetSourceFormat().CharSize();
                }
            }
        }
        else if(data_type == MapNode::MapNodeType)
        {
            MapNode mapnode(node);

            if(write) {
                totalSize += mapnode.CalculateMessageSize(conn);
            }
            else {
                mapnode.Write(conn);
            }

            //WriteToBuffer(&mapnode, write, 0, totalLen, totalSize);
        }
        else
        {
            std::cout << "not handled: "
                      << node.ToString()
                      << std::endl;
        }
    }

    /// this
    if(data_type == 0 && node.GetType() == JSONNode::JSONOBJECT)
    {
        const JSONNode::JSONObject& object = node.GetJsonObject();
        const JSONNode::JSONObject& mobject = metadata.GetJsonObject();
        JSONNode::JSONObject::const_iterator itr  = object.begin();
        JSONNode::JSONObject::const_iterator mitr = mobject.begin();

        for(;itr != object.end(); ++itr,++mitr) {
            int cid = atoi(itr->first.c_str());
            WriteToBuffer(itr->second,mitr->second,
                          write,cid,totalLen,totalSize);
        }
    }
    else if(data_type == 0 && node.GetType() == JSONNode::JSONARRAY)
    {
        const JSONNode::JSONArray& array = node.GetArray();
        const JSONNode::JSONArray& marray = metadata.GetArray();

        for(size_t i = 0; i < array.size(); ++i)
            WriteToBuffer(array[i],marray[i],
                          write,i,totalLen,totalSize);
    }
}
