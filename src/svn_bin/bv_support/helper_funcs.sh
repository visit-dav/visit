export LOG_FILE=${LOG_FILE:-"${0##*/}_log"}

# *************************************************************************** #
# Function: errorFunc                                                         #
#                                                                             #
# Purpose: Error messages                                                     #
#                                                                             #
# *************************************************************************** #
function errorFunc
{
  echo $1
  exit 0
}

# *************************************************************************** #
# Function: log                                                               #
#                                                                             #
# Purpose: Log message for the log.                                           #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Fri Oct  3 09:37:51 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #
function log
{
    echo "$@" >> ${LOG_FILE}
}


# *************************************************************************** #
# Function: warn                                                              #
#                                                                             #
# Purpose: Echo to screen and log.                                            #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date: Tue Nov 18 15:09:47 PST 2008                                          #
#                                                                             #
# Modifications:                                                              #
#                                                                             #
#   Hank Childs, Fri Jan 28 10:32:06 PST 2011                                 #
#   Add -e to echo so tabs are printed.                                       #
#                                                                             #
# *************************************************************************** #
function warn
{
    if [[ "$REDIRECT_ACTIVE" == "yes" ]] ; then
        echo -e "$@" 1>&3
    else
        echo -e "$@"
    fi
    
    if [[ "${LOG_FILE}" != "/dev/tty" ]] ; then
        # write message to log as well
        log "$@"
    fi
}

# *************************************************************************** #
# Function: error                                                             #
#                                                                             #
# Purpose: Report an error message and exit.                                  #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Fri Oct  3 09:37:51 MDT 2008                                          #
#                                                                             #
# Modifications:                                                              #
#    Jeremy Meredith, Fri Jul 22 10:36:20 EDT 2011                            #
#    Added a little more text to help users.                                  #
# *************************************************************************** #

function error
{
    warn "$@"
    if test "${LOG_FILE}" != "/dev/tty" ; then
        warn "Error in build process.  See ${LOG_FILE} for more information."\
             "If the error is unclear, please include ${LOG_FILE} in a "\
             "message to the visit-users@ornl.gov list.  You will probably "\
             "need to compress the ${LOG_FILE} using a program like gzip "\
             "so it will fit within the size limits for email attachments."
    fi
    exit 1
}

# *************************************************************************** #
# Function: issue_command                                                     #
#                                                                             #
# Purpose: Print a command and execute it too.                                #
#                                                                             #
# Programmer: Brad Whitlock,                                                  #
# Date: Tue Feb 28 17:04:43 PST 2012                                          #
#                                                                             #
# Modifications:                                                              #
#
# *************************************************************************** #

function issue_command
{
    echo "$@"
    "$@"
    return $?
}

# *************************************************************************** #
# Function: add_extra_commandline_args                                        #
#                                                                             #
# Purpose: Allows modules to add extra arguments to VisIt                     #
#                                                                             #
# Programmer: Hari Krishnan,                                                  #
# Date: Thu Dec 15 14:38:36 PST 2011                                          #
#                                                                             #
# Modifications:                                                              #
#   Eric Brugger, Wed Jan 18 08:17:48 PST 2012                                #
#   I re-enabled the code that replaces dashes with underscores in the        #
#   name of the enable function.                                              #
#                                                                             #
# *************************************************************************** #
#global argument list for extra args..
declare -a extra_commandline_args
export EXTRA_COMMANDLINE_ARG_CALL=""

function add_extra_commandline_args 
{

  if [[ $# != 4 ]]; then
   echo "extra command line usage requires 4 parameters"
   return
  fi

  #replace all occurrences of "-" with "_"
  local enable_func="bv_$1_${2//-/_}"

  #check if function exists..
  #maybe this should be moved to build_visit rather than here..
  #in case some bash consoles don't have declare -F capabilities?
  declare -F "$enable_func" &>/dev/null || errorFunc "function pointer $enable_func not found"

  #add parameters..
  for f in "$@"; do
    extra_commandline_args[${#extra_commandline_args[*]}]="$f"
  done

  #add function pointer..
  extra_commandline_args[${#extra_commandline_args[*]}]="$enable_func"
}


function verify_required_module_exists
{
    local reqlib=$1
    #check if required functions exist..
    declare -F "bv_${reqlib}_enable" &>/dev/null || errorFunc "${reqlib} enable not found"
    declare -F "bv_${reqlib}_disable" &>/dev/null || errorFunc "${reqlib} disable not found"
    declare -F "bv_${reqlib}_initialize" &>/dev/null || errorFunc "${reqlib} initialize not found"
    declare -F "bv_${reqlib}_info" &>/dev/null || errorFunc "${reqlib} info not found"
    declare -F "bv_${reqlib}_ensure" &>/dev/null || errorFunc "${reqlib} ensure not found"
    declare -F "bv_${reqlib}_build" &>/dev/null || errorFunc "${reqlib} build not found"
    declare -F "bv_${reqlib}_depends_on" &>/dev/null || errorFunc "${reqlib} depends_on not found"
    declare -F "bv_${reqlib}_print" &>/dev/null || errorFunc "${reqlib} print not found"
    declare -F "bv_${reqlib}_print_usage" &>/dev/null || errorFunc "${reqlib} print_usage not found"
    declare -F "bv_${reqlib}_dry_run" &>/dev/null || errorFunc "${reqlib} dry_run not found"
    declare -F "bv_${reqlib}_is_installed" &>/dev/null || errorFunc "${reqlib} is_installed not found"
    declare -F "bv_${reqlib}_is_enabled" &>/dev/null || errorFunc "${reqlib} is_enabled not found"
}


function verify_optional_module_exists
{
    local optlib=$1
    declare -F "bv_${optlib}_enable" &>/dev/null || errorFunc "${optlib} enable not found"
    declare -F "bv_${optlib}_disable" &>/dev/null || errorFunc "${optlib} disable not found"
    declare -F "bv_${optlib}_initialize" &>/dev/null || errorFunc "${optlib} info not found"
    declare -F "bv_${optlib}_info" &>/dev/null || errorFunc "${optlib} info not found"
    declare -F "bv_${optlib}_ensure" &>/dev/null || errorFunc "${optlib} ensure not found"
    declare -F "bv_${optlib}_build" &>/dev/null || errorFunc "${optlib} build not found"
    declare -F "bv_${optlib}_depends_on" &>/dev/null || errorFunc "${optlib} depends_on not found"
    declare -F "bv_${optlib}_print" &>/dev/null || errorFunc "${optlib} print not found"
    declare -F "bv_${optlib}_print_usage" &>/dev/null || errorFunc "${optlib} print_usage not found"
    declare -F "bv_${optlib}_host_profile" &>/dev/null || errorFunc "${optlib} host_profile not found"
    declare -F "bv_${optlib}_graphical" &>/dev/null || errorFunc "${optlib} graphical not found"
    declare -F "bv_${optlib}_dry_run" &>/dev/null || errorFunc "${optlib} dry_run not found"
    declare -F "bv_${optlib}_is_installed" &>/dev/null || errorFunc "${optlib} is_installed not found"
    declare -F "bv_${optlib}_is_enabled" &>/dev/null || errorFunc "${optlib} is_enabled not found"
}

# *************************************************************************** #
# Function: info_box                                                          #
#                                                                             #
# Purpose: Show an information box with a message.                            #
#                                                                             #
# Programmer: Brad Whitlock,                                                  #
# Date: Thu Apr 5 14:38:36 PST 2007                                           #
#                                                                             #
# *************************************************************************** #

function info_box
{
    if [[ "$GRAPHICAL" == "yes" ]] ; then
        $DLG --backtitle "$DLG_BACKTITLE" --infobox "$1" $DLG_HEIGHT $DLG_WIDTH
    fi
    return 0
}

# *************************************************************************** #
# Function: info                                                              #
#                                                                             #
# Purpose: Give an informative message to the user.                           #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Fri Oct  3 09:41:50 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #
function info
{
    if [[ "$REDIRECT_ACTIVE" == "yes" ]] ; then
        if test "${GRAPHICAL}" = "yes" ; then
            info_box "$@" 1>&3
        else
            echo "$@" 1>&3
        fi
    else
        if test "${GRAPHICAL}" = "yes" ; then
            info_box "$@"
        else
            echo "$@"
        fi
    fi

    if [[ "${LOG_FILE}" != "/dev/tty" ]] ; then
        # write message to log as well
        log "$@"
    fi
}

# *************************************************************************** #
# Function: info_box_large                                                    #
#                                                                             #
# Purpose: Show a large information box with a message.                       #
#                                                                             #
# Programmer: Eric Brugger,                                                   #
# Date: Thu Jul 21 08:25:52 PDT 2011                                          #
#                                                                             #
# *************************************************************************** #

function info_box_large
{
    if [[ "$GRAPHICAL" == "yes" ]] ; then
        $DLG --backtitle "$DLG_BACKTITLE" --infobox "$1" $DLG_HEIGHT_TALL $DLG_WIDTH
    fi
    return 0
}

# *************************************************************************** #
# Function: uncompress_untar
#                                                                             #
# Purpose: Uncompress and untar the file, checking if GNU tar can be used.    #
#                                                                             #
# Programmer: Thomas R. Treadway                                              #
# Date: Tue May 15 16:48:01 PDT 2007                                          #
#                                                                             #
# *************************************************************************** #

function uncompress_untar
{
    # Check if GNU tar
    if [[ $(echo $1 | egrep "\.gz$" ) != "" ]] ; then
        COMPRESSTYPE="gzip"
    elif [[ $(echo $1 | egrep "\.bz2$" ) != "" ]] ; then
        COMPRESSTYPE="bzip"
    elif [[ $(echo $1 | egrep "\.tgz$" ) != "" ]] ; then
        COMPRESSTYPE="targzip"
    else
        warn "unsupported uncompression method"
        return 1
    fi
    TARVERSION=$($TAR --version >/dev/null 2>&1)
    if [[ $? == 0 ]] ; then
        case $COMPRESSTYPE in
            gzip|targzip) $TAR zxf $1;;
            bzip) $TAR jxf $1;;
        esac
    else
        case $COMPRESSTYPE in
            gzip) 
               gunzip $1
               $TAR xf ${1%.gz}
               ;;
            targzip) 
               gunzip $1
               $TAR xf "${1%.tgz}.tar"
               ;;
            bzip)
               bunzip2 $1
               $TAR xf ${1%.bz2}
               ;;
        esac
    fi
}

# *************************************************************************** #
# Function: verify_checksum                                                   #
#                                                                             #
# Purpose: Verify the checksum of given file                                  #
#                                                                             #
# verify_md5_checksum: checks md5                                             #
# verify_sha_checksum: checks sha (256,512)                                   #
# Programmer: Hari Krishnan                                                   #
# *************************************************************************** #

function verify_md5_checksum
{
    checksum=$1
    dfile=$2

    tmp=`which md5sum`
    if [[ $? != 0 ]]; then
        info "could not find md5sum, disabling check"
        return 0
    fi
    tmp=`md5sum $dfile | awk '{print $1}'`
    if [[ $tmp == ${checksum} ]]; then
        info "verified"
        return 0
    fi

    info "md5sum failed: looking for $checksum got $tmp"
    return 1
}

function verify_sha_checksum
{
    checksum_algo=$1
    checksum=$2
    dfile=$3

    tmp=`which shasum`
    if [[ $? != 0 ]]; then
        info "could not find shasum, disabling check"
        return 0
    fi

    tmp=`shasum -a $checksum_algo $dfile`
    if [[ $? == 0 ]]; then
        tmp=`echo $tmp| awk '{print $1}'`
        if [[ $tmp == $checksum ]]; then
            info "verified"
            return 0
        else
            info "shasum -a $checksum_algo failed: looking for $checksum got $tmp"
            return 1
        fi
    fi

    info "shasum does not support $checksum_algo, check disabled"
    return 0
}

function verify_checksum
{
    checksum_type=$1
    checksum=$2
    dfile=$3

    info "verifying type $checksum_type, checksum $checksum for $dfile . . ."

    if [[ "$checksum_type" == "MD5" ]]; then
        verify_md5_checksum $checksum $dfile
        return $?
    fi

    if [[ $checksum_type = "SHA256" ]]; then
        verify_sha_checksum 256 $checksum $dfile
        return $?
    fi

    if [[ $checksum_type = "SHA512" ]]; then
        verify_sha_checksum 512 $checksum $dfile
        return $?
    fi

    #since this is an optional check, all cases should pass if it gets here..
    info "checksum string not MD5, SHA256, or SHA512, check disabled"
    return 0
}


# *************************************************************************** #
# Function: download_file                                                     #
#                                                                             #
# Purpose: Downloads a file using wget and show a dialog screen.              #
#                                                                             #
# Programmer: Brad Whitlock,                                                  #
# Date: Thu Apr 5 14:38:36 PST 2007                                           #
#                                                                             #
# Modifications:                                                              #
#                                                                             #
#   Hank Childs, Mon Oct 15 15:55:22 PDT 2007                                 #
#   Fail gracefully if wget is not available.                                 #
#                                                                             #
#   Thomas R. Treadway, Tue Nov 27 16:37:21 PST 2007                          # 
#   Deal with LLNL's invalid certificates                                     #
#                                                                             #
#   Cyrus Harrison, Mon Nov 17 16:22:54 PST 2008                              # 
#   Check return value of svn cat or download for errors. Clean up a          #
#   partially downloaded file.                                                #
#                                                                             #
#   Hank Childs, Fri Dec 12 09:28:35 PST 2008                                 #
#   Add special logic for Ice-T.                                              #
#                                                                             #
#   Mark C. Miller, Thu Feb 19 09:16:46 PST 2009                              #
#   Added argument to specify a download_path. Removed special IceT coding.   #
#                                                                             #
#   Mark C. Miller, Thu Feb 19 12:21:55 PST 2009                              #
#   Changed to support multiple sites as well as default visit places.        #
#                                                                             #
#   Cyrus Harrison, Thu Feb 19 12:21:55 PST 2009                              #
#   Fixed problem where if a download path was given, svn mode was skipped    #
#                                                                             #
#   Cyrus Harrison, Thu Apr  9 19:21:13 PDT 2009                              #
#   Applied patch from Rick Wagner to fix curl downloads on OSX.              #
#                                                                             #
#   Mark C. Miller, Wed Feb  3 09:47:52 PST 2010                              #
#   Made it fall back to anon svn checkout                                    #
#                                                                             #
#   Eric Brugger, Mon Jan 10 16:00:13 PST 2011                                #
#   I made it always fall back to anonymous svn checkout.                     #
#                                                                             #
#   Eric Brugger, Tue Jul 19 10:19:50 PDT 2011                                #
#   I made it use the anonymous svn site as the fallback download site        #
#   instead of llnl's web site.                                               #
#                                                                             #
# *************************************************************************** #

function download_file
{
# $1 is the file name to download
# $2...$* [OPTIONAL] list of sites to obtain the file from
#
# Since we always pass optional download sites to this function for
# some third party libs - we can't skip svn mode just b/c this info is given.
#
    typeset dfile=$1
    info "Downloading $dfile . . ." 
    shift
    
    # If SVN is requested, try that first before anything else
    if [[ "$DO_SVN" == "yes" ]] ; then
        svn cat $SVN_ROOT_PATH/trunk/third_party/$dfile > $dfile
        if [[ $? == 0 && -e $dfile ]] ; then
            info "SVN download succeeded: $SVN_ROOT_PATH/trunk/third_party/$dfile"
            return 0
        else
            warn "Normal svn failed. Trying anonymous svn." 
            svn cat $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile > $dfile
            if [[ $? == 0 && -e $dfile ]] ; then
                info "Anonymous SVN download succeeded: $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile"
                return 0
            fi
        fi
        warn "SVN download attempt failed: $SVN_ROOT_PATH/trunk/third_party/$dfile"
        warn "Anonymous SVN download attempt failed: $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile"
        rm -f $dfile
    elif [[ "$DO_SVN_ANON" == "yes" ]] ; then
        svn cat $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile > $dfile
        if [[ $? == 0 && -e $dfile ]] ; then
            info "Anonymous SVN download succeeded: $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile"
            return 0
        fi
        warn "Anonymous SVN download attempt failed: $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile"
        rm -f $dfile
    fi

    # If the visit source code is requested try that next.
    if [[ "$dfile" == "$VISIT_FILE" ]] ; then
        try_download_file $SVN_ANON_ROOT_PATH/trunk/releases/$VISIT_VERSION/$dfile $dfile
        if [[ $? == 0 ]] ; then
            return 0
        fi
    fi

    # Now try the various places listed.
    if [[ "$1" != "" ]] ; then
        for site in $* ; do
            try_download_file $site/$dfile $dfile
            if [[ $? == 0 ]] ; then
                return 0
            fi
        done
    fi

    # Now try anonymous svn unless we tried it above.
    if [[ "$DO_SVN" != "yes" && "$DO_ANON_SVN" != "yes" ]] ; then
        svn cat $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile > $dfile
        if [[ $? == 0 && -e $dfile ]] ; then
            info "Anonymous SVN download succeeded: $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile"
            return 0
        fi
        warn "Anonymous SVN download attempt failed: $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile"
        rm -f $dfile
    fi

    # Now try the anonymous svn site with wget or curl.
    try_download_file $SVN_ANON_ROOT_PATH/trunk/third_party/$dfile $dfile
    if [[ $? == 0 ]] ; then
        return 0
    fi
    info "Failed to download $dfile"
    return 1
}

# *************************************************************************** 
# Function: try_download_file
#
# Purpose: DONT USE THIS FUNCTION. USE download_file.
# Downloads a file using wget or curl. 
#
# Programmer: Refactored from other sources (Mark C. Miller)
# Creation: February 19, 2009
#
#   Cyrus Harrison, Tue 24 Mar 13:44:31 PST 2009 
#   As an extra guard, check that the downloaded file actually exisits. 
#   (Firewalls can cause strange files to be created.)
# 
#   Cyrus Harrison, Thu Apr  9 19:21:13 PDT 2009
#   Applied patch from Rick Wagner to fix curl downloads on OSX.
#
#   Tom Fogal, Sun Jul 26 17:19:26 MDT 2009
#   Follow redirects.  Don't use a second argument.
#
#   Gunther H. Weber, Fri Oct 23 13:17:34 PDT 2009
#   Specify explicit path to system curl so that we do not use another
#   version without SSL support
#
# *************************************************************************** 

function try_download_file
{
    if [[ "$OPSYS" == "Darwin" ]]; then
        # MaxOS X comes with curl
        /usr/bin/curl -ksfLO $1
    else
        check_wget
        if [[ $? != 0 ]] ; then
            error "Need to download $1, but \
                   cannot locate the wget utility to do so."
        fi
        wget $WGET_OPTS -o /dev/null $1
    fi

    if [[ $? == 0 && -e `basename $1` ]] ; then
        info "Download succeeded: $1"
        return 0
    else    
        warn "Download attempt failed: $1"
        rm -f `basename $1`
        return 1
    fi
}


# *************************************************************************** #
# Function: check_svn_client                                                  #
#                                                                             #
# Purpose: Helper that checks if a svn client is available.                    #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date:  Mon Nov 17 14:52:37 PST 2008                                         #
#                                                                             #
# *************************************************************************** #

function check_svn_client
{
    # check for svn client
    SVN_CLIENT=$(which svn)
    if [[ $SVN_CLIENT == "" ]] ; then
        return 1
    fi
    return 0
}

# *************************************************************************** #
# Function: check_wget                                                        #
#                                                                             #
# Purpose: Helper that checks if a wget is available.                         #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date:  Mon Nov 17 14:52:37 PST 2008                                         #
#                                                                             #
# *************************************************************************** #

function check_wget
{
    WGET_CLIENT=$(which wget)
    # check for wget
    if [[ $WGET_CLIENT == "" ]] ; then
        return 1
    fi
    return 0
}

# *************************************************************************** #
# Function: check_if_installed                                                #
#                                                                             #
# Purpose: Checks if $VISITDIR/$1/$2/$VISITARCH exists.                       #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date: Wed Nov 19 07:31:08 PST 2008                                          #
#                                                                             #
# *************************************************************************** #
function check_if_installed
{
    BUILD_NAME=$1
    BUILD_VERSION=""

    if [[ $# == 2 ]]; then
        BUILD_VERSION=$2
    fi
    
    if [[ $BUILD_VERSION != "" ]]; then
        INSTALL_DIR=$VISITDIR/$BUILD_NAME/$BUILD_VERSION/$VISITARCH
    else
        INSTALL_DIR=$VISITDIR/$BUILD_NAME/$VISITARCH
    fi

    if [[ -d ${INSTALL_DIR} ]] ; then
        return 0
    else
        return 1
    fi
}

# *************************************************************************** #
# Function: ensure_built_or_ready                                             #
#                                                                             #
# Purpose: Helper that checks for proper installed version. If this doesn't   #
#  exist, makes sure the source file is avalaible for building.               #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date: Fri Nov 14 08:23:26 PST 2008                                          #
#                                                                             #
# *************************************************************************** #
function ensure_built_or_ready
{
    BUILD_NAME=$1
    BUILD_VERSION=$2
    INSTALL_DIR=$VISITDIR/$BUILD_NAME/$BUILD_VERSION/$VISITARCH
    BUILD_DIR=$3
    SRC_FILE=$4
    DOWNLOAD_PATH=$5

    info "Checking for ${BUILD_NAME}-${BUILD_VERSION}"
    
    ALREADY_INSTALLED="NO"
    HAVE_TARBALL="NO"
    
    check_if_installed $BUILD_NAME $BUILD_VERSION
    if [[ $? == 0 || -d $BUILD_DIR ]] ; then
        ALREADY_INSTALLED="YES"
    fi
    if [[ -e ${SRC_FILE%.gz} || -e ${SRC_FILE} ]] ; then
        HAVE_TARBALL="YES"
    fi
    
    if [[ "$ALREADY_INSTALLED" == "NO" && "$HAVE_TARBALL" == "NO" ]] ; then
        download_file ${SRC_FILE} ${DOWNLOAD_PATH}
        if [[ $? != 0 ]] ; then
            warn "Error: Cannot obtain source for $BUILD_NAME."
            return 1
        fi
    fi
    return 0
}


# *************************************************************************** #
# Function: prepare_build_dir                                                 #
#                                                                             #
# Purpose: Helper that prepares a build directory from a src file.            #
#                                                                             #
# Returns:                                                                    #
#          -1 on failure                                                      #
#           0 for success without untar                                       #
#           1 for success with untar                                          #
#           2 for failure with checksum                                       #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date: Thu Nov 13 09:28:26 PST 2008                                          #
#                                                                             #
# *************************************************************************** #
function prepare_build_dir
{
    BUILD_DIR=$1
    SRC_FILE=$2
    
    #optional
    CHECKSUM_TYPE=$3
    CHECKSUM_VALUE=$4

    untarred_src=0
    if [[ -d ${BUILD_DIR} ]] ; then
       info "Found ${BUILD_DIR} . . ."
       untarred_src=0
    elif [[ -f ${SRC_FILE} ]] ; then
       if [[ $CHECKSUM != "" && $CHECKSUM_TYPE != "" ]]; then
            verify_checksum $CHECKSUM_TYPE $CHECKSUM ${SRC_FILE}
            if [[ $? != 0 ]]; then
                return 2
            fi
       fi
       info "Unzipping/Untarring ${SRC_FILE} . . ."
       uncompress_untar ${SRC_FILE}
       untarred_src=1
       if [[ $? != 0 ]] ; then
          warn \
"Unable to untar $SRC_FILE  Corrupted file or out of space on device?"
          return -1
       fi
    elif [[ -f ${SRC_FILE%.*} ]] ; then
       info "Untarring ${SRC_FILE%.*} . . ."
       $TAR xf ${SRC_FILE%.*}
       untarred_src=1
       if [[ $? != 0 ]] ; then
          warn \
"Unable to untar ${SRC_FILE%.*}.  Corrupted file or out of space on device?"
          return -1
       fi
    fi
    
    return $untarred_src
}

# *************************************************************************** #
#                   Function 1.3, check_optional_3rdparty                     #
# --------------------------------------------------------------------------- #
# This function will check to make sure that all of the necessary source      #
# files for the optional 3rd party libraries actually exist.                  #
#
# *************************************************************************** #

function check_optional_3rdparty
{
    info "Checking optional 3rd party libs"

    for (( i = 0; i < ${#optlibs[*]}; ++i ))
    do
        ensure="bv_${optlibs[$i]}_ensure"
        $ensure
        if [[ $? != 0 ]] ; then
            return 1
        fi
    done
}


# *************************************************************************** #
#                    Function 1.1, check_required_3rdparty                    #
# --------------------------------------------------------------------------- #
# This function will check to make sure that all of the necessary files       #
# for the required third party libraries actually exist.                      #
# *************************************************************************** #

function check_required_3rdparty
{
    info "Checking for files . . ."

    for (( i = 0; i < ${#reqlibs[*]}; ++i ))
    do
        ensure="bv_${reqlibs[$i]}_ensure"
        $ensure
        if [[ $? != 0 ]] ; then
            return 1
        fi
    done

    return 0
}


# *************************************************************************** #
#                         Function 1.0, check_files                           #
# --------------------------------------------------------------------------- #
# This function will check to make sure that all of the necessary files       #
# actually exist.                                                             #
# *************************************************************************** #

function check_files
{
    check_required_3rdparty
    if [[ $? != 0 ]]; then
        return 1
    fi

    if [[ "$DO_VISIT" == "yes" ]] ;  then
       bv_visit_ensure_built_or_ready
       if [[ $? != 0 ]]; then
           return 1
       fi
    fi

    check_optional_3rdparty
    if [[ $? != 0 ]]; then
        return 1
    fi
    return 0
}


# *************************************************************************** #
#                         Function 2.0, check_more_options                    #
# --------------------------------------------------------------------------- #
# This function will display variables and optionally allow changing          #
# *************************************************************************** #

function check_more_options
{

    # Override variable settings dialog
    #
    if [[ "$DO_MORE" == "yes" && "$GRAPHICAL" == "yes" ]] ; then
        result=$($DLG --backtitle "$DLG_BACKTITLE" \
        --title "More build options" \
        --checklist \
"Version: specify version of VisIt to download and build\n"\
"Build: build VisIt, disable to build 3rd party only\n"\
"Required: build required 3rd party libraries\n"\
"Logging: display build log to stdout\n"\
"Symbol: turn on -g, debugging flag\n"\
"Group: specify the group name for install\n"\
"Path: specify the root directory for libraries\n"\
"to use the given Path rather than the default [@executable_path/../lib]\n"\
"Trace: print a trace of commands and arguments during build\n\n"\
"Select build and installed options:" 0 0 0 \
           "Version"   "specify VisIt version [$VISIT_VERSION]" $ON_VERSION \
           "Build"     "enable building VisIt"                  $ON_VISIT \
           "Logging"   "disable logging to file"                $ON_LOG \
           "Symbol"    "enable debug compiling"                 $ON_DEBUG \
           "Group"     "specify group name for install"         $ON_GROUP \
           "HostConf"  "create host.conf file"                  $ON_HOSTCONF \
           "Path"      "specify library path [$THIRD_PARTY_PATH]" $ON_PATH \
           "Trace"     "enable SHELL debugging"      $ON_VERBOSE 3>&1 1>&2 2>&3)
        retval=$?

    # Remove the extra quoting, new dialog has --single-quoted
        choice="$(echo $result | sed 's/"//g' )"
        case $retval in
          0)
            DO_VERSION="no"
            DO_VISIT="no"
            DO_REQUIRED_THIRD_PARTY="no"
            DO_LOG="no"
            DO_DEBUG="no"
            DO_GROUP="no"
            DO_HOSTCONF="no"
            DO_PATH="no"
            DO_VERBOSE="no"
            for OPTION in $choice
            do
                case $OPTION in
                  Version)
                     result=$($DLG --backtitle "$DLG_BACKTITLE" \
                        --nocancel --inputbox \
"Enter $OPTION value:" 0 $DLG_WIDTH_WIDE "$VISIT_VERSION"   3>&1 1>&2 2>&3) 
                     VISIT_VERSION="$result"
                     VISIT_FILE="visit${VISIT_VERSION}.tar.gz"
                     DO_VERSION="yes";;
                  Build)
                     DO_VISIT="yes";;
                  Logging)
                     DO_LOG="yes";;
                  Symbol)
                     DO_DEBUG="yes";;
                  Group)
                     result=$($DLG --backtitle "$DLG_BACKTITLE" \
                        --nocancel --inputbox \
"Enter $OPTION value:" 0 $DLG_WIDTH_WIDE "$GROUP" 3>&1 1>&2 2>&3)
                     GROUP="$result"
                     DO_GROUP="yes";;
                  HostConf)
                      DO_HOSTCONF="yes";;
                  Path)
                     result=$($DLG --backtitle "$DLG_BACKTITLE" \
                        --nocancel --inputbox \
"Enter $OPTION value:" 0 $DLG_WIDTH_WIDE "$THIRD_PARTY_PATH" 3>&1 1>&2 2>&3)
                     THIRD_PARTY_PATH="$result"
                     DO_PATH="yes";;
                  Trace)
                     DO_VERBOSE="yes";;
                esac
            done
            ;;
          1)
            warn "Cancel pressed."
            exit 1;;
          255)
            warn "ESC pressed.";;
          *)
            warn "Unexpected return code: $retval";;
        esac
    fi
    return 0
}

# *************************************************************************** #
#                          extract_parallel_ldflags                           #
# --------------------------------------------------------------------------- #
# VisIt's cmake config wants lib names stripped of "-l"                       #
# If PAR_LIBS is used to pass parallel LDFLAGS we need to separate the libs   #
# from the linker flags and strip the "-l" prefixes.                          #
# This function accomplishes this and creates two new  variables:             #
#   PAR_LINKER_FLAGS & PAR_LIBRARY_NAMES                                      #
# *************************************************************************** #
function process_parallel_ldflags
{
    export PAR_LINKER_FLAGS=""
    export PAR_LIBRARY_NAMES=""

    for arg in $1; do
        pos=`echo "$arg" | awk '{ printf "%d", index($1,"-l"); }'`
        if [[ "$pos" != "0" ]] ; then
            # we have a lib, remove the "-l" prefix & add it to the running
            # list
            LIB_NAME=${arg#-l}
            export PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES$LIB_NAME "
        else
            # we have a linker flag, add it to the running list.
            export PAR_LINKER_FLAGS="$PAR_LINKER_FLAGS$arg "
        fi
    done

}


# *************************************************************************** #
#                         Function 2.1, check_parallel                        #
# --------------------------------------------------------------------------- #
# This function will check to make sure that parallel options have been setup #
# if we're going to build a parallel version of VisIt.                        #
# *************************************************************************** #

function check_parallel
{
    rv=0

    #
    # Parallelization
    #
    if [[ "$parallel" == "yes" ]] ; then

        #
        # VisIt's cmake build can obtain all necessary MPI flags from
        # a MPI compiler wrapper.
        # Check if PAR_COMPILER is set & if so use that.
        #
        export VISIT_MPI_COMPILER=""
        if [[ "$PAR_COMPILER" != "" ]] ; then
            export VISIT_MPI_COMPILER=$PAR_COMPILER
            info \
                "Configuring with mpi compiler wrapper: $VISIT_MPI_COMPILER"
            return 0
        fi

        #
        # Check the environment that mpicc would set up as a first stab.
        #
        MPICC_CPPFLAGS=""
        MPICC_LDFLAGS=""
        MPIWRAPPER=$(which mpic++)
        if [[ "${MPIWRAPPER#no }" != "${MPIWRAPPER}" ]] ; then
           MPIWRAPPER=""
        fi
        if [[ "$MPIWRAPPER" == "" ]] ; then
            MPIWRAPPER=$(which mpicxx)
            if [[ "${MPIWRAPPER#no }" != "${MPIWRAPPER}" ]] ; then
                MPIWRAPPER=""
            fi
            if [[ "$MPIWRAPPER" == "" ]] ; then
                MPIWRAPPER=$(which mpiCC)
                if [[ "${MPIWRAPPER#no }" != "${MPIWRAPPER}" ]] ; then
                    MPIWRAPPER=""
                fi
                if [[ "$MPIWRAPPER" == "" ]] ; then
                    MPIWRAPPER=$(which mpicc)
                    if [[ "${MPIWRAPPER#no }" != "${MPIWRAPPER}" ]] ; then
                        MPIWRAPPER=""
                    fi
                    if [[ "$MPIWRAPPER" == "" ]] ; then
                        warn "Unable to find mpiCC..."
                    fi
                fi
            fi
        fi

        #
        # VisIt's cmake build can obtain all necessary MPI flags from
        # a MPI compiler wrapper. If we have found one & the user
        # did not set PAR_LIBS or PAR_INCLUDE  we are done.
        #
        if [[ "$PAR_INCLUDE" == "" && "$PAR_LIBS" == "" && "$MPIWRAPPER" != "" ]] ; then
            export VISIT_MPI_COMPILER=$MPIWRAPPER
            info \
                "Configuring with mpi compiler wrapper: $VISIT_MPI_COMPILER"
            return 0
        fi

        #
        # If we have not found a MPI compiler wrapper. 
        # Keep trying to discover mpi setttings.
        #
        if [[ "$PAR_CPPFLAGS" == "" ]] ; then
            warn \
"We have no guesses as to where MPI might reside. Look for it..."
            if [[ -e /usr/include/mpi.h ]] ; then
                PAR_CPPFLAGS="-I/usr/include"
                PAR_LDFLAGS="-L/usr/lib -lmpi"
            fi
        fi

        if [[ "$GRAPHICAL" == "yes" ]] ; then
            # We have suggestions from the user or mpicc as to where mpi might
            # be located. See what the user thinks of the options.
            tryagain=1
            while [[ $tryagain == 1 ]]; do
                $DLG --backtitle "$DLG_BACKTITLE" --yesno \
"The CPPFLAGS for MPI are:\n\n$PAR_CPPFLAGS\n\nDo these look right?" \
                15 $DLG_WIDTH
                if [[ $? == 1 ]] ; then
                    tryagain=1
                    result=$($DLG --backtitle "$DLG_BACKTITLE" \
                    --nocancel --inputbox \
"Enter CPPFLAGS needed for MPI:" 0 $DLG_WIDTH_WIDE "$PAR_CPPFLAGS" 3>&1 1>&2 2>&3) 
                    PAR_CPPFLAGS="$result"
                else
                    tryagain=0
                fi
            done

            PAR_INCLUDE=$PAR_CPPFLAGS

            # We have suggestions from the user or mpicc as to where mpi might
            # be located. See what the user thinks of the options.
            tryagain=1
            while [[ $tryagain == 1 ]]; do
                $DLG --backtitle "$DLG_BACKTITLE" --yesno \
"The LDFLAGS for MPI are:\n\n$PAR_LDFLAGS\n\nDo these look right?" 15 $DLG_WIDTH
                if [[ $? == 1 ]] ; then
                    tryagain=1
                    result=$($DLG --backtitle "$DLG_BACKTITLE" \
                    --nocancel --inputbox \
"Enter LDFLAGS needed for MPI:" 0 $DLG_WIDTH_WIDE "$PAR_LDFLAGS"  3>&1 1>&2 2>&3) 
                    PAR_LDFLAGS="$result"
                else
                    tryagain=0
                fi
            done

            PAR_LIBS=$PAR_LDFLAGS
        fi

        # if we are using PAR_LIBS, call helper to split this into:
        # PAR_LIBRARY_NAMES & PAR_LINKER_FLAGS
        process_parallel_ldflags "$PAR_LIBS"

        # The script pretty much assumes that you *must* have some flags 
        # and libs to do a parallel build.  If that is *not* true, 
        # i.e. mpi.h is in your include path, then, congratulations, 
        # you are working on a better configured system than I have 
        # ever encountered.
        if [[ "$PAR_INCLUDE" == "" || "$PAR_LIBRARY_NAMES" == "" || "$PAR_LINKER_FLAGS" == "" ]] ; then
            warn \
"To configure parallel VisIt you must satisfy one of the following conditions:
    The PAR_COMPILER env var provides a path to a mpi compiler wrapper (such as mpic++).
    A mpi compiler wrapper (such as mpic++) to exists in your path.
    The PAR_INCLUDE & PAR_LIBS env vars provide necessary CXX & LDFLAGS to use mpi.

 To build ICE-T the PAR_INCLUDE env var must provide the include path to your mpi headers.
    "
            rv=1
        fi

        if [[ $rv != 0 ]] ; then
           return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                         Function 2.2, check_variables                       #
# --------------------------------------------------------------------------- #
# This function will display variables and optionally allow changing          #
# *************************************************************************** #

function check_variables_dialog
{
    local var="$1"
    local input="$2"

    result=$($DLG --backtitle "$DLG_BACKTITLE" \
                           --nocancel --inputbox \
"Enter $var value:" 0 $DLG_WIDTH_WIDE "$input"  3>&1 1>&2 2>&3)
    echo "$result"
}

function check_variables
{

    # Override variable settings dialog
    #
    if [[ "$verify" == "yes" ]] ; then
        if [[ "$GRAPHICAL" == "yes" ]] ; then
            result=$($DLG --backtitle "$DLG_BACKTITLE" \
            --title "Variable settings" \
            --checklist \
"These variables use these system dependent defaults, but can be overridden "\
"through this interface or using environment variables.\n\n"\
"OPSYS: the default value returned from 'uname -s'\n"\
"ARCH: architecure info (Darwin, linux, aix, irix64, ...)\n"\
"C_COMPILER and CXX_COMPILER: the C and C++ compiler, respectively\n"\
"CFLAGS and CXXFLAGS: the flags to use for all compiles (e.g. -fPIC)\n"\
"C_OPT_FLAGS and CXX_OPT_FLAGS: the optimization flags to use for C and C++\n"\
"VISITARCH: unique architecture info, appended to library path installation\n"\
"REVISION: checkout a cwspecific SVN revision using supplied argument\n\n"\
"Select the variables you wish to modify:" 28 $DLG_WIDTH 8 \
           "OPSYS"            "$OPSYS"             "off" \
           "ARCH"             "$ARCH"              "off" \
           "C_COMPILER"       "$C_COMPILER"        "off" \
           "CXX_COMPILER"     "$CXX_COMPILER"      "off" \
           "CFLAGS"           "$CFLAGS"       "off" \
           "CXXFLAGS"         "$CXXFLAGS"     "off" \
           "C_OPT_FLAGS"      "$C_OPT_FLAGS"       "off" \
           "CXX_OPT_FLAGS"    "$CXX_OPT_FLAGS"     "off" \
           "FC_COMPILER"      "$FC_COMPILER"       "off" \
           "FCFLAGS"          "$FCFLAGS"       "off" \
           "VISITARCH"        "$VISITARCHTMP"      "off" \
           "REVISION"         "$SVNREVISION"       "off"   3>&1 1>&2 2>&3) 
           retval=$?

           # Remove the extra quoting, new dialog has --single-quoted
           choice="$(echo $result | sed 's/"//g' )"
           tmp_var=0
           case $retval in
             0)
               for OPTION in $choice
               do

                   #this code uses the name to get and set the option value
                   #please use this convention, otherwise you will have to create
                   #exception as the ones below..
                   [[ $OPTION == "VISITARCH" ]] && OPTION="VISITARCHTMP"
                   [[ $OPTION == "REVISION" ]] && OPTION="SVNREVISION"

                   eval "tmp_var=\"\$$OPTION\""
                   tmp_var=$(check_variables_dialog $OPTION "$tmp_var")
                   eval "$OPTION=\"$tmp_var\""

                   if [[ $OPTION == "SVNREVISION" ]]; then
                        echo "Revision set to $SVNREVISION"
                        DO_SVN="yes"
                        DO_REVISION="yes"
                   fi
               done
               ;;
             1)
               warn "Cancel pressed."
               ;;
             255)
               warn "ESC pressed.";;
             *)
               warn "Unexpected return code: $retval";;
           esac
       fi
    fi

    return 0
}


# *************************************************************************** #
#                          Function 9, build_hostconf                         #
#                                                                             #
# Mark C. Miller, Wed Oct 27 19:29:19 PDT 2010                                #
# Adjusted ordering of database lib variables to ensure LIBDEP gets processed #
# correctly. Added comments to host conf file regarding ordering issue.       #
#                                                                             #
# Kathleen Bonnell, Wed Feb 16 08:35:40 PST 2011                              #
# Remove setting of CMAKE_BUILD_TYPE                                          #
#                                                                             #
# Kathleen Biagas, Mon Aug 8 08:12:37 MST 2011                                #
# Use FILEPATH type for compilers, STRING type for libdep.                    # 
# *************************************************************************** #

function build_hostconf
{
    #
    # Set up environment variables for the configure step.
    #
    PARFLAGS=""
    if [[ "$parallel" == "yes" ]] ; then
       PARFLAGS="--enable-parallel"
    fi

    #
    # Set up the config-site file, which gives configure the information it
    # needs about the third party libraries.

    export HOSTCONF="$(hostname).cmake"
    info "Creating $HOSTCONF"

    # First line of config-site file provides a hint to the location
    # of cmake.

    THIRD_PARTY_ABS_PATH=$(pushd $THIRD_PARTY_PATH 1,2>/dev/null; pwd; popd 1,2>/dev/null)
    if [[ "$CMAKE_INSTALL" != "" ]]; then
        echo "#$CMAKE_INSTALL/cmake" > $HOSTCONF
    else
        echo "#$THIRD_PARTY_ABS_PATH/cmake/$CMAKE_VERSION/$VISITARCH/bin/cmake" > $HOSTCONF
    fi
    echo "##" >> $HOSTCONF
    echo "## $0 generated host.cmake" >> $HOSTCONF
    echo "## created: $(date)" >> $HOSTCONF
    echo "## system: $(uname -a)" >> $HOSTCONF
    echo "## by: $(whoami)" >> $HOSTCONF
    echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## Setup VISITHOME & VISITARCH variables." >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "SET(VISITHOME $VISITDIR)" >> $HOSTCONF
    echo "SET(VISITARCH $VISITARCH)" >> $HOSTCONF
    echo >> $HOSTCONF

#####
    echo "## Compiler flags." >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "VISIT_OPTION_DEFAULT(VISIT_C_COMPILER $C_COMPILER TYPE FILEPATH)">> $HOSTCONF
    echo "VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER $CXX_COMPILER TYPE FILEPATH)" >> $HOSTCONF

    if [[ "$USE_VISIBILITY_HIDDEN" == "yes" ]] ; then
        echo "VISIT_OPTION_DEFAULT(VISIT_C_FLAGS \"$CFLAGS -fvisibility=hidden\" TYPE STRING)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS \"$CXXFLAGS -fvisibility=hidden\" TYPE STRING)" >> $HOSTCONF
    else
        echo "VISIT_OPTION_DEFAULT(VISIT_C_FLAGS \"$CFLAGS\" TYPE STRING)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS \"$CXXFLAGS\" TYPE STRING)" >> $HOSTCONF
    fi

    if [[ "$parallel" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Parallel Build Setup." >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)" >> $HOSTCONF
        # we either set an mpi wrapper compiler in the host conf
        if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
            echo "## (configured w/ mpi compiler wrapper)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER $VISIT_MPI_COMPILER TYPE FILEPATH)"  >> $HOSTCONF
        else
            # or we just set the flags.
            echo "## (configued w/ user provided CXX (PAR_INCLUDE) & LDFLAGS (PAR_LIBS) flags)" \
             >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS \"$PAR_INCLUDE\" TYPE STRING)"     >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  \"$PAR_LINKER_FLAGS\" TYPE STRING)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS        $PAR_LIBRARY_NAMES TYPE STRING)" >> $HOSTCONF
        fi
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## Static build" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_STATIC ON TYPE BOOL)" >> $HOSTCONF
    fi

    echo >> $HOSTCONF
    echo \
"##############################################################" >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## Database reader plugin support libraries" >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## The HDF4, HDF5 and NetCDF libraries must be first so that" >> $HOSTCONF
    echo "## their libdeps are defined for any plugins that need them." >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## For libraries with LIBDEP settings, order matters." >> $HOSTCONF
    echo "## Libraries with LIBDEP settings that depend on other" >> $HOSTCONF
    echo "## Library's LIBDEP settings must come after them." >> $HOSTCONF
    echo \
"##############################################################" >> $HOSTCONF

    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        initialize="bv_${reqlibs[$bv_i]}_host_profile"
        $initialize
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        initialize="bv_${optlibs[$bv_i]}_host_profile"
        $initialize
    done
    echo >> $HOSTCONF

    #
    # Patch for Ubuntu 11.04
    #
    if test -d "/usr/lib/x86_64-linux-gnu" ; then
        numLibs=$(ls -1 /usr/lib/x86_64-linux-gnu | wc -l)
        if (( $numLibs > 10 )) ; then
           rm -f $HOSTCONF.tmp
           cat $HOSTCONF | sed "s/\/usr\/lib/\/usr\/lib\/x86_64-linux-gnu/" > $HOSTCONF.tmp
           rm $HOSTCONF
           mv $HOSTCONF.tmp $HOSTCONF
        fi
    fi

    cd "$START_DIR"
    echo "Done creating $HOSTCONF"
    return 0
}

function printvariables  
{
  printf "The following is a list of user settable environment variables\n"
  printf "\n"
  printf "%s%s\n" "OPSYS=" "${OPSYS}"
  printf "%s%s\n" "PROC=" "${PROC}"
  printf "%s%s\n" "REL=" "${REL}"
  printf "%s%s\n" "ARCH=" "${ARCH}"
  printf "%s%s\n" "VISITARCH=" "${VISITARCHTMP}"

  printf "%s%s\n" "C_COMPILER=" "${C_COMPILER}"
  printf "%s%s\n" "CXX_COMPILER=" "${CXX_COMPILER}"
  printf "%s%s\n" "CFLAGS=" "${CFLAGS}"
  printf "%s%s\n" "CXXFLAGS=" "${CXXFLAGS}"
  printf "%s%s\n" "C_OPT_FLAGS=" "${C_OPT_FLAGS}"
  printf "%s%s\n" "CXX_OPT_FLAGS=" "${CXX_OPT_FLAGS}"
  printf "%s%s\n" "PAR_INCLUDE=" "${PAR_INCLUDE}"
  printf "%s%s\n" "PAR_LIBS=" "${PAR_LIBS}"

  printf "%s%s\n" "MAKE=" "${MAKE}"
  printf "%s%s\n" "THIRD_PARTY_PATH=" "${THIRD_PARTY_PATH}"
  printf "%s%s\n" "GROUP=" "${GROUP}"
  printf "%s%s\n" "LOG_FILE=" "${LOG_FILE}"
  printf "%s%s\n" "LOG_FILE=" "${LOG_FILE}"
  printf "%s%s\n" "WGET_OPTS=" "${WGET_OPTS}"
  printf "%s%s\n" "SVNREVISION=" "${SVNREVISION}"
  
  bv_visit_print
  for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
  do
      initialize="bv_${reqlibs[$bv_i]}_print"
      $initialize
  done

  for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
  do
      initialize="bv_${optlibs[$bv_i]}_print"
      $initialize
  done
}

function usage  
{
  printf "Usage: %s [options]\n" $0
  printf "A download attempt will be made for all files which do not exist."
  printf "\n\n"
  printf "BOOLEAN FLAGS\n"
  printf "\tThese are used to enable or disable specific functionality.  They do not take option values.\n\n"
  printf "%-15s %s [%s]\n" "--dry-run"  "Dry run of the presented options" "false"
  printf "%-15s %s [%s]\n" "--build-mode" "VisIt build mode (Debug or Release)" "$VISIT_BUILD_MODE"
  printf "%-15s %s [%s]\n" "--console" "Do not use dialog ('graphical') interface" "!$GRAPHICAL"
  printf "%-15s %s [%s]\n" "--dbio-only" "Disables EVERYTHING but I/O." "$DO_DBIO_ONLY"
  printf "%-15s %s [%s]\n" "--engine-only" "Only build the compute engine." "$DO_ENGINE_ONLY"
  printf "%-15s %s [%s]\n" "--debug"   "Enable debugging for this script" "false"
  printf "%-15s %s [%s]\n" "--download-only" "Only download the specified packages" "false"
  printf "%-15s %s [%s]\n" "--flags-debug" "Add '-g' to C[XX]FLAGS" "false"
  printf "%-15s %s [%s]\n" "--group" "Group name of installed libraries" "$GROUP"
  printf "%-15s %s [%s]\n" "-h" "Display this help message." "false"
  printf "%-15s %s [%s]\n" "--help" "Display this help message." "false"
  printf "%-15s %s [%s]\n" "--java" "Build with the Java client library" "${DO_JAVA}"
  printf "%-15s %s [%s]\n" "--no-hostconf" "Do not create host.conf file." "$ON_HOSTCONF"
  printf "%-15s %s [%s]\n" "--parallel" "Enable parallel build, display MPI prompt" "$parallel"
  printf "%-15s %s [%s]\n" "--prefix" "The directory to which VisIt should be installed once it is built" "$VISIT_INSTALL_PREFIX"
  printf "%-15s %s [%s]\n" "--print-vars" "Display user settable environment variables" "false"
  printf "%-15s %s [%s]\n" "--python-module" "Build with the VisIt Python module" "$DO_MODULE"
  printf "%-15s %s [%s]\n" "--server-components-only" "Only build VisIt's server components (mdserver,vcl,engine)." "$DO_SERVER_COMPONENTS_ONLY"
  printf "%-15s %s [%s]\n" "--slivr" "Build with SLIVR shader support" "$DO_SLIVR"
  printf "%-15s %s [%s]\n" "--static" "Build using static linking" "$DO_STATIC_BUILD"
  printf "%-15s %s [%s]\n" "--stdout" "Write build log to stdout" "$LOG_FILE"
  
  for (( bv_i=0; bv_i<${#grouplibs_comment[*]}; ++bv_i ))
  do
        name=${grouplibs_name[$bv_i]}
        comment=${grouplibs_comment[$bv_i]}
        printf "%-15s %s [%s]\n" "--$name" "$comment" ""
  done

  bv_visit_print_usage

  for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
  do
      initialize="bv_${reqlibs[$bv_i]}_print_usage"
      $initialize
  done

  for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
  do
      initialize="bv_${optlibs[$bv_i]}_print_usage"
      $initialize
  done

  printf "%s\n" ""
  printf "OPTIONS\n"
  printf "These values all take a special value.  If given, they require an associated value to be provided as well.\n\n"
  printf "%-15s \n\t%s [%s]\n" "--installation-build-dir"  "Specify the directory visit will use for building" "output-filename"
  printf "%-15s \n\t%s [%s]\n" "--write-unified-file"  "Write single unified build_visit file" "output-filename"
  printf "%s <%s> %s [%s]\n" "--arch" "architecture" "Set architecture" "$VISITARCHTMP"
  printf "\t  %s\n" "   This variable is used in constructing the 3rd party"
  printf "\t  %s\n" "   library path; usually set to something like"
  printf "\t  %s\n" "   'linux_gcc-3.4.6' or 'Darwin_gcc-4.0.1'"
  printf "%-11s %s [%s]\n" "--cflag"   "Append a flag to CFLAGS" "${CFLAGS}"
  printf "%-11s %s [%s]\n" "--cxxflag" "Append a flag to CXXFLAGS" "$CXXFLAGS"
  printf "%-11s %s [%s]\n" "--cflags"  "Explicitly set CFLAGS" "$CFLAGS"
  printf "%-11s %s [%s]\n" "--cxxflags" "Explicitly set CXXFLAGS" "$CXXFLAGS"
  printf "%-11s %s [%s]\n" "--cc"  "Explicitly set C_COMPILER" "$C_COMPILER"
  printf "%-11s %s [%s]\n" "--cxx" "Explicitly set CXX_COMPILER" "$CXX_COMPILER"
  printf "%-11s <%s> %s [%s]\n" "--makeflags" "flags" "Flags to 'make'" "$MAKE_OPT_FLAGS"
  printf "%s <%s> %s\n" "--svn" \
    "Obtain VisIt source code and third party libraries from the SVN server"
  printf "\t%s\n" "    [svn co $SVN_REPO_ROOT_PATH/$SVN_SOURCE_PATH]"
  printf "%s <%s> %s\n" "--svn-anonymous" \
    "Obtain VisIt source code and third party libraries using the anonymous SVN mirror."
  printf "\t%s\n" "    [svn co $SVN_ANON_ROOT_PATH/$SVN_SOURCE_PATH]"
  printf "%s <%s> %s\n" "--svn-revision" "revision" \
    "Specify the SVN revision of the VisIt source code and third party libraries to download.  Used in conjunction with --svn or --svn-anonymous."
  printf "%s <%s> %s [%s]\n" "--tarball" "file" "tarball to extract VisIt from" "$VISIT_FILE"
  printf "%-11s <%s> \n%s [%s]\n" "--thirdparty-path" "/path/to/directory" \
    "             Specify the root directory name under which the 3rd party
             libraries have been installed.  If defined, it would typically
             mean the 3rd party libraries are pre-built and are installed
             somewhere like /usr/gapps/visit." "${THIRD_PARTY_PATH}"
  printf "%s <%s> %s [%s]\n" "--version" "version" "The VisIt version to build" "$VISIT_VERSION"
}


#TODO: pass these two variables from command line..
mangle_src="VTK"
mangle_dest="MTK"
uc_mangled_src=`echo $mangle_src | tr '[a-z]' '[A-Z]'`
uc_mangled_dest=`echo $mangle_dest | tr '[a-z]' '[A-Z]'`
lc_mangled_src=`echo $mangle_src | tr '[A-Z]' '[a-z]'`
lc_mangled_dest=`echo $mangle_dest | tr '[A-Z]' '[a-z]'`

function mangle_file
{
    local input_file="$1"
    local output_file="$2"

    cat "$input_file" | sed -e s/${lc_mangled_src}/${lc_mangled_dest}/g -e s/${uc_mangled_src}/${uc_mangled_dest}/g > "$output_file"
    
    #chmod --reference=$input_file $output_file
    if [[ -r "$input_file" ]]; then
        chmod u+r "$output_file"
    fi
    if [[ -w "$input_file" ]]; then
        chmod u+r "$output_file"
    fi
    if [[ -x "$input_file" ]]; then
        chmod u+x "$output_file"
    fi
}

function mangle_libraries
{
    local input_dir="$1"
    local mangled_dir="$2"

    if [[ ! -d "$input_dir" ]]; then
        info "Input directory $input_dir does not exist"
        return 1
    fi

    if [[ -d "$mangled_dir" ]]; then
        info "Found pre-existing mangled directory $mangled_dir, removing"
        rm -fR "$mangled_dir"
    fi

    info "mangling $input_dir $mangled_dir"
    #get all files from directory..
    local args=`find "${input_dir}" -name "*"`
    local i=0
    for i in `echo $args`
    do
        #replace all occurrences of $mangled_src with mangled_dest
        newpath=${i/${input_dir}/}
        newpath=${newpath//${uc_mangled_src}/${uc_mangled_dest}}
        newpath=${newpath//${lc_mangled_src}/${lc_mangled_dest}}
        mangled_path="${mangled_dir}/${newpath}"
        newdir=`dirname "${mangled_path}"`
    
        #create new dir
        mkdir -p "$newdir"
        #cat old file replace ${mangled_src} with ${mangled_dest}
        if [[ ! -d $i ]]; then
            mangle_file "$i" "${mangled_path}" 
        else
            #chmod --reference=$i $newdir
            if [[ -r "$i" ]]; then
                chmod u+r "$newdir"
            fi
            if [[ -w "$i" ]]; then
                chmod u+r "$newdir"
            fi
            if [[ -x "$i" ]]; then
                chmod u+x "$newdir"
            fi
        fi
    done
    return 0
}
