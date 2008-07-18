#!python -tt
# Tom Fogal, Tue Jul 15 15:52:27 EDT 2008
#####
# Generates gnuplot files from SQLite DB of render results.
#####
from __future__ import with_statement
import sqlite3
import sys

prog_name = "format-gnu.py"
if __name__ == "__main__":
    prog_name = sys.argv[0]

class Singleton:
    __s = None
    def __init__(self):
        if(Singleton.__s):
            raise Singleton.__s
        Singleton.__s = self

class Options(Singleton):
    def __init__(self):
        Singleton.__init__(self)
        self._db = None

    def db(self): return self._db
    def set_db(self, d): self._db = d

def OptHandle():
    try:
        single = Options()
    except Singleton, s:
        single = s
    return single

def usage():
    global prog_name
    print "Usage:", prog_name, "[-f <input>] [-o <output>]"
    print "Creates a gnuplot datafile from an SQLite DB of rendering data."
    print ""
    print "\t%-10s %-10s %-50s" % ("-f", "<string>", "SQLite DB file")

def parse_opt(argv):
    from getopt import getopt, GetoptError
    try:
        opts, args = getopt(argv, "f:h")
    except GetoptError, e:
        usage()
        sys.exit(2)

    for o,a in opts:
        if o in ("-f"):
            OptHandle().set_db(a)
        if o in ("-h"):
            usage()
            sys.exit(0)

def gnuplot_from_cursor(f, cursor):
    '''Pushes out a gnuplot-appropriate file from a DB cursor.'''
    f.write("# n_proc n_cells n_pixels r_time\n")

    proc = 0
    for row in cursor:
        f.write("%3d %15d %15d %15.9f\n" % \
                (row["n_proc"], row["n_cells"], row["n_pixels"],
                 row[3]))

if __name__ == "__main__":
    parse_opt(sys.argv[1:])

    conn = sqlite3.connect(OptHandle().db())
    conn.row_factory = sqlite3.Row

    sr = conn.cursor()
    sql = 'SELECT n_proc,n_cells,n_pixels,avg(r_time) ' \
          'FROM rendering '                             \
          'WHERE icet=? '                               \
          'GROUP BY n_proc,n_cells,n_pixels '           \
          'HAVING icet=? '                              \
          'ORDER BY n_proc'
    sr.execute(sql, (0,0))
    with open("sr.data", "w") as f:
        gnuplot_from_cursor(f, sr)

    sr.execute(sql, (1,1))
    with open("icet.data", "w") as f:
        gnuplot_from_cursor(f, sr)
