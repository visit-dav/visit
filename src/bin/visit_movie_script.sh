#!/bin/csh

# Make sure there are enough arguments
if ($#argv < 5) then
    echo "Missing movie name and/or number of frames to generate ... exiting"
    echo "Usage: $0 <movie name> <start frame> <end frame> <max frames to render> <session file>"
    echo "       [-batch use the batch system]"
    echo "       [-jobs <number of batch jobs>]"
    echo "       [-version version of VisIt to execute]"
    echo "       [-launch launchMethod]"
    echo "       [-queue <queue>]"
    echo "       [-geometry <width>x<height>]"
    echo "       [-format <image format>]"
    echo "       [-nn <Number of nodes>]"
    echo "       [-npn <Number of processors per node>]"
    echo "       [-time <Maximum job run time>]"
    echo "       [-bank <Bank from which to draw resources>]"
    echo "       [-partition <Partition to utilize>]"
    echo ""
    echo "Examples where four frames are to be generated, 0,1,2, and 3. Each"
    echo "time VisIt is run it will produce two frames; 0 and 1, and 2 and 3."
    echo ""
    echo "Run VisIt twice via the command line"
    echo "    ./visit_movie_script_master.sh mymovie 0 3 2 movie.session"
    echo ""
    echo "Create a batch job that will run VisIt twice"
    echo "    ./visit_movie_script_master.sh mymovie 0 3 2 movie.session -batch"
    echo ""
    echo "Create two batch jobs, each batch job will run VisIt once"
    echo "    ./visit_movie_script_master.sh mymovie 0 3 2 movie.session -batch -jobs 2"
    exit
endif

# Get the base hostname
if ( `hostname` =~ "cooley"* ) then
    set hostname = `hostname | sed -e 's/login[0-9]*$//'`
else if ( `hostname` =~ "rhea"* ) then
    set hostname = `hostname | sed -e 's/-login[0-9]*[a-z]*$//'`
else
    set hostname = `hostname | sed -e 's/[0-9]*$//'`
endif

# Report which machine profile is being used.
if ("$hostname" == "ash") then
    echo "Using UCHPC ash profile"
else if ("$hostname" == "cooley") then
    echo "Using ANL cooley profile"
else if ("$hostname" == "rhea") then
    echo "Using ORNL rhea profile"
else if ("$hostname" == "surface") then
    echo "Using LLNL surface profile"
else
    echo "Using machine profile for $hostname ... use at your own risk."
endif

# Set the input arguments
set movieName = $1
set startFrame = $2
set endFrame = $3
set maxFramesToRender = $4
set sessionFile = $5

# Check for the session file to exist
if (! -e $sessionFile) then
    echo "The session file, $sessionFile does not exist ... exiting"
    exit
endif

# Check for the movie directory to exist
set movieRoot = $movieName:h

if ($movieRoot != $movieName) then
    if (! -e $movieRoot) then
        echo "The directory $movieRoot does not exist ... creating $movieRoot"
        mkdir -p $movieRoot

        if (! -e $movieRoot) then
            echo "The directory $movieRoot could not be created ... exiting"
            exit
        endif
    endif
endif

# Check for the visit executable to exist
set visit = `which visit`

if ("$visit:t" != "visit") then
    if (${?VISITHOME}) then
        set visit = $VISITHOME/bin/visit
    else if ("$hostname" == "ash") then
        set visit = "/uufs/chpc.utah.edu/sys/installdir/VisIt/install/bin/visit"
    else if ("$hostname" == "cooley") then
        set visit = "/soft/visualization/visit/bin/visit"
    else if ("$hostname" == "rhea") then
        set visit = "/sw/redhat6/visit/bin/visit"
    else if ("$hostname" == "surface") then
        set visit = "/usr/gapps/visit/bin/visit"
    else
        echo "Can not find VISITHOME or a machine for the visit executable ... exiting"
        exit
    endif

    if (! -e $visit) then
        echo "Can not find the visit executable in $visit ... exiting"
        exit
    endif
endif

# Process the the optional arguments to find whether running
# command line or using the batch queue.
set batch = "false"

set i = 6

while ($i <= $#argv)

    if (X${argv[$i]} =~ X-b*) then
       set batch = true
       break
    endif

    @ i = $i + 1
end

# Copy the original session file to a temporary session file that will be
# modifed.
set tmpSessionFile = "${movieName}_tmp.session"

if (-e $tmpSessionFile) then
    rm -f $tmpSessionFile
endif

cp $sessionFile $tmpSessionFile

# Use the temporary session file
set sessionFile = $tmpSessionFile

# The assumption is that the session file was creaetd via client -
# server. Thus the source and host profile (i.e. the machine name)
# must be modified now that the session file is on the remote machine.

# When running locally on the remote machine the source is no longer
# the remote machine name but "localhost"

# When running via the command line the machine profile hostname must
# be the local machine name. It may be the login node hostname
# (myhost-login1), or the internal machine name (myhost666),
# or the internal domain machine name (myhost1.mydomain), or
# a combination (myhost-login1.mydomain)

# When running via the batch system the machine profile hostname must
# be the local name as well.  It may be the first node assigned by the
# batch system (mynode123), or the internal machine name (myhost666),
# or the internal domain machine name (myhost.mydomain).

# Ash
# Before parsing the session replace certain parameters - machine dependent.
if ("$hostname" == "ash") then

    # Replace ash.chpc.utah.edu by localhost for the SourceMap
    sed -i 's/\"SOURCE00\" type=\"string\">ash[0-9]*.chpc.utah.edu:/\"SOURCE00\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE01\" type=\"string\">ash[0-9]*.chpc.utah.edu:/\"SOURCE01\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE02\" type=\"string\">ash[0-9]*.chpc.utah.edu:/\"SOURCE02\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE03\" type=\"string\">ash[0-9]*.chpc.utah.edu:/\"SOURCE03\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE04\" type=\"string\">ash[0-9]*.chpc.utah.edu:/\"SOURCE04\" type=\"string\">localhost:/g' $sessionFile
                    
    # Replace ash.chpc.utah.edu by localhost for the SourcePlugins
    sed -i 's/Field name=\"ash[0-9]*.chpc.utah.edu:/Field name=\"localhost:/g' $sessionFile
                
    if($batch == "true") then
        # Replace ash.chpc.utah.edu by ash.wasatch.peaks
        # for the MachineProfile host
        sed -i 's/\"host\" type=\"string\">ash[0-9]*.chpc.utah.edu/\"host\" type=\"string\">ash.wasatch.peaks/g' $sessionFile

    else #if($batch == "false") then
        # Replace ash.chpc.utah.edu by ash1.wasatch.peaks or ash2.wasatch.peaks
        # for the MachineProfile host
        set host = `hostname`
        sed -i "s/ash[0-9]*.chpc.utah.edu/${host}.wasatch.peaks/g" $sessionFile

#       if (`hostname` == "ash1") then
#           sed -i 's/\"host\" type=\"string\">ash[0-9]*.chpc.utah.edu/\"host\" type=\"string\">ash1.wasatch.peaks/g' $sessionFile
#        else if (`hostname` == "ash2") then
#           sed -i 's/\"host\" type=\"string\">ash[0-9]*.chpc.utah.edu/\"host\" type=\"string\">ash2.wasatch.peaks/g' $sessionFile
#       endif
    endif

else if ("$hostname" == "cooley") then

    # Replace ash.chpc.utah.edu by localhost for the SourceMap
    sed -i 's/\"SOURCE00\" type=\"string\">cooley.alcf.anl.gov:/\"SOURCE00\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE01\" type=\"string\">cooley.alcf.anl.gov:/\"SOURCE01\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE02\" type=\"string\">cooley.alcf.anl.gov:/\"SOURCE02\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE03\" type=\"string\">cooley.alcf.anl.gov:/\"SOURCE03\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE04\" type=\"string\">cooley.alcf.anl.gov:/\"SOURCE04\" type=\"string\">localhost:/g' $sessionFile
                    
    # Replace cooley.alcf.anl.gov by localhost for the SourcePlugins
    sed -i 's/Field name=\"cooley.alcf.anl.gov:/Field name=\"localhost:/g' $sessionFile
                
    if($batch == "true") then
        # Replace cooley.alcf.anl.gov by cooley.fst.alcf.anl.gov
        # for the MachineProfile host
        sed -i 's/\"host\" type=\"string\">cooley.alcf.anl.gov/\"host\" type=\"string\">cooley.fst.alcf.anl.gov/g' $sessionFile

    else #if($batch == "false") then
        # Replace cooley.alcf.anl.gov by cooleylogin#.fst.alcf.anl.gov
        # for the MachineProfile host
        set host = `hostname`
        sed -i "s/cooley.alcf.anl.gov/${host}.fst.alcf.anl.gov/g" $sessionFile
    endif

# Rhea
# Before parsing the session replace certain parameters - machine dependent.
else if ("$hostname" == "rhea") then

    # Replace rhea by localhost for the source
    sed -i 's/\"SOURCE00\" type=\"string\">rhea.ccs.ornl.gov:/\"SOURCE00\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE01\" type=\"string\">rhea.ccs.ornl.gov:/\"SOURCE01\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE02\" type=\"string\">rhea.ccs.ornl.gov:/\"SOURCE02\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE03\" type=\"string\">rhea.ccs.ornl.gov:/\"SOURCE03\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE04\" type=\"string\">rhea.ccs.ornl.gov:/\"SOURCE04\" type=\"string\">localhost:/g' $sessionFile

    # Replace rhea by localhost for the field name
    sed -i 's/Field name=\"rhea.ccs.ornl.gov:/Field name=\"localhost:/g' $sessionFile

    if($batch == "true") then
        # Replace rhea.ccs.ornl.gov by rhea.ccs.ornl.gov
        # for the MachineProfile host
        # This step is done in the batch script as there is no local name.
#       sed -i 's/\"host\" type=\"string\">rhea.ccs.ornl.gov/\"host\" type=\"string\">rhea.ccs.ornl.gov/g' $sessionFile

    else #if($batch == "false") then
        # Replace rhea.ccs.ornl.gov by rhea-login#g.ccs.ornl.gov
        # for the MachineProfile host
        set host = `hostname`
        sed -i "s/rhea.ccs.ornl.gov/${host}.ccs.ornl.gov/g" $sessionFile
    endif

# Surface
# Before parsing the session replace certain parameters - machine dependent.
else if ("$hostname" == "surface") then

    # Replace surface by localhost for the source
    sed -i 's/\"SOURCE00\" type=\"string\">surface.llnl.gov:/\"SOURCE00\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE01\" type=\"string\">surface.llnl.gov:/\"SOURCE01\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE02\" type=\"string\">surface.llnl.gov:/\"SOURCE02\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE03\" type=\"string\">surface.llnl.gov:/\"SOURCE03\" type=\"string\">localhost:/g' $sessionFile
    sed -i 's/\"SOURCE04\" type=\"string\">surface.llnl.gov:/\"SOURCE04\" type=\"string\">localhost:/g' $sessionFile

    # Replace surface by localhost for the field name
    sed -i 's/Field name=\"surface.llnl.gov:/Field name=\"localhost:/g' $sessionFile

    # Replace surface by surface86 for the MachineProfile host
    sed -i 's/\"host\" type=\"string\">surface.llnl.gov/\"host\" type=\"string\">surface86.llnl.gov/g' $sessionFile
endif


# Parse the session file and locate the Machine Profiles. If they exist
# parse the format, height, and width.

set runningEnginesFound =  `grep '"RunningEngines"' $sessionFile | sed 's/^[^"]*"\([^"]*\)".*/\1/'`

if ("$runningEnginesFound" == "") then
    echo "The session file does not contain any runningEngine information"
    echo "and will cause problems getting the right type of engine launched."
    echo "This will occur if the session file was saved after an engine"
    echo "crash  ... exiting."
    echo ""
    echo "Note: The session file can still be used interactively. Start the"
    echo "VisIt GUI, restore the session, and resave the session once the"
    echo "engine has properly launched and all plots have been restored."
    exit
endif

#sed -e '1,/"hostNickname" type="string">ash/d' pal_session | sed -n '/\<LaunchProfile\>/,/\<\<parallel\>/{/\<LaunchProfile\>/{h;d};H;/\<\<parallel\>/{x;p}}'

set version = ""

# NOTE if any parallel args are provided on the script command line
# all are needed. So get them from the session file regardless of
# running from the command line or using the batch queue.

#if ($batch == false) then
#    set queue = ""
#    set launchMethod = ""
#    set launchArgs = ""
#    set nNodes = ""
#    set nProcsPerNode = ""
#    set time = ""
#    set bank = ""
#    set partition = ""
#else
    # Launch method and queue
    set launchMethodSet = `grep '"launchMethodSet"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`

    if ("$launchMethodSet" == "true") then
        set launchMethod = `grep '"launchMethod"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    else if ("$hostname" == "ash") then
        set launchMethod = sbatch/mpirun
    else if ("$hostname" == "cooley") then
        set launchMethod = qsub/mpirun
    else if ("$hostname" == "rhea") then
        set launchMethod = qsub/mpirun
    else if ("$hostname" == "surface") then
        set launchMethod = msub/srun
    else
        echo "Can not figure out a launchMethod for this machine : $hostname ... bailing out"
        exit
    endif

    set queue = $launchMethod:h
    set launchMethod = $launchMethod:t

    # Addiional launch arguments
    set launchArgsSet = `grep '"launchArgsSet"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    if ("$launchArgsSet" == "true") then
        set launchArgs = `grep '"launchArgs"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//' -e 's/"//g'`
    else
        set launchArgs = ""
    endif

    # Number of nodes
    set nNodesSet = `grep '"numNodesSet"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    if ("$nNodesSet" == "true") then
        set nNodes = `grep '"numNodes"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    else
        set nNodes = 4
    endif

    set nProcs = `grep '"numProcessors"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`

    # Total number of processors (ranks)
    if ("$nProcs" != "") then
        @ nProcsPerNode = $nProcs / $nNodes
    else if ("$hostname" == "ash") then
        set nProcsPerNode = 12
    else if ("$hostname" == "cooley") then
        set nProcsPerNode = 12
    else if ("$hostname" == "rhea") then
        set nProcsPerNode = 16
    else if ("$hostname" == "surface") then
        set nProcsPerNode = 16
    else
        echo "Can not figure out a the number of processors per node for this machine : $hostname ... bailing out"
        exit
    endif

    # Time limit
    set timeLimitSet = `grep '"timeLimitSet"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    if ("$timeLimitSet" == "true") then
        set time = `grep '"timeLimit"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    else
        set time = "0:25:00"
    endif

    # Bank (project)
    set bankSet = `grep '"bankSet"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    if ("$bankSet" == "true") then
        set bank = `grep '"bank"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    else if ("$hostname" == "ash") then
        set bank = "smithp-ash-cs"
    else if ("$hostname" == "cooley") then
        set bank = "SoPE_2"
    else if ("$hostname" == "rhea") then
        set bank = "CMB122"
    else if ("$hostname" == "surface") then
        set bank = "utah"
    else
        echo "Can not figure out a the bank for this machine : $hostname ... bailing out"
        exit
    endif

    # Partition
    set partitionSet = `grep '"partitionSet"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    if ("$partitionSet" == "true") then
        set partition = `grep '"partition"' $sessionFile | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
    else if ("$hostname" == "ash") then
        set partition = "smithp-ash"
    else if ("$hostname" == "cooley") then
        set partition = ""
    else if ("$hostname" == "rhea") then
        set partition = ""
    else if ("$hostname" == "surface") then
        set partition = "pbatch"
    else
        set partition = ""
    endif
#endif

# Parse the session file and locate the Save Window Attributes. If they exist
# parse the format, height, and width.
set format = ""
set height = ""
set width  = ""

set windowAttsSet = `grep '"SaveWindowAttributes"' $sessionFile`
if ("$windowAttsSet" != "") then
    set format = `sed -e '1,/"SaveWindowAttributes"/d' -e '/<Object/,$d' $sessionFile | grep '"format"' | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//' -e 's/\(.*\)/\L\1/'`
    set height = `sed -e '1,/"SaveWindowAttributes"/d' -e '/<Object/,$d' $sessionFile | grep '"height"' | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`

    set width  = `sed -e '1,/"SaveWindowAttributes"/d' -e '/<Object/,$d' $sessionFile | grep '"width"'  | sed -e 's/<[^>]*>//g' -e 's/^[ \t]*//'`
else
    echo "Can not find SaveWindowAttributes, using internal defaults."
endif

# Rather than use the VisIt defaults use the following values.
if ("$format" == "") then
    set format = "tiff"
endif

if ("$height" == "") then
    set height = 512
endif

if ("$width" == "") then
    set width  = 1024
endif

set geometry = "${width}x${height}"
set nJobs = 1

# Process the rest of the optional arguments.
set i = 6
set j = 7

while ($i <= $#argv)

    if (X${argv[$i]} =~ X-b* ) then
       set batch = true

    else if (X${argv[$i]} =~ X-*) then

        if (X${argv[$i]} =~ X-v*) then             # version
            set version = ${argv[$j]}
        else if (X${argv[$i]} =~ X-l*) then        # launch method
            set launchMethod = ${argv[$j]}
        else if (X${argv[$i]} =~ X-q*) then        # queue
            set queue = ${argv[$j]}
        else if (X${argv[$i]} =~ X-nn) then        # number of nodes
            set nNodes = ${argv[$j]}
        else if (X${argv[$i]} =~ X-npn) then       # number of processors/nodes
            set nProcsPerNode = ${argv[$j]}
        else if (X${argv[$i]} =~ X-t*) then        # time
            set time = ${argv[$j]}
        else if (X${argv[$i]} =~ X-b*) then        # bank
            set bank = ${argv[$j]}
        else if (X${argv[$i]} =~ X-p*) then        # partition
            set partition = "${argv[$j]}"
        else if (X${argv[$i]} =~ X-g*) then        # geometry
            set geometry = "${argv[$j]}"
        else if (X${argv[$i]} =~ X-fo*) then       # format
            set format = "${argv[$j]}"
        else if (X${argv[$i]} =~ X-j*) then        # jobs
            set nJobs = "${argv[$j]}"
        else
            echo "Unknown option ${argv[$i]} ... exiting"
            echo "Usage: $0 <movie name> <start frame> <end frame> <frames per batch> <session file>"
            echo "       [-batch use the batch system]"
            echo "       [-jobs <number of batch jobs>]"
            echo "       [-version version of VisIt to execute]"
            echo "       [-launch launchMethod] [-queue <queue>]"
            echo "       [-geometry <width>x<height>] [-format <image format>]"
            echo "       [-nn <Number of nodes>] [-npn <Number of processors per node>] [-time <Maximum job run time>]"
            echo "       [-bank <Bank from which to draw resources>] [-partition <Partition to utilize>]"
            exit
        endif

        @ i = $i + 1
        @ j = $j + 1

    endif

    @ i = $i + 1
    @ j = $j + 1

end

# If needed calculate the number of nodes, processors per node, etc.
if ($batch == "true") then
    @ nProcs = $nNodes * $nProcsPerNode
else
    if( $nJobs > 1 ) then
        echo "Not running in batch mode ignoring the number of jobs request"
        set nJobs = 1
    endif
        
    if( $nNodes == "" && $nProcsPerNode == "") then
        set nProcs = ""
    else if ($nNodes == "") then
        set nNodes = 8
        @ nProcs = $nNodes * $nProcsPerNode
    else if ("$nProcsPerNode" == "") then
        set nProcsPerNode = 12
        @ nProcs = $nNodes * $nProcsPerNode
    else
        @ nProcs = $nNodes * $nProcsPerNode
    endif

    if ("$queue" != "" && $queue != $launchMethod) then
        set launchMethod = $queue/$launchMethod
        set queue = ""
    endif
endif

if ($batch == "true") then
    if (X"${launchArgs}" =~ X*-l*) then
      echo "The session file contains launch args ($launchArgs) that will conflict with $launchMethod. Edit the session file and set launchArgsSet to false."
      exit
    endif
endif

# Set some required default command line arguments
echo "Successfully parsed the session file and command line. The following parameters were found:"

if ($version != "")        echo "  VisIt version = $version"
if ($batch == "true")      echo "  Queue = $queue"
if ($launchMethod != "")   echo "  Launch method = $launchMethod"
if ("$launchArgs" != "")   echo "  Launch args = $launchArgs"
if ($nNodes != "")         echo "  Number of nodes = $nNodes"
if ($nProcs != "")         echo "  Number of processors = $nProcs"
if ($nProcsPerNode != "")  echo "  Number of processors per node = $nProcsPerNode"
if ($bank != "")           echo "  Bank = $bank"
if ($time != "")           echo "  Time = $time"
if ($partition != "")      echo "  Partition = $partition"

if ($format != "")         echo "  Format = $format"
if ($geometry != "")       echo "  Image size = $geometry"

if ($version != "") set version = "-v $version"

if ($batch == "false") then
    if ($launchMethod != "") set launchMethod = "-l $launchMethod"
    if ($nNodes != "")       set nNodes = "-nn $nNodes"
    if ($nProcs != "")       set nProcs = "-np $nProcs"
    if ($partition != "")    set partition = "-p $partition"
    if ($bank != "")         set bank = "-b $bank"
    if ($time != "")         set time = "-t $time"
endif

if ($format   != "") set format   = "-format $format"
if ($geometry != "") set geometry = "-geometry $geometry"

# Calculate the number frames and batches to generate.
@ nFrames = $endFrame - $startFrame + 1

if ($nFrames <= 0) then
    echo "The number of frames is zero or negative ... exiting"
    exit
endif

@ nFramesPerJob      = $nFrames / $nJobs
@ nExtraFramesPerJob = $nFrames % $nJobs

# Loop through and generate the jobs - the number of jobs will be greate
# than one only for the batch queue
if( $nJobs > 1 ) then
    echo "Creating $nJobs batch jobs."
endif

set job = 0

while ($job < $nJobs)

    if($job < $nExtraFramesPerJob) then
        @ start = $startFrame + $job * ($nFramesPerJob + 1)
        @ end   = $start + ($nFramesPerJob + 1) - 1
    else
        @ start = $startFrame + $job * ($nFramesPerJob) + $nExtraFramesPerJob
        @ end   = $start + $nFramesPerJob - 1
    endif

    # For each job figure out how many times VisIt must be run.
    @ nFramesToRender = $end - $start + 1
    @ nRuns           = $nFramesToRender / $maxFramesToRender

    if( $nFramesToRender % $maxFramesToRender != 0 ) then
        @ nRuns = $nRuns + 1
    endif

    set fullMovieName = "-output $movieName"

    # Batch
    if( $batch == "true" ) then

        if( $nJobs == 1 ) then
            echo "Creating $nRuns VisIt runs in a single batch job."
        endif

        set jobName = $movieName:t_job_$job
        echo "Creating frames $start through $end using job name $jobName"

# $queue = qsub && $launchmethod = mpirun
        if ("$hostname" == "QSUB_STDIN") then
#            echo "$queue -l nodes=${nNodes}:ppn=$nProcsPerNode,walltime=$time $launchArgs -A $bank -N $jobName"

#            echo "cd $cwd"

#            echo "$visit $version -movie -start $start -end $end -frame $start $geometry $format $fullMovieName -sessionfile $sessionFile -ignoresessionengine -par -l $launchMethod -nn $nNodes -np $nProcs -bank $bank -p $partition -machinefile \${PBS_NODEFILE}"

# No indentation because white spaces are passed to qsub and
# will screw up the EOF sentinal. 
     $queue -l nodes=${nNodes}:ppn=$nProcsPerNode,walltime=$time $launchArgs -A $bank -N $jobName << EOF

cd $cwd

$visit $version -movie -start $start -end $end -frame $start $geometry $format $fullMovieName -sessionfile $sessionFile -ignoresessionengines -par -l $launchMethod -nn $nNodes -np $nProcs -bank $bank -p $partition -machinefile \${PBS_NODEFILE}

EOF

# Ash
# $queue = sbatch && $launchmethod = mpirun
        else if ("$hostname" == "ash") then

            echo "Creating $queue script for $jobName"
            /bin/rm -f $jobName
            /bin/touch $jobName
            echo "#! /bin/csh" >> $jobName
            echo "#SBATCH -N $nNodes" >> $jobName
            echo "#SBATCH -n $nProcs" >> $jobName
            echo "#SBATCH -p $partition" >> $jobName
            echo "#SBATCH -A $bank" >> $jobName
            echo "#SBATCH -t $time" >> $jobName
            if ("$launchArgs" != "") echo "#SBATCH $launchArgs" >> $jobName
            echo "#SBATCH -J $jobName" >> $jobName
            echo " " >> $jobName
            echo "cd $cwd" >> $jobName
            echo " " >> $jobName
            echo '#==============visit command below======================'  >> $jobName
            set run = 0

            while ($run < $nRuns)
                @ runStart = $start + $run * $maxFramesToRender
                @ runEnd   = $runStart + $maxFramesToRender - 1

                if( $runEnd > $end ) set runEnd = $end

#               echo 'srun hostname | sort > nodefile.${SLURM_JOBID}' >> $jobName
                echo -n "$visit $version " >> $jobName
                echo -n "-movie -start $runStart -end $runEnd " >> $jobName
                echo -n "-frame $runStart " >> $jobName
                echo -n "$geometry $format $fullMovieName " >> $jobName
                echo -n "-sessionfile $sessionFile " >> $jobName
                echo -n "-ignoresessionengines " >> $jobName
                echo -n "-par -l $launchMethod " >> $jobName
                echo -n "-nn $nNodes -np $nProcs " >> $jobName
                #echo-n '-machinefile nodefile.${SLURM_JOBID}' >> $jobName
                echo " " >> $jobName
                @ run = $run + 1
            end

            echo "Submitting $queue script for $jobName"
            $queue $jobName

# Cooley
# $queue = qsub && $launchmethod = mpirun
        else if ("$hostname" == "cooley") then

            echo "Creating cobalt script for $jobName"
            /bin/rm -f $jobName
            /bin/touch $jobName
            /bin/chmod 755 $jobName
            echo "#! /bin/csh" >> $jobName
            echo " " >> $jobName
            echo "cd $cwd" >> $jobName
            echo " " >> $jobName
            echo '#==============visit command below======================'  >> $jobName
            set run = 0

            while ($run < $nRuns)
                @ runStart = $start + $run * $maxFramesToRender
                @ runEnd   = $runStart + $maxFramesToRender - 1

                if( $runEnd > $end ) set runEnd = $end

                echo -n "$visit $version " >> $jobName
                echo -n "-movie -start $runStart -end $runEnd " >> $jobName
                echo -n "-frame $runStart " >> $jobName
                echo -n "$geometry $format $fullMovieName " >> $jobName
                echo -n "-sessionfile $sessionFile " >> $jobName
                echo -n "-ignoresessionengines " >> $jobName
                echo -n "-par -l $launchMethod " >> $jobName
                echo -n "-nn $nNodes -np $nProcs " >> $jobName
                echo -n '-machinefile ${COBALT_NODEFILE} ' >> $jobName
                echo " " >> $jobName
                @ run = $run + 1
            end

            echo "Submitting $queue script for $jobName"
            $queue --nodecount=$nNodes --proccount=$nProcs --project=$bank --time=$time $launchArgs --jobname=$jobName --mode=script $jobName


# Rhea
# $queue = qsub && $launchmethod = mpirun
        else if ("$hostname" == "rhea") then

            # Create a job specific session file as the Machine Profile will
            # need to specify the first node allocated.
            set jobSessionFile = "${jobName}.session"

            if (-e $jobSessionFile) then
                rm -f $jobSessionFile
            endif

            cp $sessionFile $jobSessionFile

            echo "Creating $queue script for $jobName"
            /bin/rm -f $jobName
            /bin/touch $jobName
            echo "#! /bin/csh" >> $jobName
            # For rhea do not add procs because one gets the whole node.
            echo "#PBS -l nodes=${nNodes},walltime=$time" >> $jobName
            echo "#PBS -A $bank" >> $jobName
            if ("$launchArgs" != "") echo "#PBS $launchArgs" >> $jobName
            echo "#PBS -N $jobName" >> $jobName
            echo " " >> $jobName

            # add xalt because the default throw errors
            echo "module add xalt/0.7.5" >> $jobName
            echo " " >> $jobName
            echo "cd $cwd" >> $jobName
            echo " " >> $jobName

            # Set the hostname to be the first node allocated 
            # in the session file.
            echo 'set host = `hostname`' >> $jobName
            echo " " >> $jobName
            echo -n 'sed -i "s/rhea.ccs.ornl.gov/${host}.ccs.ornl.gov/g" ' >> $jobName
            echo "$jobSessionFile " >> $jobName
            echo " " >> $jobName
            echo '#==============visit command below======================'  >> $jobName
            set run = 0

            while ($run < $nRuns)
                @ runStart = $start + $run * $maxFramesToRender
                @ runEnd   = $runStart + $maxFramesToRender - 1

                if( $runEnd > $end ) set runEnd = $end

                echo -n "$visit $version " >> $jobName
                echo -n "-movie -start $runStart -end $runEnd " >> $jobName
                echo -n "-frame $runStart " >> $jobName
                echo -n "$geometry $format $fullMovieName " >> $jobName
                echo -n "-sessionfile $jobSessionFile " >> $jobName
                echo -n "-ignoresessionengines " >> $jobName
                echo -n "-par -l $launchMethod " >> $jobName
                echo -n "-nn $nNodes -np $nProcs " >> $jobName
                echo -n '-machinefile ${PBS_NODEFILE} ' >> $jobName
                echo " " >> $jobName
                @ run = $run + 1
            end

            echo "Submitting $queue script for $jobName"
            $queue $jobName

# Surface           
# $queue = msub && $launchmethod = srun
        else if ("$hostname" == "surface") then

            echo "Creating msub script for $jobName"
            /bin/rm -f $jobName
            /bin/touch $jobName
            echo "#MSUB -S /bin/csh" >> $jobName
            echo "#MSUB -q pbatch" >> $jobName
            echo "#MSUB -l nodes=${nNodes}:ppn=$nProcsPerNode" >> $jobName
            echo "#MSUB -l partition=$hostname" >> $jobName
            echo "#MSUB -A $bank" >> $jobName
            echo "#MSUB -l walltime=$time" >> $jobName
            if ("$launchArgs" != "") echo "#MSUB $launchArgs" >> $jobName
            echo "#MSUB -N $jobName" >> $jobName
            echo " " >> $jobName
            echo "cd $cwd" >> $jobName
            echo " " >> $jobName
            echo '#==============visit command below======================'  >> $jobName
            set run = 0

            while ($run < $nRuns)
                @ runStart = $start + $run * $maxFramesToRender
                @ runEnd   = $runStart + $maxFramesToRender - 1

                if( $runEnd > $end ) set runEnd = $end

                echo -n "$visit $version " >> $jobName
                echo -n "-movie -start $runStart -end $runEnd " >> $jobName
                echo -n "-frame $runStart " >> $jobName
                echo -n "$geometry $format $fullMovieName " >> $jobName
                echo -n "-sessionfile $sessionFile " >> $jobName
                echo -n "-ignoresessionengines " >> $jobName
                echo -n "-par -l $launchMethod " >> $jobName
                echo -n "-nn $nNodes -np $nProcs" >> $jobName
                echo " " >> $jobName
                @ run = $run + 1
            end

            echo "Submitting $queue script for $jobName"
            $queue $jobName

        endif

# Sequential
    else
        echo "Running $nRuns VisIt runs via the command line."

        set run = 0

        while ($run < $nRuns)
            @ runStart = $start + $run * $maxFramesToRender
            @ runEnd   = $runStart + $maxFramesToRender - 1

            if( $runEnd > $end ) set runEnd = $end
        
            set logFile = $movieName$run.script_log
            echo "Creating frames $runStart through $runEnd using logfile $logFile"

            /bin/rm -f $logFile
            /bin/touch $logFile
            
            echo "$visit $version -movie -start $runStart -end $runEnd -frame $runStart $geometry $format $fullMovieName -sessionfile $sessionFile -par $launchMethod $nNodes $nProcs $time $bank $partition" >> $logFile

            nohup $visit $version -movie -start $runStart -end $runEnd -frame $runStart $geometry $format $fullMovieName -sessionfile $sessionFile -par $launchMethod $nNodes $nProcs $time $bank $partition >>&! $logFile

            @ run = $run + 1
        end
    endif

    @ job = $job + 1
end
