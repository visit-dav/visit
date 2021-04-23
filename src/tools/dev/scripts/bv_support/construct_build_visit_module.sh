#!/bin/bash

# *****************************************************************************
#   Script: construct_build_visit_module
#
#   Purpose:
#        Constructs a new build visit module with default parameters filled out..
#
#   Modifications:
#     Paul Selby, Wed  4 Feb 17:30:32 GMT 2015
#     Fixed name of _ensure function and check_if_installed call in _build

output=$1
uppercase_out=`echo $1 | tr '[a-z]' '[A-Z]'`

if [[ $output == "" ]]; then
    echo "Please enter module name /path/to/visit/construct_build_visit_module module_name"
    exit 0
fi


filename="bv_${output}.sh"

echo "# Module automatically read in from construct_build_visit_module" > $filename
echo "# Insert header and comments" >> $filename

echo "
function bv_${output}_initialize
{
    export DO_${uppercase_out}=\"no\"
    export USE_SYSTEM_${uppercase_out}=\"no\"
    add_extra_commandline_args \"${output}\" \"system-${output}\" 0 \"Using system ${output}\"
    add_extra_commandline_args \"${output}\" \"alt-${output}-dir\" 1 \"Use alternate ${output}\"
}

function bv_${output}_enable
{
    DO_${uppercase_out}=\"yes\"
}

function bv_${output}_disable
{
    DO_${uppercase_out}=\"no\"
}

function bv_${output}_system_${output}
{
#    TEST=\`which ${output}-config\`
#    [ \$? != 0 ] && error \"System ${output}-config not found, cannot configure ${output}\"

    bv_${output}_enable
    USE_SYSTEM_${uppercase_out}=\"yes\"
    ${uppercase_out}_INSTALL_DIR=\"\$1\"
    info \"Using System ${output}: \${${uppercase_out}_INSTALL_DIR}\"
}

function bv_${output}_alt_${output}_dir
{
    bv_${output}_enable
    USE_SYSTEM_${uppercase_out}=\"yes\"
    ${uppercase_out}_INSTALL_DIR=\"\$1\"
    info \"Using Alternate ${output}: \${${uppercase_out}_INSTALL_DIR}\"
}

function bv_${output}_depends_on
{
    local depends_on=\"\"

#    if [[ \"\${DO_XYZ}\" == \"yes\" ]]; then
#        depends_on=\"\${depends_on} xyz\"
#    fi

    echo \${depends_on}
}

function bv_${output}_initialize_vars
{
    if [[ \"\${USE_SYSTEM_${uppercase_out}}\" == \"no\" ]]; then
        ${uppercase_out}_INSTALL_DIR=\"\${VISITDIR}/${output}/\${${uppercase_out}_VERSION}/\${VISITARCH}\"
    fi
}

function bv_${output}_info
{
    export ${uppercase_out}_VERSION=\${${uppercase_out}_VERSION:-\"1.0.0\"}
    export ${uppercase_out}_FILE=\${${uppercase_out}_FILE:-\"${output}-\${${uppercase_out}_VERSION}.tar.gz\"}
    export ${uppercase_out}_COMPATIBILITY_VERSION=\"1.0\"}
    export ${uppercase_out}_URL=\${${uppercase_out}_URL:-\"http://www.${output}.org\"}
    export ${uppercase_out}_SRC_DIR=\${${uppercase_out}_SRC_DIR:-\"\${${uppercase_out}_FILE%.tar*}\"}
    export ${uppercase_out}_BUILD_DIR=\${${uppercase_out}_BUILD_DIR:-\"\${${uppercase_out}_SRC_DIR}-build\"}
#    export ${uppercase_out}_MD5_CHECKSUM=\"\"
#    export ${uppercase_out}_SHA256_CHECKSUM=\"\"
}

function bv_${output}_print
{
    printf \"%s%s\n\" \"${uppercase_out}_FILE=\" \"\${${uppercase_out}_FILE}\"
    printf \"%s%s\n\" \"${uppercase_out}_VERSION=\" \"\${${uppercase_out}_VERSION}\"
    printf \"%s%s\n\" \"${uppercase_out}_COMPATIBILITY_VERSION=\" \"\${${uppercase_out}_COMPATIBILITY_VERSION}\"
    printf \"%s%s\n\" \"${uppercase_out}_SRC_DIR=\" \"\${${uppercase_out}_SRC_DIR}\"
    printf \"%s%s\n\" \"${uppercase_out}_BUILD_DIR=\" \"\${${uppercase_out}_BUILD_DIR}\"
}

function bv_${output}_print_usage
{
    printf \"%-20s %s [%s]\n\" \"--${output}\" \"Build ${output} support\" \"\${DO_${uppercase_out}}\"
    printf \"%-20s %s [%s]\n\" \"--no-${output}\" \"Prevent ${output} from being built\"
    printf \"%-20s %s [%s]\n\" \"--system-${output}\" \"Use the system installed ${output}\"
    printf \"%-20s %s [%s]\n\" \"--alt-${output}-dir\" \"Use ${output} from an alternative directory\"
}

function bv_${output}_host_profile
{
    if [[ \"\${DO_${uppercase_out}}\" == \"yes\" ]]; then
        echo >> \${HOSTCONF}
        echo \"##\" >> \${HOSTCONF}
        echo \"## ${output}\" >> \${HOSTCONF}
        echo \"##\" >> \${HOSTCONF}

        if [[ \"\${USE_SYSTEM_${uppercase_out}}\" == \"yes\" ]]; then
            echo \"VISIT_OPTION_DEFAULT(VISIT_${uppercase_out}_DIR \${${uppercase_out}_INSTALL_DIR})\" >> \${HOSTCONF}
        else
            echo \"SETUP_APP_VERSION(${output} \${${uppercase_out}_VERSION})\" >> \${HOSTCONF}
            echo \"VISIT_OPTION_DEFAULT(VISIT_${uppercase_out}_DIR \\\${VISITHOME}/${output}/\\\${${uppercase_out}_VERSION}/\\\${VISITARCH})\" >> \${HOSTCONF}
        fi
    fi
}

function bv_${output}_ensure
{
    if [[ \"\${DO_${uppercase_out}}\" == \"yes\" && \"\${USE_SYSTEM_${uppercase_out}}\" == \"no\" ]] ; then
        check_installed_or_have_src \"${output}\" \${${uppercase_out}_VERSION} \${${uppercase_out}_BUILD_DIR} \${${uppercase_out}_FILE} \${${uppercase_out}_URL}
        if [[ \$? != 0 ]] ; then
            ANY_ERRORS=\"yes\"
            DO_${uppercase_out}=\"no\"
            error \"Unable to build ${output}. \${${uppercase_out}_FILE} not found.\"
        fi
    fi
}

function bv_${output}_dry_run
{
    if [[ \"\${DO_${uppercase_out}}\" == \"yes\" ]] ; then
        echo \"Dry run option not set for ${output}\"
    fi
}

function apply_${output}_patch
{
    cd \"\${${uppercase_out}_SRC_DIR}\" || error \"Can't cd to ${output} source dir.\"

#    info \"Patching ${output} . . .\"

#    apply_xyz_patch
#    if [[ \$? != 0 ]] ; then
#        return 1
#    fi

    cd \"\${START_DIR}\"

    return 0
}

function bv_${output}_is_enabled
{
    if [[ \"\${DO_${uppercase_out}}\" == \"yes\" ]]; then
        return 1
    fi
    return 0
}

function bv_${output}_is_installed
{
    if [[ \"\${USE_SYSTEM_${uppercase_out}}\" == \"yes\" ]]; then
        return 1
    fi

    check_if_installed \"${output}\" \${${uppercase_out}_VERSION}
    if [[ \$? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_${output}_build
{
    cd \"\${START_DIR}\"

    if [[ \"\${DO_${uppercase_out}}\" == \"yes\" && \"\${USE_SYSTEM_${uppercase_out}}\" == \"no\" ]] ; then
        check_if_installed \"${output}\" \${${uppercase_out}_VERSION}
        if [[ \$? == 0 ]] ; then
            info \"Skipping ${output} build. ${output} is already installed.\"
        else
            info \"Building ${output} (~20 minutes)\"
            build_${output}
            if [[ \$? != 0 ]] ; then
                error \"Unable to build or install ${output}. Bailing out.\"
            fi
            info \"Done building ${output}\"
        fi
    fi
}

function build_${output}
{
    #
    # Uncompress the source file
    #
    uncompress_src_file \${${uppercase_out}_SRC_DIR} \${${uppercase_out}_FILE}
    untarred_${output}=\$?
    if [[ \${untarred_${output}} == -1 ]] ; then
        warn \"Unable to uncompress ${output} source file. Giving Up!\"
        return 1
    fi

    #
    # Apply patches
    #
    apply_${output}_patch
    if [[ \$? != 0 ]] ; then
        if [[ \${untarred_${output}} == 1 ]] ; then
            warn \"Giving up on ${output} build because the patch failed.\"
            return 1
        else
            warn \"Patch failed, but continuing. It is believed that this\n\" \\
                 \"script tried to apply a patch to an existing directory\n\" \\
                 \"that had already been patched. That is, the patch is\n\" \\
                 \"failing harmlessly on a second application.\"
        fi
    fi

    #
    # Make a build directory for an out-of-source build.
    #
    cd \"\${START_DIR}\"
    if [[ ! -d \${${uppercase_out}_BUILD_DIR} ]] ; then
        echo \"Making build directory \${${uppercase_out}_BUILD_DIR}\"
        mkdir \${${uppercase_out}_BUILD_DIR}
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf \${${uppercase_out}_BUILD_DIR}/CMakeCache.txt \${${uppercase_out}_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd \"\${${uppercase_out}_BUILD_DIR}\"

    #
    # Configure ${output}
    #
    info \"Configuring ${output} . . .\"

    ${output}_opts=\"\"

    #
    # Several platforms have had problems with the cmake configure command
    # issued simply via \"issue_command\". This issue was first discovered
    # on BGQ and then occurred randomly for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - write a simple
    # script that is invoked with bash which calls cmake with all of the
    # properly arguments. This strategy is being used for all platforms.
    #
    CMAKE_BIN=\"\${CMAKE_INSTALL}/cmake\"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo \"\\\"\${CMAKE_BIN}\\\"\" \${${output}_opts} ../\${${uppercase_out}_SRC_DIR}/src > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ \$? != 0 ]] ; then
        warn \"${output} configure failed. Giving up\"
        return 1
    fi

    #
    # Now build ${output}.
    #
    info \"Building ${output} . . .\"
    \$MAKE \$MAKE_OPT_FLAGS
    if [[ \$? != 0 ]] ; then
        warn \"${output} build failed. Giving up\"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info \"Installing ${output} . . .\"
    \$MAKE install
    if [[ \$? != 0 ]] ; then
        warn \"${output} install failed. Giving up\"
        return 1
    fi

    if [[ \"\${DO_GROUP}\" == \"yes\" ]] ; then
        chmod -R ug+w,a+rX \"\${VISITDIR}/${output}\"
        chgrp -R \${GROUP} \"\${VISITDIR}/${output}\"
    fi

    cd \"\$START_DIR\"
    info \"Done with ${output}\"
    return 0
}" >> $filename
