#!/bin/sh -x

if [[ -n "$(uname | grep -i darwin)" ]]; then
    MDCMD=md5
else
    MDCMD=md5sum
fi

basestr=""
parallel=4
xargsdbg=""
dirs=""

# Needed in sub-routines called by xargs
export debug=0
export dontrm=0
export compress=0
export undo=0

usage()
{
    echo >&2 "Convert each dir's contents to an .mfem_cat file."
    echo >&2 ""
    echo >&2 "usage: $0 -basestr <string> [-dontrm] [-compress] [-par[allel] <int>] [-undo] [dir1, dir2, ... dirN]"
    echo >&2 ""
    echo >&2 "    -basestr is a string holding base name of mfem root files "
    echo >&2 "     and dirs not including cycle numbers. To operate on an"
    echo >&2 "     explicit list of dirs instead of all dirs identified by"
    echo >&2 "     by a common base name, just list the dirs at the end of"
    echo >&2 "     of the command line."
    echo >&2 ""
    echo >&2 "    -dontrm means to not remove the original files. Note:"
    echo >&2 "     files are *never* removed except after a confirmation"
    echo >&2 "     checksum is performed. -dontrm disables such removal."
    echo >&2 ""
    echo >&2 "    -compress means to compress each file when creating the"
    echo >&2 "     mfem_cat file."
    echo >&2 ""
    echo >&2 "    -parallel <int> means to use <int> parallel tasks."
    echo >&2 "     Default is 4 parallel tasks. Be sure you are non-login"
    echo >&2 "     nodes if using more than 4 parallel tasks."
    echo >&2 ""
    echo >&2 "    -undo means to revers the catmfem operation returning"
    echo >&2 "     the files to their original state."

    exit 1
}

process_args()
{
    while [ $# -gt 0 ]
    do
        case "$1" in
            -basestr) basestr="$2"; shift;;
            -par*) parallel=$2; shift;;
            -dontrm) dontrm=1;;
            -compress) compress=1;;
            -debug) debug=1;;
            -undo) undo=1;;
            *) test -d "$1" && dirs="$dirs $1"; shift;;
        esac
        shift
    done

    if [[ $debug -ne 0 ]]; then
        echo "basestr = \"$basestr\""
        echo "dontrm = $dontrm"
        echo "compress = $compress"
        echo "parallel = $parallel"
        echo "debug = $debug"
        echo "undo = $undo"
        echo "dirs = \"$dirs\""
        xargsdbg="-t"
        set -x
    fi
}

#
# This is the main workhorse shell function that gets executed by
# xargs for each directory. It uses inner shell functions to execute
#
process_dir() {
    if [[ $debug -ne 0 ]]; then
        set -x
    fi

    dname=$0

    add_file_sizing() {
        size=$(wc -c "$f" | sed -e 's/^ *//' | cut -d' ' -f1)
        echo $f $size $size_sum
        size_sum=$(expr $size_sum + $size)
    }

    cat_one_file() {
        cat "$f"
    }
    
    remove_one_file() {
       rm -f "$f"
    }
    
    iterate_files() {
        for fb in $file_base_names
        do
            fname=$(echo $fb | tr '@' ' ')
            for f in "$fname"*
            do
                $1
            done
        done
    }

    echo "Processing dir $dname"

    # Go into the directory
    pushd $dname 1>/dev/null

    if [[ -e $dname.mfem_cat && $undo -ne 0 ]]; then
        header_char_count=$(head -n 1 $dname.mfem_cat)
        header_line_count=$(head -c $header_char_count $dname.mfem_cat | wc -l)
        n=0
        while [[ $n -lt $header_line_count ]]; do
            read line
            if [[ $n -gt 0 ]]; then
                file=$(echo $line | rev | cut -d' ' -f3- | rev)
                size=$(echo $line | rev | cut -d' ' -f2 | rev)
                offset=$(echo $line | rev | cut -d' ' -f1 | rev)
                offset=$(expr $offset + $header_char_count)
                dd bs=1 if=$dname.mfem_cat of="$file" count=$size skip=$offset 2>/dev/null
            fi
            n=$(expr $n + 1)
        done < $dname.mfem_cat

        # Get the list of files in this dir excluding their MPI ranks.
        # Use '@' to capture spaces in filenames
        file_base_names=$(ls -1 | grep -v mfem_cat | cut -d'.' -f1 | sort | uniq | tr ' ' '@')

        ck1=$((iterate_files cat_one_file) | $MD5CMD)

        hs=$(expr $header_char_count + 1)
        ck2=$(tail -c +$hs $dname.mfem_cat | $MD5CMD)

        if [[ "$ck1" != "$ck2" ]]; then
            echo "Checksum comparison failed for $dname.mfem_cat. Removing!"
            iterate_files remove_one_file
            return 1
        fi

        if [[ $dontrm -eq 0 ]]; then
            rm -f $dname.mfem_cat
        fi

        return 0
    fi

    # Get the list of files in this dir excluding their MPI ranks.
    # Use '@' to capture spaces in filenames
    file_base_names=$(ls -1 | grep -v mfem_cat | cut -d'.' -f1 | sort | uniq | tr ' ' '@')

    # Initialize mfem_cat file
    rm -f $dname.mfem_cat
    echo "##########" > $dname.mfem_cat

    # Iterate on the files adding file sizing info to mfem_cat header
    size_sum=0 
    iterate_files add_file_sizing >> $dname.mfem_cat

    # Overwrite the ####... first line of mfem_cat file with the
    # total size of all the header info. This value will be added
    # to all offsets when reading into VisIt
    header_size=$(wc -c $dname.mfem_cat | sed -e 's/^ *//' | cut -d' ' -f1)
    header_size_len=$(echo $header_size | wc -c)
    header_size_len=$(expr $header_size_len - 1)
    header_size=$(expr $header_size - 10)
    header_size=$(expr $header_size + $header_size_len)
    sed -i -e "1 s/##########/$header_size/" $dname.mfem_cat

    # Now, cat all the files to the mfem_cat file
    iterate_files cat_one_file >> $dname.mfem_cat

    # Compute checksum on all the cat'd files
    ck1=$((iterate_files cat_one_file) | $MD5CMD)

    # Compute checksum on mfem_cat file (minus header)
    hs=$(expr $header_size + 1)
    ck2=$(tail -c +$hs $dname.mfem_cat | $MD5CMD)

    # Fail the whole operation if checksum's don't match
    if [[ "$ck1" != "$ck2" ]]; then
        echo "Checksum comparison failed for $dname.mfem_cat. Removing!"
        rm -f $dname.mfem_cat
        return 1
    fi

    # Remove the old files unless dont_remove is set
    if [[ $dontrm -eq 0 ]]; then
        iterate_files remove_one_file
    fi

    # Adjust the mfem_root file to include "catpath" member at top-level
    # The funky tr usage here is to permit sed to be portably used to
    # handle newlines
    if [[ -e ../$dname.mfem_root ]]; then
        cat ../$dname.mfem_root | tr '\n' '@' | \
            sed -e 's/@\( *\)"fields": {/@\1"catpath": "'"$dname\/$dname.mfem_cat"'",@\1"fields": {/' | \
            tr '@' '\n' > ../${dname}_cat.mfem_root
        if [[ $? -eq 0 && $dontrm -eq 0 ]]; then
            rm -f ../$dname.mfem_root
        else
            echo "sed insertion of \"catpath\" member in $dname.mfem_root failed."
            return 1
        fi
    fi

    popd 1>/dev/null
}
export -f process_dir

#
# Process command-line arguments
#
process_args $*

#
# Run the algorithm as an iteration of find and xargs
# 
if [[ -n "$basestr" ]]; then
    ls -d ${basestr}[0-9][0-9][0-9][0-9][0-9][0-9] | xargs -n 1 -P $parallel $xargsdbg sh -c "process_dir"
elif [[ -n "$dirs" ]]; then
    ls -d $dirs | xargs -n 1 -P $parallel $xargsdbg sh -c "process_dir"
fi
