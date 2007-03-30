// ************************************************************************* //
//  File: avtSiloDumpFilter.h
// ************************************************************************* //

#ifndef AVT_SiloDump_FILTER_H
#define AVT_SiloDump_FILTER_H


#include <avtPluginStreamer.h>
#include <SiloDumpAttributes.h>
#include <string>
#include <vector>

class  vtkDataSet;
struct DBfile;


// ****************************************************************************
//  Class: avtSiloDumpFilter
//
//  Purpose:
//      A plugin operator for SiloDump.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  5, 2002
//
// ****************************************************************************

class avtSiloDumpFilter : public avtPluginStreamer
{
  public:
                         avtSiloDumpFilter();
    virtual             ~avtSiloDumpFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtSiloDumpFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Dumping to Silo file."; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    SiloDumpAttributes    atts;
    vector<int>           domains;
    vector<std::string>   vars;
    DBfile               *db;

    virtual void          PreExecute();
    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);
    virtual void          PostExecute();
};


#endif
