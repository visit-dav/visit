#include <MDServerProxy.h>
#include <VisItException.h>
#include <HostProfile.h>
#include <string>
#include <unistd.h>
#include <Init.h>
#include <avtSIL.h>

using std::string;

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the test mdserver program.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   
//
// Modifications:
//   Brad Whitlock, Fri May 11 13:48:50 PST 2001
//   I added code to pass extra command line arguments down to the mdserver
//   proxy object. This allows us to pass things like "-beta".
//
//   Jeremy Meredith, Thu Feb 27 14:09:50 PST 2003
//   Some compilers don't understand std::strings in conjuction with the
//   <iostream.h> (instead of <iostream>) headers, so I worked around it.
//
//   Brad Whitlock, Fri Mar 28 12:54:37 PDT 2003
//   I updated things to conform the MDServerProxy's new interface.
//
//   Jeremy Meredith, Thu Oct  9 14:05:05 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    string hostName("localhost");

    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <hostname> [options]" << endl;
        exit(-1);
    }

    // Initialize error logging.
    Init::Initialize(argc, argv);
    Init::SetComponentName("mdserver");

    MDServerProxy *mdserver = new MDServerProxy();
    if(argc > 1)
    {
        hostName = string(argv[1]);
        for(int i = 2; i < argc; ++i)
            mdserver->AddArgument(argv[i]);
    }

    string cwd;
    const MDServerProxy::FileList *fl;
    avtDatabaseMetaData md;

TRY
{
    cout << "----------------------------------------" << endl;
    cout << "Creating MDServer" << endl;
    mdserver->Create(argv[1], HostProfile::MachineName, "", false, 0);

    cout << "----------------------------------------" << endl;
    cout << "Getting Directory" << endl;
    cwd = mdserver->GetDirectory();
    cout << "\tdirectory == " << cwd.c_str() << endl;


    cout << "----------------------------------------" << endl;
    cout << "Changine directory to '/'" << endl;
    mdserver->ChangeDirectory("/");

    cout << "Getting Directory" << endl;
    cwd = mdserver->GetDirectory();
    cout << "\tdirectory == " << cwd.c_str() << endl;


    cout << "----------------------------------------" << endl;
    cout << "Reading Files" << endl;
    fl = mdserver->GetFileList("*", false);
    cout << "\tDIRECTORIES:\n";
    for (int i=0; i<fl->dirs.size(); i++)
    {
        cout << "\t\t";
        if (fl->dirs[i].CanAccess()) cout << "+ "; else cout << "- ";
        cout << fl->dirs[i].name.c_str() << endl;
    }
    cout << "\tFILES:\n";
    for (int i=0; i<fl->files.size(); i++)
    {
        cout << "\t\t";
        if (fl->files[i].CanAccess()) cout << "+ "; else cout << "- ";
        cout << fl->files[i].size << "\t\t"<< fl->files[i].name.c_str() << endl;
    }


    cout << "----------------------------------------" << endl;
    cout << "Changine directory to '~meredith/visit/data/silo/3d'" << endl;
    mdserver->ChangeDirectory("~meredith/visit/data/silo/3d");

    cout << "Getting Directory" << endl;
    cwd = mdserver->GetDirectory();
    cout << "\tdirectory == " << cwd.c_str() << endl;

    cout << "----------------------------------------" << endl;
    cout << "Getting metadata for curv3d.silo" << endl;
    md = *(mdserver->GetMetaData("curv3d.silo"));
    cout << "Metadata = " << endl;
    md.Print(cout);

   
    cout << "----------------------------------------" << endl;
    cout << "Getting SIL for curv3d.silo" << endl;
    avtSIL s1(*(mdserver->GetSIL("curv3d.silo")));
    cout << "SIL = " << endl;
    s1.Print(cout);

    cout << "----------------------------------------" << endl;
    cout << "Getting metadata for ~meredith/visit/data/silo/2d/curv2d.silo" 
         << endl;
    md = *(mdserver->GetMetaData("~meredith/visit/data/silo/2d/curv2d.silo"));
    cout << "Metadata = " << endl;
    md.Print(cout);

    cout << "----------------------------------------" << endl;
    cout << "Getting SIL for ~meredith/data/curv2d.silo" << endl;
    avtSIL s2(*(mdserver->GetSIL("~meredith/data/curv2d.silo")));
    cout << "SIL = " << endl;
    s2.Print(cout);


    cout << "----------------------------------------" << endl;
    cout << "Closing MDServer" << endl;
    mdserver->Close();
}
CATCH2(VisItException, ve)
{
    cerr << "Caught a VisItException of type: " << ve.GetExceptionType().c_str() << endl;
}
ENDTRY

    cout << "Exiting...." << endl;

    return 0;
}
