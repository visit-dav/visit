#!bash
#$Id: shellfuncs.sh,v 1.1 2008/09/26 20:01:42 rcook Exp $
# TO DO : 
# boolean support should be added for getargs
# it is not an error to have undefined variables:
set +u
# could be useful to avoid multiple parsing
export shellfuncs_parsed=1
#=================================
# these scripts assume sed and egrep are in PATH, so set $PATH 
#if ! which egrep >/dev/null 2>&1; then 
    export PATH=$PATH:/usr/bin:/bin
 #   if ! which egrep >/dev/null 2>&1; then 
#    echo egrep command not found, cannot continue parsing shellfuncs.sh
#    exit 1
#    fi
#fi
#=================================
#if ! which sed >/dev/null 2>&1; then 
#    export PATH=$PATH:/usr/bin:/bin
#    if ! which sed  >/dev/null 2>&1 ; then 
#    echo sed command not found, cannot continue parsing shellfuncs.sh
#    exit 1
#    fi
#fi
#=================================
#=================================
function hostbasename () {
    uname -n | sed 's/[0-9]*//g'
}

#================================
# $1 is array to append to
# the rest of args are quoted things to append
function append_array () {
    append_array_array_name=$1
    #eval "$append_array_array_name=()"
    shift
    while nonnull $1; do 
        cmd="$append_array_array_name=( "'${'"$append_array_array_name"'[*]}'" $1)"
        eval $cmd
        shift
    done
}

#==========================================
# split PATH into its components
function breakup_path() {
    echo $PATH | awk -F: '{ for (i = 1; i <= NF; i++) print $i }'
}

#=================================
# allow -h to give a header
# -q means be silent and just say if you found it
function psfind() {
    psfind_header=false
    psfind_silent=
    if [ "$1" == '-h' ]; then 
    psfind_header=true
    shift
    elif [ "$1" == '-q' ]; then 
    psfind_silent=">/dev/null 2>&1"
    shift
    fi
    if uname | egrep Darwin >/dev/null; then 
        if [ $psfind_header == true ]; then 
            ps -ww -o "user pid ppid pcpu pmem command" | egrep -v egrep | egrep COMMAND 
        fi
        ps -Awwx -o "user pid ppid pcpu pmem command" | egrep -v egrep | eval  egrep -- '"$1"' $psfind_silent
        return $?
    elif uname | egrep AIX >/dev/null; then 
        if [ $psfind_header == true ]; then 
            ps -Xf -o "user pid ppid pcpu pmem comm args" |  egrep -v egrep | egrep COMMAND 
        fi
            ps -AXf -o "user pid ppid pcpu pmem comm args" | egrep -v egrep | eval  egrep -- '"$1"' $psfind_silent
        return $?
    elif uname -n | egrep vertex >/dev/null; then 
        if [ $psfind_header == true ]; then 
            ps -ww  -o "user pid ppid pcpu pmem command" | egrep -v egrep | egrep COMMAND
        fi
        ps -eww  -o "user pid ppid pcpu pmem command" | egrep -v egrep | eval egrep -- '"$1"' $psfind_silent
        return $?
    else # assume linux
        if [ $psfind_header == true ]; then 
            ps -ww  -o "user pid ppid pcpu pmem command" | egrep -v egrep | egrep COMMAND
        fi
        ps -eww  -o "user pid ppid pcpu pmem command" | egrep -v egrep | eval egrep -- '"$1"' $psfind_silent
        return $?
    fi
}

#=================================
# return nonzero if any processes could not be killed
function killbyname () {
    pids=($(psfind "$1" | awk '{print $2}'))
    if isnull $pids; then 
        return 1
    fi
    killbyname_retval=0
    for pid in ${pids[*]}; do 
        kill ${2:-"-2"} $pid || killbyname_retval=2
    done
    return $killbyname_retval
}   

#=================================
# trap all signals and execute $1 on any of them
function trapall () {
    trap "$1" 0 1 2 3 4 5 6 7 8 10 12 13 14 15 
}

#=================================
function echo_to_stderr() {
    echo $* 1>&2
} 

#=================================
function db_to_stderr() {
    __echo=echo_to_stderr    
}

#=================================
function db_to_stdout() {
    __echo=echo
}

#=================================
verbose=${verbose:-0}
function dbecho () {
    __echo=${__echo:-echo}
    #echo dbecho in $0 with args "$@"
    if nonnull $verbose && [ $verbose -ge "$1"  ] ; then
    shift
    $__echo "$@"
    __echo=
    fi
}

#=================================
# $1 is verbosity level 
# use -q to suppress command output
# use -id to echo $0 
function dbidrunecho () {
    dbidrunecho_level=$1
    shift
    dbidrunecho_quiet=false
    dbidrunecho_id=
    dbidrunecho_prefix="${1:0:1}"
    while [ x${dbidrunecho_prefix} == x- ]; do 
        if [ x$1 == x-q ]; then 
            dbidrunecho_quiet=true
            shift 
        elif [ x$1 == x-id ]; then 
            dbidrunecho_id="$0: " # note space
            shift
        fi
        dbidrunecho_prefix="${1:0:1}"
    done
    # note lack of space
    dbecho $dbidrunecho_level "${dbidrunecho_id}""$@"
    dbidrunecho_level=
    if [ $dbidrunecho_quiet == true ]; then 
        "$@" >/dev/null
        return $?
    else
        "$@"
        return $?
    fi
    echo error in runecho: should not get here
    return -1
    dbecho $1 "$@"
    "$@"
}

#=================================
# conditionally echo command -- same syntax actually as dbidrunecho
function dbrunecho () {
    dbrunecho_level=$1
    shift
    dbidrunecho $dbrunecho_level "$@"    
}
#=================================
# $1 can be -q to suppress cmd output
function runecho () {
    dbrunecho 0 "$@"
}

#=================================
function idecho() {
    echo $(basename $0): "$@"
}

#=================================
function dbidecho() {
    dbidecho_level=$1
    shift
    dbecho $dbidecho_level $(basename $0): "$@"
}

#=================================
function errexit() {
    idecho "$1"
    exit ${2:-1}
}
#=================================
function returnerr () {
    echo "$1"
    return ${2:-1}
}

#=================================
# test if a string has nonzero length
function nonnull () { 
    if [ x"$1" == x ] ; then 
    return 1
    fi
    return 0
}
#=================================
function isnull() {
    if [ x"$1" == x ]; then 
    return 0; 
fi
    return 1
}
#=================================
# return true if first letter is a dash
function isoption() {
    if isnull "$1" || [ ${1#-} == "$1" ]; then 
    return 1; 
    fi
    return 0    
}

# usage:  testbool boolean
# example testbool $myvar
# returns 1 (false) if boolean is false, 0, or undefined
# returns 0 otherwise 
# intended to work with the getargs stuff
function testbool () {
    (nonnull "$1" && [ "$1" != false ] && [ "$1" != no ] && [ "$1" != 0 ] && return 0) || return 1
}


function defined () {
    nonnull $*
}


function isinpath_type() {
    varname="$1"
    thing=$2
    cmd="if echo \$$varname | egrep $2 >/dev/null; then return 0; fi"
    eval $cmd
    return 1
}
function  isinpath () {
    isinpath_type PATH $*
} 

# note that line numbers are 1-based in sed and friends.  The line is inserted BEFORE the given line... 
function insertlineinfile() {
    file=$1
    linenum="${2:-1}"
    insert="$3"
    if isnull $file; then 
    echo "insertlineinfile: Error no filename given" 
    return 1; 
    fi
    sed -e "${linenum}i"'\
'"$insert"  $file >${file}.new
    mv $file $file.backup
    mv $file.new $file
    return 0
} 
# insert  text $1 into all files listed in $2, $3...
function insertatfilehead() {
    text="$1"
    shift 1
    for filename in "$@"; do    
        if [ ! -e "$filename" ] ; then 
            echo "file \"$filename\" does not exist!"
            return 2
        fi
        insertlineinfile $filename 1 "$text"
    done
}
#-------------------------------------------
# make a directory and be sure it succeeds or exit
function makedir() {
    dir="$1"
    [ -d $dir ] || mkdir -p $dir
    if ! [ -d $dir ] ; then
    echo "Could not make or find directory $dir"
    return 1
    fi
}

#-------------------------------------------
function checkstatus ()  {    
    cmd_status=
    #echo checkstatus "$@"
    "$@"
    export cmd_status=$?
    return $cmd_status
}

function setverbose () {
    verbose=${1:-${verbose:-0}}
    if testbool $debug || echo $* | egrep -- -debug >/dev/null || [ $verbose -gt 4 ]; then 
    set -vx
    fi
}
#-------------------------------------------
# if given a list, cat them into an order list and send together, else send one file
function mailfiles () {
    if ! testbool $2; then
    echo "No files given to mail!" && return 1
    fi
    tmpfile=$HOME/.bugreport_mail_clump
    recip="$1"
    shift
    if testbool $2; then 
    echo '' > $tmpfile
    for file in $*; do 
        cat >>$tmpfile <<EOF
`echo`
`echo '************************************************************'`
`echo $file`
`echo '***********************'`
`echo`
EOF
        if [ -e $file ]; then 
        cat $file  >>$tmpfile
        else
        echo "file \"$file\" DOES NOT EXIST" >>$tmpfile
        fi
        cat >>$tmpfile <<EOF
`echo`
`echo '************************************************************'`
`echo `
EOF
    done
    file=$tmpfile
    else
    file="$1"    
    fi
    cat $file | mail -s "$file from `whoami` on `uname -n` at `date`" $recip || return 1
    return 0
}

#-------------------------------------------
# usage: makeflagifset var1 var2...
# if each var is true, then set it as -$varname $var, else set it to ""
# e.g. if dog=1 and cat="" and chicken='false'
# then makeflags dog cat chicken
# sets dog='-dog 1' and cat='' and chicken=''
# if the -bool option is used, then just sets boolean flags if they are true, else sets them to null, e.g.: 
# dog="-dog" and cat="" and chicken=""
function makeflagifset() {
    # a bit of irony here:
    if echo $* | egrep -- -bool >/dev/null ; then 
    dobool=true
    shift 1
    else
    dobool=
    fi
    # now get to work
    for var in $*; do
    if  testbool $dobool; then 
        eval "if testbool \$$var; then  $var=-$var; else $var=; fi"
    else
        eval "if testbool \$$var; then  $var=\"-$var \$$var\"; else $var=; fi"
    fi
    done
}

# --------------------------------------------
# a new approach:  allow everything to get added and at the end call this: 
# I use n^2 algorithm because I don't want to sort the list if I can help it, maybe order is important.  I always keep the first item and toss subsequent ones
function remove_duplicates_from_path_type() {
    typename="$1"
    eval value=\$$typename # ok even with spaces
    value=$(eval echo \$$typename | sed 's/:/ /g') # turn colons to spaces
    newvalue=:
    for item in $value; do
    #standardize by removing any ending slash
    item=${item%/}
    # go item by item and discard duplicates, keeping the first seen
    if echo $newvalue | grep -v ":${item}:" >/dev/null 2>&1 ; then 
        newvalue="${newvalue}${item}:"
    fi
    done
    eval ${typename}="$newvalue"
}

#-------------------------------------------
# assumes there are no semicolons in the path type given by varname, probably a safe assumption;  also assumes there is no colon
# example:  removefrompath_type PATH /my/special/bin
function removefrompath_type() {   
    #echo ERROR: REMOVEFROMPATH CALLED!
    #echo
    #echo removefrompath_type $*
    #set -vx
    if [ x$2 == x ] ; then
    echo removefrompath_type error -- must have typename and item
    fi
    typename="$1"
    eval typeval=\$$typename
    if [ x"$typeval" == x ] ; then 
    return 0
    fi
    #eval echo $typename is $`echo $typename`
    shift
    removearg="$1"
    while [ x$removearg != x ] ; do 
    #set -vx
    if echo $typeval | egrep $removearg >/dev/null; then 
        # need two cases to prevent removing colons from both sides
        eval typeval=$(echo $typeval | sed  -e 's;[:^]*'${removearg}'[/]*[:$]*;:;g' -e 's/::/:/g')
    fi
    #set +vx
    shift
    removearg="$1"
   done
    eval export $typename="$typeval"
    #set +vx
   #for removearg in $*; do 
    #    export PATH=`echo $PATH | sed 's;'"$removearg"';;g'`
    #    done
    # clean up any messes
   #eval echo after removal $typename is $`echo $typename`
   #echo 
}

function removefrompath () { 
    removefrompath_type PATH $* 
}
function removefromldpath () { 
    removefrompath_type LD_LIBRARY_PATH $* 
}
function removefrommanpath () { 
    removefrompath_type MANPATH $* 
}

function prependtopath () { 
    export PATH=$1:$PATH
    #prependtopath_type PATH $1
}
function prependtoldpath () { 
    export LD_LIBRARY_PATH=$1:$LD_LIBRARY_PATH
    #prependtopath_type LD_LIBRARY_PATH $1
}
function prependtomanpath () { 
    export MANPATH=$1:$MANPATH
    #prependtopath_type MANPATH $1
}

function appendtopath () { 
    export PATH=$PATH:$1
    #appendtopath_type PATH $* 
}
function appendtoldpath () { 
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$1
    #appendtopath_type LD_LIBRARY_PATH $* 
}
function appendtomanpath () { 
    export MANPATH=$MANPATH:$1
    #appendtopath_type MANPATH $* 
}


# convenience function 
function cdloc () {
    cd $1/$2
}

  
#=================================================================
# makeabsolute $child $parent
# make $child absolute using `pwd` or  $parent, as follows:
# if child begins with '/', then echo $child
# if child does not begin with '/', then echo $parent'/'$child
# (or `pwd` if no parent given)
function makeabsolute () {
    if [ x"$1" = x ] ; then
    echo 'usage: makeabsolute child-path parent-path'
    fi
    child="$1"
    parent=$2
    if [ x$parent = x ] ; then parent=`pwd`; fi

    if echo $child | egrep  '^/'; then 
        : # do nothing, since child was just echoed!
    else
        echo $parent'/'$child
    fi
}
function fullpath () {
    makeabsolute "$@"
}

#=================================================================
# abspath <varname> [$parent]
#  varname is the NAME of the variable, without any $ sign!  abspath change the named var to contain an absolute pathname.  For example, if blah=., then 
# makevarabsolute blah 
# makes blah=$(pwd)
# note that there is no $blah as arg, you give the NAME of the variable

function abspath () {
    abspath_arg=$(eval echo '$'$1)
    if [ ${abspath_arg:0:1} != '/' ]; then 
        abspath_dirname=$(eval dirname '$'$1)
        abspath_path=$(cd $abspath_dirname; pwd)
    else
        abspath_path=$(dirname $abspath_arg)
    fi
    abspath_basename=$(basename $abspath_arg)    
    eval $1="$abspath_path"/$abspath_basename
}
  
#=================================================================
# atomic lockfiles!  per "Unix Power Tools," page 773
# note -- this probably won't work across NFS
# --------------------------------
function test_and_set () {
    TNS_LOCKFILE=${1:".lockfile"}
    msg=${2:-"PID $$ HOST $(uname -n) PROCESS $0"}
    until (umask 222; echo $msg >$LOCKFILE) >/dev/null 2>&1; do
        sleep 5
        dbecho 5 "waiting for lockfile $TNS_LOCKFILE, currently owned by $(cat $TNS_LOCKFILE)"
    done
}

# --------------------------------
function unlock () {
    rm -f ${1:".lockfile"} 2>/dev/null
}

#=================================================================
# --------------------------------
# getargs
# usage: 
# to use non-boolean options, set up two arrays:  OPTS_ARRAY and VARS_ARRAY
# to use boolean options, set up two arrays: BOOL_ARRAY and BOOL_VARS_ARRAY
# not setting the arrays means you aren't using them.  Setting both means you use both. 
# Variable names obviously do not have to match the option names.
# then just call getargs "$@"
# example:
# from calling script, if you do the following:
# 
# BOOL_OPTS_ARRAY=( -help -exit )
# BOOL_VARS_ARRAY=( help exit )
# OPTS_ARRAY=( -dontchange -first -last -verbose )
# VARS_ARRAY=( dontchange first last verbose )
# getargs "$@"
#
# now if the script is called by 
# script.sh -first 1 -last 2 -ver 3 -help -exit
#
# then when it is done, you will have
# first=1, last=2, and verbose=3
# the boolean variable "help" will be set to "true"
# all other variables will be unchanged
# All unconsumed args are stored in ${REMAINING_ARGS[*]}
# The special arg '--' means stop parsing args and append everything remaining to ${REMAINING_ARGS[*]}

# Return 0 on success.  If IGNORE_BAD_ARGS is set, then an unrecognized arg is ignored and silently skipped, otherwise function immediately stops and returns code 1 for  unknown arguments (if they begin with -).
# 
function getargs () {
    argarray=("$@")
    numargs=${#argarray[*]}
    numopts=${#OPTS_ARRAY[*]}
    numbools=${#BOOL_OPTS_ARRAY[*]}
    REMAINING_ARGS=()
    if [ x$numopts = x ] ; then numopts=0; fi
    if [ x$numbools = x ] ; then numbools=0; fi
    argnum=0
    anymatched=
    while [ $argnum -lt ${numargs} ] ; do
        matched=
        # check for bool match.      
        optnum=0
        thearg="${argarray[$argnum]}"
        if [ "${thearg:0:1}" == '-' ]; then  # arg is an option of some kind
            if [ "$thearg" == "--" ]; then 
                #stop parsing and return
                let argnum++
                while [ $argnum -lt ${numargs} ] ; do
                    append_array REMAINING_ARGS "${argarray[$argnum]}"
                    let argnum++
                done
                return 
            fi
            while [ $optnum -lt ${numbools} ]; do 
                if [ "x$thearg" == "x${BOOL_OPTS_ARRAY[$optnum]}" ] ; then
                cmd="${BOOL_VARS_ARRAY[$optnum]}"'=true'
                dbecho 2 `eval $cmd`
                eval $cmd
                matched=1
                break
                fi
                let optnum++
                #If no match, reset optnum to 0.  Else, set to ${numopts}
            done
            
            # regular options: 
            if testbool $matched; then optnum=$numopts; else optnum=0; fi
            while [ $optnum -lt ${numopts} ]; do 
                if [ "x$thearg" == "x${OPTS_ARRAY[$optnum]}" ] ; then
                let argnum++
                cmd="${VARS_ARRAY[$optnum]}"'="${argarray[$argnum]}"'
                dbecho 2 `eval $cmd`
                eval $cmd
                matched=1
                break
                fi
                let optnum++
            done
            if ! testbool $matched; then 
                append_array REMAINING_ARGS "${argarray[$argnum]}"
                if ! testbool $IGNORE_BAD_ARGS  ; then
                    echo 
                    echo "************************"
                    echo "ERROR BAD ARGUMENT number $(expr $argnum + 1): \"$nextarg\""
                    echo "************************"
                    echo 
                    return 1
                fi
            fi
        else # arg did not begin with "-"
            append_array REMAINING_ARGS "${argarray[$argnum]}"
        fi           
        let argnum++
    done
    return 0
}

function usegetargs () {
    BOOL_OPTS_ARRAY=( -bool1 -bool2 -bool3 )
    BOOL_VARS_ARRAY=( bool1 bool2 bool3 ) 
    OPTS_ARRAY=( -first -last -verbose )
    VARS_ARRAY=( first last verbose )
    getargs "$@"
    for var in first last verbose bool1 bool2 bool3; do 
    cmd="echo $var is "'$'$var
    eval $cmd
    done
}

function testgetargs() {
    for var in first last verbose bool1 bool2 bool3; do 
    cmd="$var="
    eval $cmd
    done
    usegetargs -first 1 -verbose 2 -badarg -bool2
}

# again, we assume an array called VARS_ARRAY is set, as with getargs.  We just echo the values of the variables named therein
function echoargs () {
    for var in ${VARS_ARRAY[*]}; do 
    eval echo $var is '$'$var
    done
}

#========================================
# bool2opt()
# pass the NAME of a bool variable 
# if $varname != 'no', varname="-varname" 
# else varname=''
function bool2opt() {
    if eval [ x'$'"$1" == xno ] ; then 
    eval "$1"=
    else
    eval ${1}='-"$1"'
    fi
}

# --------------------------------
# length
# usage: length word
# echo the length of $1
function length () {
    len=`echo "$1" | wc -m`
    expr $len - 1
}

# --------------------------------
# pad2length
# usage: padzeros num length
# pad shortnum with initial zeros until its length is as long as longnum
function pad2length () {
    output="$1"
    shortlen=`length "$1"`
    longlen=$2
    while [ $shortlen -lt $longlen ] ; do
    output=0$output
    shortlen=`expr $shortlen + 1`
    done
    echo $output
}

# --------------------------------
# padzeros
# usage: padzeros shortnum longnum 
# pad shortnum with initial zeros until its length is as long as longnum
function padzeros () {
    longlen=`length $2`
    pad2length "$1" $longlen
}

# --------------------------------
# getprocnum
# echo our SLURM_PROCID (which is 0 based)
# I wrote this mainly to document where to get this info
# only works on SLURM machines.  
function getprocnum () {    
    retval=0
    if [ x${SLURM_PROCID} = x ] ; then
    retval=1
    fi
    export procnum=${SLURM_PROCID:-0}
    echo $procnum 
    return $retval
}


# --------------------------------
# getnumprocs
# return the total number of processors in the current batch job -- useful to determine chunk sizes in conjunction with getprocnum above
# only works on SLURM machines.  
function getnumprocs () {
    retval=0
    if [ x${SLURM_NPROCS} = x ] ; then
    retval=1
    fi
    export numprocs=${SLURM_NPROCS:-1}
    echo $numprocs 
    return $retval
}


# sedfiles [opts] (expression | -e expression1 -e expression2 ...) files
# opts:
#  --nobackups :  don't make backup files 
# execute some sed expressionsfor some files, replacing it with fixed file
# for multiple expressions, use the -e syntax from sed
# if the first argument is not -e, then assume $1 is the only sed expression and the rest are the files
function sedfilesusage() {
    echo "usage: sedfiles [opts] (expression | -e expression1 -e expression2 ...) files"
    echo "OPTIONS:"
    echo '--nobackups:  live dangerously'
    return
}

function sedfiles () {
    if isnull $1 || [ "$1" == -h ]; then 
        sedfilesusage 
        return;
    fi
    while [ ${1:0:1} == - ]; do 
    case "$1" in 
        --nobackups) nobackups=true; shift;;
        *) break;;
    esac
    done
    #set -vx
    expressions=()
    expnum=0    
    while [ x"$1" == x-e ] ; do    
    expressions[expnum]="$1"
    let expnum++
    shift
    expressions[expnum]="$1"
    let expnum++
    shift
    done
    if [ x"${expressions[0]}" == x ] ; then 
    expressions[expnum]="$1"
    shift
    fi
    for file in "$@"; do
    dbecho 1 sed "${expressions[@]}"  $file
    [ -e "$file" ] || returnerr "file \"$file\" does not exist"
    if ! testbool $nobackups; then
        cp "$file" "$file.bak" || returnerr "cannot copy \"$file\" to \"$file.bak\""
    fi
    sed "${expressions[@]}" "$file" > "$file.new" || returnerr  "sed ${expressions[@]} \"$file\" failed" 
    mv "$file.new" "$file" || returnerr Cannot move \"$file\" to \"$file.new\"
    done
    return 0
    #set +vx
}

# usage findfiles "filelist" "paths" (defaults to $PATH)
# echos all locations found, returns 0 if success, or 1 if failed
# if oneonly is "true" or "1", then stop after first match
function findfiles () {
    theFiles="$1"
    thePaths="${2:-$PATH}"
    thePaths=`echo $thePaths | sed 's/:/ /g'`
    failed=1
    oneonly=${oneonly:-false}
    for file in $theFiles; do 
    for dirname in $thePaths; do
        if [ -e $dirname/$file ] ; then 
        echo $dirname/$file
        failed=0
        if testbool $oneonly; then 
            return 0
        fi
        fi
    done
    done
    return $failed
}


function findexes () {
    findfiles "$1" "$PATH"
}

function findexe () {
    oneonly=true findfiles "$1" "$PATH"
}

function findlibs () {
    #ok to mix in colon-delimited with space delimited:
    paths="${2:-/sw/lib /usr/local/lib /usr/lib /lib /usr/X11R6/lib $LD_LIBRARY_PATH} /usr/local/tools/libjpeg/lib /usr/local/tools/libpng/lib" 
    findfiles "$1" "$paths"
}

function findlib () {
    oneonly=true findlibs "$1" "$2"
}

function findheaders () {
    paths="${2:-/usr/local/include /usr/include /include /usr/X11R6/include /usr/local/tools/libjpeg/include /usr/local/tools/libpng/include}" 
    findfiles "$1" "$paths"
}

function findheader () {
    oneonly=true findheaders "$1" "$2"
}

# simple integer seconds timer
# has start, stop, time functions
# if time is called without start, just returns seconds since the epoch
# can have multiple simultaneous timers if you use the "name" argument
function timer() {    
    cmd=${1:-start}
    name=${2:-generic}
    timer_dir=$HOME/.timer
    if [ ! -d $timer_dir ] ; then mkdir -p $timer_dir; fi 
    python -c 'import time; print int(time.time())' > $timer_dir/current_time_$name    
    expr $timer_current_time - $timer_internal_time  
    if [ $cmd == start ]; then 
    cp $timer_dir/current_time_$name $timer_dir/start_time_$name    
    rm $timer_dir/cumulative_time_$name  
    elif [ $cmd == time ]; then 
    if [ -e $timer_dir/cumulative_time_$name ] ; then 
        cat $timer_dir/cumulative_time_$name 
    elif ! [ -e $timer_dir/start_time_$name ]; then 
        cat $timer_dir/current_time_$name
    else
        expr $(cat $timer_dir/current_time_$name) - $(cat $timer_dir/start_time_$name)
    fi
    elif [ $cmd == stop ]; then
    expr $timer_current_time - $timer_internal_time > $timer_dir/cumulative_time_$name    
    else
    echo unknown command $cmd
    return 1
    fi
    return 0
}
    

#echo done with shellfuncs $(date)
