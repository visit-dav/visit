/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


//!    File   : Scripting.h
//!    Author : Jens Krueger
//!             SCI Institute
//!             University of Utah
//!    Date   : January 2009
//
//!    Copyright (C) 2008 SCI Institute

#pragma once

#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "../StdTuvokDefines.h"
#include "Scriptable.h"
#include <string>
#include <vector>

class MasterController;

class ScriptableListElement {
  public:
    ScriptableListElement(Scriptable* source, const std::string& strCommand, const std::string& strParameters, const std::string& strDescription);

    Scriptable* m_source;
    std::string m_strCommand;
    std::vector<std::string> m_vParameters;
    UINT32 m_iMaxParam;
    UINT32 m_iMinParam;
    std::string m_strDescription;
};

class Scripting : public Scriptable
{
  public:
    Scripting();
    ~Scripting();

    bool ParseCommand(const std::vector< std::string >& strTokenized, std::string& strMessage);
    bool ParseLine(const std::string& strLine);
    bool ParseFile(const std::string& strFilename);

    bool RegisterCommand(Scriptable* source, const std::string& strCommand, const std::string& strParameters, const std::string& strDescription);

    // Scriptable
    virtual void RegisterCalls(Scripting* pScriptEngine);
    virtual bool Execute(const std::string& strCommand, const std::vector< std::string >& strParams, std::string& strMessage);


  private:
    std::vector<ScriptableListElement*> m_ScriptableList;
    bool                                m_bEcho;

};

#endif // SCRIPTING_H
