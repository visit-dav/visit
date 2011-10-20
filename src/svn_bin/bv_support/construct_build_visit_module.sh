#!/bin/bash

# *****************************************************************************
#   Script: construct_build_visit_module
#
#   Purpose:
#        Constructs a new build visit module with default parameters filled out..
#

output=$1
uppercase_out=`echo $1 | tr '[a-z]' '[A-Z]'`

if [[ $output == "" ]]; then
   echo "Please enter module name /path/to/visit/construct_build_visit_module module_name"
   exit 0
fi


filename="bv_${output}.sh"

echo "# Module automatically read in from construct_build_visit" > $filename
echo "# Insert header and comments" >> $filename

echo "

#initialize all the variables
function bv_${output}_initialize
{
    export DO_${uppercase_out}=\"no\"
    export ON_${uppercase_out}=\"off\"
}

#enable the module for install
function bv_${output}_enable
{ 
    DO_${uppercase_out}=\"yes\"
    ON_${uppercase_out}=\"on\"
}

#disable the module for install
function bv_${output}_disable
{
    DO_${uppercase_out}=\"no\"
    ON_${uppercase_out}=\"off\"
}

#add any dependency with comma separation
function bv_${output}_depends_on
{
    return \"\"
}

#add information about how to get library..
function bv_${output}_info
{
    export ${uppercase_out}_FILE=\${${uppercase_out}_FILE:-\"enter-file-name\"}
    export ${uppercase_out}_VERSION=\${${uppercase_out}_VERSION:-\"1\"}
    export ${uppercase_out}_COMPATIBILITY_VERSION=\${${uppercase_out}_COMPATIBILITY_VERSION:-\"1\"}
    export ${uppercase_out}_BUILD_DIR=\${${uppercase_out}_BUILD_DIR:-\"${output}\"}
}

#print variables used by this module
function bv_${output}_print
{
    printf \"%s%s\n\" \"${uppercase_out}_FILE=\" \"\${${uppercase_out}_FILE}\"
    printf \"%s%s\n\" \"${uppercase_out}_VERSION=\" \"\${${uppercase_out}_VERSION}\"
    printf \"%s%s\n\" \"${uppercase_out}_COMPATIBILITY_VERSION=\" \"\${${uppercase_out}_COMPATIBILITY_VERSION}\"
    printf \"%s%s\n\" \"${uppercase_out}_BUILD_DIR=\" \"\${${uppercase_out}_BUILD_DIR}\"
}

#print how to install and uninstall module..
function bv_${output}_print_usage
{
    printf \"%-15s %s [%s]\n\" \"--${output}\"   \"Build ${uppercase_out}\" \"\$DO_${uppercase_out}\"
    printf \"%-15s %s [%s]\n\" \"--no-${output}\"   \"Do not Build ${uppercase_out}\" \"\$DO_${uppercase_out}\"
}

#how to show information for graphical editor
function bv_${output}_graphical
{
    local graphical_out=\"${uppercase_out}    \$${uppercase_out}_VERSION(\$${uppercase_out}_FILE)    \$ON_${uppercase_out}\"
    echo \$graphical_out
}

#values to add to host profile, write to \$HOSTCONF
function bv_${output}_host_profile
{
    #Add code to write values to variable \$HOSTCONF
    if [[ \"\$DO_${uppercase_out}\" == \"yes\" ]] ; then
        echo >> \$HOSTCONF
        echo \"##\" >> \$HOSTCONF
        echo \"## ${uppercase_out}\" >> \$HOSTCONF
    fi
}

#prepare the module and check whether it is built or is ready to be built.
function bv_${output}_ensure_built_or_ready
{
    if [[ \"$DO_${uppercase_out}\" == \"yes\" ]] ; then
        ensure_built_or_ready \"${uppercase_out}\" \$${uppercase_out}_VERSION \$${uppercase_out}_BUILD_DIR \$${uppercase_out}_FILE
        if [[ \$? != 0 ]] ; then
            ANY_ERRORS=\"yes\"
            DO_${uppercase_out}=\"no\"
            error \"Unable to build ${uppercase_out}.  \${${uppercase_out}_FILE} not found.\"
        fi
    fi
}


#print what the module will do for building
function bv_${output}_print_build_command
{
   #print the build command..
   echo \"${output} has no build commands set\"
}

function bv_${output}
{
    echo \"Build the module\"
}

#the build command..
function bv_${output}_build
{

    if [[ \"$DO_${uppercase_out}\" == \"yes\" ]] ; then
        check_if_installed \"${uppercase_out}\" ${uppercase_out}_VERSION
        if [[ \$? == 0 ]] ; then
            info \"Skipping ${uppercase_out} build.  ${uppercase_out} is already installed.\"
        else
            info \"Building ${uppercase_out} (XX minutes)\"

            #Build the Module 
            #(Please enter custom configure and install instructions here)
            build_${output}
            if [[ \$? != 0 ]] ; then
                error \"Unable to build or install ${uppercase_out}.  Bailing out.\"
            fi
            info \"Done building ${uppercase_out}\"
       fi
    fi
}" >> $filename

