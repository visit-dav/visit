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

#include <PyMapNode.h>
#include <PyVariant.h>
#include <snprintf.h>

///////////////////////////////////////////////////////////////////////////////
//
// VisIt module helper method.
//
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PyMapNode_Wrap
//
// Purpose: 
//   Converts a MapNode to a python dictonary. 
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:20:25 PST 2007
//
// Modifications:
//   Eric Brugger, Fri Jan 11 09:54:04 PST 2008
//   I added some Python version specific coding to get around a problem
//   with older versions not having a const qualifier for the string argument
//   to PyDict_SetItemString.
//
// ****************************************************************************

PyObject *
PyMapNode_Wrap(const MapNode &node)
{
    if(node.Type() == EMPTY_TYPE && node.GetNumEntries() == 0)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    // must be a variant, use variant helper
    if(node.Type() != EMPTY_TYPE)
    {
        return PyVariant_Wrap(node);
    }
    
    // we have a dict with map nodes as entries
    PyObject *dict = PyDict_New();
    stringVector entry_names;
    node.GetEntryNames(entry_names);    
    
    for(size_t i=0;i<entry_names.size();i++)
    {
        const MapNode *child_node = node.GetEntry(entry_names[i]);
        if(child_node == NULL)
            continue;
        PyObject *child = PyMapNode_Wrap(*child_node);
#if (PY_MAJOR_VERSION < 2) || ((PY_MAJOR_VERSION == 2) && (PY_MINOR_VERSION < 5))
        char *str = new char[entry_names[i].length()+1];
        strcpy(str, entry_names[i].c_str());
        PyDict_SetItemString(dict, str, child);   
        delete [] str;
#else
        PyDict_SetItemString(dict, entry_names[i].c_str(), child);   
#endif
    }

    return dict;
}

