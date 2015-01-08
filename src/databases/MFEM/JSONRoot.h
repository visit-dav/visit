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

// ************************************************************************* //
//                                 JSONRoot.h                                //
// ************************************************************************* //

#include <string>
#include <vector>
#include <map>
#include <sstream>


// ****************************************************************************
//  Method: avtMFEMFileFormat::JSONRootPath
//
//  Purpose:
//   Holds a file system or in db path. Will domain id to file mapping.
// 
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
class JSONRootPath
{
    public:
                 JSONRootPath();
    virtual     ~JSONRootPath();
    
    std::string  Expand(int domain=0) const;
    std::string  Get() const { return path;}
    void         Set(const std::string &path);
    
    private:
        std::string path;
            
};

// ****************************************************************************
//  Method: avtMFEMFileFormat::JSONRootEntry
//
//  Purpose: 
//   Used to hold a path and collection of tags (key, value strings pairs).
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
class JSONRootEntry
{
    public:
                   JSONRootEntry();
    virtual       ~JSONRootEntry();

    JSONRootPath  &Path();
    
    void           Tags(std::vector<std::string> &tag_names) const;
    std::string   &Tag(const std::string &tag_name);

    bool           HasTag(const std::string &tag_name) const;

    private:
        JSONRootPath path;
        std::map<std::string,std::string> tags;
};

// ****************************************************************************
//  Method: avtMFEMFileFormat::JSONRootDataSet
//
//  Purpose: 
//   Holds meta data for dataset (mesh + fields).
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
class JSONRootDataSet
{
    public:
                        JSONRootDataSet();
        virtual        ~JSONRootDataSet();
        
        int             NumberOfDomains() const;
        void            SetNumberOfDomains(int ndomains);    
        
        int             Cycle() const;
        void            SetCycle(int value);
        bool            HasCycle() const;        

        double          Time() const;
        void            SetTime(double value);
        bool            HasTime() const;        
        

        JSONRootEntry  &Mesh();          
        
       
        void            Fields(std::vector<std::string> &field_names) const;
        JSONRootEntry  &Field(const std::string &field_name); 

    private:
        // TODO: uint64 ...
        int                                 ndomains;
        bool                                validCycle;
        int                                 cycle;
        bool                                validTime;
        double                              timev;
        
        JSONRootEntry                       mesh;
        std::map<std::string,JSONRootEntry> fields;
};

// ****************************************************************************
//  Method: avtMFEMFileFormat::JSONRoot
//
//  Purpose:
//   Holds meta data for mulitple named datasets.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
//  Modifications:
//   Cyrus Harrison, Wed Sep 24 10:47:00 PDT 2014
//   Added helper for abs path logic.
//
// ****************************************************************************
class JSONRoot
{
public:

                     JSONRoot();
                     JSONRoot(const std::string &json_root);
    virtual         ~JSONRoot();
    
    void             DataSets(std::vector<std::string> &dset_names) const;
    JSONRootDataSet &DataSet(const std::string &dset_name);

    int              NumberOfDataSets() const;
    std::string      ToJson();
    void             ToJson(std::ostringstream &oss);

  private:
    void             ParseJSON(const std::string &json_root);
    std::string      ResolveAbsolutePath(const std::string &root_dir,const std::string &file_path);
    
    std::map<std::string,JSONRootDataSet> dsets;
    
};

