#!python -tt
# Tom Fogal, Tue Jul 15 15:52:27 EDT 2008
#####
# Generates gnuplot files from an SQLite DB of render results.
#####

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
        self._avg = None
        self._max_proc = None

    def db(self): return self._db
    def set_db(self, d): self._db = d

    def average(self): return self._avg
    def set_average(self, v): self._avg = v

    def max_processors(self): return self._max_proc
    def set_max_processors(self, mp): self._max_proc = mp

def OptHandle():
    try:
        single = Options()
    except Singleton as s:
        single = s
    return single

def usage():
    global prog_name
    print("Usage:", prog_name, "-f <input> [-a] [-n #]")
    print("Creates a gnuplot datafile from an SQLite DB of rendering data.")
    print("")
    print("\t%-10s %-10s %-50s" % ("-f", "<string>",  "SQLite DB file"))
    print("\t%-10s %-10s %-50s" % ("-a", "",          "Aggregate results"))
    print("\t%-10s %-10s %-50s" % ("-n", "<integer>", "max # of processors"))

def parse_opt(argv):
    from getopt import getopt, GetoptError
    try:
        opts, args = getopt(argv, "af:hn:")
    except GetoptError as e:
        usage()
        sys.exit(2)

    OptHandle().set_max_processors(16777216)
    OptHandle().set_average(False)
    for o,a in opts:
        if o in ("-a"):
            OptHandle().set_average(True)
        if o in ("-f"):
            OptHandle().set_db(a)
        if o in ("-n"):
            OptHandle().set_max_processors(a)
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
    if OptHandle().average():
        select = 'n_proc, n_cells, n_pixels, avg(r_time)'
        clause = 'GROUP BY n_proc,n_cells,n_pixels ' \
                 'HAVING icet=:icet '
    else:
        select = 'n_proc, n_cells, n_pixels, r_time'
        clause = ''
    sql = ''.join([s for s in ('SELECT ', select, ' ',
                               'FROM rendering ',
                               'WHERE icet=:icet ', clause, ' ',
                               'AND n_proc <= :nproc ',
                               'ORDER BY n_proc')])

    sr.execute(sql, {"icet": 0, "nproc": OptHandle().max_processors()})
    with open("sr.data", "w") as f:
        gnuplot_from_cursor(f, sr)

    sr.execute(sql, {"icet": 1, "nproc": OptHandle().max_processors()})
    with open("icet.data", "w") as f:
        gnuplot_from_cursor(f, sr)
