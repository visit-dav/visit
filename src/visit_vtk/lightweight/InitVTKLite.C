// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                InitVTKLite.C                              //
// ************************************************************************* //

#include <InitVTKLite.h>

#include <vtkObjectFactory.h>
#include <vtkDebugStream.h>
#include <vtkVersion.h>
#include <vtkLogger.h>
#include <DebugStream.h>

//
// Include any classes that will override vtk classes.
//


//
// A callback for use with vtkLogger, allows capturing to debug logs.
// userData should be one of our DebugStreams.
// Currently only using DebugStream::Stream1().
//
auto vtk_logger_callback = [](void* userData, const vtkLogger::Message& message)
{
    std::ostream& s = *reinterpret_cast<std::ostream*>(userData);
    s << message.preamble << message.message << std::endl;
};

//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
//
class VISIT_VTK_LIGHT_API vtkVisItFactory : public vtkObjectFactory
{
  public:
    vtkVisItFactory();
    static vtkVisItFactory* New() { return new vtkVisItFactory; }
    const char* GetVTKSourceVersion() override;
    const char* GetDescription() override { return "vtkVisItFactory"; }

  protected:
    vtkVisItFactory(const vtkVisItFactory&);
    void operator=(const vtkVisItFactory&);
};

//
// Necessary for each object that will override a vtkObject.
//


const char*
vtkVisItFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

//
//  Create the overrides so that VTK will use VisIt's class instead.
//
vtkVisItFactory::vtkVisItFactory()
{
}


// ****************************************************************************
//  Method: InitVTKLite::Initialize
//
//  Purpose:
//      Initialize the VTK portion of the program.  Separated from Init so
//      there would not be a VTK dependence on the GUI.  The only
//      initialization that happens currently is the vtk print statements are
//      re-routed to the debug stream.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 10 18:27:54 PDT 2003
//    Register the factory that will allow VisIt to override vtkObjects.
//
//    Kathleen Bonnell, Wed Nov 12 16:51:56 PST 2003
//    Comment out the VisItFactory until it is actually needed again.
//
//    Hank Childs, Thu Jan 22 16:47:27 PST 2004
//    Renamed to InitVTKLite.
//
//    Kathleen Biagas, Wed Oct 1, 2025
//    Turn off writing to stderr for vtkLogger.
//    Redirect vtkLogger output to DebugStream1() via callback,
//    if logging turned on.
//
// ****************************************************************************

void
InitVTKLite::Initialize(const std::string &component)
{
    vtkDebugStream::Initialize();

    // Prevent vtk logger from writing to stderr.
    vtkLogger::SetStderrVerbosity(vtkLogger::Verbosity::VERBOSITY_OFF);
    if(DebugStream::Level1())
    {
        // for debug levels 1-2, 
        auto verbosityLevel = vtkLogger::Verbosity::VERBOSITY_ERROR;

        // Change the verbosity based on debug level.
        if(DebugStream::Level3())
        {
            verbosityLevel = vtkLogger::Verbosity::VERBOSITY_WARNING;
        }
        else if(DebugStream::Level5())
        {
            verbosityLevel = vtkLogger::Verbosity::VERBOSITY_INFO;
        }

        // only write to debug1 to prevent multiple versions of the
        // same message
        std::string callbackId = component + "_vtkLog";
        vtkLogger::AddCallback(callbackId.c_str(),
                               vtk_logger_callback,
                               &DebugStream::Stream1(),
                               verbosityLevel);
    }
#if 0
    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItFactory *factory = vtkVisItFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
#endif
}

