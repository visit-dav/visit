import sys, string

###############################################################################
# Function: main
#
# Purpose:    This is the main function for the program.
#
# Programmer: Brad Whitlock
# Date:       Mon Jul 28 15:35:54 PST 2003
#
# Modifications:
#
###############################################################################

def main():
    makemovie = ""
    for arg in sys.argv[1:]:
        if string.find(arg, "makemovie") != -1:
            makemovie = string.replace(arg, "makemoviemain", "makemovie")
            break

    if makemovie == "":
        print "The makemoviemain script could not locate the makemovie script!"
        sys.exit(-1)
    else:
        # Source the makemovie.py script, which contains all of the 
        # code for the MakeMovie class.
        Source(makemovie)

        movie = MakeMovie()
        movie.ProcessArguments()
        movie.GenerateFrames()
        if(movie.EncodeFrames()):
            movie.Cleanup()
        sys.exit(0)

#
# Call the main function.
#
main()
