// ************************************************************************* //
//                              CommandLine.C                                //
// ************************************************************************* //

#include <ctype.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>

#include "CommandLine.h"


// 
// Declaration of static consts.  They are not allowed as initializers in
// class definitions.
//

const int          CommandLine::lresN_DEFAULT       = 64;
const int          CommandLine::mresN_DEFAULT       = 128;
const int          CommandLine::nfiles_DEFAULT      = 1;
const char * const CommandLine::nfiles_DEFAULT_WORD = "one";


// ****************************************************************************
//  Method: CommandLine constructor
//
//  Purpose:
//      Constructor that only initializes the object.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

CommandLine::CommandLine()
{
    Initialize();
}


// ****************************************************************************
//  Method: CommandLine destructor
//
//  Purpose:
//      Frees any allocated memory.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

CommandLine::~CommandLine()
{
    if (basename != NULL)
    {
        delete [] basename;
    }
    if (inputname != NULL)
    {
        for (int i1 = 0 ; i1 < inputnameN ; i1++)
        {
            if (inputname[i1] != NULL)
            {
                delete [] inputname[i1];
            }
        }
        delete [] inputname;
    }

    if (vars != NULL)
    {
        for (int i2 = 0 ; i2 < varsN ; i2++)
        {
            if (vars[i2] != NULL)
            {
                delete [] vars[i2];
            }
        }
        delete [] vars;
    }
}


// ****************************************************************************
//  Method: CommandLine::Initialize
//
//  Purpose:
//      Initializes the object with all of the default values.  This is the
//      "true" constructor.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added initialization of siloobj switch.
//
//      Hank Childs, Mon Jun 26 09:55:14 PDT 2000
//      Changed initialization of mresN and lresN since they are now arrays.
//
// ****************************************************************************

void
CommandLine::Initialize(void)
{
    //
    // Initialize switches
    //
    bsp     = true;
    connect = true;
    lres    = true;
    matbnd  = true;
    mres    = true;
    siloobj = true;
    
    //
    // Initialize other values
    //
    basename   = NULL;
    inputname  = NULL;
    inputnameN = 0;
    lresN[0] = lresN[1] = lresN[2] = lresN_DEFAULT;
    mresN[0] = mresN[1] = mresN[2] = mresN_DEFAULT;
    nfiles     = nfiles_DEFAULT;   
    vars       = NULL;
    varsN      = 0;
}


// ****************************************************************************
//  Method: CommandLine::Parse
//
//  Purpose:
//      Parses a command line and modifies the switches and values 
//      appropriately.
//
//  Arguments:
//      argc    The number of strings in argv.
//      argv    A list of strings from the command line.
//      out     An ostream object to print errors to.
//
//  Returns:    0 on success, < 0 on failure
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added parsing of siloobj flag.
//
//      Hank Childs, Mon Jun 26 09:55:14 PDT 2000
//      Added capability of specifying the resampling resolution for each
//      dimension.
//
//      Hank Childs, Wed Aug 30 15:27:32 PDT 2000
//      Allow -lres and -mres to be specified even when -none hasn't been set.
//      Also allowed for variables to be quoted into one command line argument.
//
// ****************************************************************************

int 
CommandLine::Parse(int argc, char *argv[], ostream &out)
{
    int   i;

    // 
    // Find the -o flag and process that and everything after it.
    //
    int oFlag = 0;
    while (oFlag < argc && strcmp(argv[oFlag], "-o") != 0)
    {
        oFlag++;
    }

    // 
    // Check to see if we got the -o flag.
    //
    if (oFlag >= argc)
    {
        out << "Error: Must use the -o flag." << endl;
        return -1;
    }

    //
    // Check to make sure that everything after the -o is a file name.
    //
    for (i = oFlag+1 ; i < argc ; i++)
    {
        if (argv[i][0] == '-')
        {
            out << "Error: The -o flag must be the last flag specified." 
                << endl;
            return -1;
        }
    }
 
    //
    // Check to see if we specified a base name with the -o flag and then
    // make a copy of the base name.
    //
    int currentIndex = oFlag+1;
    if (currentIndex >= argc)
    {
        out << "Error: Must specify a base name with the -o flag." << endl;
        return -1;
    }
    basename = new char[strlen(argv[currentIndex]) + 1];
    strcpy(basename, argv[currentIndex]);

    //
    // Now that we have -o parsed, we know that everything after it is a file
    // for our file list.  Put all of these into inputname array.
    //
    currentIndex++;
    if (currentIndex >= argc)
    {
        out << "Error: Must specify an input file list." << endl;
        return -1;
    }
    inputname  = new char*[argc-currentIndex];
    inputnameN = argc-currentIndex;
    for (int j = currentIndex ; j < argc ; j++)
    {
        inputname[j-currentIndex] = new char[strlen(argv[j])+1];
        strcpy(inputname[j-currentIndex], argv[j]);
    }

    //
    // Process all of the args but the last few, since we have already dealt
    // with those.
    //
    int    processableArgs = oFlag;
    bool   specifiedNone   = false;

    for (i = 1 ; i < processableArgs ; i++)
    {
        if (strcmp(argv[i], "-none") == 0)
        {
            bsp     = false;
            connect = false;
            lres    = false;
            matbnd  = false;
            mres    = false;
            siloobj = false;
            specifiedNone = true;
        }   // end case for -none

        else if (strcmp(argv[i], "-lres") == 0)
        { 
            //
            // Allow this even if none has not been set, since the user just
            // might be changing the resolution.
            //
            lres = true;
            if ( i+1 < processableArgs && isdigit(argv[i+1][0]) )
            {
                lresN[0] = atoi(argv[i+1]);
                if (lresN[0] < 2)
                {
                    out << "Warning: the resolution specified with -lres must"
                        <<                  " be two or more.\n"
                        << "         Setting to " << lresN_DEFAULT << "." 
                        << endl;
                    lresN[0] = lresN_DEFAULT;
                }
                i++;
                if ( i+2 < processableArgs && isdigit(argv[i+1][0]) )
                {
                    lresN[1] = atoi(argv[i+1]);
                    lresN[2] = atoi(argv[i+2]);
                    i += 2;
                    if (lresN[1] < 2)
                    {
                        out << "Warning: the resolution specified with -lres must"
                            <<                  " be two or more.\n"
                            << "         Setting to " << lresN_DEFAULT << "." 
                            << endl;
                        lresN[1] = lresN_DEFAULT;
                    }
                    if (lresN[2] < 2)
                    {
                        out << "Warning: the resolution specified with -lres must"
                            <<                  " be two or more.\n"
                            << "         Setting to " << lresN_DEFAULT << "." 
                            << endl;
                        lresN[2] = lresN_DEFAULT;
                    }
                }
                else
                {
                    lresN[2] = lresN[1] = lresN[0];
                }
            }
            else
            {
                lresN[0] = lresN[1] = lresN[2] = lresN_DEFAULT;
            }
        }   // end case for -lres

        else if (strcmp(argv[i], "-mres") == 0)
        { 
            //
            // Allow this even if none has not been set, since the user just
            // might be changing the resolution.
            //
            mres = true;
            if ( i+1 < processableArgs && isdigit(argv[i+1][0]) )
            {
                mresN[0] = atoi(argv[i+1]);
                if (mresN[0] < 2)
                {
                    out << "Warning: the resolution specified with -mres must"
                        <<                  " be two or more.\n"
                        << "         Setting to " << mresN_DEFAULT << "." 
                        << endl;
                    mresN[0] = mresN_DEFAULT;
                }
                i++;
                if ( i+2 < processableArgs && isdigit(argv[i+1][0]) )
                {
                    mresN[1] = atoi(argv[i+1]);
                    mresN[2] = atoi(argv[i+2]);
                    i += 2;
                    if (mresN[1] < 2)
                    {
                        out << "Warning: the resolution specified with -mres must"
                            <<                  " be two or more.\n"
                            << "         Setting to " << mresN_DEFAULT << "." 
                            << endl;
                        mresN[1] = mresN_DEFAULT;
                    }
                    if (mresN[2] < 2)
                    {
                        out << "Warning: the resolution specified with -lres must"
                            <<                  " be two or more.\n"
                            << "         Setting to " << mresN_DEFAULT << "." 
                            << endl;
                        mresN[2] = mresN_DEFAULT;
                    }
                }
                else
                {
                    mresN[2] = mresN[1] = mresN[0];
                }
            }
            else
            {
                mresN[0] = mresN[1] = mresN[2] = mresN_DEFAULT;
            }
        }   // end case for -mres

        else if (strcmp(argv[i], "-matbnd") == 0)
        {
            if (specifiedNone == false)
            {
                out << "Warning: You may not specify -matbnd before "
                    <<                          "specifying none.\n"
                    << "         Ignoring flag." << endl;
                continue;
            }
            matbnd = true;
        }   // end case for -matbnd

        else if (strcmp(argv[i], "-connect") == 0)
        {
            if (specifiedNone == false)
            {
                out << "Warning: You may not specify -connect before "
                    <<                          "specifying none.\n"
                    << "         Ignoring flag." << endl;
                continue;
            }
            connect = true;
        }   // end case for -connect

        else if (strcmp(argv[i], "-bsp") == 0)
        {
            if (specifiedNone == false)
            {
                out << "Warning: You may not specify -bsp before specifying "
                    <<                          "none.\n"
                    << "         Ignoring flag." << endl;
                continue;
            }
            bsp = true;
        }   // end case for -bsp

        else if (strcmp(argv[i], "-siloobj") == 0)
        {
            if (specifiedNone == false)
            {
                out << "Warning: You may not specify -siloobj before "
                    << "specifying none.\n"
                    << "        Ignoring flag." << endl;
                continue;
            }
            siloobj = true;
        }   // end case for -siloobj
         
        else if (strcmp(argv[i], "-nfiles") == 0)
        {
            if ( i+1 < processableArgs && isdigit(argv[i+1][0]) )
            {
                nfiles = atoi(argv[i+1]);
                if (nfiles <= 0)
                {
                    out << "Warning: Number of files specified with nfiles "
                        <<              "must be positive.\n"
                        << "         Using default." << endl;
                }
                i++;
            }
            else
            {
                out << "Warning: Must specify the number of files if the"
                    <<          " -nfiles is specified.\n"
                    << "         Using default." << endl;
                nfiles = nfiles_DEFAULT;
            }
        }   // end case for -nfiles 

        else if (strcmp(argv[i], "-vars") == 0)
        {
            if (varsN != 0)
            {
                out << "Warning: Can only specify -vars flag one time.\n"
                    << "         Ignoring second variable list." << endl;
                // Skip over second variable list.
                while (i+1 < processableArgs && argv[i+1][0] != '-')
                {
                    i++;
                }
                continue;
            }
            int numVars = 0;
            int curVar  = i+1;
            while (curVar < processableArgs && argv[curVar][0] != '-')
            {
                int length = strlen(argv[curVar]);
                bool  onWhitespace = true;
                for (int j = 0 ; j < length ; j++)
                {
                    if (isspace(argv[curVar][j]))
                    {
                        if (! onWhitespace)
                        {
                            onWhitespace = true;
                            numVars++;
                        }
                    }
                    else
                    {
                        onWhitespace = false;
                    }
                }
                if (! onWhitespace)
                {
                    numVars++;
                }
                curVar++;
            }
            if (numVars > 0)
            {
                vars = new char *[numVars];
                while (i+1 < processableArgs && argv[i+1][0] != '-')
                {
                    int length = strlen(argv[i+1]);
                    bool  onWhitespace = true;
                    int   start = 0;
                    for (int j = 0 ; j < length ; j++)
                    {
                        if (isspace(argv[i+1][j]))
                        {
                            if (! onWhitespace)
                            {
                                onWhitespace = true;
                                if ( (j - start) > 0 )
                                {
                                    vars[varsN] = new char[j-start+1];
                                    strncpy(vars[varsN], argv[i+1]+start, 
                                                         j-start);
                                    vars[varsN][j-start] = '\0';
                                    varsN++;
                                }
                            }
                        }
                        else
                        {
                            if (onWhitespace)
                            {
                                start = j;
                            }
                            onWhitespace = false;
                        }
                    }
                    if (! onWhitespace)
                    {
                        if ( (length - start) > 0 )
                        {
                            vars[varsN] = new char[length-start+1];
                            strncpy(vars[varsN], argv[i+1]+start,length-start);
                            vars[varsN][length-start] = '\0';
                            varsN++;
                        }
                    }
                    i++;
                }
            }
            if (varsN == 0)
            {
                out << "Warning: Must specify a variable list with the -vars"
                    <<                            " flag.\n"
                    << "         Using all variables." << endl;
            }
        }   // end case for -vars

        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0
                    || strcmp(argv[i], "-?") == 0)
        {
            // Just want the usage, so no output.
            return -1;
        }

        else
        {
            out << "Warning: Unrecognized flag \"" << argv[i] << "\".\n"
                << "         Ignoring..." << endl;
        }
    }

    return 0;
}


// ****************************************************************************
//  Method: CommandLine::Usage
//
//  Purpose:
//      Prints a message to the user about how to invoke meshtvprep.  
//
//  Arguments:
//      out     The ostream object to print the usage to.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//    Added output for siloobj flag.
//
//    Hank Childs, Mon Jun 26 09:55:14 PDT 2000
//    Print that resolution for resampling can be specified for each dimension.
//
// ****************************************************************************

void
CommandLine::Usage(ostream &out)
{
out 
    << "Usage: meshtvprep [-none] [-lres [resolution][resY resZ]]\n"
    << "       [-mres [resolution][resY resZ]] [-matbnd] [-connect] [-bsp]\n"
    << "       [-vars variable_list] [-nfiles #_files_per_state]\n"
    << "       -o output_file_base input_file_list\n"
    << endl
    << "       When using the -lres, -mres, -matbnd, -connect, -bsp, and \n"
    << "       -vars options, they should be preceded by the -none option to\n"
    << "       turn off all output.  The default is that -lres, -mres, \n"
    << "       -matbnd, -connect, -bsp, and -siloobj are all on.\n"
    << endl
    << "        Command line options are: \n"
    << endl
    << "        -none     Turn off all output, usually preced options which\n"
    << "                  select specific types of output.\n"
    << "        -lres     Output a regularly sampled low resolution version \n"
    << "                  of the specified variables.  The optional \n"
    << "                  resolution specifies the number of samples to be \n"
    << "                  taken in each direction.  The default resolution \n" 
    << "                  is " << lresN_DEFAULT << ".  One parameters may\n"
    << "                  be specified to specify the number of samples for\n"
    << "                  all dimensions or three parameters to specify each\n"
    << "                  dimension.\n"
    << "        -mres     Output a regularly sampled medium resolution \n"
    << "                  version of the specified variables.  The optional\n"
    << "                  resolution specifies the number of samples to be\n"
    << "                  taken in each direction.  The default resolution\n"
    << "                  is " << mresN_DEFAULT << ".  One parameters may\n"
    << "                  be specified to specify the number of samples for\n"
    << "                  all dimensions or three parameters to specify each\n"
    << "                  dimension.\n"
    << "        -matbnd   Output the material boundaries and the specified\n"
    << "                  variables on the material boundaries.\n"
    << "        -connect  Output the neighboring cell information for any\n"
    << "                  unstructures meshes.\n"
    << "        -bsp      Output an interval tree of the domains organized\n"
    << "                  in a binary space partitioning tree for all the\n"
    << "                  specified fields.\n"
    << "        -siloobj  Convert the Silo native object to VisIt's silo\n"
    << "                  format.\n"
    << "        -vars     Only process the specified variables.  The default\n"
    << "                  is to process al the variables.\n"
    << "        -nfiles   The number of files to split a single time state\n"
    << "                  into.  The default value is "
    <<                                        nfiles_DEFAULT_WORD << ".\n"
    << "        -o        The base name to use for naming the output files.\n"
    << flush;
}


// ****************************************************************************
//  Function: PrintSelf
//
//  Purpose:
//      Prints the command line object.  Intended for debugging purposes only.
//
//  Arguments:
//      out    The ostream to print out to.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

void
CommandLine::PrintSelf(ostream &out)
{
    out << "Switches: " << endl;
    out << "\tbsp     = " << (bsp?"true":"false") << endl;
    out << "\tconnect = " << (connect?"true":"false") << endl;
    out << "\tlres    = " << (lres?"true":"false") << endl;
    out << "\tmres    = " << (mres?"true":"false") << endl;
    out << "\tmatbnd  = " << (matbnd?"true":"false") << endl;
    out << "Basename:  \"" << basename << "\"." << endl;
    out << "Inputname: \"" << inputname << "\"." << endl;
    out << "lresN  = " << lresN << endl;
    out << "mresN  = " << mresN << endl;
    out << "nfiles = " << nfiles << endl;
    if (varsN == 0)
    {
        out << "All variables should be processed." << endl;
    }
    else
    {
        out << "Var list: " ;
        for (int i = 0 ; i < varsN ; i++)
            out << vars[i] << " ";
        out << endl;
    }
}


// ****************************************************************************
//  Method: CommandLine::GetLowResolution
//
//  Purpose:
//      Copies over the resolution for resampling in each dimension.
//
//  Arguments:
//      out    The array to copy the resolutions into.
//
//  Programmer: Hank Childs
//  Creation:   June 26, 2000
//
// ****************************************************************************

void
CommandLine::GetLowResolution(int out[3])
{
    out[0] = lresN[0];
    out[1] = lresN[1];
    out[2] = lresN[2];
}


// ****************************************************************************
//  Method: CommandLine::GetMedResolution
//
//  Purpose:
//      Copies over the resolution for resampling in each dimension.
//
//  Arguments:
//      out    The array to copy the resolutions into.
//
//  Programmer: Hank Childs
//  Creation:   June 26, 2000
//
// ****************************************************************************

void
CommandLine::GetMedResolution(int out[3])
{
    out[0] = mresN[0];
    out[1] = mresN[1];
    out[2] = mresN[2];
}


