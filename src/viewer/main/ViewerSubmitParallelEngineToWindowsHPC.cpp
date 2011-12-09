/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <atlbase.h>

// Thank you Argonne for this snippet of import code!

/* The Microsoft.Hpc.Scheduler.tlb and Microsoft.Hpc.Scheduler.Properties.tlb type
    libraries are included in the Microsoft HPC Pack 2008 SDK. The type libraries are
    located in the "Microsoft HPC Pack 2008 SDK\Lib\i386" or \amd64 folder. Include the rename 
    attributes to avoid name collisions.
*/
#import <Microsoft.Hpc.Scheduler.tlb> named_guids no_namespace raw_interfaces_only \
    rename("SetEnvironmentVariable","SetHpcEnvironmentVariable") \
    rename("AddJob", "AddHpcJob")
#import <Microsoft.Hpc.Scheduler.Properties.tlb> named_guids no_namespace raw_interfaces_only 

#include <QDate>
#include <QApplication>
#include <Qwidget>

#include <CouldNotConnectException.h>
#include <DebugStream.h>
#include <LaunchProfile.h>
#include <MachineProfile.h>
#include <ViewerBase.h>
#include <ViewerProperties.h>
#include <vectortypes.h>

#include <algorithm>
#include <map>
#include <string>

// Thank you Internet for this bit of conversion code for BSTR/std::string.
namespace convert
{
    std::wstring str_to_wstr( const std::string& str )
    {
        std::wstring wstr( str.length()+1, 0 );
  
        MultiByteToWideChar( CP_ACP,
             0,
             str.c_str(),
             static_cast<int>(str.length()),
             &wstr[0],
             static_cast<int>(str.length()) );
        return wstr;
    }

    std::string wstr_to_str( const std::wstring& wstr )
    {
        size_t size = wstr.length();
        std::string str( size + 1, 0 );
  
        WideCharToMultiByte( CP_ACP,
             0,
             wstr.c_str(),
             static_cast<int>(size),
             &str[0],
             static_cast<int>(size),
             NULL,
             NULL );
        return str;
    }
}

//
// Simple wrapper for creating BSTR from std::string and to make sure they get
// auto-deleted.
//
class BSTRObject
{
public:
    BSTRObject(const std::string &s)
    {
        bstr = SysAllocString(convert::str_to_wstr(s).c_str());
    }

    ~BSTRObject()
    {
        if(bstr != NULL)
            SysFreeString(bstr);
    }
    
    BSTR bstr;
};

// ****************************************************************************
// Method: ViewerSubmitParallelEngineToWindowsHPC
//
// Purpose: 
//   Create a job for the engine on the Windows HPC cluster.
//
// Arguments:
//      remoteHost     The name of the Windows HPC cluster.
//      args           The arguments to pass to the engine (we'll ignore many
//                     of them because the command line we're given by the
//                     RemoteProcess/ExistingRemoteProcess classes is not
//                     formatted how we'd like).
//      data           A pointer to the MachineProfile that we will use to
//                     launch the engine.
//
// Note:       If we can't launch the engine then we'll throw an exception
//             that we can catch from higher up.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 5 16:03:47 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubmitParallelEngineToWindowsHPC(const std::string &remoteHost, 
    const stringVector &args, void *data)
{
    const char *mName = "ViewerSubmitParallelEngineToWindowsHPC: ";
    const MachineProfile *profile = (const MachineProfile *)data;
    const LaunchProfile *lp = profile->GetActiveLaunchProfile();

    debug1 << mName << "start" << endl;

    // These are the engine arguments we'll preserve.
    std::map<std::string,int> argCounts;
    argCounts["-ui-bcast-thresholds"] = 3;
    argCounts["-host"] = 2;
    argCounts["-port"] = 2;
    argCounts["-key"] = 2;
    argCounts["-debug"] = 2;
    argCounts["-idle-timeout"] = 2;
    argCounts["-exec-timeout"] = 2;
    argCounts["-dump"] = 2;
    argCounts["-noloopback"] = 1;
    argCounts["-timing"] = 1;
    argCounts["-forcestatic"] = 1;
    argCounts["-forcedynamic"] = 1;
    argCounts["-icet"] = 1;
    argCounts["-no-icet"] = 1;
    argCounts["-clobber_vlogs"] = 1;
    argCounts["-lb-block"] = 1;
    argCounts["-lb-absolute"] = 1;
    argCounts["-lb-random"] = 1;
    argCounts["-lb-stream"] = 1;
    argCounts["-allowdynamic"] = 1;

    // Pull out some of the important stuff and ignore the rest since the command line
    // will be a little malformed. We don't need the typical stuff like -v, -dir
    // and so on since we're not launching using a VisIt script.
    debug1 << mName << "remoteHost=" << remoteHost << ", args={";
    stringVector preservedArgs;
    for(size_t i = 1; i < args.size(); ++i)
    {
        std::string argv(args[i]);
        debug1 << argv << ", ";
        std::map<std::string,int>::const_iterator it = argCounts.find(argv);
        if(it != argCounts.end() && ((i + it->second-1) < args.size()))
        {
            for(int j = 0; j < it->second-1; ++j)
                debug1 << args[i+1+j] << ", ";
            if(std::find(preservedArgs.begin(), preservedArgs.end(), argv) == preservedArgs.end())
            {
                preservedArgs.push_back(argv);
                for(int j = 0; j < it->second-1; ++j)
                    preservedArgs.push_back(args[i+1+j]);
            }
            i += it->second-1;
        }
    }
    debug1 << "}" << endl;

    // Use COM to connect to the scheduler.
    bool connectionError = false;  
    CoInitialize(NULL);
    TRY
    {
        CComPtr<IScheduler> scheduler;
        scheduler.CoCreateInstance(CLSID_Scheduler);

        BSTRObject clusterName(remoteHost);

        // Connect to the scheduler
        if(scheduler->Connect(clusterName.bstr) == S_OK)
        {
            debug1 << mName << "Connected to scheduler for " << remoteHost << endl;

            // We're not running without windows so set the interface mode to 
            // non-console so we'll get a graphical password prompt.
            if(!ViewerBase::GetViewerProperties()->GetNowin())
            {
                QWidget *topWindow = NULL;
                foreach (QWidget *widget, QApplication::topLevelWidgets())
                {
                    if (topWindow == NULL || widget->windowTitle().indexOf("Window") != -1)
                    {
                        topWindow = widget;
                    }
                }
                if(topWindow != NULL)                
                    scheduler->SetInterfaceMode(VARIANT_FALSE, (long)topWindow->winId());
            }

            // Create a job
            CComPtr<ISchedulerJob> job = NULL;
            if(scheduler->CreateJob(&job) == S_OK)
            {
                debug1 << mName << "Created new job" << endl;

                BSTRObject jobName(std::string("VisIt Parallel Compute Job - ") +
                                   QDate::currentDate().toString().toStdString());
                job->put_Name(jobName.bstr);

                debug1 << "\tjob: unittype: core" << endl;
                job->put_UnitType(JobUnitType_Core);

                if(lp->GetTimeLimitSet())
                {
                    long runtime = 0L;
                    if(sscanf(lp->GetTimeLimit().c_str(), "%ld", &runtime) == 1)
                    {
                        debug1 << "\tjob: runtime=" << runtime << endl;
                        job->put_Runtime(runtime);
                    }
                }

                if(lp->GetNumNodesSet())
                {
                    debug1 << "\tjob: autocalculatemin: true" << endl;
                    debug1 << "\tjob: autocalculatemax: true" << endl;
                    job->put_AutoCalculateMin(VARIANT_TRUE);
                    job->put_AutoCalculateMax(VARIANT_TRUE);

                    debug1 << "\tjob: minimumnumberofnodes=" << lp->GetNumNodes() << endl;
                    job->put_MinimumNumberOfNodes((long)lp->GetNumNodes());
                }
                else
                {
                    debug1 << "\tjob: autocalculatemin: false" << endl;
                    debug1 << "\tjob: autocalculatemax: false" << endl;
                    job->put_AutoCalculateMin(VARIANT_FALSE);
                    job->put_AutoCalculateMax(VARIANT_FALSE);

                    debug1 << "\tjob: minimumnumberofcores=" << lp->GetNumProcessors() << endl;
                    debug1 << "\tjob: maximumnumberofcores=" << lp->GetNumProcessors() << endl;
                    job->put_MinimumNumberOfCores(lp->GetNumProcessors());
                    job->put_MaximumNumberOfCores(lp->GetNumProcessors());
                }

                debug1 << "\tjob: failontaskfailure: true" << endl;
                job->put_FailOnTaskFailure(VARIANT_TRUE);

                // Set some environment variables on the task. Maybe they should be on the job...
                BSTRObject kVISITHOME("VISITHOME");
                BSTRObject vVISITHOME(profile->GetDirectory());
                job->SetHpcEnvironmentVariable(kVISITHOME.bstr, vVISITHOME.bstr);
                debug1 << "\tjob: environmentvariable: VISITHOME=" << profile->GetDirectory() << endl;

                BSTRObject kVISITPLUGINDIR("VISITPLUGINDIR");
                BSTRObject vVISITPLUGINDIR(profile->GetDirectory());
                job->SetHpcEnvironmentVariable(kVISITPLUGINDIR.bstr, vVISITPLUGINDIR.bstr);
                debug1 << "\tjob: environmentvariable: VISITPLUGINDIR=" << profile->GetDirectory() << endl;

                CComPtr<ISchedulerTask> task = NULL;
                if(job->CreateTask(&task) == S_OK)
                {
                    BSTRObject taskName("VisIt Compute Engine");
                    task->put_Name(taskName.bstr);

                    // Create the command line for the parallel engine.
                    std::string cmdline;
                    if(lp->GetNumProcessors() > 1)
                        cmdline = std::string("mpiexec \"") + profile->GetDirectory() + std::string("\\engine_par.exe\"");
                    else
                        cmdline = std::string("\"") + profile->GetDirectory() + std::string("\\engine_ser.exe\"");
                    for(size_t i = 0; i < preservedArgs.size(); ++i)
                        cmdline += (std::string(" ") + preservedArgs[i]);
                    BSTRObject commandLine(cmdline);
                    task->put_CommandLine(commandLine.bstr);
                    debug1 << "\t\ttask: commandline=" << cmdline << endl;

                    debug1 << "\t\ttask: type=Basic" << endl;
                    task->put_Type(TaskType_Basic);

                    if(lp->GetNumNodesSet())
                    {
                        debug1 << "\t\ttask: minimumnumberofnodes=" << lp->GetNumNodes() << endl;
                        task->put_MinimumNumberOfNodes((long)lp->GetNumNodes());
                    }
                    else if(lp->GetNumProcessors() > 1)
                    {
                        debug1 << "\t\ttask: minimumnumberofcores=" << lp->GetNumProcessors() << endl;
                        debug1 << "\t\ttask: maximumnumberofcores=" << lp->GetNumProcessors() << endl;
                        task->put_MinimumNumberOfCores(lp->GetNumProcessors());
                        task->put_MaximumNumberOfCores(lp->GetNumProcessors());
                    }

                    debug1 << "\t\ttask: workDirectory=" << profile->GetDirectory() << endl;
                    BSTRObject workDir(profile->GetDirectory());
                    task->put_WorkDirectory(workDir.bstr);

                    debug1 << mName << "Adding task to job" << endl;
                    job->AddTask(task);

                    debug1 << mName << "Submitting job" << endl;
                                                // username, password
                    if(scheduler->SubmitJob(job, NULL, NULL) != S_OK)
                    {
                        debug1 << mName << "Could not submit the job" << endl;
                        connectionError = true;
                    }
                }
                else
                {
                    debug1 << mName << "Could not create the task" << endl;
                    connectionError = true;
                }
            }
            else
            {
                debug1 << mName << "Could not create the new job" << endl;
                connectionError = true;
            }
        }
        else
        {
            debug1 << mName << "Could not connect to the cluster" << endl;
            connectionError = true;
        }
    }
    CATCHALL
    {
        debug1 << mName << "Catching unknown exception" << endl;
        connectionError = true;
    }
    ENDTRY
    CoUninitialize();

    debug1 << mName << "end" << endl;

    if(connectionError)
    {
        EXCEPTION0(CouldNotConnectException);
    }
}
