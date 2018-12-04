# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  silo_datatypes.py 
#
#  Tests:      All varieties of datatypes, centering, variable types,
#              meshtypes and force-single mode. 
#
#  Programmer: Mark C. Miller, Thu Jul 15 14:42:12 PDT 2010
#
#  Modifications:
#    Mark C. Miller, Tue Jul 20 19:26:04 PDT 2010
#    Adjusted names of temporary output files so they don't stomp on each
#    other.
#
#    Mark C. Miller, Wed Jul 21 08:51:30 PDT 2010
#    I side-stepped around a subtle problem with long long data from PDB.
#    Turns out on alastor where tests are being run as of July, 2010, sizeof
#    long is 8 and that is the same as size of long long. On the HDF5 driver,
#    if it reads integer data of N bytes, it will put it into the smallest
#    native type that fits. So, when it reads long long data of 8 bytes
#    but discovers a native long on the machine where the data is being read
#    is 8 bytes, it returns long instead. So, HDF5 driver never returns
#    a type of vtkLongLongArray. But, PDB driver does. And, for some reason
#    that one single case is getting handled subtly wrong. Its data extents
#    wind up getting set to [0,1] (which I assume is some sort of default).
#    So, the plot looks bad. So, here, in that case, I manually set the
#    extents and get around this problem. I am thinking there is a VTK
#    bug in computing the data extents for vtkLongLongArray type. But, thats
#    just a guess.
#
#    Mark C. Miller, Mon Jul 26 17:08:59 PDT 2010
#    Override pass/fail status for long long data until we switch to silo-4.8
#
#    Mark C. Miller, Thu Sep 23 21:23:42 PDT 2010
#    Remove override of pass/fail for long long data since now using silo-4.8
# ----------------------------------------------------------------------------
TurnOffAllAnnotations() # defines global object 'a'

#
# We do this quick open and immediate close because one cannot
# set FileOptions for a plugin type without having already opened
# at least one file of that type.
#
dummydb = silo_data_path("one_quad.silo") 
OpenDatabase(dummydb)
CloseDatabase(dummydb)

def SetForceSingle(val):
    # Turn off force single precision for this test
    # Note: We can't successfully do that until we open some database
    readOptions=GetDefaultFileOpenOptions("Silo")
    if val == "on":
        readOptions["Force Single"] = 1 
    else:
        readOptions["Force Single"] = 0 
    SetDefaultFileOpenOptions("Silo", readOptions)

#
# Test reading various datatypes from Silo
#
TestSection("Silo Data Types")
DeleteAllPlots()
mtypes=("quad","ucd")
dtypes=("c","s","i","l","L","f","d")
vtypes=("n","z")

#
# Test 'baseline' for node/zone centered quad/ucd from float data
#
backup = GetSaveWindowAttributes()
for mt in mtypes:
    dbname=silo_data_path("specmix_%s.silo" % mt)
    OpenDatabase(dbname)
    for v in vtypes:
        varname="%s%svar"%("f",v) # using only 'f'loat for baseline
        AddPlot("Pseudocolor",varname)
        DrawPlots()
        ResetView()
        Test("silo_datatypes_%s_%s"%(mt,varname))
        DeleteAllPlots()
    CloseDatabase(dbname)

# Build PC plot attributes to be used to set min/max
# Only necessary for long long case from PDB. Why?
pa = PseudocolorAttributes()
pa.minFlag = 1
pa.maxFlag = 1
pa.min = 0
pa.max = 20 

for path in ["silo_datatypes",
             pjoin("silo_datatypes","current"),
             pjoin("silo_datatypes","diff")]:
    if not os.path.isdir(path):
        os.mkdir(path)
for smode in ("hdf5", "pdb"):
    for fsmode in ("off", "on"): # force single modes
        SetForceSingle(fsmode)
        diffResults=""
        for mt in mtypes:
            dbname=silo_data_path("specmix_%s.silo" % mt )
            OpenDatabase(dbname)
            for v in vtypes:
                for d in dtypes:
                    varname="%s%svar"%(d,v)
                    fvarname="%s%svar"%("f",v)
                    filename="silo_datatypes_%s_fs%s_%s_%s"%(smode,fsmode,mt,varname)
                    AddPlot("Pseudocolor",varname)
                    if (smode == "pdb" and d == "L"):
                        if (v == "n"):
                            pa.max = 20
                        else:
                            pa.max = 19
                        SetPlotOptions(pa)
                    DrawPlots()
                    ResetView()
                    swa=SaveWindowAttributes()
                    swa.outputToCurrentDirectory = 0
                    swa.outputDirectory = pjoin("silo_datatypes","current")
                    swa.screenCapture=1
                    swa.family   = 0
                    swa.fileName = filename
                    swa.format   = swa.PNG
                    SetSaveWindowAttributes(swa)
                    SaveWindow()
                    DeleteAllPlots()
                    tPixs = pPixs = dPixs = 0
                    davg = 0.0
                    if TestEnv.params["use_pil"]:
                         file=filename
                         cur  = pjoin("silo_datatypes","current","%s.png" % filename)
                         diff = pjoin("silo_datatypes","diff","%s.png" % filename)
                         base = test_root_path("baseline","databases",
                                               "silo_datatypes",
                                               "silo_datatypes_%s_%s.png"%(mt,fvarname))
                         (tPixs, pPixs, dPixs, davg) = DiffUsingPIL(file, cur, diff, base, "")
                         result = "PASSED"
                         if (dPixs > 0 and davg > 1):
                             result = "FAILED, %f %f"%(dPixs,davg)
                         diffResults += "%s_%s:    %s\n"%(mt,varname,result)
            CloseDatabase(dbname)
        TestText("silo_datatypes_diffs_%s_fs%s"%(smode,fsmode),diffResults)
shutil.rmtree("silo_datatypes")
SetSaveWindowAttributes(backup)

Exit()
