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

#ifndef XML_NODE_H
#define XML_NODE_H
#include <state_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <vector>
#include <map>


// ****************************************************************************
//  Class:  XMLNode
//
//  Purpose:
//    Very basic tree based xml generator and parser. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
//  Modifications:
//
// ****************************************************************************

class STATE_API XMLNode
{
  public:
    XMLNode();
    XMLNode(std::istream &);
    XMLNode(const std::string&);
    XMLNode(const XMLNode&);
    virtual  ~XMLNode();
    XMLNode  &operator=(const XMLNode&);    

    std::string  &Name() { return name;}
    std::string  &Text() { return text;}

    const std::string  &Name() const { return name;}
    const std::string  &Text() const { return text;}
    
    std::string  &Attribute(const std::string &att_name) 
                    { return attributes[att_name];}
    const std::string  &Attribute(const std::string &) const;
    bool     HasAttribute(const std::string &) const;
    bool     DeleteAttribute(const std::string &);
    void     GetAttributeNames(stringVector &) const;
    
    bool     HasChildren()    const { return children.size() > 0; }
    int      GetNumChildren() const { return children.size(); } 
    
    XMLNode *AddChild(const std::string  &);
    XMLNode *AddChild(const XMLNode &);
    
    XMLNode *GetChild(int index) const;
    XMLNode *TakeChild(int index);
    bool     DeleteChild(int index);
    
    int           FindIndex(const std::string &name,
                            int index_start = 0) const;
    std::string   ToString(const std::string &indent="") const;

 private:
    // init & destroy helpers
    void    Init(const XMLNode&);
    void    Cleanup();
    
    // xml parsing methods
    void    Parse(istream &);
    void    Parse(const std::string &);
    
    // Parse helpers
    static bool        Check(const std::string&,std::istream &);
    static void        Eat(const std::string&,std::istream &);
    static std::string GrabName(std::istream &);
    static std::string GrabString(unsigned char,std::istream &);
    static std::string GrabEntityValue(std::istream &);
    static std::string EscapeString(const std::string&);
    
    static std::string unset_str;
    
    std::string name;
    std::string text;
    std::map<std::string,std::string> attributes;
    std::vector<XMLNode *>  children;
    
};

#endif


