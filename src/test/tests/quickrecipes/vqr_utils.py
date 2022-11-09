# ----------------------------------------------------------------------------
# Mark C. Miller, Mon Nov  7 11:32:55 PST 2022
#
# Utilities used in quickrecipe code blocks to enable paths and other things
# to look natural where it is literalincluded in the docs but still access
# files in their "normal" places in our test logic.
#
# ----------------------------------------------------------------------------

#
# Map ordinary string paths used here which appear in rendered docs
# to the paths ordinarily used in VisIt testing...
#
#    /home/juan/visit/xyz maps to tests_path('quickrecipes','xyz')
#    ~juanita/silo/stuff/xyz maps to silo_data_path('xyz')
#    /Users/amina/data/abc maps to data_path('abc')
#
def vqr_path(p):
    print("got here with", p)
    if p.startswith('/home/juan/visit/'):
        print(p[17:])
        return tests_path('quickrecipes',p[17:])
    elif p.startswith('~juanita/silo/stuff/'):
        print(p[20:])
        return silo_data_path(p[20:])
    elif p.startswith('/Users/amina/data/'):
        print(p[18:])
        return tests_path(p[18:])
    else:
        print("just returning p")
        return p

#
# Utility to generalize cleanup after a block
#
def vqr_cleanup():
  pl = GetPlotList()
  DeleteAllPlots()
  for p in range(pl.GetNumPlots()):
      CloseDatabase(pl.GetPlots(p).GetDatabaseName())

#
# Map VisIt functions to so they use vqr_path to map paths
# before invoking the real VisIt functions
#
def vqr_RestoreSession(f, vdirFlag=0):
    return real_RestoreSession(vqr_path(f),vdirFlag)
real_RestoreSession = RestoreSession
RestoreSession = vqr_RestoreSession

def vqr_OpenDatabase(db, ti=0, pname=''):
    return real_OpenDatabase(vqr_path(db),ti,pname)
real_OpenDatabase = OpenDatabase
OpenDatabase = vqr_OpenDatabase

