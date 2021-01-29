// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPythonQuery.h                              //
// ************************************************************************* //

#ifndef AVT_PYTHON_QUERY_H
#define AVT_PYTHON_QUERY_H

#include <query_exports.h>

#include <avtDataObjectQuery.h>
#include <avtDatasetSink.h>
#include <QueryAttributes.h>

#include <string>
#include <vector>

class avtPythonFilterEnvironment;


// ****************************************************************************
//  Class: avtPythonQuery
//
//  Purpose:
//      Interface to python queries.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  9 15:15:41 PST 2010
//
//  Modifications:
//   Cyrus Harrison, Tue Sep 21 11:14:21 PDT 2010
//   Added SetPythonArgs()
//
//   Cyrus Harrison, Wed Jan 12 11:32:42 PST 2011
//   Added queryType & queryDescription members.
//
//   Cyrus Harrison, Fri Mar 30 13:51:24 PDT 2012
//   Convert python query filter to use new query params infrastructure.
//
//   Eric Brugger, Tue Jan 26 13:17:19 PST 2021
//   Modified the python args to be a char vector instead of a string.
//
// ****************************************************************************

class QUERY_API avtPythonQuery :  public avtDataObjectQuery,
                                  public avtDatasetSink
{
  public:
                                avtPythonQuery();
    virtual                    ~avtPythonQuery();
    void                        CleanUp();
    virtual void                SetInputParams(const MapNode &);
    void                        SetPythonScript(const std::string &py_script);
    void                        SetPythonArgs(const std::vector<char> &py_args);

    virtual const char         *GetType(void);
    virtual const char         *GetDescription(void);
    virtual void                PerformQuery(QueryAttributes *qa);

    void                        SetVariableNames(const std::vector<std::string> &vars)
                                    { varNames = vars;}

    virtual void                GetSecondaryVariables(std::vector<std::string> &res);

    void                        SetResultMessage(const std::string &msg)
                                 { resultMessage = msg; };
    void                        SetResultValues(const doubleVector &d)
                                 { resultValues = d;}
    void                        SetXmlResult(const std::string &xml)
                                 { resultXml= xml; };

    virtual std::string         GetResultMessage(void)
                                 { return resultMessage;}

 private:

    virtual void                PreExecute(void);
    virtual void                PostExecute(void);
    virtual void                UpdateContract(void);

    virtual void                Execute();

    QueryAttributes             queryAtts;

    avtPythonFilterEnvironment *pyEnv;
    std::string                 pyScript;
    std::vector<char>           pyArgs;

    stringVector                varNames;

    std::string                 resultMessage;
    doubleVector                resultValues;
    std::string                 resultXml;

    std::string                 queryType;
    std::string                 queryDescription;
};


#endif


