/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
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
//                                 JSONRoot.C                                //
// ************************************************************************* //


#include <fstream>
#include "rapidjson/document.h"


#include "JSONRoot.h"

#include <iostream>
using namespace std;


// ****************************************************************************
//  Method: JSONRootPath Constructor
//
//  Purpose: Inits an instance of a JSONRootPath object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootPath::JSONRootPath()
:path("")
{
}

// ****************************************************************************
//  Method: JSONRootPath Destructor
//
//  Purpose: Cleans up an instance of a JSONRootPath object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootPath::~JSONRootPath()
{
}

// ****************************************************************************
//  Method: JSONRootPath::Expand
//
//  Purpose: Maps a doman id to is file system or in db path.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
std::string
JSONRootPath::Expand(int domain) const
{
    //
    // TODO: handle domain id mapping (likley printf style expansion)
    //
    
    return path;
}

// ****************************************************************************
//  Method: JSONRootPath::Set
//
//  Purpose: Sets the base pattern for a path.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
void 
JSONRootPath::Set(const string &path)
{
    this->path = path;
}

// ****************************************************************************
//  Method: JSONRootEntry Constructor
//
//  Purpose: Inits an instance of a JSONRootEntry object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootEntry::JSONRootEntry()
{

}

// ****************************************************************************
//  Method: JSONRootEntry Destructor
//
//  Purpose: Cleans up an instance of a JSONRootEntry object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootEntry::~JSONRootEntry()
{

}
// ****************************************************************************
//  Method: JSONRootEntry::Path
//
//  Purpose: Returns the path object associated with this entry.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootPath &
JSONRootEntry::Path()
{
    return path;
}

// ****************************************************************************
//  Method: JSONRootEntry::Tags
//
//  Purpose: Returns the names of the tags associated with this entry.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
void
JSONRootEntry::Tags(std::vector<std::string> &tag_names) const
{
    tag_names.clear();
    for(map<string,string>::const_iterator itr = tags.begin();
        itr!=tags.end();
        itr++)
        {
            tag_names.push_back(itr->first);
        }
}

// ****************************************************************************
//  Method: JSONRootEntry::HasTag
//
//  Purpose: Returns if this entry has a tag with the given name.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
bool
JSONRootEntry::HasTag(const std::string &tag_name) const
{
  
   return (tags.find(tag_name) != tags.end());
}

// ****************************************************************************
//  Method: JSONRootEntry::Tag
//
//  Purpose:
//     Returns the tag with the given name. 
//     The tag is created if it doesn't already exist.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
std::string &
JSONRootEntry::Tag(const std::string &tag_name)
{
    return tags[tag_name];
}


// ****************************************************************************
//  Method: JSONRootDataSet Constructor
//
//  Purpose: Inits an instance of a JSONRootEntry object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootDataSet::JSONRootDataSet()
{};

// ****************************************************************************
//  Method: JSONRootDataSet Destructor
//
//  Purpose: Cleans up an instance of a JSONRootEntry object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRootDataSet::~JSONRootDataSet()
{};


// ****************************************************************************
//  Method: JSONRootDataSet::NumberOfDomains
//
//  Purpose: Returns the number of domains in the dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
int
JSONRootDataSet::NumberOfDomains() const
{
    return ndomains;
}
   
// ****************************************************************************
//  Method: JSONRootDataSet::SetNumberOfDomains
//
//  Purpose: Sets the number of domains in the dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************                     
void         
JSONRootDataSet::SetNumberOfDomains(int ndomains)
{
    this->ndomains = ndomains;
}

// ****************************************************************************
//  Method: JSONRootDataSet::Mesh
//
//  Purpose: Returns the mesh entry associated with this mesh. 
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************   
JSONRootEntry   &
JSONRootDataSet::Mesh()
{
    return mesh;
}
      
// ****************************************************************************
//  Method: JSONRootDataSet::Fields
//
//  Purpose: Returns the names of the fields associated with this dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************         
void
JSONRootDataSet::Fields(std::vector<std::string> &field_names) const
{
    field_names.clear();
    for(map<string,JSONRootEntry>::const_iterator itr = fields.begin();
        itr!=fields.end();
        itr++)
        {
            field_names.push_back(itr->first);
        }
}

// ****************************************************************************
//  Method: JSONRootDataSet::Field
//
//  Purpose: 
//     Returns the field with the given name. 
//     The field is created if it doesn't already exist.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
JSONRootEntry  &
JSONRootDataSet::Field(const std::string &field_name)
{
    return fields[field_name];
}

// ****************************************************************************
//  Method: JSONRoot Constructor
//
//  Purpose: Inits an instance of a JSONRootEntry object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRoot::JSONRoot()
{

}

// ****************************************************************************
//  Method: JSONRoot Constructor 
//
//  Purpose: Inits a JSONRoot object using the ParseJSON() method.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
JSONRoot::JSONRoot(const std::string &json_root)
{
    ParseJSON(json_root);
}

// ****************************************************************************
//  Method: JSONRoot Destructor
//
//  Purpose: Cleans up  an instance of a JSONRootEntry object.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// ****************************************************************************
JSONRoot::~JSONRoot()
{

}

// ****************************************************************************
//  Method: JSONRootDataSet::DataSets
//
//  Purpose: Returns the names of the datasets in this root file.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
void
JSONRoot::DataSets(vector<string> &dset_names) const
{
    dset_names.clear();
    for(map<string,JSONRootDataSet>::const_iterator itr = dsets.begin();
        itr!=dsets.end();
        itr++)
        {
            dset_names.push_back(itr->first);
        }
}

// ****************************************************************************
//  Method: JSONRootDataSet::DataSet
//
//  Purpose:
//     Returns the dataset with the given name. 
//     The dataset is created if it doesn't already exist.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
JSONRootDataSet &
JSONRoot::DataSet(const std::string &dset_name)
{
    return dsets[dset_name];
}

// ****************************************************************************
//  Method: JSONRootDataSet::NumberOfDataSets
//
//  Purpose: Returns the number of datasets described by this root file.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
int
JSONRoot::NumberOfDataSets() const
{
    return dsets.size();
}


// ****************************************************************************
//  Method: JSONRoot::ParseJSON
//
//  Purpose: Parses a JSON string into this JSONRoot object. 
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
void 
JSONRoot::ParseJSON(const std::string &json_root)
{
    // clear existing structure
    dsets.clear();

    // open root file and read its contents
    ifstream iroot;
    iroot.open(json_root.c_str());
    std::string json((std::istreambuf_iterator<char>(iroot)), 
                      std::istreambuf_iterator<char>());

    // parse with rapidjson
    rapidjson::Document document;
    if(document.Parse<0>(json.c_str()).HasParseError())
    {
        // TODO: Throw VisIt Exception
        cout << "ERROR PARSING JSON DATA" <<endl;
    }

    if(document.IsObject())
    {
        // handle datasets
        if (document.HasMember("dsets"))
        {
            const rapidjson::Value &json_dsets = document["dsets"];
            for (rapidjson::Value::ConstMemberIterator dsets_itr = json_dsets.MemberBegin(); 
                 dsets_itr != json_dsets.MemberEnd(); ++dsets_itr)
            {
                string curr_dset_name = dsets_itr->name.GetString();
                JSONRootDataSet &curr_dset = DataSet(curr_dset_name);
                const rapidjson::Value &json_dset = dsets_itr->value;
                // handle # of domains, meshes w/ tags
                curr_dset.SetNumberOfDomains(json_dset["domains"].GetInt());
                curr_dset.Mesh().Path().Set(json_dset["mesh"]["path"].GetString());
                if(json_dset["mesh"].HasMember("tags"))
                {
                    const rapidjson::Value &mesh_tags = json_dset["mesh"]["tags"];
                    for (rapidjson::Value::ConstMemberIterator tags_itr = mesh_tags.MemberBegin(); 
                         tags_itr != mesh_tags.MemberEnd(); ++tags_itr)
                    {      
                        curr_dset.Mesh().Tag(tags_itr->name.GetString()) = tags_itr->value.GetString();
                    }
                }
                // handle fields
                if(json_dset.HasMember("fields"))
                {
                    const rapidjson::Value &json_fields =json_dset["fields"];
                    for (rapidjson::Value::ConstMemberIterator fields_itr = json_fields.MemberBegin(); 
                         fields_itr != json_fields.MemberEnd(); ++fields_itr)
                     {
                        const rapidjson::Value &json_field = fields_itr->value;
                        string curr_field_name = fields_itr->name.GetString();
                        JSONRootEntry &curr_field = curr_dset.Field(curr_field_name);
                        curr_field.Path().Set(json_field["path"].GetString());
                        const rapidjson::Value &json_tags = json_field["tags"];
                        for (rapidjson::Value::ConstMemberIterator tags_itr = json_tags.MemberBegin(); 
                             tags_itr != json_tags.MemberEnd(); ++tags_itr)
                        {      
                            curr_field.Tag(tags_itr->name.GetString()) = tags_itr->value.GetString();
                        }
                     }
                 }
            }
        }
    }
}

// ****************************************************************************
//  Method: JSONRoot::ToJson
//
//  Purpose: Constructs a std::string with JSON that represent the JSONRoot.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
std::string
JSONRoot::ToJson() 
{
    std::ostringstream oss;
    ToJson(oss);
    return oss.str();
}

// ****************************************************************************
//  Method: JSONRoot::ToJson
//
//  Purpose: Writes JSON that represent the JSONRoot to the passed ostream.
//
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
// **************************************************************************** 
void
JSONRoot::ToJson(ostringstream &oss) 
{
    oss << "{\"dsets\":\n";
    vector<string>dset_names;
    DataSets(dset_names);
    // loop over data sets
    for(int i=0;i<dset_names.size();i++)
    {
        // domain and mesh data
        oss << "   \"" << dset_names[i] << "\":{\n";
        JSONRootDataSet &dset =  DataSet(dset_names[i]);
        oss << "     \"domains\": " << dset.NumberOfDomains() <<",\n";
        oss << "     \"mesh\": {\"path\": \"" << dset.Mesh().Path().Expand() << "\"},\n";
        oss << "     \"fields\": {\n";
        // loop over fields
        vector<string>field_names;
        dset.Fields(field_names);
        for(int j=0;j<field_names.size();j++)
        {
            JSONRootEntry &field = dset.Field(field_names[j]);
                oss << "        \"" << field_names[j] << " \": {";
                oss << "\"path\": \"" << field.Path().Expand() << "\", \"tags\":{";
            vector<string>tag_names;
            field.Tags(tag_names);
            for(int k=0;k<tag_names.size();k++)
            {
                oss << "\"" << tag_names[k] << "\": \"" << field.Tag(tag_names[k]) << "\"";
                if(k < tag_names.size()-1)
                    oss <<", ";
            }
            oss << "}\n";
        }
        oss << "   }\n";
    }
    oss << "}\n";
}

