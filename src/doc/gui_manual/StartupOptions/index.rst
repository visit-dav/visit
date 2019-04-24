.. _StartupOptions:

Startup Options
---------------

You can get help on starting VisIt with the commands

::

    visit -help
    visit -fullhelp

For convenience, the output from ``visit -fullhelp`` is shown below.

USAGE: visit [options]::

    Interface options
    ---------------------------------------------------------------------------
        -gui                 Run with the Graphical User Interface (default).
        -cli                 Run with the Command Line Interface.

    Movie making options
    ---------------------------------------------------------------------------
        -movie               Run the CLI in a movie making mode. Must be
                             combined with -sessionfile. Will produce a simple
                             movie by drawing all the plots in the specified
                             session for every timestep of the database.

    Startup options
    ---------------------------------------------------------------------------
        -o <filename>        Open the specified data file at startup.
        -s <filename>        Run the specified VisIt script. Note: This
                             argument only takes effect with -cli or -movie.
        -sessionfile <filename>  Open the specified session file at startup
                             Note that this argument only takes effect with
                             -gui or -movie.
        -config <filename>   Initialize the viewer at startup using the named
                             config file.  If an absolute path is not given,
                             the file is assumed to be in the .visit directory.
        -noconfig            Don't process configuration files at startup.
        -launchengine <host> Launch an engine at startup. The <host> parameter
                             is optional. If it is not specified, the engine
                             will be launched on the local host. If you wish
                             to launch an engine on a remote host, specify 
                             the host's name as the <host> parameter.
        -nosplash            Do not display the splash screen at startup.

    Window options
    ---------------------------------------------------------------------------
        -small               Use a smaller desktop area/window size.
        -geometry <spec>     What portion of the screen to use.  This is a
                             standard X Windows geometry specification. This
                             option can be used to set the size of images
                             generated from scripts and movies.

        -viewer_geometry <spec> What portion of the screen the viewer windows
                                will use. This is a standard X Windows geometry
                                specification. This option overrides the
                                -geometry option that the GUI passes to the
                                viewer.

        -window_anchor <x,y> The x,y position on the screen where VisIt's GUI
                             will show its windows (Main window excluded).
        -style <style>       One of: windows,cde,motif,sgi.
        -locale <locale>     The locale that you want VisIt to use when displaying
                             translated menus and controls. VisIt will use the
                             default locale if the -locale option is not
                             provided.
        -background <color>  Background color for GUI.
        -foreground <color>  Foreground color for GUI.
        -nowin               Run with viewer windows off-screen (i.e. OSMesa).
                             This is typically used with the -cli option.
        -stereo              Enable active stereo, also known as the
                             page-flipping, or 'CrystalEyes' mode.
        -nowindowmetrics     Prevents X11 from grabbing and moving a test
                             widget used in calculating window borders. This
                             option can be useful if VisIt hangs when 
                             displaying to an Apple X-server.

    Version options
    ---------------------------------------------------------------------------
        -version             Do NOT run VisIt. Just print the current version.
        -git_version         Do NOT run VisIt. Just print the Git version it
                             was built from.
        -beta                Run the current beta version.
        -v <version>         Run a specified version. Specifying 2 digits,
                             such as X.Y, will run the latest patch release
                             for that version. Specifying 3 digits, such as
                             X.Y.Z, will run that specific version.

    Other resources for help
    ---------------------------------------------------------------------------
        run-time:            While running VisIt, look under the "Help" menu.
        on-line:             https://visit.llnl.gov
        email:               visit-users@ornl.gov


    ***************************************************************************
                              ADDITIONAL OPTIONS
    ***************************************************************************

    Parallel launch options
    ---------------------------------------------------------------------------
        Notes: All of these options are ordinarily obtained from host profiles.
        However, the command line options override anything in the profiles. 

        When parallel arguments are added but the engine is not the
        component being launched, -launchengine is implied.  Explicitly
        add -launchengine to launch a remote parallel engine.
    ---------------------------------------------------------------------------
        -setupenv            Use the VisIt script to set up the environment
                             for the engine on the compute nodes.
        -par                 Run the parallel version. This option is implied
                             by any of the other parallel options listed below.
        -l    <method>       Launch in parallel using the given method.
        -pl   <method>       Launch only the engine in parallel as specified.
        -la   <args>         Additional arguments for the parallel launcher.
        -sla  <args>         Additional arguments for the parallel sub-launcher.
        -np   <# procs>      The number of processors to use.
        -nn   <# nodes>      The number of nodes to allocate.
        -p    <part>         Partition to run in.
        -n    <name>         The parallel job name.
        -b    <bank>         Bank from which to draw resources.
        -t    <time>         Maximum job run time.
        -machinefile <file>  Machine file.
        -expedite            Makes DPCS give priority scheduling.

        -icet                In scalable rendering mode, use the IceT parallel
                             image compositor (default).
        -no-icet             Do not use the IceT parallel compositor.

    Hardware accelerated parallel (scalable) rendering options
    ---------------------------------------------------------------------------
        Notes: These options should only be used with parallel clusters that
        have graphics cards.  If you are using a serial version of VisIt, you
        are already getting hardware acceleration and these options are not 
        needed.  Furthermore, you must be in scalable rendering mode for VisIt
        to utilize a cluster's GPUs.  By default, VisIt is configured to
        switch into scalable rendering mode when rendering complexity exceeds
        a predefined limit.

        VisIt can manage the creation and tear down of X servers for you.  It
        will do this automatically if you specify the -launch-x parameter,
        but you can customize the process with the -x-args and -display
        parameters, which respect %l and %n format specifiers.

        See the VisIt wiki for more information:

          http://visitusers.org/index.php?title=Parallel_Hardware_Acceleration

    ---------------------------------------------------------------------------
        -hw-accel              Tells VisIt that it should use graphics cards.
        -n-gpus-per-node <int> Number of GPUs per node of the cluster (1).
        -launch-x              Tell VisIt to manage the X servers
        -no-launch-x           Let the cluster manager X servers [default]
        -display               Tells VisIt which display to use.
        -x-args '<string>'     Extra arguments to X server.

    Load balance options
    ---------------------------------------------------------------------------
        Note: Each time VisIt executes a pipeline the relevant domains for the
        execution are assigned to processors. This list of domains is sorted in
        increasing global domain number. The options below effect how domains
        in this list are assigned to processors. Assuming there are D domains
        and P processors...
    ---------------------------------------------------------------------------
        -lb-block            Assign the first D/P domains to processor 0, the
                             next D/P domains to processor 1, etc.
        -lb-stride           Assign every Pth domain starting from the first
                             to processor 0, every Pth domain starting from the
                             second to processor 1, etc.
        -lb-absolute         Assign domains by absolute domain number % P. This
                             guarantees a given domain is always processed
                             by the same processor but can also lead to poor
                             balance when only a subset of domains is selected.
        -lb-random           Randomly assign domains to processors.
        -allowdynamic        Dedicate one processor to spreading the work 
                             dynamically among the other processors.  This mode
                             has limitations in the types of queries it can 
                             perform.  Under development.
        -lb-stream           Similar to -lb-block, but have the domains travel
                             down the pipeline one at a time, instead of all
                             together.  Under development.

    Database differencing options
    ---------------------------------------------------------------------------
        Use the '-diff <ldb> <rdb>' option to run VisIt in a database
        differencing mode. VisIt will generate expressions to facilitate
        visualization and analysis of the difference between the left-database,
        <ldb>, and right-database, <rdb>. VisIt will open windows to display
        both the left and right databases as well as their difference.

        VisIt uses the Cross-Mesh Field Evaluation (CMFE) expression functions
        to help generate these differences. A CMFE function creates an instance
        of a variable from another (source) mesh on the specified (destination)
        mesh. VisIt can use two variants of CMFE expression functions depending
        on how similar the source and destination meshes are; connectivity-based
        (conn_cmfe) which assumes the underlying mesh(s) for the left and right
        databases have identical connectivity and position-based (pos_cmfe) which
        does not make this assumption. VisIt will attempt to automatically select
        which variant of CMFE expression to use based on some simple heuristics.
        For meshes with identical connectivity, conn_cmfe expressions are
        preferrable because they are higher performance and do not require VisIt
        to perform any interpolation. In fact, the conn_cmfe operation is
        perfectly anti-symmetric. That is <ldb> - <rdb> = -(<rdb> - <ldb>).
        The same cannot be said for pos_cmfe expressions. However, pos_cmfe
        expressions will attempt to generate useful results regardless of the
        similarity of the underlying meshes.

        Note that the differences VisIt will compute in this mode are single
        precision. This is true regardless of whether the input data is itself
        double precision. VisIt will convert double precision to single 
        precision before processing it. Although this is a result of earlier
        visualization-specific design requirements and constraints, the intention
        is that eventually double precision will be supported.

        Finally, be sure to bring up Controls->Macros in the GUI to find a set
        of useful operations specifically tailored to database differencing. Also,
        typing 'help()' (including the '()') at the python prompt after starting
        'visit -diff' will generate a more detailed help message.

        -diff <ldb> <rdb>    Indicate you wish to run VisIt in database
                             differencing mode and specify the two databases
                             to difference.

                             Note: All options occurring on the command-line
                             *after* the '-diff' option are treated as options
                             to the differencing script while all options
                             occurring *before* the '-diff' option are treated
                             as options to VisIt.

        -diffsum <ldb> <rdb> Run only the difference summary method of the
                             'visit -diff' script, in nowin mode so its fast,
                             print the results, and immediately exit.

        -force_pos_cmfe      Force use of position-based CMFE expressions.

    Advanced options
    ---------------------------------------------------------------------------
        -guesshost           Try to guess the client host name from one of
                             the SSH_CLIENT, SSH2_CLIENT, or SSH_CONNECTION
                             environment variables.
        -noloopback          Disable use of the 127.0.0.1 loopback device.
        -sshtunneling        Tunnel all remote connections through ssh.  NOTE:
                             this overrides values set in the host profiles.
        -noint               Disable interruption capability.
        -nopty               Run without PTYs.
        -verbose             Prints status information during pipeline
                             execution.
        -dir <directory>     Run a version of VisIt in the specified directory.
                             The directory argument should specify the 
                             path to a VisIt installation directory.
                             /bin is automatically appended to this path.
        -forceversion <ver>  Force the given version.  Overrides all
                             intelligent version selection logic.
        -publicpluginsonly   Disable all plugins but the default ones.
        -compiler <cc>       Require version built with the specified compiler.
        -objectmode <mode>   Require a specific object file mode.
        -forceinteractivecli Force the CLI to behave interactively, even if run
                             with no terminal; similar to python's '-i' flag.
        -fullscreen          Create the viewer window in full screen mode.
                             May not be compatible with all window managers.
        -viewerdisplay <dpy> Have the viewer use a different display than the
                             current value of DISPLAY.  Can be useful for
                             power wall displays with a separate console.
        -cycleregex <string> A regex-style regular expression to be used
                             in extracting cycle numbers from file names. It
                             is best to bracket this string in single
                             quotes (') to avoid shell interpretation of
                             special characters such as star (*). The format
                             of the string begins with an opening '<' character,
                             followed by the regular expression itself followed
                             by a closing '>' character, optionally followed by
                             a space ' ' character and sub-expression reference
                             to indicate which part of the regular expression is
                             the cycle number.  Default behavior is as if
                             -cycleregex '<([0-9]+)[^0-9]*\$> \\0'
                             was specified meaning the last sequence of one
                             or more digits before the end of the string found
                             is used as the cycle number. Do a 'man 7 regex'
                             to get more information on regular expression
                             syntax.
        -ui-bcast-thresholds <int1> <int2> 
                             Two integers controlling behavior of parallel
                             engine waiting in a broadcast for the next RPC
                             from the viewer. VisIt used to rely solely upon
                             MPI_Bcast for this. However, many implementations
                             of MPI_Bcast use a polling loop that winds up
                             keeping all processors busy and can make them
                             unuseable by other processes. This is particularly
                             bad for SMPs. So, VisIt implemented its own
                             broadcast using MPI's send/receive methods. <int1>
                             specifies the number of nanoseconds a processor
                             sleeps while polling for completion of the
                             broadcast. Specifying a value of zero (0) for <int1>
                             results in falling back to older behavior using
                             MPI's MPI_Bcast. <int1> effectively controls how
                             'busy' processors will be, polling for completion
                             of the broadcast. <int2> specifies the number of
                             seconds all processors should spin, polling as fast
                             as possible, checking for completion of the
                             broadcast BEFORE inserting sleeps into their
                             polling loops. <int2> effectively controls how
                             many seconds VisIt's server will be maximally
                             responsive (although also keeping all processors
                             occupied) before becoming more 'friendly' to
                             other processes on the same node. The defaults
                             are <int1> = 50000000 nanoseconds (1/20th of a sec)
                             and <int2> = 5 seconds meaning VisIt will spin
                             processors maximally for 5 seconds before inserting
                             sleeps such that polling happens at the rate of 20
                             times per second.
        -idle-timeout <int>  An integer representing the number of minutes an
                             engine is allowed to idle (e.g. sit there doing no
                             execution whatsoever, waiting for commands from
                             the viewer). If this timeout is reached, the engine
                             will terminate itself. The default is 480 minutes
                             (8 hours).
        -exec-timeout <int>  An integer representing the number of minutes an
                             executing engine is allowed to remain in the
                             execution of any single command from the viewer.
                             If this timeout is reached, the engine will
                             terminate itself. the default is 30 minutes.
                             Beware that among other things, this timeout
                             effects how long orphaned parallel processes will
                             hang around, tying up parallel compute resources,
                             following an exit-triggering error condition on
                             any one process.
                     
    Developer options (most for xml2... tools)
    ---------------------------------------------------------------------------
        -public              xml2cmake: force install plugins publicly
        -private             xml2cmake: force install plugins privately
        -clobber             Permit xml2... tools to overwrite old files
        -noprint             Silence debugging output from xml2... tools
        -outputtoinputdir    Force xml2... tools to write output files to
                             the directory containing the input XML file
        -arch                print supported architecture(s) and exit

    Debugging options
    ---------------------------------------------------------------------------
        Note: Debugging options may degrade performance 
    ---------------------------------------------------------------------------
        -debug <level>       Run with <level> levels of output logging.
                             <level> must be between 1 and 5. This will generate
                             debug logs (called 'vlogs' for ALL components.
                             Note that debug logs are unbuffered. However, if
                             you also specify 'b' immediately after the digit
                             indicating the debug level (e.g. '-debug 3b'), the
                             logs will be buffered. This can substantially improve
                             performance when a lot of debug output is generated.
                             However, also beware that when debug logs are buffered,
                             there isn't necessarily any guarantee they will contain
                             the most recent debug output just prior to a crash.
        -debug_<compname> <level>
                             Run specified component with <level> of output
                             logging. For example, '-debug_mdserver 4' will run
                             the mdserver with level 4 debugging. Multiple
                             '-debug_<compname> <level>' args are allowed.
        -debug_engine_rank <r>
                             Restrict debug output to the specified rank.
        -debug-processor-stride N
                             Have only every Nth processor output debug logs.
                             Prevents overwhelming parallel file systems.
        -clobber_vlogs       By default, VisIt maintains debug logs from the 5
                             most recent invocations or restarts of each VisIt
                             component. They are named something like
                             A.mdserver.5.vlog, A.engine_ser.5.vlog, etc with
                             the leading letter (A-E) indicating most to least
                             recent. The clobber_vlogs flag causes VisIt to remove
                             all debug logs and begin creating them anew.
        -vtk-debug           Turn on debugging of VTK objects used in pipelines.
        -pid                 Append process ids to the names of log files.
        -timing              Save timing data to files.
        -withhold-timing-output
                             Withhold timing output during execution. Prevents
                             output of timing information from affecting
                             performance.
        -never-output-timings
                             Never output timings files.  This is used when
                             you want the timer to be enabled (for usage by
                             developers to measure inner loops), but you
                             want to avoid blowing memory with the bookkeeping
                             for each and every timing call.
        -timing-processor-stride N
                             Have only every Nth processor output timing info.
                             Prevents overwhelming parallel file systems.
        -env                 Print env. variables VisIt will use when run.
        -dump (dump_dir)     Dump intermediate results from AVT filters,
                             scalably rendered images, and html pages.
                             Takes an optional argument that specifies the 
                             directory for -dump output files.
        -info-dump (dump_dir) 
                             Dump html pages only. 
                             Takes an optional argument that specifies the 
                             directory for -info-dump output files.
        -gdb <args> <comp>   Run gdb with <args> on component <comp>.
                             Default <args> is whitespace.
        -break <funcname>    Add the specified breakpoint in gdb.
        -xterm               With -gdb-something, run gdb in an xterm window.
        -newconsole          Run any VisIt component in a new console window.
        -totalview <args> <comp>
                             Run totalview with <args> on component <comp>.
                             Default <args> is whitespace.
        -valgrind <args> <comp>
                             Run valgrind with <args> on component <comp>.
                             Default <args> is --tool=memcheck --error-limit=no
                             --num-callers=50.
        -strace <args> <comp>
                             Run strace with <args> on component <comp>.
                             Default <args> is -ttt -T.

                             In the above, all arguments between the tool name
                             and the VisIt component name are treated as args
                             to the tool.

        -apitrace <args> <comp>
                             Run apitrace with <args> on component <comp>.
                             Default <args> is trace --api gl.

                             In the above, all arguments between the tool name
                             and the VisIt component name are treated as args
                             to the tool.

        -debug-malloc <args> <comp>
                             Run the component with the libMallocDebug library
                             on MacOS X systems. The libMallocDebug library
                             lets the MallocDebug application attach to the
                             instrumented application and retrieve memory
                             allocation statistics. The -debug-malloc flag
                             also sets up the environment for the leaks and
                             heap tools.

                             Printing heap allocations:
                             % visit -debug-malloc gui &
                             % Get the gui's <pid>
                             % heap <pid>

                             Printing memory leaks:
                             % visit -debug-malloc gui &
                             % Get the gui's <pid>
                             % leaks <pid>

                             Run with MallocDebug:
                             Perl does not seem to be happy with libMallocDebug
                             so you can run the GUI like this:
                             % visit -cli 
                             >>> OpenGUI('-debug-malloc', 'MallocDebug', 'gui')
                             Connect to the gui with MallocDebug and do your
                             sampling.

        -numrestarts <#>     Number of attempts to restart a failed engine.
        -quiet               Don't print the Running message.
        -protocol            Print the definitions of the state objects that
                             comprise the VisIt protocol so they can be compared
                             against the values on other computers.


.. toctree::
    :maxdepth: 2
