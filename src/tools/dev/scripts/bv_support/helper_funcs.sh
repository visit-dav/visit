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
    declare -F "bv_${optlib}_dry_run" &>/dev/null || errorFunc "${optlib} dry_run not found"
    declare -F "bv_${optlib}_is_installed" &>/dev/null || errorFunc "${optlib} is_installed not found"
    declare -F "bv_${optlib}_is_enabled" &>/dev/null || errorFunc "${optlib} is_enabled not found"
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
        echo "$@" 1>&3
    else
        echo "$@"
    fi

    if [[ "${LOG_FILE}" != "/dev/tty" ]] ; then
        # write message to log as well
        log "$@"
    fi
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
    elif [[ $(echo $1 | egrep "\.tar.gz$" ) != "" ]] ; then
        COMPRESSTYPE="targzip"
    elif [[ $(echo $1 | egrep "\.zip$" ) != "" ]] ; then
        COMPRESSTYPE="zip"
    elif [[ $(echo $1 | egrep "\.xz$" ) != "" ]] ; then
        COMPRESSTYPE="xz"
    else
        warn "unsupported decompression method"
        return 1
    fi
    TARVERSION=$($TAR --version >/dev/null 2>&1)
    if [[ $? == 0 ]] ; then
        case $COMPRESSTYPE in
            gzip|targzip) $TAR zxf $1;;
            bzip) $TAR jxf $1;;
            zip) unzip $1;;
            xz) $TAR xf $1;;
        esac

        if [[ $? != 0 ]]; then
            warn "error decompressing $1"
            return 1
        fi

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
            zip)
                unzip $1
                ;;
        esac

        if [[ $? != 0 ]]; then
            warn "error decompressing $1"
            return 1
        fi
    fi
}

# *************************************************************************** #
# Function: verify_checksum                                                   #
#                                                                             #
# Purpose: Verify the checksum of the given file                              #
#                                                                             #
#          verify_md5_checksum: checks md5                                    #
#          verify_sha_checksum: checks sha (256,512)                          #
#          verfiy_checksum_by_lookup: pick which checksum method to use       #
#                                     based on if they are defined giving     #
#                                     preference to the strongest checksums.  #
#                                                                             #
# Programmer: Hari Krishnan                                                   #
#                                                                             #
# Modifications:                                                              #
#   Eric Brugger, Thu Apr 11 15:51:25 PDT 2019                                #
#   Modified verify_checksum_by_lookup to also check that the checksum is     #
#   not blank in addition to being defined before using it.                   #
#                                                                             #
# *************************************************************************** #

function verify_md5_checksum
{
    checksum=$1
    dfile=$2
    md5cmd=md5sum

    tmp=`which $md5cmd`
    if [[ $? != 0 ]]; then
        tmp=`which md5`
        if [[ $? != 0 ]]; then
            info "could not find md5sum or md5 commands, disabling check"
            return 0
        fi
        md5cmd=md5
    fi
    tmp=`$md5cmd $dfile | tr ' ' '\n' | grep '^[0-9a-f]\{32\}'`
    if [[ $tmp == ${checksum} ]]; then
        info "verified"
        return 0
    fi

    info "md5 checksum failed: looking for $checksum got $tmp"
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

    set -x

    if [[ $checksum_algo == 512 ]]; then
        tmp=`shasum -a $checksum_algo $dfile | tr ' ' '\n' | grep '^[0-9a-f]\{128\}'`
    else
        tmp=`shasum -a $checksum_algo $dfile | tr ' ' '\n' | grep '^[0-9a-f]\{64\}'`
    fi
    if [[ "$tmp" == "$checksum" ]]; then
        info "verified"
        return 0
    else
        info "shasum -a $checksum_algo failed: looking for $checksum got $tmp"
        return 1
    fi

    info "shasum does not support $checksum_algo, check disabled"
    return 0
}

function verify_checksum
{
    checksum_type=$1
    checksum=$2
    dfile=$3

    info "verifying $checksum_type checksum $checksum for $dfile . . ."

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

function verify_checksum_by_lookup
{
    dlfile=$(basename $1) # the downloaded file name

    # search for all shell vars with name of the form XXX_FILE defined
    # that have a value that is this file. The +-o posix stuff is to cull
    # out function names and definitions from the search
    for var in $(set -o posix; set | grep _FILE=; set +o posix); do
        var=$(echo $var | cut -d '=' -f1)
        if [ ${!var} = $dlfile ]; then
            varbase=$(echo $var | sed -e 's/_FILE$//')
            md5sum_varname=${varbase}_MD5_CHECKSUM
            sha256_varname=${varbase}_SHA256_CHECKSUM
            sha512_varname=${varbase}_SHA512_CHECKSUM
            if [ ! -z ${!sha512_varname} ]; then
                verify_checksum SHA512 ${!sha512_varname} $dlfile
                return $?
            elif [ ! -z ${!sha256_varname} ]; then
                verify_checksum SHA256 ${!sha256_varname} $dlfile
                return $?
            elif [ ! -z ${!md5sum_varname} ]; then
                verify_checksum MD5 ${!md5sum_varname} $dlfile
                return $?
            fi
        fi
    done

    # since this is an optional check, all cases should pass if it gets here.
    info "unable to find a MD5, SHA256, or SHA512, checksum associated with $dlfile; check disabled"
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
#   Eric Brugger, Fri Feb  1 14:56:58 PST 2019
#   I modified it to work post git transition.
#
# *************************************************************************** #

function download_file
{
    # $1 is the file name to download
    # $2...$* [OPTIONAL] list of sites to obtain the file from

    typeset dfile=$1
    info "Downloading $dfile . . ."
    shift

    # If the visit source code is requested, handle that now.
    site="${nerscroot}/${VISIT_VERSION}"
    if [[ "$dfile" == "$VISIT_FILE" ]] ; then
        try_download_file $site/$dfile $dfile
        if [[ $? == 0 ]] ; then
            return 0
        fi
    fi

    # It must be a third party library, handle that now.
    #
    # First try NERSC.
    site="${nerscroot}/${VISIT_VERSION}/third_party"
    try_download_file $site/$dfile $dfile
    if [[ $? == 0 ]] ; then
        return 0
    fi

    # Now try the various places listed.
    if [[ "$1" != "" ]] ; then
        for site in $* ; do
            # check if we have a google shortened url that won't accept
            # the actual file name (we need this for mfem's urls)
            if [[ $site == *goo.gl* ]] ; then
                try_download_file_from_shortened_url $site $dfile
                if [[ $? == 0 ]] ; then
                    return 0
                fi
            else
                try_download_file $site/$dfile $dfile
                if [[ $? == 0 ]] ; then
                    return 0
                fi
            fi
        done
    fi

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

    verify_checksum_by_lookup `basename $1`
    if [[ $? == 0 && -e `basename $1` ]] ; then
        info "Download succeeded: $1"
        return 0
    else
        warn "Download attempt failed: $1"
        rm -f `basename $1`
        return 1
    fi

}

# ***************************************************************************
# Function: try_download_file_from_shortened_url
#
# Purpose: DONT USE THIS FUNCTION. USE download_file.
#
# New variant of try_download_file, downloads a file using wget or curl
# using an explicit file name. This is necessary for shortened urls.
#
# Programmer: Cyrus Harrison
# Creation: June 1, 2016
#
# ***************************************************************************

function try_download_file_from_shortened_url
{
    if [[ "$OPSYS" == "Darwin" ]]; then
        # MaxOS X comes with curl
        /usr/bin/curl -o $2 -ksfLO $1
    else
        check_wget
        if [[ $? != 0 ]] ; then
            error "Need to download $1, but \
                   cannot locate the wget utility to do so."
        fi
        wget $WGET_OPTS -O $2 -o /dev/null $1
    fi

    verify_checksum_by_lookup $2
    if [[ $? == 0 && -e $2 ]] ; then
        info "Download succeeded: $1"
        return 0
    else
        warn "Download attempt failed: $1"
        rm -f $2
        return 1
    fi
}



# *************************************************************************** #
# Function: check_git_client                                                  #
#                                                                             #
# Purpose: Helper that checks if a git client is available.                    #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date:  Mon Nov 17 14:52:37 PST 2008                                         #
#                                                                             #
# Modifications:
#   Eric Brugger, Fri Feb  1 14:56:58 PST 2019
#   I modified it to work post git transition.
#
# *************************************************************************** #

function check_git_client
{
    # check for git client
    GIT_CLIENT=$(which git)
    if [[ $GIT_CLIENT == "" ]] ; then
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
# Modifications:                                                              #
#                                                                             #
#   Paul Selby, Wed  4 Feb 17:25:22 GMT 2015                                  #
#   Fixed typo which prevented verify_checksum being called                   #
# *************************************************************************** #
function prepare_build_dir
{
    echo "prepare_build_dir:" $1 $2
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
        if [[ $CHECKSUM_VALUE != "" && $CHECKSUM_TYPE != "" ]]; then
            verify_checksum $CHECKSUM_TYPE $CHECKSUM_VALUE ${SRC_FILE}
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
#                          process_parallel_ldflags                           #
# --------------------------------------------------------------------------- #
# This routine processes the PAR_LIBS variable into three other variables.    #
#   PAR_LINKER_FLAGS :        Any linker flags that aren't libraries (don't   #
#                             start with "-l".                                #
#   PAR_LIBRARY_NAMES:        The library names with the "-l" stripped out.   #
#   PAR_LIBRARY_LINKER_FLAGS: The library names with the "-l".                #
# *************************************************************************** #
function process_parallel_ldflags
{
    export PAR_LINKER_FLAGS=""
    export PAR_LIBRARY_NAMES=""
    export PAR_LIBRARY_LINKER_FLAGS=""

    for arg in $1; do
        pos=`echo "$arg" | awk '{ printf "%d", index($1,"-l"); }'`
        if [[ "$pos" != "0" ]] ; then
            # We have a library.
            # Add it to the running list of library names with the "-l".
            export PAR_LIBRARY_LINKER_FLAGS="$PAR_LIBRARY_LINKER_FLAGS$arg "
            # Remove the "-l" prefix & add it to the running list of library
            # names without the "-l".
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

    if [[ "$DO_MPICH" == "yes" && "$parallel" == "no" ]] ; then
        parallel="yes"
    fi

    # If we are using PAR_LIBS, call helper to split this into:
    # PAR_LINKER_FLAGS, PAR_LIBRARY_NAMES & PAR_LIBRARY_LINKER_FLAGS
    process_parallel_ldflags "$PAR_LIBS"

    # If we are using PAR_INCLUDE, store the directory name without the
    # "-I"
    export PAR_INCLUDE_PATH=`echo "$PAR_INCLUDE" | sed "s/-I//"`

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
        export VISIT_MPI_COMPILER_CXX=""
        if [[ "$PAR_COMPILER" != "" ]] ; then
            export VISIT_MPI_COMPILER="$PAR_COMPILER"
            info \
                "Configuring with mpi compiler wrapper: $VISIT_MPI_COMPILER"
            if [[ "$PAR_COMPILER_CXX" != "" ]] ; then
                export VISIT_MPI_COMPILER_CXX="$PAR_COMPILER_CXX"
                info \
                    "Configuring with mpi c++ compiler wrapper: $VISIT_MPI_COMPILER_CXX"
            fi
            return 0
        fi

        #
        # VisIt's build_visit can obtain all necessary MPI flags from
        # bv_mpich. If we are building mpich and the user
        # did not set PAR_LIBS or PAR_INCLUDE we are done.
        #
        if [[ "$DO_MPICH" == "yes" && "$PAR_INCLUDE" == "" && "$PAR_LIBS" == "" && "$MPIWRAPPER" == "" ]] ; then

            export MPICH_COMPILER="${VISITDIR}/mpich/$MPICH_VERSION/${VISITARCH}/bin/mpicc"
            export MPICH_COMPILER_CXX="${VISITDIR}/mpich/$MPICH_VERSION/${VISITARCH}/bin/mpic++"
            export VISIT_MPI_COMPILER="$MPICH_COMPILER"
            export VISIT_MPI_COMPILER_CXX="$MPICH_COMPILER_CXX"
            export PAR_COMPILER="$MPICH_COMPILER"
            export PAR_COMPILER_CXX="$MPICH_COMPILER_CXX"
            export PAR_INCLUDE="-I${VISITDIR}/mpich/$MPICH_VERSION/${VISITARCH}/include"
            info  "Configuring parallel with mpich build: "
            info  "  PAR_COMPILER: $PAR_COMPILER"
            info  "  PAR_COMPILER_CXX: $PAR_COMPILER_CXX"
            info  "  PAR_INCLUDE: $PAR_INCLUDE"
            return 0
        fi

        #
        # Check the environment that mpicc would set up as a first stab.
        # Since VisIt currently only ever uses MPI's C interface, we need
        # only the information to link to MPI's implementation of its C
        # interface. So, although VisIt is largely a C++ code, it is fine
        # and correct to utilize an MPI C compiler here.
        #
        MPICC_CPPFLAGS=""
        MPICC_LDFLAGS=""
        MPIWRAPPER=$(which mpicc)
        if [[ "${MPIWRAPPER#no }" != "${MPIWRAPPER}" ]] ; then
            MPIWRAPPER=""
        fi
        if [[ "$MPIWRAPPER" == "" ]] ; then
            if [[ "$CRAY_MPICH_DIR" != "" ]] ; then
                warn "Unable to find mpicc..."
            fi
        fi

        #
        # VisIt's cmake build can obtain all necessary MPI flags from
        # a MPI compiler wrapper. If we have found one & the user
        # did not set PAR_LIBS or PAR_INCLUDE we are done.
        #
        if [[ "$PAR_INCLUDE" == "" && "$PAR_LIBS" == "" && "$MPIWRAPPER" != "" ]] ; then
            export VISIT_MPI_COMPILER=$MPIWRAPPER
            export PAR_COMPILER=$MPIWRAPPER
            info \
                "Configuring with mpi compiler wrapper: $VISIT_MPI_COMPILER"
            return 0
        fi

        #
        # VisIt's build_visit can obtain all necessary MPI flags from
        # bv_mpich. If we are building mpich and the user
        # did not set PAR_LIBS or PAR_INCLUDE we are done.
        #
        if [[ "$DO_MPICH" == "yes" && "$PAR_INCLUDE" == "" && "$PAR_LIBS" == "" && "$MPIWRAPPER" == "" ]] ; then

            export MPICH_COMPILER="${VISITDIR}/mpich/$MPICH_VERSION/${VISITARCH}/bin/mpicc"
            export VISIT_MPI_COMPILER="$MPICH_COMPILER"
            export PAR_COMPILER="$MPICH_COMPILER"
            info \
                "Configuring with build mpich: $MPICH_COMPILER"
            return 0
        fi

        #
        # Try and use the Cray wrapper compiler to get MPI options.
        #
        if [[ "$CRAY_MPICH_DIR" != "" ]] ; then
             # NOTE: Unload darshan and cray-libsci. Otherwise keep the
             #       programming environment that is in effect.
             CCOUT=$(module unload darshan; module unload cray-libsci; CC --cray-print-opts=all)
             ingroup="no"
             arg_rpath=""
             for arg in $CCOUT ;
             do
                 # NOTE: adding the -Wl,-Bstatic/-Wl,-Bdynamic around the group is
                 # a workaround to linking with the "darshan" libraries that come
                 # in via CCOUT on cori.nersc.gov
                 if [[ "$arg" == "-Wl,--start-group" ]] ; then
                     ingroup="yes"
                     if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
                         PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES $arg"
                     else
                         PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES -Wl,-Bstatic $arg"
                     fi
                 elif [[ "$arg" == "-Wl,--end-group" ]] ; then
                     ingroup="no"
                     if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
                         PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES $arg"
                     else
                         PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES -Wl,-Bdynamic $arg"
                     fi
                 elif [[ "$ingroup" == "yes" ]] ; then
                     PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES $arg"
                 else
                     A2=$(echo $arg | cut -c 1-2)
                     A3=$(echo $arg | cut -c 1-3)
                     if [[ "$A2" == "-I" ]] ; then
                         PAR_INCLUDE="$PAR_INCLUDE $arg"
                     elif [[ "$A2" == "-L" ]] ; then
                         arg_rpath="$arg_rpath -Wl,-rpath,$(echo $arg | cut -c 3-)"
                         PAR_LINKER_FLAGS="$PAR_LINKER_FLAGS $arg"
                     elif [[ "$A3" == "-Wl" ]] ; then
                         PAR_LINKER_FLAGS="$PAR_LINKER_FLAGS $arg"
                     elif [[ "$A2" == "-l" ]] ; then
                         PAR_LIBRARY_NAMES="$PAR_LIBRARY_NAMES $(echo $arg | cut -c 3-)"
                     fi
                 fi
             done
             if [[ "$DO_STATIC_BUILD" == "no" ]] ; then
                 PAR_LINKER_FLAGS="$PAR_LINKER_FLAGS$arg_rpath"
             fi
        fi

        # The script pretty much assumes that you *must* have some flags
        # and libs to do a parallel build.  If that is *not* true,
        # i.e. mpi.h is in your include path, then, congratulations,
        # you are working on a better configured system than I have
        # ever encountered.
        if [[ "$PAR_INCLUDE" == "" || "$PAR_LIBRARY_NAMES" == "" || "$PAR_LINKER_FLAGS" == "" ]] ; then
            warn \
                        "To configure parallel VisIt you must satisfy one of the following conditions:
    The PAR_COMPILER env var provides a path to a mpi compiler wrapper (such as mpicc).
    A mpi compiler wrapper (such as mpicc) to exists in your path.
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
hostconf_library_success=""
function hostconf_library
{
    local build_lib=$1
    local depends_on=""

    # if already in success list then ignore..
    if [[ "$hostconf_library_success" == *$build_lib* ]]; then
        return
    fi

    depends_on=$("bv_${build_lib}_depends_on")

    #replace commas with spaces if there are any..
    depends_on=${depends_on//,/ }

    for depend_lib in `echo $depends_on`;
    do
        hostconf_library $depend_lib
    done

    #build ..
    $"bv_${build_lib}_host_profile"
    hostconf_library_success="${hostconf_library_success} ${build_lib}"
}

# *************************************************************************** #
# Function: build_hostconf                                                    #
#                                                                             #
# Purpose: builds the config-site file for this host                          #
#                                                                             #
# Modifications:                                                              #
#   Kathleen Biagas, Thu Mar 14 11:28:38 PDT 2019                             #
#   Don't put the C or CXX OPT_FLAGS in the host file. These will be handled  #
#   by CMake when CMAKE_BUILD_TYPE is selected.                               #
#                                                                             #
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

    if [[ "${VISIT_HOSTNAME}" != "" ]]; then
        info "VISIT_HOSTNAME env variable found: Using ${VISIT_HOSTNAME}.cmake"
        HOSTCONF="${VISIT_HOSTNAME}.cmake"
    fi

    if [[ "${EXTERNAL_HOSTNAME}" != "" ]]; then
        info "External Hostname variable found: Using ${EXTERNAL_HOSTNAME}"
        HOSTCONF="${EXTERNAL_HOSTNAME}"
    fi

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
    if [[ "$FC_COMPILER" != "" ]] ; then
        echo "VISIT_OPTION_DEFAULT(VISIT_FORTRAN_COMPILER $FC_COMPILER TYPE FILEPATH)" >> $HOSTCONF
    fi

    if [[ "$USE_VISIBILITY_HIDDEN" == "yes" ]] ; then
        echo "VISIT_OPTION_DEFAULT(VISIT_C_FLAGS \"$CFLAGS -fvisibility=hidden\" TYPE STRING)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS \"$CXXFLAGS -fvisibility=hidden\" TYPE STRING)" >> $HOSTCONF
    else
        if test -n "$CFLAGS" ; then
            echo "VISIT_OPTION_DEFAULT(VISIT_C_FLAGS \"$CFLAGS\" TYPE STRING)" >> $HOSTCONF
        fi
        if test -n "$CXXFLAGS" ; then
            echo "VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS \"$CXXFLAGS\" TYPE STRING)" >> $HOSTCONF
        fi
    fi

    if [[ "$VISIT_INSTALL_PREFIX" != "" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VisIt install location." >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX $VISIT_INSTALL_PREFIX TYPE FILEPATH)" >> $HOSTCONF
    fi
    if [[ "$VISIT_INSTALL_NETWORK" != "" ]] ; then
        echo "VISIT_OPTION_DEFAULT(VISIT_INSTALL_PROFILES_TO_HOSTS \"$VISIT_INSTALL_NETWORK\" TYPE STRING)" >> $HOSTCONF
    fi

    if [[ "${DO_JAVA}" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VisIt Java Option." >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_JAVA ON TYPE BOOL)" >> $HOSTCONF
    fi

    if [[ "$BUILD_VISIT_BGQ" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## BG/Q-specific settings" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SET(CMAKE_CROSSCOMPILING    ON)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_USE_X            OFF TYPE BOOL)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_USE_GLEW         OFF TYPE BOOL)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_DISABLE_SELECT   ON  TYPE BOOL)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_USE_NOSPIN_BCAST OFF TYPE BOOL)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OPENGL_DIR       \${VISITHOME}/mesa/$MESA_VERSION/\${VISITARCH})" >> $HOSTCONF
        echo "ADD_DEFINITIONS(-DVISIT_BLUE_GENE_Q)" >> $HOSTCONF
        echo >> $HOSTCONF
    fi

    if [[ "$parallel" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Parallel Build Setup." >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)" >> $HOSTCONF
        # we either set an mpi wrapper compiler in the host conf
        if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
            if [[ "$BUILD_VISIT_BGQ" == "yes" ]] ; then
                echo "## (inserted by build_visit for BG/Q. Configuration as of 10/8/2014.)" >> $HOSTCONF
                echo "## (LC rolled back this ppcfloor configuration from V1R2M2 to V1R2M0 10/16/2014.)" >> $HOSTCONF
                echo "#SET(BLUEGENEQ /bgsys/drivers/ppcfloor)" >> $HOSTCONF
                echo "#VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)" >> $HOSTCONF
                echo "#VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS \"-I\${BLUEGENEQ} -I\${BLUEGENEQ}/comm/include -I\${BLUEGENEQ}/spi/include -I\${BLUEGENEQ}/spi/include/kernel/cnk\" TYPE STRING)" >> $HOSTCONF
                echo "#VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   \"-I\${BLUEGENEQ} -I\${BLUEGENEQ}/comm/include -I\${BLUEGENEQ}/spi/include -I\${BLUEGENEQ}/spi/include/kernel/cnk\" TYPE STRING)" >> $HOSTCONF
                echo "#VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  \"-L\${BLUEGENEQ}/spi/lib -L\${BLUEGENEQ}/comm/lib -R/opt/ibmcmp/lib64/bg/bglib64\" TYPE STRING)" >> $HOSTCONF
                echo "#VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich-xl opa-xl mpl-xl pami-gcc SPI SPI_cnk rt pthread stdc++ pthread TYPE STRING)" >> $HOSTCONF
                echo "" >> $HOSTCONF
                echo "## (inserted by build_visit for BG/Q. Configuration as of 10/15/2014.)" >> $HOSTCONF
                echo "SET(BLUEGENEQ /bgsys/drivers/V1R2M0/ppc64)" >> $HOSTCONF
                echo "VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS \"-I\${BLUEGENEQ} -I\${BLUEGENEQ}/comm/sys/include -I\${BLUEGENEQ}/spi/include -I\${BLUEGENEQ}/spi/include/kernel/cnk -I\${BLUEGENEQ}/comm/xl/include\" TYPE STRING)" >> $HOSTCONF
                echo "VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   \"-I\${BLUEGENEQ} -I\${BLUEGENEQ}/comm/sys/include -I\${BLUEGENEQ}/spi/include -I\${BLUEGENEQ}/spi/include/kernel/cnk -I\${BLUEGENEQ}/comm/xl/include\" TYPE STRING)" >> $HOSTCONF
                echo "VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  \"-L\${BLUEGENEQ}/spi/lib -L\${BLUEGENEQ}/comm/sys/lib -L\${BLUEGENEQ}/spi/lib -L\${BLUEGENEQ}/comm/xl/lib -R/opt/ibmcmp/lib64/bg/bglib64\" TYPE STRING)" >> $HOSTCONF
                echo "VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich opa mpl pami SPI SPI_cnk rt pthread stdc++ pthread TYPE STRING)" >> $HOSTCONF
            else
                echo "## (configured w/ mpi compiler wrapper)" >> $HOSTCONF
                echo "VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER $VISIT_MPI_COMPILER TYPE FILEPATH)"  >> $HOSTCONF
            fi
        else
            # or we just set the flags.
            echo "## (configured w/ user provided CXX (PAR_INCLUDE) & LDFLAGS (PAR_LIBS) flags)" \
             >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   \"$PAR_INCLUDE\" TYPE STRING)"     >> $HOSTCONF
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
        if [[ "$CRAY_MPICH_DIR" != "" ]] ; then
            echo "# Force static executables on Cray to be 100% statically linked." >> $HOSTCONF
            echo "SET(VISIT_EXE_LINKER_FLAGS \"-static -static-libgcc -static-libstdc++ -pthread -Wl,-Bstatic\")" >> $HOSTCONF
        fi
    fi
    if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]]; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Server components only" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_SERVER_COMPONENTS_ONLY ON TYPE BOOL)" >> $HOSTCONF
    fi
    if [[ "$DO_ENGINE_ONLY" == "yes" ]]; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Engine components only" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_ENGINE_ONLY ON TYPE BOOL)" >> $HOSTCONF
    fi

    if [[ "$DO_STATIC_BUILD" == "yes"  && $DO_OSMESA == "yes" ]] ; then
        if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
            # Turn off VisIt's use of X
            echo "VISIT_OPTION_DEFAULT(VISIT_USE_X OFF TYPE BOOL)" >> $HOSTCONF
        fi
    fi
    # Are we on Cray? We might need the socket relay.
    if [[ "$CRAY_MPICH_DIR" != "" ]] ; then
        echo "VISIT_OPTION_DEFAULT(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE ON)" >> $HOSTCONF
    fi

    if [[ "$DO_XDB" == "yes" ]]; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## XDB" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_ENABLE_XDB ON TYPE BOOL)" >> $HOSTCONF
    fi

    echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## VisIt Thread Option" >> $HOSTCONF
    echo "##" >> $HOSTCONF
    if [[ "$DO_THREAD_BUILD" == "yes" ]] ; then
        echo "VISIT_OPTION_DEFAULT(VISIT_THREAD ON TYPE BOOL)" >> $HOSTCONF
    else
        echo "VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)" >> $HOSTCONF
    fi

    if [[ "${DO_PARADIS}" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VisIt paraDIS Option." >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PARADIS ON TYPE BOOL)" >> $HOSTCONF
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
     hostconf_library ${reqlibs[$bv_i]}
 done

 for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
 do
     hostconf_library ${optlibs[$bv_i]}
 done
 echo >> $HOSTCONF

 #
 # Patch for Ubuntu 11.04
 #
 #if test -d "/usr/lib/x86_64-linux-gnu" ; then
 #    numLibs=$(ls -1 /usr/lib/x86_64-linux-gnu | wc -l)
 #    if (( $numLibs > 10 )) ; then
 #       rm -f $HOSTCONF.tmp
 #       cat $HOSTCONF | sed "s/\/usr\/lib/\/usr\/lib\/x86_64-linux-gnu/" > $HOSTCONF.tmp
 #       rm $HOSTCONF
 #       mv $HOSTCONF.tmp $HOSTCONF
 #    fi
 #fi

 cd "$START_DIR"
 echo "Done creating $HOSTCONF"
 return 0
}

# *************************************************************************** #
#
# Modifications:
#   Eric Brugger, Fri Feb  1 14:56:58 PST 2019
#   I modified it to work post git transition.
#
# *************************************************************************** #

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
    printf "%s%s\n" "FC_COMPILER=" "${FC_COMPILER}"
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
    initialize_build_visit

    printf "Usage: %s [options]\n" $0
    printf "%-15s %s [%s]\n" "--skip-opengl-context-check" "Skip check for minimum OpenGL context." "false"

    printf "\n"
    printf "BUILD OPTIONS\n"
    printf "\n"

    printf "%-20s %s [%s]\n" "--build-mode" "VisIt build mode (Debug or Release)" "$VISIT_BUILD_MODE"
    printf "%-20s %s [%s]\n" "--create-rpm" "Enable creation of RPM packages" "$CREATE_RPM"
    printf "%-20s %s [%s]\n" "--cflag"   "Append a flag to CFLAGS" "${CFLAGS}"
    printf "%-20s %s [%s]\n" "--cxxflag" "Append a flag to CXXFLAGS" "$CXXFLAGS"
    printf "%-20s %s [%s]\n" "--cflags"  "Explicitly set CFLAGS" "$CFLAGS"
    printf "%-20s %s [%s]\n" "--cxxflags" "Explicitly set CXXFLAGS" "$CXXFLAGS"
    printf "%-20s %s [%s]\n" "--cc"  "Explicitly set C_COMPILER" "$C_COMPILER"
    printf "%-20s %s [%s]\n" "--cxx" "Explicitly set CXX_COMPILER" "$CXX_COMPILER"
    printf "%-20s %s [%s]\n" "--debug" "Add '-g' to C[XX]FLAGS" "no"
    printf "%s <%s>  %s [%s]\n" "--makeflags" "flags" "Flags to 'make'" "$MAKE_OPT_FLAGS"
    printf "%-20s %s [%s]\n" "--fortran" "Enable compilation of Fortran sources" "no"
    printf "%-20s %s\n"      "--fc" "Explicitly set FC_COMPILER"
    printf "%-20s [%s]\n"    ""     "$FC_COMPILER"
    printf "%-20s %s [%s]\n" "--no-qt-silent" "Disable make silent operation for QT." "no"
    printf "%-20s %s [%s]\n" "--parallel" "Enable parallel build, display MPI prompt" "$parallel"
    printf "%-20s %s [%s]\n" "--static" "Build using static linking" "$DO_STATIC_BUILD"
    printf "%-20s <%s> %s\n" "--installation-build-dir" "path"
    printf "%-20s %s [%s]\n" "" "Specify the directory visit will use for building" "$VISIT_INSTALLATION_BUILD_DIR"

    printf "\n"
    printf "INSTALLATION OPTIONS\n"
    printf "\n"

    printf "%s <%s> %s [%s]\n" "--arch" "architecture" "Set architecture" "$VISITARCHTMP"
    printf "\t  %s\n" "   This variable is used in constructing the 3rd party"
    printf "\t  %s\n" "   library path; usually set to something like"
    printf "\t  %s\n" "   'linux_gcc-3.4.6' or 'Darwin_gcc-4.0.1'"
    printf "%-11s  %s [%s]\n" "--group" "Group name of installed libraries" "$GROUP"
    printf "%-11s <%s> \n%s [%s]\n" "--thirdparty-path" "/path/to/directory" \
           "             Specify the root directory name under which the 3rd party
             libraries have been installed.  If defined, it would typically
             mean the 3rd party libraries are pre-built and are installed
             somewhere like /usr/gapps/visit." "${THIRD_PARTY_PATH}"

    printf "\n"
    printf "GROUPING\n"
    printf "\n"

    for (( bv_i=0; bv_i<${#grouplibs_name[*]}; ++bv_i ))
    do
        name=${grouplibs_name[$bv_i]}
        comment=${grouplibs_comment[$bv_i]}
        enabled=${grouplibs_enabled[$bv_i]}
        printf "%-15s %s [%s]\n" "--$name" "$comment" "$enabled"
    done
    printf "\n"

    printf "\n"
    printf "VISIT-SPECIFIC OPTIONS\n"
    printf "\n"
    printf "%-20s %s [%s]\n" "--install-network" "Install specific network config files." "${VISIT_INSTALL_NETWORK}"
    printf "%s <%s>    %s [%s]\n" "--prefix" "prefix" "The directory to which VisIt should be installed once it is built" "$VISIT_INSTALL_PREFIX"
    printf "%s <%s>     %s [%s]\n" "--tarball" "file" "tarball to extract VisIt from" "$VISIT_FILE"
    printf "%s <%s>  %s [%s]\n" "--version" "version" "The VisIt version to build" "$VISIT_VERSION"
    printf "%-20s %s [%s]\n" "--no-hostconf" "Do not create host.conf file." "$DO_HOSTCONF"
    printf "%-20s %s [%s]\n" "--java" "Build with the Java client library" "${DO_JAVA}"
    printf "%-20s %s [%s]\n" "--paradis" "Build with the paraDIS client library" "$DO_PARADIS"
    printf "%-20s %s [%s]\n" "--xdb" "Enable FieldView XDB plugin." "$DO_XDB"
    bv_visit_initialize
    bv_visit_print_usage

    printf "\n"
    printf "THIRD-PARTY LIBRARIES\n"
    printf "  A download attempt will be made for all files which do not exist.\n"
    printf "\n"
    printf "  REQUIRED -- These are built by default unless --no-thirdparty flag is used.\n"
    printf "\n"

    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${reqlibs[$bv_i]}_initialize"
        $initializeFunc
        printUsageFunc="bv_${reqlibs[$bv_i]}_print_usage"
        $printUsageFunc
    done

    printf "\n"
    printf "  OPTIONAL\n"
    printf "\n"

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${optlibs[$bv_i]}_initialize"
        $initializeFunc
        printUsageFunc="bv_${optlibs[$bv_i]}_print_usage"
        $printUsageFunc
    done

    printf "\n"
    printf "GIT OPTIONS\n"
    printf "\n"

    printf "%-26s %s\n"      "--git" "Obtain the VisIt source code"
    printf "%-26s %s [%s]\n" "" "from the GIT server" "$DO_GIT"

    printf "\n"
    printf "MISC OPTIONS\n"
    printf "\n"

    printf "%-20s %s [%s]\n" "--bv-debug"   "Enable debugging for this script" "no"
    printf "%-20s %s [%s]\n" "--dry-run"  "Dry run of the presented options" "no"
    printf "%-20s %s [%s]\n" "--download-only" "Only download the specified packages" "no"
    printf "%-20s %s [%s]\n" "--engine-only" "Only build the compute engine." "$DO_ENGINE_ONLY"
    printf "%-20s %s [%s]\n" "-h, --help" "Display this help message." "no"
    printf "%-20s %s [%s]\n" "--print-vars" "Display user settable environment variables" "no"
    printf "%-20s %s\n" "--server-components-only" ""
    printf "%-20s %s\n" "" "Only build VisIt's server components"
    printf "%-20s %s [%s]\n" "" "(mdserver,vcl,engine)." "$DO_SERVER_COMPONENTS_ONLY"
    printf "%-20s %s [%s]\n" "--stdout" "Write build log to stdout" "no"
    printf "%-20s <%s>\n" "--write-unified-file"  "filename"
    printf "%-20s %s [%s]\n" ""  "Write single unified build_visit file using the provided filename" "$WRITE_UNIFIED_FILE"
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

        #check if we have completely mangled the library before..
        if [[ -e "$mangled_dir/done_mangling_library" ]]; then
            info "library was mangled earlier, skipping (please exit if this is not true)"
            return 0
        fi
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
    touch "$mangled_dir"/done_mangling_library
    return 0
}
