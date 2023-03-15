// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 JSONRoot.C                                //
// ************************************************************************* //


#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "JSONRoot.h"

#include <StringHelpers.h>
#include <FileFunctions.h>

#include <InvalidFilesException.h>

#include "mfem.hpp"
#include <visit_gzstream.h>

#include <iostream>
using namespace std;

// helper that creates human readable error messages from
// json parsing errors
namespace detail
{

//---------------------------------------------------------------------------//
void  RapidJSONParseErrorDetails(const std::string &json,
                                 const rapidjson::Document &document,
                                 std::ostream &os)
{
    // provide message with line + char from rapidjson parse error offset 
    int doc_offset = document.GetErrorOffset();
    std::string json_curr = json.substr(0,doc_offset);

    int  doc_line   = 0;
    int  doc_char   = 0;

    // remove any `\r` that may linger so we can split on `\n`
    json_curr = StringHelpers::Replace(json_curr,"\r","");

    std::vector<std::string> lines = StringHelpers::split(json_curr,'\n');

    if(lines.size() > 0)
    {
        doc_line = lines.size()-1;
        // char is the len of the last line
        doc_char = lines[lines.size()-1].size();
    }

    os << " parse error message: " << std::endl
       << GetParseError_En(document.GetParseError()) << std::endl
       << " offset: "    << doc_offset << std::endl
       << " line: "      << doc_line << std::endl
       << " character: " << doc_char << std::endl
       << " json:\n"     << json << std::endl; 
}

}; // end detail


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
//  Purpose: Maps a domain id to is file system or in db path.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
//  Modifications:
//   Cyrus Harrison, Tue Sep 23 14:42:52 PDT 2014
//   Added support for simple domain expansion.
//
//   Cyrus Harrison, Wed Jan 21 15:02:21 PST 2015
//   Added support for "%06d", which is the new MFEM standard format string
//
// ****************************************************************************
std::string
JSONRootPath::Expand(int domain) const
{
    //
    // Note: This currently only handles "%05d" or "%06d" as the format string.
    //
    
    std::size_t path_pattern = path.find("%05d");

    if(path_pattern != std::string::npos)
    {
        char buff[64];
        snprintf(buff,64,"%05d",domain);
        return StringHelpers::Replace(path,
                                      "%05d",
                                      std::string(buff));
    }
    
    path_pattern = path.find("%06d");
    
    if(path_pattern != std::string::npos)
    {
        char buff[64];
        snprintf(buff,64,"%06d",domain);    
        return StringHelpers::Replace(path,
                                      "%06d",
                                      std::string(buff));
    }

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
: ndomains(0),
  validCycle(false),
  cycle(0),
  validTime(false),
  timev(0.0),
  catpath()
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
//  Method: JSONRootDataSet::Cycle
//
//  Purpose: Returns cycle metadata value for this dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Oct 15 11:10:05 PDT 2014
//
// ****************************************************************************
int
JSONRootDataSet::Cycle() const
{
    return cycle;
}
 
// ****************************************************************************
//  Method: JSONRootDataSet::SetCycle
//
//  Purpose: Returns sets the cycle metadata value for this dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Oct 15 11:10:05 PDT 2014
//
// ****************************************************************************
void
JSONRootDataSet::SetCycle(int value)
{
    cycle = value;
    validCycle = true;
}

// ****************************************************************************
//  Method: JSONRootDataSet::HasCycle
//
//  Purpose: Returns if cycle metadata has been set.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Oct 15 11:10:05 PDT 2014
//
// ****************************************************************************
bool
JSONRootDataSet::HasCycle() const
{
    return validCycle;
}
 

// ****************************************************************************
//  Method: JSONRootDataSet::Time
//
//  Purpose: Returns time metadata value for this dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Oct 15 11:10:05 PDT 2014
//
// ****************************************************************************
double
JSONRootDataSet::Time() const
{
    return timev;
}
 
// ****************************************************************************
//  Method: JSONRootDataSet::SetTime
//
//  Purpose: Returns sets the time metadata value for this dataset.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Oct 15 11:10:05 PDT 2014
//
// ****************************************************************************
void
JSONRootDataSet::SetTime(double value)
{
    timev = value;
    validTime = true;
}

// ****************************************************************************
//  Method: JSONRootDataSet::HasTime
//
//  Purpose: Returns if time metadata has been set.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Oct 15 11:10:05 PDT 2014
//
// ****************************************************************************
bool
JSONRootDataSet::HasTime() const
{
    return validTime;
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
//  Modifications:
//   Cyrus Harrison, Fri Mar  3 10:50:30 PST 2023
//   Refactor to support direct reads of mfem mesh files.
//
// **************************************************************************** 
JSONRoot::JSONRoot(const std::string &json_root_file)
{
    if(StringHelpers::ends_with(json_root_file,std::string(".mesh")))
    {
        std::string root_file = FileFunctions::Absname(".",json_root_file);
        std::string root_dir =  FileFunctions::Dirname(root_file);
        std::string root_json = GenerateMocRootJSON(json_root_file);
        ParseJSONString(root_json,root_dir);
    }
    else  // main case
    {
        ParseJSONFile(json_root_file);
    }
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
//  Method: JSONRootDataSet::Expressions
//
//  Purpose: Returns the names of the expressions in this root file.
//
//  Mark C. Miller, Tue Sep 20 18:09:03 PDT 2016
// **************************************************************************** 
void
JSONRoot::Expressions(vector<string> &expr_names) const
{
    expr_names.clear();
    for(map<string,JSONRootExpr>::const_iterator itr = exprs.begin();
        itr!=exprs.end();
        itr++)
        {
            expr_names.push_back(itr->first);
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
//  Method: JSONRootDataSet::Expression
//
//  Purpose: Return the expression with the given name.
//     The dataset is created if it doesn't already exist.
//
//  Mark C. Miller, Tue Sep 20 18:10:38 PDT 2016
// **************************************************************************** 
JSONRootExpr &
JSONRoot::Expression(const std::string &expr_name)
{
    return exprs[expr_name];
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
    return (int)dsets.size();
}


// ****************************************************************************
//  Method: JSONRoot::ResolveAbsolutePath
//
//  Purpose: Helper for abs path logicl.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Wed Sep 24 10:47:00 PDT 2014
//
// **************************************************************************** 
std::string      
JSONRoot::ResolveAbsolutePath(const std::string &root_dir,
                              const std::string &file_path)
{
    return FileFunctions::Absname(root_dir,file_path);
}


// ****************************************************************************
//  Method: JSONRoot::ParseJSONFile
//
//  Purpose: Parses a JSON file into this JSONRoot object.
//    (Refactored from previous JSONRoot::ParseJSON method)
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
//  Modifications:
//   Cyrus Harrison, Wed Sep 24 10:47:00 PDT 2014
//   Handle abs path logic.
//
//   Mark C. Miller, Tue Sep 20 18:07:42 PDT 2016
//   Add support for expressions
//
//   Cyrus Harrison, Fri Mar  3 10:50:30 PST 2023
//   Refactor to split parsing json string and reading from file
//
// **************************************************************************** 
void 
JSONRoot::ParseJSONFile(const std::string &json_root_file)
{

    std::string root_file = FileFunctions::Absname(".",json_root_file);
    std::string root_dir =  FileFunctions::Dirname(root_file);

    // open root file and read its contents
    ifstream iroot;
    iroot.open(root_file.c_str());

    if(!iroot.is_open())
    {
        ostringstream msg;
        msg << "Failed to open file: " << json_root_file;
        EXCEPTION1(InvalidFilesException, msg.str());
    }

    std::string json((std::istreambuf_iterator<char>(iroot)), 
                      std::istreambuf_iterator<char>());

    ParseJSONString(json, root_dir);
}


// ****************************************************************************
//  Method: JSONRoot::ParseJSONString
//
//  Purpose: Parses a JSON string into this JSONRoot object. 
//    (Refactored from previous JSONRoot::ParseJSON method)
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//
//  Modifications:
//   Cyrus Harrison, Wed Sep 24 10:47:00 PDT 2014
//   Handle abs path logic.
//
//   Mark C. Miller, Tue Sep 20 18:07:42 PDT 2016
//   Add support for expressions
//
//   Cyrus Harrison, Fri Mar  3 10:50:30 PST 2023
//   Refactor to split parsing json string and reading from file
//
// **************************************************************************** 
void 
JSONRoot::ParseJSONString(const std::string &json,
                          const std::string &root_dir)
{
    // clear existing structure
    dsets.clear();

    // parse with rapidjson
    rapidjson::Document document;
    if(document.Parse<0>(json.c_str()).HasParseError())
    {
        ostringstream msg;
        msg << "Failed to parse MFEM JSON Root: " << json;
        detail::RapidJSONParseErrorDetails(json,document,msg);
        EXCEPTION1(InvalidFilesException, msg.str());
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
                
                if(json_dset.HasMember("time"))
                {
                   curr_dset.SetTime(json_dset["time"].GetDouble()); 
                }

                if(json_dset.HasMember("cycle"))
                {
                   curr_dset.SetCycle(json_dset["cycle"].GetInt()); 
                }
                
                if(json_dset.HasMember("catpath"))
                {
                   std::string cat_file_path = json_dset["catpath"].GetString();
                   curr_dset.CatPath().Set(ResolveAbsolutePath(root_dir,cat_file_path));
                }
                
                // handle # of domains, meshes w/ tags
                curr_dset.SetNumberOfDomains(json_dset["domains"].GetInt());
                std::string mesh_file_path = json_dset["mesh"]["path"].GetString();
                curr_dset.Mesh().Path().Set(ResolveAbsolutePath(root_dir,mesh_file_path));
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
                        std::string field_file_path = json_field["path"].GetString();
                        curr_field.Path().Set(ResolveAbsolutePath(root_dir,field_file_path));
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

        if (document.HasMember("expressions"))
        {
            const rapidjson::Value &json_exprs = document["expressions"];
            for (rapidjson::Value::ConstMemberIterator exprs_itr = json_exprs.MemberBegin(); 
                 exprs_itr != json_exprs.MemberEnd(); ++exprs_itr)
            {
                string curr_expr_name = exprs_itr->name.GetString();
                const rapidjson::Value &json_expr = exprs_itr->value;
                if (json_expr.HasMember("definition") && json_expr.HasMember("type"))
                {
                    JSONRootExpr curr_expr(json_expr["definition"].GetString(),
                                           json_expr["type"].GetString());
                    exprs[curr_expr_name] = curr_expr;
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: JSONRoot::GenerateMocRootJSON
//
//  Purpose: Creates a moc json to describe a lone mfem mesh file
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Fri Mar  3 10:50:30 PST 2023
//
//  Modifications:
//    Cyrus Harrison, Wed Mar 15 12:25:13 PDT 2023
//    Escape file system paths to avoid issues with JSON parsing
//    vs windows paths.
//
// **************************************************************************** 
std::string  
JSONRoot::GenerateMocRootJSON(const std::string &mfem_mesh_file)
{
    // we need to read the mesh file to understand the spatial_dim and topo dim.
    visit_ifstream imesh(mfem_mesh_file.c_str());
    if(imesh().fail())
    {
        //failed to open mesh file
        ostringstream msg;
        msg << "Failed to open MFEM mesh: " << mfem_mesh_file;
        EXCEPTION1(InvalidFilesException, msg.str());
    }

    mfem::Mesh mesh(imesh(), 1, 0, false);

    int spatial_dim = mesh.SpaceDimension();
    int topo_dim = mesh.Dimension();

    std::ostringstream moc_json;
    
    std::string mfem_mesh_file_escaped = StringHelpers::EscapeSpecialChars(mfem_mesh_file);

    moc_json << "{" << std::endl
             << "\"dsets\":{" << std::endl
             << "   \"main\":{" << std::endl
             << "       \"domains\": 1" << "," << std::endl 
             << "       \"mesh\": { \"path\": \"" << mfem_mesh_file_escaped << "\"," <<  std::endl
             << "                   \"tags\": { \"spatial_dim\":" 
                                             << "\"" << spatial_dim  << "\" ,"
                                             << "\"topo_dim\":" 
                                             << "\"" << topo_dim  << "\" ,"
                                             << "\"max_lods\": \"25\" }}"  << std::endl
            << "                  }" << std::endl
            << "             }" << std::endl
            << "}" << std::endl;

    return moc_json.str();
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
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Jun 12 16:02:35 PDT 2014
//  
//  Modifications
//    Mark C. Miller, Tue Sep 20 18:07:42 PDT 2016
//    Add support for expressions
//
//    Cyrus Harrison, Wed Mar 15 12:25:13 PDT 2023
//    Escape file system paths to avoid issues with JSON parsing
//    vs windows paths.
//
// **************************************************************************** 
void
JSONRoot::ToJson(ostringstream &oss) 
{
    oss << "{\"dsets\":\n";
    vector<string>dset_names;
    DataSets(dset_names);
    // loop over data sets
    for(int i=0;i<(int)dset_names.size();i++)
    {
        std::string mesh_path = StringHelpers::EscapeSpecialChars(dset.Mesh().Path().Expand());

        // domain and mesh data
        oss << "   \"" << dset_names[i] << "\":{\n";
        JSONRootDataSet &dset =  DataSet(dset_names[i]);
        oss << "     \"domains\": " << dset.NumberOfDomains() <<",\n";
        oss << "     \"mesh\": {\"path\": \"" << mesh_path << "\"},\n";
        oss << "     \"fields\": {\n";
        // loop over fields
        vector<string>field_names;
        dset.Fields(field_names);
        for(int j=0;j<(int)field_names.size();j++)
        {

            JSONRootEntry &field = dset.Field(field_names[j]);
            std::string field_path = StringHelpers::EscapeSpecialChars(field.Path().Expand());
            oss << "        \"" << field_names[j] << " \": {";
            oss << "\"path\": \"" << field_path << "\", \"tags\":{";
            vector<string>tag_names;
            field.Tags(tag_names);
            for(int k=0;k<(int)tag_names.size();k++)
            {
                oss << "\"" << tag_names[k] << "\": \"" << field.Tag(tag_names[k]) << "\"";
                if(k < (int)tag_names.size()-1)
                    oss <<", ";
            }
            oss << "}\n";
        }
        oss << "   }\n";
    }
    oss << "{\"expressions\":\n";
    vector<string>expr_names;
    Expressions(expr_names);
    for(int i=0;i<(int)expr_names.size();i++)
    {
        JSONRootExpr &expr = Expression(expr_names[i]);
        oss << "   \"" << expr_names[i]
            << "\": { \"defn\": \"" << expr.Defn()
            << "\", \"type\": \"" << expr.Type() << "\"}";
        if(i < (int)expr_names.size()-1)
            oss <<",";
        oss << "\n";
    }
    oss << "}\n";
}

