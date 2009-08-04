/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtFileFormatInterface.C                         //
// ************************************************************************* //

#include <avtFileFormatInterface.h>

#include <avtFileFormat.h>


using     std::vector;


// ****************************************************************************
//  Method: avtFileFormatInterface destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFileFormatInterface::~avtFileFormatInterface()
{
    ;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::SetCache
//
//  Purpose:
//      Sets the cache for each file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::SetCache(avtVariableCache *cache)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->SetCache(cache);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::RegisterVariableList
//
//  Purpose:
//      Registers a variable list with each file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::RegisterVariableList(const char *var,
                                             const vector<CharStrRef> &vars2nd)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->RegisterVariableList(var, vars2nd);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::RegisterDataSelections
//
//  Purpose:
//      Registers data selections with each file format and a vector of bools
//      in which format can indicate whether a given selection was indeed
//      applied. This is typically used
//      just prior to avtGenericDatabase' attempt to get data from a format
//      to give the format an opportunity to handle the selection on read.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************

void
avtFileFormatInterface::RegisterDataSelections(
    const vector<avtDataSelection_p> &selList,
    vector<bool> *wasApplied) 
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->RegisterDataSelections(selList, wasApplied);
    }
}

// ****************************************************************************
//  Method: avtFileFormatInterface::GetType
//
//  Purpose:
//      Gets the type of file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

const char *
avtFileFormatInterface::GetType(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->GetType();
    }

    return "Unknown Format";
}


// ****************************************************************************
//  Method: avtFileFormatInterface::HasVarsDefinedOnSubMeshes
//
//  Purpose:
//      Declares whether or not some variables are defined on sub-meshes
//      only.  This can affect caching strategies.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasVarsDefinedOnSubMeshes(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->HasVarsDefinedOnSubMeshes();
    }

    return false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::PerformsMaterialSelection
//
//  Purpose:
//      Determines whether or not the underlying file format wants to do its
//      own material selection.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************
 
bool
avtFileFormatInterface::PerformsMaterialSelection(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->PerformsMaterialSelection();
    }

    return false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOff
//
//  Purpose:
//      Tells the file format that it should not do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::TurnMaterialSelectionOff(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->TurnMaterialSelectionOff();
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOn
//
//  Purpose:
//      Tells the file format that it should do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::TurnMaterialSelectionOn(const char *matname)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->TurnMaterialSelectionOn(matname);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::CanDoStreaming
//
//  Purpose:
//      Indicates if the file format supports streaming.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

bool
avtFileFormatInterface::CanDoStreaming(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        if (! ff->CanDoStreaming())
        {
            return false;
        }
    }

    return true;
}

// ****************************************************************************
//  Method: avtFileFormatInterface::HasInvariantMetaData
//
//  Purpose:
//      Indicates if the file format can have metadata that varies with time 
//
//  Programmer: Mark C. Miller
//  Creation:   September 30, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasInvariantMetaData(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        if (! GetFormat(i)->HasInvariantMetaData())
            return false;
    }

    return true;
}

// ****************************************************************************
//  Method: avtFileFormatInterface::HasInvariantSIL
//
//  Purpose:
//      Indicates if the file format can have a SIL that varies with time 
//
//  Programmer: Mark C. Miller
//  Creation:   September 30, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasInvariantSIL(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        if (! GetFormat(i)->HasInvariantSIL())
            return false;
    }

    return true;
}

// ****************************************************************************
//  Method: avtFileFormatInterface::CanCacheVariable
//
//  Purpose:
//      Indicates if the file format can cache the given named variable 
//
//  Programmer: Mark C. Miller
//  Creation:   December 10, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::CanCacheVariable(const char *var_name)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        if (! GetFormat(i)->CanCacheVariable(var_name))
            return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::SetResultMustBeProducedOnlyOnThisProcessor
//
//  Purpose:
//      This method is for telling formats that do their own domain
//      decomposition whether or not they can count on other processors to 
//      help them produce results.  In the case of streamlines that produce
//      point selections, the file format reader will *not* be able to count
//      on the other processors, although it can in all other cases (at the
//      time of the writing of this comment).
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2009
//
// ****************************************************************************

void
avtFileFormatInterface::SetResultMustBeProducedOnlyOnThisProcessor(bool b)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        GetFormat(i)->SetResultMustBeProducedOnlyOnThisProcessor(b);
    }
}
