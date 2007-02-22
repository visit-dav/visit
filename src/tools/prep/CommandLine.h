/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            CommandLine.h                                  //
// ************************************************************************* //

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

// 
// KAI compiler prevents forward declaration of classes, so we must include
// all headers.
//
#include <visitstream.h>


// ****************************************************************************
//  Class: CommandLine
//  
//  Purpose:
//      Used to contain all of the information parsed out of the command line.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added flag for doing Silo object conversion.
//
//      Hank Childs, Mon Jun 26 09:47:08 PDT 2000
//      Changed parsing of resolution for resampling.
//
// ****************************************************************************

class CommandLine
{
  public:
    //  Public Methods
             CommandLine();
            ~CommandLine();

    int      Parse(int, char **, ostream &);
    void     Usage(ostream &);
    void     PrintSelf(ostream &);

    char   **GetFiles()         { return inputname; };
    int      GetNumFiles()      { return inputnameN; };
    char   **GetVarList()       { return vars; };
    int      GetNumVars()       { return varsN; };
    char    *GetOutputPrefix()  { return basename; };
    int      GetNFiles()        { return nfiles; };
    bool     GetLowResSwitch()  { return lres; };
    bool     GetMedResSwitch()  { return mres; };
    bool     GetBSPSwitch()     { return bsp; };
    bool     GetConnectSwitch() { return connect; };
    bool     GetMatBndSwitch()  { return matbnd; };
    bool     GetSiloObjSwitch() { return siloobj; };
    void     GetLowResolution(int [3]);
    void     GetMedResolution(int [3]);

  private:
    //  Private Fields
    char    *basename;

    bool     bsp;

    bool     connect;

    char   **inputname;
    int      inputnameN;

    bool     lres;
    int      lresN[3];

    bool     matbnd;

    bool     mres;
    int      mresN[3];
    
    int      nfiles;

    bool     siloobj;

    char   **vars;
    int      varsN;

    // Private Methods
    void     Initialize();

    //  Constants
    static const int           lresN_DEFAULT;
    static const int           mresN_DEFAULT;
    static const int           nfiles_DEFAULT;
    static const char * const  nfiles_DEFAULT_WORD;
};


#endif


