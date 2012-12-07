#!/bin/bash

declare -a xmlp_filecontents
declare -a xmlp_licenses
declare -a xmlp_licenses_range
declare -a xmlp_alllibs
declare -a xmlp_tmp_array
declare -a xmlp_reqlibs
declare -a xmlp_optlibs
declare -a xmlp_grouplibs_name
declare -a xmlp_grouplibs_deps
declare -a xmlp_grouplibs_comment

function xmlp_removeSingleLineComment
{
    if [[ "$1" == *\<\!--*--\>* ]]; then
        echo "${1//\<\!--*--\>}"
    else
        echo "$1"
    fi
}

function xmlp_isCommentStart
{
    if [[ $1 == *\<\!--* ]]; then
        return 1
    fi
    return 0
}

function xmlp_isCommentEnd
{
    if [[ $1 == *--\>* ]]; then 
        return 1
    fi
    return 0
}

function readXmlModuleFile
{
    local filename=$1
    local i=0
    local isComment=0
    local range_index=0
    local addlibs=""

    if [[ ! -e $filename ]]; then 
        echo "File $filename does not exist"
        return 0
    fi

    while read line
    do
        line=$(xmlp_removeSingleLineComment "$line")
        xmlp_isCommentStart "$line"
        
        if [[ $? == 1 ]]; then
            isComment=1
        fi

        #remove comments and empty lines..
        if [[ $isComment == 0 && ! -z "$line" ]]; then
            #record which license the files go to..
            if [[ $line == *\<license* ]]; then 
                tmp=`echo $line | sed -e s/^.*=\"// -e s/\".*$//`
                range_index=${#xmlp_licenses[*]}
                xmlp_licenses[$range_index]="$tmp" 
                xmlp_licenses_range[$range_index]="$i"
            fi

            if [[ $line == *\</license\>* ]]; then 
                xmlp_licenses_range[$range_index]="${xmlp_licenses_range[$range_index]} $i"
            fi

            if [[ $line == *\<lib* ]]; then
                addlib=`echo ${line/no-} | sed -e s/^.*=\"// -e s/\".*$//`
                addlibs="${addlibs} $addlib"
            fi
            #trim white space
            xmlp_filecontents[$i]=`echo $line`
            let i++
        fi 

        xmlp_isCommentEnd "$line"

        if [[ $? == 1 ]]; then
            isComment=0
        fi
    done < $filename

    if [[ ${#xmlp_licenses[*]} == 0 ]]; then
        echo "No valid licenses found"
        return 0
    fi

    #get list of all libs, sort and get unique set..
    uniq_libs=`echo $addlibs | sed -e s/\ /\\\\n/g | sort | uniq`
    for lib in $uniq_libs;
    do
        xmlp_alllibs[${#xmlp_alllibs[*]}]="$lib"
    done

    return 1
}

function parseXmlModules
{
    local startReading=0
    local x=0
    local startPattern=$1
    local endPattern=$2
    local lstart=$3
    local lend=$4

    xmlp_tmp_array=()
    #find required tag and parse all its parameters..
    for (( i=$lstart; i < $lend; ++i ))
    do
        if [[ ${xmlp_filecontents[$i]} == *$endPattern* ]]; then
            startReading=0
        fi

        if [[ $startReading == 1 ]]; then
            #remove everything to first string..
            local tmp="${xmlp_filecontents[$i]}"
            tmp=`echo $tmp | sed -e s/^.*=\"// -e s/\".*$//`
            xmlp_tmp_array[$x]="$tmp"
            let x++
        fi

        if [[ ${xmlp_filecontents[$i]} == *$startPattern* ]]; then
            startReading=1
        fi
    done
}

function parseXmlGroupModules
{
    local startReading=0
    local startPattern="<group"
    local endPattern="</group>"
    local lstart=$1
    local lend=$2
    local title=""
    local deps=""
    local comment=""
    #find required tag and parse all its parameters..
    for (( i=$lstart; i < $lend; ++i ))
    do
        if [[ ${xmlp_filecontents[$i]} == *$endPattern* ]]; then
            startReading=0
            #remove whitespace with echo
            #echo "Title: $title Comment: $comment Deps $deps"
            xmlp_grouplibs_name[${#xmlp_grouplibs_name[*]}]=`echo $title`
            xmlp_grouplibs_deps[${#xmlp_grouplibs_deps[*]}]=`echo $deps`
            xmlp_grouplibs_comment[${#xmlp_grouplibs_comment[*]}]=`echo $comment`
            title=""
            deps=""
        fi

        if [[ $startReading == 1 ]]; then
            #remove everything to first string..
            local tmp="${xmlp_filecontents[$i]}"
            tmp=`echo $tmp | sed -e s/^.*=\"// -e s/\".*$//`
            deps="$deps $tmp"
        fi

        if [[ ${xmlp_filecontents[$i]} == *$startPattern* ]]; then
            startReading=1
            local tmp="${xmlp_filecontents[$i]}"
            #has comment
            if [[ "$tmp" == *comment* ]]; then
                tmp_comment=${tmp/*comment}
                tmp_comment=`echo $tmp | sed -e s/^.*=\"// -e s/\".*$//`
                comment="$tmp_comment"
                tmp=${tmp/comment*}
                tmp=`echo $tmp | sed -e s/^.*=\"// -e s/\".*$//`
                title="$tmp"
            else
                tmp=`echo $tmp | sed -e s/^.*=\"// -e s/\".*$//`
                title="$tmp"
                comment=""
            fi
        fi
    done
}


#check if input argument is actually a license
function xmlp_licenseMatch
{
    local license=${1/--}
    local bv_i=0

    for (( bv_i=0; bv_i < ${#xmlp_licenses[*]}; ++bv_i ))
    do
        local xmlp_lic=${xmlp_licenses[$bv_i]//\|/ }
        for lic in `echo $xmlp_lic`;
        do
            if [[ "$lic" == "$license" ]]; then
                return 1
            fi
        done
    done 
    return 0
}

#loop through argument list and extract license
function xmlp_get_license
{
    local defaultLicense="" 
    for arg in "$@" ; do
        #potential input license..
        xmlp_licenseMatch "${arg/--}"
        if [[ $? == 1 ]]; then
            defaultLicense="$defaultLicense ${arg/--}"
        fi
    done

    if [[ "$defaultLicense" == "" ]]; then
        defaultLicense="${xmlp_licenses[0]/\|*}"
    fi

    echo "$defaultLicense"
}

function parseXmlModuleContents
{
    local license=$1
    local lstart=-1
    local lend=-1
    local i=0

    xmlp_reqlibs=()
    xmlp_optlibs=()
    xmlp_grouplibs_name=()
    xmlp_grouplibs_deps=()
    xmlp_grouplibs_comment=()

    for (( i=0; i < ${#xmlp_licenses[*]}; ++i ))
    do
        local range=${xmlp_licenses_range[$i]}
        local xmlp_lic=${xmlp_licenses[$i]//\|/ }
        for lic in `echo $xmlp_lic`;
        do
            if [[ "$lic" == "$license" ]]; then
                lstart=${range/ *}
                lend=${range/* }
            fi
        done
    done

    if [[ $lstart == -1 || $lend == -1 ]]; then
        echo "License $license not found"
        return 0
    fi

    len=${#xmlp_filecontents[*]}
    if [[ $len -lt 2 ]]; then 
        echo "Incomplete Module file"
        return 0
    fi

    if [[   ${xmlp_filecontents[0]} != *\<modules* || 
            ${xmlp_filecontents[${#xmlp_filecontents[*]}-1]} != *\</modules\>* ]]; then
        echo "Module file not formatted properly must start and end with <module> </module> tag"
        return 0
    fi

    #echo "parsing license $license, start=$lstart, end=$lend"

    #parse required
    parseXmlModules "<required>" "</required>" $lstart $lend
    xmlp_reqlibs=( "${xmlp_tmp_array[@]}" )

    #parse optional
    parseXmlModules "<optional>" "</optional>" $lstart $lend
    xmlp_optlibs=( "${xmlp_tmp_array[@]}" )

    #parse any groups
    parseXmlGroupModules $lstart $lend

    if [[   ${#xmlp_reqlibs[*]} == 0 || 
            ${#xmlp_optlibs[*]} == 0 ]]; then
        echo "Required and Optional Modules not present in module files"
        return 0
    fi

    #for (( i = 0; i < ${#xmlp_reqlibs[*]}; ++i ))
    #do
    #    echo "required: ${xmlp_reqlibs[$i]}"
    #done

    #for (( i = 0; i < ${#xml_optlibs[*]}; ++i ))
    #do
    #    echo "optional: ${xmlp_optlibs[$i]}"
    #done

    #for (( i = 0; i < ${#xmlp_grouplibs_name[*]}; ++i ))
    #do
    #    echo "group names: ${xmlp_grouplibs_name[$i]}"
    #    echo "group deps: ${xmlp_grouplibs_deps[$i]}"
    #done
    return 1
}

#readXmlModuleFile "modules.xml"
##parseXmlModuleContents "lgpl"
