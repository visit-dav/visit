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

    # pull off any hostname
    if p.startswith('thunder:'):
        p = p[8:]

    if p.startswith('/home/juan/visit/'):
        return tests_path('quickrecipes',p[17:])
    elif p.startswith('~juanita/silo/stuff/'):
        return silo_data_path(p[20:])
    elif p.startswith('/Users/amina/data/'):
        return data_path(p[18:])
    else:
        return p

#
# Utility to generalize cleanup after a block
#
def vqr_cleanup():
  DeleteAllPlots()
  for db in GetGlobalAttributes().sources:
      CloseDatabase(db)

#
# Map VisIt functions so they use vqr_path to map paths
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

#
# Adjust compute engine open hostname and args to
# match whatever this host uses or default if no
# profiles specified for this host
#
def vqr_OpenComputeEngine(hn, la):
    lhn = GetLocalHostName()
    lm = None 
    if lhn in GetMachineProfileNames():
        p = GetMachineProfile(lhn)
        for i in range(p.GetNumLaunchProfiles()):
            if 'arallel' in p.GetLaunchProfiles(i).profileName and \
                p.GetLaunchProfiles(i).launchMethodSet:
                    lm = p.GetLaunchProfiles(i).launchMethod
                    break
    newla = la
    if '-np' in la:
        next = ''
        newla = []
        for a in la:
            if next == 'skip':
                next = ''
                continue
            if a == '-l': # specify this host's launcher
                if lm:
                    newla += [a]
                    newla += [lm]
                    next = 'skip'
                else:
                    next = 'skip'
                continue
            if a == '-np': # keep cpu count small
                newla += [a]
                newla += ['8']
                next = 'skip'
                continue
            if a == '-nn': # keep node count small
                newla += [a]
                newla += ['2']
                next = 'skip'
                continue
            if a == '-p': # filter out partition
                next = 'skip'
                continue
            if a == '-b': # filter out bank
                next = 'skip'
                continue
            newla += [a]           
        newla = tuple(newla)
    return real_OpenComputeEngine(lhn,newla)
real_OpenComputeEngine = OpenComputeEngine
OpenComputeEngine = vqr_OpenComputeEngine
