// ************************************************************************* //
//                            VisWindowManager.C                             //
// ************************************************************************* //

#include <iostream.h>

#include <VisWindowManager.h>


// ****************************************************************************
//  Method: VisWindowManager constructor
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 17:13:55 PDT 2000
//    Removed unused variable toplevel.
//
// ****************************************************************************

VisWindowManager::VisWindowManager(int *argc, char **argv)
{
    //
    // Allocate memory for the vis windows.
    //
    windows  = new VisWindow*[10];
    nWindows = 10;
    iWindows = 0;

    //
    // Initialize the X stuff
    //
    XtSetLanguageProc(NULL, NULL, NULL);
    XtVaAppInitialize(&app, "Visit", NULL, 0, argc, argv, NULL, NULL);
}


// ****************************************************************************
//  Method: VisWindowManager destructor
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

VisWindowManager::~VisWindowManager()
{
    if (windows != NULL)
    {
        for (int i = 0 ; i < iWindows ; i++)
        {
            if (windows[i] != NULL)
            {
                delete windows[i];
                windows[i] = NULL;
            }
        }
        delete [] windows;
        windows = NULL;
    }
}


// ****************************************************************************
//  Method: VisWindowManager::AddWindow
//
//  Purpose:
//      Adds a window.
//
//  Returns:    The index associated with the window.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

int
VisWindowManager::AddWindow()
{
    if (iWindows+1 >= nWindows)
    {
        //
        // Allocate more memory to hold the windows.
        //
        VisWindow **old_windows = windows;
        nWindows *= 2;
        windows = new VisWindow*[nWindows];
        for (int i = 0 ; i < iWindows ; i++)
        {
            windows[i] = old_windows[i];
        }
        delete [] old_windows;
    }

    windows[iWindows] = new VisWindow(app);
    iWindows++;
    return (iWindows-1);
}


// ****************************************************************************
//  Method: VisWindowManager::GetWindow
//
//  Purpose:
//      Fetches the VisWindow associated with the specified index.
//
//  Arguments:
//      index     The index of the VisWindow of interest.
// 
//  Returns:    A pointer to the VisWindow.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

VisWindow *
VisWindowManager::GetWindow(int index)
{
    if (index < 0 || index >= iWindows)
    {
        cerr << "Specified invalid index " << index << endl;
        // throw
        return NULL;
    }

    return windows[index];
}


// ****************************************************************************
//  Method: VisWindowManager::Start
//
//  Purpose:
//      Starts the event handling.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

void
VisWindowManager::Start(void)
{
    XtAppMainLoop(app);
}


