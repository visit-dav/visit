# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  closedatabase.py
#
#  Tests:      mesh      - 3D, curvilinear, single domain
#              plots     - Pseudocolor
#              databases - Silo
#
#  Purpose:    This test case tests that the viewer can close a database.
#
#  Bugs:       '3283
#
#  Programmer: Brad Whitlock
#  Date:       Fri Mar 19 08:57:09 PDT 2004
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Wed Apr 14 15:41:11 PDT 2010
#    Added test to check if database expressions are cleared after
#    database is closed.
#
# ----------------------------------------------------------------------------

#
# Look at astring representation of the GlobalAttributes to see the list
# of sources, etc.
#
def TestGlobalAttributes(testname):
    # Get the window information, convert it to a string, and use it as
    # a test case.
    TestText(testname, str(GetGlobalAttributes()))

def GetCloseString(db):
    if(CloseDatabase(db) == 1):
        testString = "The database %s was closed." % os.path.abspath(db)
    else:
        testString = GetLastError()
    return testString

# Open a database and create a plot
db = silo_data_path("wave*.silo database")
OpenDatabase(db, 32)
AddPlot("Pseudocolor", "pressure")
DrawPlots()

# Get a picture of the plot.
Test("closedatabase00")

# Look at the list of sources.
TestGlobalAttributes("closedatabase01")

# Try and close the database. VisIt should not let us close it since there
# are plots that use it.
TestText("closedatabase02", GetCloseString(db))

# Create another window that uses the database.
CloneWindow()
SetActiveWindow(2)
DrawPlots()
Test("closedatabase03")

# Delete all the plots in window 1 that use the database
SetActiveWindow(1)
DeleteAllPlots()
Test("closedatabase04")

# Try and close the database. VisIt should not let us close it since there
# are still plots in window 2 that use it.
TestText("closedatabase05", GetCloseString(db))

SetActiveWindow(2)
DeleteAllPlots()

# Try and close the database. VisIt should let us close it this time.
testString = GetCloseString(db) + "\n" + str(GetGlobalAttributes())
TestText("closedatabase06", testString)


# Try adding some databases
dbs = (silo_data_path("wave.visit") ,
       silo_data_path("globe.silo") ,
       silo_data_path("curv3d.silo") )

for db in dbs:
    OpenDatabase(db)

# Look at the list of sources.
TestGlobalAttributes("closedatabase07")

# Start removing databases
testindex = 8
for i in range(len(dbs)):
    index = len(dbs) - 1 - i
    testString = GetCloseString(dbs[index]) + "\n" + str(GetGlobalAttributes())
    TestText("closedatabase%02d" % testindex, testString)
    testindex = testindex + 1

# Check that database expressions are cleared when database is closed.
OpenDatabase(dbs[1])
TestText("closedatabase_exprs00","Expressions() = " + str(Expressions()))
test_str=GetCloseString(dbs[1]) + "\nExpressions() = " + str(Expressions())
TestText("closedatabase_exprs01",test_str)

Exit()
