// ************************************************************************* //
//                            CommandLine.h                                  //
// ************************************************************************* //

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

// 
// KAI compiler prevents forward declaration of classes, so we must include
// all headers.
//
#include <iostream.h>


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


