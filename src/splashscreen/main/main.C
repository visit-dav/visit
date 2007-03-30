#include <SplashScreenApplication.h>
#include <VisItException.h>
#include <qapplication.h>
#if defined(_WIN32)
#include <Init.h>
#endif

// ****************************************************************************
// Method: main
//
// Purpose: 
//   This is the main function for the splashscreen application.
//
// Arguments:
//   argc : The number of command line arguments.
//   argv : The command line arguments.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 4 08:57:36 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 4 08:59:40 PDT 2001
//   Modified the code so it catches VisItExceptions.
//
//   Brad Whitlock, Wed Aug 21 16:14:11 PST 2002
//   I added the call to Init::Initialize back in after it was removed. The
//   splashscreen crashes on Windows without that function call. Frankly,
//   I don't know how other platforms keep working after it was removed.
//
//    Jeremy Meredith, Tue Jan 14 17:12:34 PST 2003
//    Added setColorSpec(ManyColor) so that it will try to get a 24-bit
//    visual for the GUI windows, even if 8-bit is the default.
//
// ****************************************************************************

int
main(int argc, char **argv)
{
    int retval = 0;

#if defined(_WIN32)
    // Initialize error logging.
    Init::Initialize(argc, argv);
#endif

    TRY
    {
        QApplication::setColorSpec(QApplication::ManyColor);
        SplashScreenApplication app(argc, argv);
        app.Go();
    }
    CATCH(VisItException)
    {
        retval = -1;
    }
    ENDTRY

    return retval;
}
