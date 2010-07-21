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
# ----------------------------------------------------------------------------
TurnOffAllAnnotations() # defines global object 'a'

#
# We do this quick open and immediate close because one cannot
# set FileOptions for a plugin type without having already opened
# at least one file of that type.
#
dummydb = "../data/silo_%s_test_data/one_quad.silo"%SILO_MODE
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
    dbname="../data/silo_%s_test_data/specmix_%s.silo"%(SILO_MODE,mt)
    OpenDatabase(dbname)
    for v in vtypes:
        varname="%s%svar"%("f",v) # using only 'f'loat for baseline
        AddPlot("Pseudocolor",varname)
        DrawPlots()
        ResetView()
        Test("silo_datatypes_%s_%s"%(mt,varname))
        DeleteAllPlots()
    CloseDatabase(dbname)

os.system("mkdir silo_datatypes")
os.system("mkdir silo_datatypes/current")
os.system("mkdir silo_datatypes/diff")
for smode in ("hdf5", "pdb"):
    for fsmode in ("off", "on"): # force single modes
        SetForceSingle(fsmode)
        diffResults=""
        for mt in mtypes:
            dbname="../data/silo_%s_test_data/specmix_%s.silo"%(smode,mt)
            OpenDatabase(dbname)
            for v in vtypes:
                for d in dtypes:
                    varname="%s%svar"%(d,v)
                    fvarname="%s%svar"%("f",v)
                    filename="silo_datatypes_%s_fs%s_%s_%s"%(smode,fsmode,mt,varname)
                    AddPlot("Pseudocolor",varname)
                    DrawPlots()
                    ResetView()
                    swa=SaveWindowAttributes()
                    swa.outputToCurrentDirectory = 0
                    swa.outputDirectory = "silo_datatypes/current"
                    swa.screenCapture=1
                    swa.family   = 0
                    swa.fileName = filename
                    swa.format   = swa.PNG
                    SetSaveWindowAttributes(swa)
                    SaveWindow()
                    DeleteAllPlots()
                    tPixs = pPixs = dPixs = 0
                    davg = 0.0
                    if usePIL:
                         file=filename
                         cur="silo_datatypes/current/%s.png"%filename
                         diff="silo_datatypes/diff/%s.png"%filename
                         base="baseline/databases/silo_datatypes/silo_datatypes_%s_%s.png"%(mt,fvarname)
                         (tPixs, pPixs, dPixs, davg) = DiffUsingPIL(file, cur, diff, base, "")
                         result = "PASSED"
                         if (dPixs > 0 and davg > 1):
                             result = "FAILED, %f %f"%(dPixs,davg)
                         diffResults += "%s_%s:    %s\n"%(mt,varname,result)
            CloseDatabase(dbname)
        TestText("silo_datatypes_diffs_%s_fs%s"%(smode,fsmode),diffResults)
os.system("rm -rf silo_datatypes")
SetSaveWindowAttributes(backup)

Exit()
