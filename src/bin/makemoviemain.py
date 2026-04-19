import sys
from pathlib import Path

###############################################################################
# Function: main
#
# Purpose:    This is the main function for the program.
#
# Programmer: Brad Whitlock
# Date:       Mon Jul 28 15:35:54 PST 2003
#
# Modifications:
#   Kathleen Biagas, Tue Feb 10, 2026
#   Use pathlib with Source command to prevent syntax warning on Windows.
#
###############################################################################

def main():
    makemovie = ""
    for arg in sys.argv[0:]:
        if arg.find("makemovie") != -1:
            makemovie = arg.replace("makemoviemain", "makemovie")
            break

    if makemovie == "":
        print("The makemoviemain script could not locate the makemovie script!")
        sys.exit(-1)
    else:
        # Source the makemovie.py script, which contains all of the 
        # code for the MakeMovie class.
        Source(Path(makemovie).as_posix())

        movie = MakeMovie()
        movie.ProcessArguments()
        if movie.GenerateFrames() > 0:
            if movie.EncodeFrames():
                movie.Cleanup()
        sys.exit(0)

#
# Call the main function.
#
main()
