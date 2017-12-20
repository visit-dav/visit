#!/bin/sh

basestr=""
parallel=4
xargsdbg=""
dirs=""
pollstr=""

# Needed in sub-routines called by xargs
export debug=0
export dontrm=0
export compress=0
export undo=0
export OSTYPE="Linux"
export MD5CMD=md5sum

usage()
{
    echo >&2 "Convert each dir's contents to an .mfem_cat file."
    echo >&2 ""
    echo >&2 "usage: $0 [-debug] [-dontrm] [-compress] [-undo] [-par[allel] <int>] \\"
    echo >&2 "    [-poll <string>] [-basestr <string> | dir1, dir2, ... dirN]"
    echo >&2 ""
    echo >&2 "    -basestr is a string holding base name of mfem root files "
    echo >&2 "     and dirs not including cycle numbers. To operate on an"
    echo >&2 "     explicit list of dirs instead of all dirs identified by"
    echo >&2 "     by a common base name, just list the dir(s) at the end of"
    echo >&2 "     of the command line."
    echo >&2 ""
    echo >&2 "    -poll means regularly poll a specific parent dir looking there"
    echo >&2 "     for newly created sub-dir(s) matching basestr and, when found"
    echo >&2 "     process the sub-dir's contents. The <string> argument to -poll is"
    echo >&2 "     the name of a file to look for in each newly created sub-dir"
    echo >&2 "     before attempting to process its contents. This allows for the"
    echo >&2 "     possibility that the sub-dir is found before the data producer"
    echo >&2 "     has finished populating it with data and allows the data"
    echo >&2 "     producer to indicate doneness for the sub-dir by touching a"
    echo >&2 "     file there. For example, the command-line..."
    echo >&2 ""
    echo >&2 "         ./catmfem.sh -basestr gorfo_ -poll .foobar /p/lsratchd/miller86."
    echo >&2 ""
    echo >&2 "     will regularly look in /p/lscratchd/miller86 for new sub-dirs"
    echo >&2 "     matching name \"gorfo_XXXXXX\" (where X is a digit). The found"
    echo >&2 "     sub-dirs will be added to a list to process. Each sub-dir will"
    echo >&2 "     be processed once a file named \".foobar\" is found in the sub-dir."
    echo >&2 "     When run in this mode, it is best to background the catmfem.sh"
    echo >&2 "     process, capture its pid so that it can be later killed by a "
    echo >&2 "     job submission script. -poll is disables parallelism."
    echo >&2 ""
    echo >&2 "    -dontrm means to not remove the original files. Note:"
    echo >&2 "     files are *never* removed except after a confirmation"
    echo >&2 "     checksum is performed. -dontrm disables such removal"
    echo >&2 "     and kinda sorta defeats the whole reason for using"
    echo >&2 "     this tool. So don't use it unless you are testing."
    echo >&2 "     You can always reverse the process using -undo."
    echo >&2 ""
    echo >&2 "    -compress means to compress each file when creating the"
    echo >&2 "     mfem_cat file. Compression improves read time into VisIt"
    echo >&2 "     and reduces disk usage often by more than a factor of 2."
    echo >&2 ""
    echo >&2 "    -parallel <int> means to use <int> parallel tasks."
    echo >&2 "     Default is 4 parallel tasks. Be sure you are NOT on login"
    echo >&2 "     nodes if using more than 4 parallel tasks."
    echo >&2 ""
    echo >&2 "    -undo means to reverse the catmfem operation returning"
    echo >&2 "     the files to their original state."
    echo >&2 ""
    echo >&2 "    -debug turns on the shell's -x so you can debug what the"
    echo >&2 "     script is doing."
    echo >&2 ""
    echo >&2 "    -help prints this usage message and exits."

    exit 1
}

poll_cleanup()
{
    rm -f $dirs/.catmfem_poll.$$
}

process_args()
{
    if [ $# -eq 0 ]; then
        usage
    fi

    while [ $# -gt 0 ]
    do
        case "$1" in
            -basestr) basestr="$2"; shift;;
            -par*) if [ $parallel -gt 0 ]; then parallel=$2; fi; shift;;
            -dontrm) dontrm=1;;
            -compress) compress=1;;
            -debug) debug=1;;
            -undo) undo=1;;
            -poll) if [ ${2:0:1} = "-" ]; then
                       echo >&2 "$0: Probably forgot <string> arg to -poll option"
                       exit 1
                   fi                                             
                   parallel=0;
                   pollstr="$2";
                   shift;;
            -*help*) usage;;
            *) if [ -d "$1" ]; then
                   dirs="$dirs $1";
               else
                   echo "$0: Unrecognized command-line argument \"$1\""
                   exit 1
               fi
               shift;;
        esac
        shift
    done

    if [ -n "$(uname | grep -i darwin)" ]; then
        OSTYPE="Darwin"
        MD5CMD=md5
    fi

    if [ $debug -ne 0 ]; then
        echo "basestr = \"$basestr\""
        echo "poll = \"$pollstr\""
        echo "dontrm = $dontrm"
        echo "compress = $compress"
        echo "parallel = $parallel"
        echo "debug = $debug"
        echo "undo = $undo"
        echo "dirs = \"$dirs\""
        echo "MD5CMD = \"$MD5CMD\""
        echo "OSTYPE =\"$OSTYPE\""
        xargsdbg="-t"
        set -x
    fi

    if [ -n "$pollstr" ]; then
        if [ -z "$dirs" ]; then
            echo >&2 "$0: Must specify a parent directory in which to poll for sub-dirs"
            echo >&2 "as last argument on command-line"
            exit 1
        fi
        if [ -z "$basestr" ]; then
            echo >&2 "$0: Must specify a -basestr <string> to indicate "
            echo >&2 "as last argument on command-line"
            exit 1
        fi
    fi
}

#
# This is the main workhorse shell function that gets executed by
# xargs for each directory. It uses inner shell functions to execute
#
process_dir() {
    if [ $debug -ne 0 ]; then
        set -x
    fi

    dname=$0
    zext=""
    if [ $compress -ne 0 ]; then
        zext=".gz"
    fi

    add_file_sizing() {
        if [ $compress -ne 0 ]; then
            gzip --best "$f"
        fi
        size=$(wc -c "$f$zext" | sed -e 's/^ *//' | cut -d' ' -f1)
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

    sedheader() {
        if [ $compress -ne 0 ]; then
            header_size=$(expr $header_size + $header_size_len + 1)
            if [ "$OSTYPE" = "Darwin" ]; then
                sed -i '' -e "1 s/##########/${header_size}z/" $dname.mfem_cat
            else
                sed -i -e "1 s/##########/${header_size}z/" $dname.mfem_cat
            fi
        else
            header_size=$(expr $header_size + $header_size_len)
            if [ "$OSTYPE" = "Darwin" ]; then
                sed -i '' -e "1 s/##########/$header_size/" $dname.mfem_cat
            else
                sed -i -e "1 s/##########/$header_size/" $dname.mfem_cat
            fi
        fi
    }

    echo "Processing dir $dname"

    # Go into the directory
    pushd $dname 1>/dev/null

    if [ -e $dname.mfem_cat ] && [ $undo -ne 0 ]; then
        header_char_count=$(head -n 1 $dname.mfem_cat | cut -d'z' -f1)
        is_compressed=0
        if [ "$(head -n 1 $dname.mfem_cat | grep z)" ]; then
            is_compressed=1
        fi
        header_line_count=$(head -c $header_char_count $dname.mfem_cat | wc -l)
        n=0
        while [ $n -lt $header_line_count ]; do
            read line
            if [ $n -gt 0 ]; then
                file=$(echo $line | rev | cut -d' ' -f3- | rev)
                size=$(echo $line | rev | cut -d' ' -f2 | rev)
                offset=$(echo $line | rev | cut -d' ' -f1 | rev)
                offset=$(expr $offset + $header_char_count)
                if [ $is_compressed -ne 0 ]; then
                    dd bs=1 if=$dname.mfem_cat count=$size skip=$offset 2>/dev/null | gzcat > "$file"
                else
                    dd bs=1 if=$dname.mfem_cat of="$file" count=$size skip=$offset 2>/dev/null
                fi
            fi
            n=$(expr $n + 1)
        done < $dname.mfem_cat

        # Get the list of files in this dir excluding their MPI ranks.
        # Use '@' to capture spaces in filenames
        file_base_names=$(ls -1 | grep -v mfem_cat | cut -d'.' -f1 | sort | uniq | tr ' ' '@')

        ck1=$((iterate_files cat_one_file) | $MD5CMD)

        hs=$(expr $header_char_count + 1)
        ck2=$(tail -c +$hs $dname.mfem_cat | $MD5CMD)

        if [ "$ck1" != "$ck2" ]; then
            echo "Checksum comparison failed for $dname.mfem_cat. Removing!"
            iterate_files remove_one_file
            return 1
        fi

        if [ $dontrm -eq 0 ]; then
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
    sedheader

    # Now, cat all the files to the mfem_cat file
    iterate_files cat_one_file >> $dname.mfem_cat

    # Compute checksum on all the cat'd files
    ck1=$((iterate_files cat_one_file) | $MD5CMD)

    # Compute checksum on mfem_cat file (minus header)
    hs=$(expr $header_size + 1)
    ck2=$(tail -c +$hs $dname.mfem_cat | $MD5CMD)

    # Fail the whole operation if checksum's don't match
    if [ "$ck1" != "$ck2" ]; then
        echo "Checksum comparison failed for $dname.mfem_cat. Removing!"
        rm -f $dname.mfem_cat
        return 1
    fi

    # Remove the old files unless dont_remove is set
    if [ $dontrm -eq 0 ]; then
        iterate_files remove_one_file
    fi

    # Adjust the mfem_root file to include "catpath" member at top-level
    # The funky tr usage here is to permit sed to be portably used to
    # handle newlines
    if [ -e ../$dname.mfem_root ]; then
        cat ../$dname.mfem_root | tr '\n' '@' | \
            sed -e 's/@\( *\)"fields": {/@\1"catpath": "'"$dname\/$dname.mfem_cat"'",@\1"fields": {/' | \
            tr '@' '\n' > ../${dname}_cat.mfem_root
        if [ $? -eq 0 ]; then
            if [ $dontrm -eq 0 ]; then
                rm -f ../$dname.mfem_root
            fi
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
# Process the directories
#
if [ -n "$pollstr" ]; then
    # Run the algorithm in polling mode
    echo "$0-pid = $$"
    trap poll_cleanup 1 2 3 6
    pushd $dirs 1>/dev/null
    not_ready_dirs=$(find . -maxdepth 1 -name "$basestr[0-9][0-9][0-9][0-9][0-9][0-9]" | cut -d '/' -f2-)
    touch .catmfem_poll.$$
    while [ 1 ]; do
        if [ ! -e .catmfem_poll.$$ ]; then
            exit 1
        fi
        newdirs=$(find . -maxdepth 1 -name "$basestr[0-9][0-9][0-9][0-9][0-9][0-9]" -anewer .catmfem_poll.$$ | cut -d '/' -f2-)
        touch .catmfem_poll.$$
        not_ready_dirs=$(echo $not_ready_dirs | tr ' ' '\n' | sort | uniq)
        newdirs="$not_ready_dirs $newdirs"
        newdirs=$(echo $newdirs | tr ' ' '\n' | sort | uniq)
        ready_dirs=""
        not_ready_dirs=""
        for nd in $newdirs; do
            if [ -e $nd/$pollstr ]; then
                ready_dirs="$ready_dirs $nd"
            else
                not_ready_dirs="$not_ready_dirs $nd"
            fi
        done
        for rd in $ready_dirs; do
            sh -c "process_dir" $rd
            rm -f $rd/$pollstr
        done
        sleep 15
    done
    popd 1>/dev/null
    poll_cleanup
elif [ -n "$basestr" ]; then
    # Run the algorithm as an iteration of find and xargs
    ls -d ${basestr}[0-9][0-9][0-9][0-9][0-9][0-9] | xargs -n 1 -P $parallel $xargsdbg sh -c "process_dir"
elif [ -n "$dirs" ]; then
    # Run the algorithm as an iteration of ls and xargs
    ls -d $dirs | xargs -n 1 -P $parallel $xargsdbg sh -c "process_dir"
fi
