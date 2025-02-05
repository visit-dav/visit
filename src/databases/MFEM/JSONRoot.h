// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Method: avtMFEMFileFormat::JSONRootExpr
//
//  Purpose: Hold definition and type for an expression
//
//  Mark C. Miller, Tue Sep 20 18:11:55 PDT 2016
// ****************************************************************************
class JSONRootExpr
{
    public:
                 JSONRootExpr() {};
                 JSONRootExpr(char const *_defn, char const *_type)
                     : defn(_defn), type(_type) {};
        virtual ~JSONRootExpr() {};

        std::string const &Defn(void) const {return defn;};
        std::string const &Type(void) const {return type;};

    private:

        std::string defn;
        std::string type;
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
        
        JSONRootPath   &CatPath() {return catpath;};

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
        JSONRootPath                        catpath;
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
//   Mark C. Miller, Tue Sep 20 18:11:11 PDT 2016
//   Added expressions
// ****************************************************************************
class JSONRoot
{
public:

                     JSONRoot();
                     JSONRoot(const std::string &json_root_file);

    virtual         ~JSONRoot();
    
    void             DataSets(std::vector<std::string> &dset_names) const;
    JSONRootDataSet &DataSet(const std::string &dset_name);

    void             Expressions(std::vector<std::string> &expr_names) const;
    JSONRootExpr    &Expression(const std::string &expr_name);

    int              NumberOfDataSets() const;
    std::string      ToJson();
    void             ToJson(std::ostringstream &oss);

  private:
    void             ParseJSONFile(const std::string &json_root_file);
    void             ParseJSONString(const std::string &json_string,
                                     const std::string &root_dir);
    std::string      GenerateMocRootJSON(const std::string &json_root_file);

    std::string      ResolveAbsolutePath(const std::string &root_dir,const std::string &file_path);
    
    std::map<std::string,JSONRootDataSet> dsets;
    std::map<std::string,JSONRootExpr> exprs;
    
};

