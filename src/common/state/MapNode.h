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

#ifndef MAP_NODE_H
#define MAP_NODE_H

#include <state_exports.h>
#include <XMLNode.h>
#include <Variant.h>
#include <map>

// ****************************************************************************
//  Class:  MapNode
//
//  Purpose:
//    Provides a nested map type. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
//  Modifications:
//
// ****************************************************************************

class STATE_API MapNode : public Variant
{
  public:
    MapNode();
    MapNode(const MapNode&);
    MapNode(const XMLNode&);
    MapNode(const XMLNode*);
    MapNode  &operator=(const MapNode&);
    MapNode  &operator=(bool);
    MapNode  &operator=(char);
    MapNode  &operator=(unsigned char);
    MapNode  &operator=(const char *); // interp as string
    MapNode  &operator=(int);
    MapNode  &operator=(long);
    MapNode  &operator=(float);
    MapNode  &operator=(double);
    MapNode  &operator=(const std::string &);
    MapNode  &operator=(const boolVector &);
    MapNode  &operator=(const charVector &);
    MapNode  &operator=(const unsignedCharVector &);
    MapNode  &operator=(const intVector &);
    MapNode  &operator=(const longVector &);
    MapNode  &operator=(const floatVector &);
    MapNode  &operator=(const doubleVector &);
    MapNode  &operator=(const stringVector &);
    virtual  ~MapNode();
    
    MapNode             &operator[](const std::string &);
    MapNode             *GetEntry(const std::string &);
    const MapNode       *GetEntry(const std::string &) const;
    
    
    void                 RemoveEntry(const std::string &);
    bool                 HasEntry(const std::string &) const;
    void                 GetEntryNames(stringVector &) const;
    int                  GetNumEntries() const {return entries.size();}
    
    virtual std::string  ToXML() const;
    virtual XMLNode      ToXMLNode() const;
    
 private:
    void  SetValue(const XMLNode &);
    std::map<std::string,MapNode> entries;  
};

#endif

