###############################################################################
#
# Purpose: Sets up a 2x2 layout of vis windows and expressions for database
#          differencing of scalar variables (vector variables are still to
#          be added). The intersection of the scalar variable names is
#          calculated and we define CMFE expressions for them to compute 
#          dbl - dbr. Then, the first scalar in the set of CMFE expressions is 
#          plotted, along with its source values from dbl, dbr. The ell, 'l'
#          and arr, 'r', nomenclature is to indicate left and right operands
#          of the differencing operation.
#
# Usage: visit -diff dbl dbr [ -force_pos_cmfe ]
#
# Notes: dbl, dbr are the names of the databases that VisIt will difference.
#
# Future work: It would be nice if the CLI could define callback functions
#              to be executed when the user does something such as changing
#              the active variable. You can imagine that we would want to
#              change the plotted variables from other windows too when
#              executing such a callback function.
#
# Programmer: Mark C. Miller (based on original design by Brad Whitlock) 
# Date:       Wed Jul 18 10:17:11 PDT 2007 
#
##############################################################################

import sys, string, os, re, time

###############################################################################
# Function: help 
#
# Purpose:    Print a useful help message 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def help():
    print """

    This python script is intended to be used in concert with VisIt's CLI and GUI.

    It is invokved using the command 'visit -diff <dbl> <dbr> [ -force_pos_cmfe ]'.

    This script will generate the necessary Cross-Mesh Field Evaluation (CMFE)
    expressions to facilitate visualization and analysis of the differences between
    two databases. VisIt will open windows to display both the left and right
    databases as well as their difference. The windows are numbered as follows

    Upper Left (1) -- shows Left-Right    (a)Upper Right (2) -- shows Right-Left

    Lower Left (3) -- shows Left             Lower Right (4) -- shows Right

    (a) only for position-based CMFE expressions.

    VisIt uses the Cross-Mesh Field Evaluation (CMFE) expression functions
    to generate the differences. A CMFE function creates an instance
    of a variable from another (source) mesh on the specified (destination)
    mesh. Therefore, window 1 (upper left) shows the difference obtained
    when taking a variable from the mesh in the right database, applying the
    CMFE expression function to map it onto the mesh in the left database and
    then subtracting it from the same variable in the left database. 
    
    VisIt can use two variants of CMFE expression functions depending
    on how similar the source and destination meshes are; connectivity-based
    (conn_cmfe) which assumes the underlying mesh(s) for the left and right
    databases have identical connectivity and position-based (pos_cmfe) which
    does not make this assumption. VisIt will attempt to automatically select
    which variant of CMFE expression to use based on some simple heuristics.
    For meshes with identical connectivity, conn_cmfe expressions are
    preferrable because they are higher performance and do not require VisIt
    to perform any interpolation. In fact, the conn_cmfe operation is
    perfectly anti-symmetric. That is Left-Right = -(Right-Left).
    The same cannot be said for pos_cmfe expressions. For this reason,
    window 2 (upper right) is only ever active when using position-based CMFE
    expressions. It shows the (possibly different) difference obtained when
    taking a variable from the mesh in the left database, applying the CMFE
    expression function to map it onto the mesh in the right database and
    then subtracting it from the same variable in the right database.

    Pos_cmfe expressions will attempt to generate useful results regardless of
    the similarity of the underlying meshes. You can force use of pos_cmfe
    expressions by adding '-force_pos_cmfe' to the command line when running
    'visit -diff'.

    Note that the differences VisIt will compute in this mode are single
    precision. This is true regardless of whether the input data is itself
    double precision. VisIt will convert double precision to single 
    precision before processing it. Although this is a result of earlier
    visualization-specific design requirements and constraints, the intention
    is that eventually double precision will be supported.

    Expressions for the differences for all scalar variables will be under the
    'diffs' submenu. For material volume fractions, the scalar volume fraction
    variables will be under the 'matvf_comps' submenu and their differences will
    be under 'diffs/matvf_comps' submenu. Likewise for vector variables, their
    scalar components will be under the 'vector_comps' submenu and their
    differences under the 'diffs/vector_comps' submenu.

    'visit -diff' is operated using a combination of VisIt's GUI and CLI.
    There are a number of python functions defined in this script. These
    are...

    ToggleMesh()     -- Toggle the mesh plot(s) on/off.
    ToggleBoundary() -- Toggle the material boundary plot(s) on/off.
    ToggleHidePloti() -- Toggle hiding the ith plot in the plot list(s)
    DiffSummary()    -- Examine all variables in the database and report a
                        summary of differences found in each.
    ChangeVar("foo") -- Change the variable displayed in all windows to "foo".
    ZPick((1,2,3))   -- Perform a zone-pick in all windows for the zone ids 1,2,3
    NPick((4,5,6))   -- Perform a node-pick in all windows for the node ids 4,5,6

    For the functions described above with no arguments, there are pre-defined macros
    in VisIt's GUI that can be found under Controls->Macros. Not all of the convenience
    functions available in this script are actionable through the GUI. Only those that
    DO NOT reuquire some kind of user input are.

    Finally, you should be able to do whatever operations you wish in a given window
    and then synchronize all other windows to the same state. To do this, add whatever
    operators, plots, as well as adjustments to plot and operator attributes you wish
    to a given window. Then use the SyncWindows() method to bring all other windows
    into a consistent state. For example, if you add plots and operators to the window
    1 (the upper left window where L-R is displayed), then do SyncWindows(1) will bring
    all other windows into an identical state.

    SyncWindows(a)   -- Synchronize all windows to window 'a', where a is 1...4.

    There are buttons defined in Controls->Macros to perform these synchronization
    operations. For example, the SyncToL-RDiff button will synchronize all windows
    to be consistent with whatever was done in the window where L-R is displayed
    (upper left).

    Finally, if you move around in time in a given window, use the SyncTimeState()
    method to synchronise all windows to the current time state.

    SyncTimeStates(a) -- Synchronise all windows' time state to window 'a'. 

    Note that 'visit -diff' cannot currently handle differences in databases that
    have a different number of time states.
"""

###############################################################################
# Function: GetDiffVarNames 
#
# Purpose:  Given any variable's name (in diff menu or submenus) return all
#           varieties of names for it. If absolute and relative differencing
#           is added, this is the place to handle the naming.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetDiffVarNames(name):
    retval = ()
    varname = re.search("diff/(.*)", name)
    if varname != None:
        varname = varname.group(1)
        retval = (varname, name)
    else:
        retval = (name, "diff/%s"%name)
    return retval

###############################################################################
# Function: GetNDomains 
#
# Purpose:  Return number of domains for a given mesh 
#
# Programmer: Brad Whitlock 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetNDomains(metadata, meshname):
    nd = 1
    for mi in range(metadata.GetNumMeshes()):
        if metadata.GetMeshes(mi).name == meshname:
            nd = metadata.GetMeshes(mi).numBlocks
            break
    return nd

###############################################################################
# Function: GetMeshType 
#
# Purpose:  Return type of given mesh 
#
# Programmer: Brad Whitlock
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetMeshType(metadata, meshname):
    mt = -1
    for mi in range(metadata.GetNumMeshes()):
        if metadata.GetMeshes(mi).name == meshname:
            mt = metadata.GetMeshes(mi).meshType
            break
    return mt

###############################################################################
# Function: GetVarInfo
#
# Purpose: Return a named portion of a metadata object 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetVarInfo(metadata, varname):
    for i in range(metadata.GetNumScalars()):
        if metadata.GetScalars(i).name == varname:
            return metadata.GetScalars(i) 
    for i in range(metadata.GetNumMeshes()):
        if metadata.GetMeshes(i).name == varname:
            return metadata.GetMeshes(i)
    for i in range(metadata.GetNumMaterials()):
        if metadata.GetMaterials(i).name == varname:
            return metadata.GetMaterials(i)
    for i in range(metadata.GetNumVectors()):
        if metadata.GetVectors(i).name == varname:
            return metadata.GetVectors(i)
    for i in range(metadata.GetNumArrays()):
        if metadata.GetArrays(i).name == varname:
            return metadata.GetArrays(i)
    for i in range(metadata.GetNumCurves()):
        if metadata.GetCurves(i).name == varname:
            return metadata.GetCurves(i)
    for i in range(metadata.GetNumLabels()):
        if metadata.GetLabels(i).name == varname:
            return metadata.GetLabels(i)
    for i in range(metadata.GetNumTensors()):
        if metadata.GetTensors(i).name == varname:
            return metadata.GetTensors(i)
    return 0


###############################################################################
# Function: GetVarType
#
# Purpose:  Return a variable's avt type 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetVarType(metadata, varname):
    theType = "Unknown"
    vInfo = GetVarInfo(metadata, varname)
    if vInfo != 0:
        tmpType = re.search("<type 'avt([A-Z][a-z]*)MetaData'>", str(type(vInfo)))
        if tmpType != None:
            theType = tmpType.group(1)
    return theType

###############################################################################
# Function: MeshForVar
#
# Purpose:  Determine the mesh for a given variable 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def MeshForVar(metadata, varname, dontCheckExpressions=0):
    meshName = "Unknown"
    vInfo = GetVarInfo(metadata, varname)
    if vInfo != 0 and hasattr(vInfo, "meshName"):
        tmpMeshName = re.search("\nmeshName = \"(.*)\"\n",str(vInfo))
        if tmpMeshName != None:
            meshName = tmpMeshName.group(1)
    else:
        # look at meshes themselves
        for i in range(metadata.GetNumMeshes()):
            if metadata.GetMeshes(i).name == varname:
                meshName = varname 
		break

        ttab = string.maketrans("()<>,:","@@@@@@")
	# if we don't yet have an answer, look at current expressions
        if meshName == "Unknown" and dontCheckExpressions == 0:
	    exprList = Expressions() 
	    i = 0;
	    while i < len(exprList) and meshName == "Unknown":
	        theExpr = exprList[i]
		if theExpr[0] == varname:
		    defnTmp = string.translate(theExpr[1],ttab)
		    defnFields = defnTmp.split('@')
		    for f in defnFields:
		        meshNameTmp = MeshForVar(metadata, f, 1)
			if meshNameTmp != "Unknown":
			    meshName = meshNameTmp
			    break
                i = i + 1

	# if we don't yet have an answer, look at expressions from database
        if meshName == "Unknown" and dontCheckExpressions == 0:
	    exprList = metadata.GetExprList()
	    i = 0;
	    while i < exprList.GetNumExpressions() and meshName == "Unknown":
	        theExpr = exprList.GetExpressions(i)
	        if theExpr.name == varname:
		    defnTmp = string.translate(theExpr.definition,ttab)
		    defnFields = defnTmp.split('@')
		    for f in defnFields:
		        meshNameTmp = MeshForVar(metadata, f, 1)
			if meshNameTmp != "Unknown":
			    meshName = meshNameTmp
			    break
                i = i + 1

    return meshName 

###############################################################################
# Function: MatForMesh
#
# Purpose:  Return a material object for a given mesh
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def MatForMesh(metadata, meshname):
    for i in range(metadata.GetNumMaterials()):
        if metadata.GetMaterials(i).meshName == meshname:
            return metadata.GetMaterials(i).name

###############################################################################
# Function: GetVarCentering
#
# Purpose:  Return the centering for a given variable
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetVarCentering(metadata, varname):
    centering = "Unknown" 
    vInfo = GetVarInfo(metadata, varname)
    if vInfo != 0 and hasattr(vInfo, "centering"):
        tmpCentering = re.search("\ncentering = (AVT_[A-Z]*) *#.*\n",str(vInfo))
        if tmpCentering != None:
            centering = tmpCentering.group(1)
    return centering

###############################################################################
# Function: IsNotScalarVarPlotType
#
# Purpose:  Return whether or not the given plot type supports simple scalar
#           variables.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def IsNotScalarVarPlotType(plotType):
    plotTypeName = PlotPlugins()[plotType]
    if plotTypeName == "Mesh" or \
       plotTypeName == "Boundary" or \
       plotTypeName == "FilledBoundary" or \
       plotTypeName == "Vector" or \
       plotTypeName == "Molecule" or \
       plotTypeName == "Subset":
       return 1 
    return 0 

###############################################################################
# Function: GetCurrentTimeState
#
# Purpose:  Given a window id, return the current time state in that window 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def GetCurrentTimeState(win):
    SetActiveWindow(win)
    wi = GetWindowInformation()
    if wi.activeTimeSlider == -1:
        return 0
    return wi.timeSliderCurrentStates[wi.activeTimeSlider]


###############################################################################
# Function: SyncTimeStates
#
# Purpose: Ensure that the various data structures of this script are brought up
#          to date with the current time state of the specified source window.
#          Also, ensure that all windows' time states are brought up to date with
#          the specified window's time state.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def SyncTimeStates(srcWin):
    global currentTimeState
    global mdl
    global mdr
    global dbl
    global dbr

    if currentTimeState != -1:
        # no work to do if the current time state is already set
        tmpCurrentTimeState = GetCurrentTimeState(srcWin)
        if currentTimeState == tmpCurrentTimeState:
	    print "Time state is up to date"
            return

        print "Updating time state to state %d"%tmpCurrentTimeState
        currentTimeState = tmpCurrentTimeState

    else:
        print "Updating time state to state 0"
        currentTimeState = 0

    TimeSliderSetState(currentTimeState)

    # There is a bug with correlations when time arg is used to GetMetaData.
    # Without it, it  turns out we always get state zero.
#    mdl = GetMetaData(dbl, currentTimeState)
#    mdr = GetMetaData(dbr, currentTimeState)
    mdl = GetMetaData(dbl)
    mdr = GetMetaData(dbr)

    if mdl.numStates != mdr.numStates:
        print "Database \"%s\" has %d states"%(dbl, mdl.numStates)
        print "Database \"%s\" has %d states"%(dbr, mdr.numStates)
        print "Currently, 'visit -diff' is unable to handle databases with different numbers of states"
	sys.exit(4)

    UpdateExpressions(mdl, mdr)


###############################################################################
# Function: SyncTime... 
#
# Purpose:  Stubs to register as macros 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def SyncTimeL_R():
    SyncTimeStates(1)

def SyncTimeR_L():
    SyncTimeStates(2)

def SyncTimeLeft():
    SyncTimeStates(3)

def SyncTimeRight():
    SyncTimeStates(4)

###############################################################################
# Function: ProcessCLArgs 
#
# Purpose:  Read the command line arguments 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
# Modifications:
#   Mark C. Miller, Tue Aug 21 11:17:20 PDT 2007
#   Added support for difference summary mode 
#
###############################################################################
def ProcessCLArgs():
    global dbl
    global dbr
    global forcePosCMFE
    global diffSummaryOnly
    try:
        i = 1
        while i < len(sys.argv):
            if sys.argv[i] == "-vdiff":
                dbl = sys.argv[i+1]
                dbr = sys.argv[i+2]
                i = i + 2
            if sys.argv[i] == "-force_pos_cmfe":
                forcePosCMFE = 1
            if sys.argv[i] == "-summary_only":
                diffSummaryOnly = 1
            i = i + 1
    except:
        print "The -vdiff flag takes 2 database names.", dbl, dbr
        sys.exit(1)

    if dbl == "notset" or dbr == "notset":
        print "The -vdiff argument was not given."
        sys.exit(2)

###############################################################################
# Function: UpdateThisExpression 
#
# Purpose: Given the list of currently defined expressions, determine if the
#          new expression (exprName, expr) is being added, updated or left
#          unchanged.
#
# Programmer: Mark C. Miller
# Date:       Wed Jul 18 18:12:28 PDT 2007
#
###############################################################################
def UpdateThisExpression(exprName, expr, currentExpressions, addedExpressions,
    updatedExpressions, unchangedExpressions):
    # Add or update the expression.
    foundExprName = 0
    for expr_i in range(len(currentExpressions)):
        if currentExpressions[expr_i][0] == exprName:
            foundExprName = 1
            if currentExpressions[expr_i][1] == expr:
                unchangedExpressions.append(exprName)
                break
            else:
                DeleteExpression(exprName)
                DefineScalarExpression(exprName, expr)
                updatedExpressions.append(exprName)
                break
    if foundExprName == 0:
        DefineScalarExpression(exprName, expr)
        addedExpressions.append(exprName)


###############################################################################
# Function: UpdateExpressions 
#
# Purpose: Define various expressions needed to represent the difference
#          between corresponding variables in the left and right databases.
#
#          First, we get the currently defined expressions and remove any
#          that come from the database metadata.
#
#          Next, we iterate over all scalar variables defining either conn_
#          of pos_ cmfes for their difference. Note: We don't really handle
#          the R-PosCMFE(L) case yet.
#
#          Next, we iterate over all material objects, defining matvf
#          expressions for each material as a scalar and then difference
#          expressions for these scalars. Likewise for vector variables.
#
#          Finally, we use UpdateThisExpression to ensure we don't re-define
#          the same expressions and remove old expressions as we vary time
#          states.
#
# Programmer: Brad Whiltlock 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
# Modifications:
#   Mark C. Miller, Wed Jul 18 18:12:28 PDT 2007
#   Made it work on material volume fractions and vectors. Made it handle
#   changes in timesteps along with adding new expressions for new variables,
#   deleting old expressions and leaving unchanged expressions alone.
#
#   Mark C. Miller, Thu Jul 19 21:36:47 PDT 2007
#   Inverted loops to identify pre-defined expressions coming from md.
#
#   Mark C. Miller, Tue Aug 21 11:17:20 PDT 2007
#   Added support for difference summary mode 
#
###############################################################################
def UpdateExpressions(mdl, mdr):
    global forcePosCMFE
    global cmfeMode
    global diffVars

    if diffSummaryOnly == 0:
        print "Defining expressions for state %d"%currentTimeState

    cmfeModeNew = 0
    diffVarsNew = []
    addedExpressions = []
    updatedExpressions = []
    deletedExpressions = []
    unchangedExpressions = []
    currentExpressionsTmp = Expressions()
    currentExpressionsList = []

    # remove any pre-defined expressions in currentExpressions
    # coming from the metadata
    for expr_i in range(len(currentExpressionsTmp)):
        foundIt = 0
	# Look for it in the left db's metadata
        for expr_j in range(mdl.GetExprList().GetNumExpressions()):
	    if currentExpressionsTmp[expr_i][0] == \
	       mdl.GetExprList().GetExpressions(expr_j).name:
	        foundIt = 1
		break
        if foundIt == 0:
	    # Look for it in the right db's metadata
            for expr_j in range(mdr.GetExprList().GetNumExpressions()):
	        if currentExpressionsTmp[expr_i][0] == \
	           mdr.GetExprList().GetExpressions(expr_j).name:
	            foundIt = 1
		    break
	# If we didn't find it in either left or right dbs md, it is 
	# NOT a pre-defined expression. So, we can keep it.
        if foundIt == 0:
            currentExpressionsList.append(currentExpressionsTmp[expr_j])

    currentExpressions = tuple(currentExpressionsList)

    # Iterate over all the scalar variables in metadata.
    for scalar_i1 in range(mdl.GetNumScalars()):
        for scalar_i2 in range(mdr.GetNumScalars()):
            valid = mdl.GetScalars(scalar_i1).validVariable and \
                    mdr.GetScalars(scalar_i2).validVariable
            namematch = mdl.GetScalars(scalar_i1).name == \
                        mdr.GetScalars(scalar_i2).name
            if valid and namematch:
                # Create the expression name.
                if mdl.GetScalars(scalar_i1).name[0] == '/':
                    exprName = "diff" + mdl.GetScalars(scalar_i1).name
                else:
                    exprName = "diff/" + mdl.GetScalars(scalar_i1).name

                # The name of the scalar
                sName = mdl.GetScalars(scalar_i1).name
                qsName = sName
                if string.find(qsName, "/") != -1:
                    qsName = "<" + qsName + ">"

                # Determine some properties about the mesh so we can decide
                # Whether we'll use conn_cmfe or pos_cmfe.
                m1Name = mdl.GetScalars(scalar_i1).meshName
                m2Name = mdr.GetScalars(scalar_i2).meshName
                nb1 = GetNDomains(mdl, m1Name)
                mt1 = GetMeshType(mdl, m1Name)
                nb2 = GetNDomains(mdr, m2Name)
                mt2 = GetMeshType(mdr, m2Name)
                if nb1 == nb2 and mt1 == mt2 and m1Name == m2Name and forcePosCMFE != 1:
                    expr = "%s - conn_cmfe(<%s:%s>, %s)" % (qsName, dbr, sName, m1Name)
                else:
                    expr = "%s - pos_cmfe(<%s:%s>, %s, 0.)" % (qsName, dbr, sName, m1Name)
                    cmfeModeNew = 1

                diffVarsNew.append(exprName)

		UpdateThisExpression(exprName, expr, currentExpressions, addedExpressions,
		    updatedExpressions, unchangedExpressions)

    # Iterate over all the material variables in metadata.
    for mat_i1 in range(mdl.GetNumMaterials()):
        for mat_i2 in range(mdr.GetNumMaterials()):
	    matl = mdl.GetMaterials(mat_i1)
	    matr = mdr.GetMaterials(mat_i2)
            valid = matl.validVariable and matr.validVariable
            nameMatch = matl.name == matr.name
            numMatsMatch = matl.numMaterials == matr.numMaterials
	    matNamesMatch = matl.materialNames == matr.materialNames
            if valid and nameMatch and numMatsMatch and matNamesMatch:
                # Determine some properties about the mesh so we can decide
                # Whether we'll use conn_cmfe or pos_cmfe.
                m1Name = matl.meshName
                m2Name = matr.meshName
                nb1 = GetNDomains(mdl, m1Name)
                mt1 = GetMeshType(mdl, m1Name)
                nb2 = GetNDomains(mdr, m2Name)
                mt2 = GetMeshType(mdr, m2Name)
	        for m in range(matl.numMaterials):

                    # Create the matvf expression for this mat
		    matName = matl.materialNames[m]
		    altMatName = matName.replace(" ","_")
		    matNum = matName.split(' ')[0]
		    matvfExprName = "matvf_comps/" + altMatName
                    matvfexpr = "matvf(%s,[%s])"%(matl.name, matNum)
		    UpdateThisExpression(matvfExprName, matvfexpr, currentExpressions, addedExpressions,
		        updatedExpressions, unchangedExpressions)

		    # Create the expression for the difference in matvfs for this mat
		    exprName = "diff/matvf_comps/" + altMatName
                    if nb1 == nb2 and mt1 == mt2 and m1Name == m2Name and forcePosCMFE != 1:
                        expr = "<matvf_comps/%s> - conn_cmfe(<%s:matvf_comps/%s>, %s)" % (altMatName, dbr, altMatName, m1Name)
                    else:
                        expr = "<matvf_comps/%s> - pos_cmfe(<%s:matvf_comps/%s>, %s, 0.)" % (altMatName, dbr, altMatName, m1Name)
                        cmfeModeNew = 1

                    diffVarsNew.append(exprName)

		    UpdateThisExpression(exprName, expr, currentExpressions, addedExpressions,
		        updatedExpressions, unchangedExpressions)

    # Iterate over all the vector variables in metadata.
    for vec_i1 in range(mdl.GetNumVectors()):
        for vec_i2 in range(mdr.GetNumVectors()):
	    vecl = mdl.GetVectors(vec_i1)
	    vecr = mdr.GetVectors(vec_i2)
            valid = vecl.validVariable and vecr.validVariable
            nameMatch = vecl.name == vecr.name
            numDimsMatch = vecl.numDim == vecr.numDim
            if valid and nameMatch and numDimsMatch:
                # Determine some properties about the mesh so we can decide
                # Whether we'll use conn_cmfe or pos_cmfe.
                m1Name = vecl.meshName
                m2Name = vecr.meshName
                nb1 = GetNDomains(mdl, m1Name)
                mt1 = GetMeshType(mdl, m1Name)
                nb2 = GetNDomains(mdr, m2Name)
                mt2 = GetMeshType(mdr, m2Name)
	        for m in range(vecl.numDims):

                    # Create the expression to extract a component for this vector 
		    compName = vecl.name + "%02d"%m 
		    vecExprName = "vector_comps/" + compName 
                    vecexpr = "%s[%d]"%(vecl.name, m)
		    UpdateThisExpression(vecExprName, vecexpr, currentExpressions, addedExpressions,
		        updatedExpressions, unchangedExpressions)

		    # Create the expression for the difference in components
		    exprName = "diff/vector_comps/" + compName 
                    if nb1 == nb2 and mt1 == mt2 and m1Name == m2Name and forcePosCMFE != 1:
                        expr = "<vector_comps/%s> - conn_cmfe(<%s:vector_comps/%s>, %s)" % (compName, dbr, compName, m1Name)
                    else:
                        expr = "<vector_comps/%s> - pos_cmfe(<%s:vector_comps/%s>, %s, 0.)" % (compName, dbr, compName, m1Name)
                        cmfeModeNew = 1

                    diffVarsNew.append(exprName)

		    UpdateThisExpression(exprName, expr, currentExpressions, addedExpressions,
		        updatedExpressions, unchangedExpressions)

    # Finally, delete any expressions we aren't using anymore.
    for expr_i in range(len(currentExpressions)):
        foundExprName = 0
        for expr_j in range(len(unchangedExpressions)):
            if unchangedExpressions[expr_j] == currentExpressions[expr_i][0]:
                foundExprName = 1
                break
        for expr_j in range(len(updatedExpressions)):
            if updatedExpressions[expr_j] == currentExpressions[expr_i][0]:
                foundExprName = 1
                break
        for expr_j in range(len(addedExpressions)):
            if addedExpressions[expr_j] == currentExpressions[expr_i][0]:
                foundExprName = 1
                break
        if foundExprName == 0:
            DeleteExpression(currentExpressions[expr_i][0])
            deletedExpressions.append(currentExpressions[expr_i][0])

    # Print out some information about what we did
    if diffSummaryOnly == 0:
        if len(addedExpressions) > 0:
            print "    Added %d expressions..."%len(addedExpressions)
            for expr_i in range(len(addedExpressions)):
                print "       %s"%addedExpressions[expr_i]
        if len(unchangedExpressions) > 0:
            print "    Unchanged %d expressioons..."%len(unchangedExpressions)
            for expr_i in range(len(unchangedExpressions)):
                print "       %s"%unchangedExpressions[expr_i]
        if len(updatedExpressions) > 0:
            print "    Updated %d expressions..."%len(updatedExpressions)
            for expr_i in range(len(updatedExpressions)):
                print "       %s"%updatedExpressions[expr_i]
        if len(deletedExpressions) > 0:
            print "    Deleted %d expressions"%len(deletedExpressions)
            for expr_i in range(len(deletedExpressions)):
                print "       %s"%deletedExpressions[expr_i]
        print "Finished defining expressions"

    cmfeMode = cmfeModeNew
    diffVarsNew.sort()
    diffVars = diffVarsNew

###############################################################################
# Function: Initialize
#
# Purpose:  Setup the initial windows and behavior 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
# Modifications:
#   Mark C. Miller, Tue Aug 21 11:17:20 PDT 2007
#   Added support for difference summary mode 
###############################################################################
def Initialize():
    global winDbMap 
    global cmfeMode
    global oldw

    #
    # Open left and right database operands
    #
    if OpenDatabase(dbl) == 0:
       print "VisIt could not open ", dbl
       sys.exit(3)

    if OpenDatabase(dbr) == 0:
       print "VisIt could not open ", dbr
       sys.exit(3)

    #
    # Make a 2x2 window layout as follows
    #   1: L-CMFE(R)  2: R-CMFE(L) -- only when cmfeMode==1
    #   3: L          4: R
    SetCloneWindowOnFirstRef(1)
    ToggleLockTime()
    ToggleLockViewMode()
    SetWindowLayout(4)
    SyncTimeStates(0)

    # If we were able to create any expressions, let's set up some plots based on the
    # first one. That way, we can also set up some annotations.
    winDbMap = {1 : dbl, 2 : dbr, 3 : dbl, 4 : dbr}
    if len(diffVars) > 0:
        theVar = GetDiffVarNames(diffVars[0])
        windowsToVars = {1 : theVar[1], 2 : theVar[1], 3 : theVar[0], 4 : theVar[0]}
        for win in (1,2,3,4):
            SetActiveWindow(win)
	    DeleteAllPlots()
            ActivateDatabase(winDbMap[win])
            if win == 2 and cmfeMode == 0:
                continue
            AddPlot("Pseudocolor", windowsToVars[win])
    else:
        print "No plots are being set up by default since the databases did not have any scalars in common."
        sys.exit(5)

    # Set up text annotations.
    windowsToAnnots = {1 : "L-ConnCMFE(R)", 2 : "       Unused", 3 : "      Left-db", 4 : "     Right-db"}
    if cmfeMode == 1:
        windowsToAnnots = {1 : " L-PosCMFE(R)", 2 : " R-PosCMFE(L)", 3 : "      Left-db", 4 : "     Right-db"}
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        SetActiveWindow(win)
        annot = CreateAnnotationObject("Text2D")
        annot.text = windowsToAnnots[win]
        annot.width = 0.2
        annot.position = (0.80,0.95)
        annot.useForegroundForTextColor = 0
        annot.textColor = (255,0,0,255)
        annot.fontBold = 1

    SetActiveWindow(1)
    CreateDatabaseCorrelation("DIFF", (dbl, dbr), 0)

    # Open the GUI
    if diffSummaryOnly == 0:
        OpenGUI()

    # Move the viewer's window area closer to the GUI.
    SetWindowArea(410,0,1100,1100)

    # Register macro only seems to work from window 1
    SetActiveWindow(1)
    RegisterMacro("DiffSummary", DiffSummary)
    RegisterMacro("ToggleMesh", ToggleMesh)
    RegisterMacro("ToggleBoundary", ToggleBoundary)

    RegisterMacro("SyncWinsL-R", SyncWinsL_R)
    RegisterMacro("SyncWinsR-L", SyncWinsR_L)
    RegisterMacro("SyncWinsLeft", SyncWinsLeft) 
    RegisterMacro("SyncWinsRight", SyncWinsRight)

    RegisterMacro("SyncTimeL-R", SyncTimeL_R)
    RegisterMacro("SyncTimeR-L", SyncTimeR_L)
    RegisterMacro("SyncTimeLeft", SyncTimeLeft) 
    RegisterMacro("SyncTimeRight", SyncTimeRight) 

    RegisterMacro("ToggleHidePlot0", ToggleHidePlot0)
    RegisterMacro("ToggleHidePlot1", ToggleHidePlot1)
    RegisterMacro("ToggleHidePlot2", ToggleHidePlot2)
    RegisterMacro("ToggleHidePlot3", ToggleHidePlot3)
    RegisterMacro("ToggleHidePlot4", ToggleHidePlot4)
    RegisterMacro("ToggleHidePlot5", ToggleHidePlot5)

    for win in (1,2,3,4):
        SetActiveWindow(win)
	DrawPlots()
    SetActiveWindow(1)

    if diffSummaryOnly == 0:
        print "Type 'help()' to get more information on using 'visit -diff'"

###############################################################################
# Function: ChangeVar 
#
# Purpose:  Change the currently plotted variable in all windows
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def ChangeVar(new_var):
    varType = GetVarType(mdl, new_var)
    if varType == "Unknown":
        print "Unable to find variable type for variable \"%s\""%new_var
        return
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        SetActiveWindow(win)
        plotToChange = -1
        pl = GetPlotList()
        for p in range(pl.GetNumPlots()):
            plotType = pl.GetPlots(p).plotType
            plotTypeName = PlotPlugins()[plotType]
            if varType == "Material" and \
               (plotTypeName == "Boundary" or \
                plotTypeName == "FilledBoundary"):
                plotToChange = p
            elif varType == "Scalar" and \
               (plotTypeName == "Contour" or \
                plotTypeName == "Histogram" or \
                plotTypeName == "Pseudocolor" or \
                plotTypeName == "Spreadsheet" or \
                plotTypeName == "Surface" or \
                plotTypeName == "Volume"):
                plotToChange = p
            elif varType == "Vector" and \
               (plotTypeName == "Streamline" or \
                plotTypeName == "Vector" or \
                plotTypeName == "Truecolor"):
                plotToChange = p
            elif varType == plotTypeName:
                plotToChange = p
            if plotToChange != -1:
                break
        if plotToChange != -1:
            SetActivePlots((p,))
            if win == 1:
                ChangeActivePlotsVar("diff/%s"%new_var);
            else:
                ChangeActivePlotsVar(new_var);
        else:
            print "Unable to find an existing plot compatible with the variable \"%s\""%new_var
    SetActiveWindow(1)

###############################################################################
# Function: ToggleHidePlot 
#
# Purpose:  Toggle hiding a specified plot id 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def ToggleHidePlot(plotId):

    # determine target of the toggle (to hide or unhide)
    hiddenTarget = 0
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        SetActiveWindow(win)
	plotList = GetPlotList()
	if plotId >= plotList.GetNumPlots():
	    print "Plot id %d is out of range 0...%d"%(plotId,plotList.GetNumPlots()-1)
	    return
	if plotList.GetPlots(plotId).hiddenFlag == 1:
	    hiddenTarget = hiddenTarget - 1
	else:
	    hiddenTarget = hiddenTarget + 1

    # At this point, if hiddenTarget is largely negative, the target
    # state is to UNhide the plots, else hide the plots

    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        SetActiveWindow(win)
	plotList = GetPlotList()
	if plotList.GetPlots(plotId).hiddenFlag == 1:
	    if hiddenTarget <= 0:
	        SetActivePlots((plotId,))
		HideActivePlots()
	else:
	    if hiddenTarget > 0:
	        SetActivePlots((plotId,))
		HideActivePlots()

    SetActiveWindow(1)

def ToggleHidePlot0():
    ToggleHidePlot(0)

def ToggleHidePlot1():
    ToggleHidePlot(1)

def ToggleHidePlot2():
    ToggleHidePlot(2)

def ToggleHidePlot3():
    ToggleHidePlot(3)

def ToggleHidePlot4():
    ToggleHidePlot(4)

def ToggleHidePlot5():
    ToggleHidePlot(5)

###############################################################################
# Function: TogglePlot 
#
# Purpose:  Toggle a specified plot type on/off 
#
# Determine all <plotTypeName> plots to be displayed or hidden based on
# the plot variables currently in window 1. First, find all the
# plots that are <plotTypeName> plots and record their hidden state in
# the plotInfo map. Next, find all the plots that are not <plotTypeName>,
# and see if the associated <plotTypeName> for those plots is already in
# the plotInfo map. If it is, then that variable's <plotTypeName> is already
# present and its status is recorded. Otherwise, that variable's
# <plotTypeName> gets added to the plotInfo map with a status of 0 (!exist)
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def TogglePlot(plotTypeName):
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        plotInfo = {}
        SetActiveWindow(win)
        pl = GetPlotList()
        for p in range(pl.GetNumPlots()):
            plot = pl.GetPlots(p)
            if PlotPlugins()[plot.plotType] == plotTypeName:
                plotName = plot.plotVar
                try:
                   plotName = re.search("diff/(.*)",plot.plotVar).group(1)
                except:
                    try:
                        plotName = re.search("(.*)",plot.plotVar).group(1)
                    except:
                        plotName = plot.plotVar
                if plot.hiddenFlag == 1:
                    plotInfo[plotName] = (1, p) # exists and is hidden
                else:
                    plotInfo[plotName] = (2, p) # exists and is displayed

        #
        # Second pass for the non-<plotTypeName> plots. Will determine only
	# <plotTypeName> plots that need to be added.
        #
        for p in range(pl.GetNumPlots()):
            plot = pl.GetPlots(p)
            if PlotPlugins()[plot.plotType] != plotTypeName:
                varName = plot.plotVar
                try:
                    varName = re.search("diff/(.*)",plot.plotVar).group(1)
                except:
                    try:
                        varName = re.search("(.*)",plot.plotVar).group(1)
                    except:
                        varName = plot.plotVar
                plotName ="Unknown"
                if plotTypeName == "Mesh":
                    plotName = MeshForVar(mdl,varName)
                elif plotTypeName == "Boundary":
                    plotName = MeshForVar(mdl,varName)
                    plotName = MatForMesh(mdl,plotName)
                if plotName == "Unknown":
                    continue
                if plotName not in plotInfo:
                    plotInfo[plotName] = (0, p)

        #
        # At this point, plotInfo is populated with the names of all the <plotTypeName> 
        # plots and whether they are currently non-existant (0), hidden (1) or
        # displayed (2) along with their index (p) in the plot list. So, now,
        # we determine the target state of the TogglePlot command. Should the
        # <plotTypeName> plot(s) be on (that is exist and displayed) or off (not-exist
        # or hidden)? In general, the situation can be mixed at this point and
        # so we determine based on majority status
        #
        if win == 1:
            targetState = 0
            for m in plotInfo.keys():
                if plotInfo[m][0] == 0 or plotInfo[m][0] == 1:
                    targetState = targetState + 1
                else:
                    targetState = targetState - 1

        #
        # First handle toggling of existing plots (hidden or displayed)
        #
        plotsToToggle = []
        for m in plotInfo.keys():
            if targetState > 0 and plotInfo[m][0] == 1:
                plotsToToggle.append(plotInfo[m][1])
            if targetState <= 0 and plotInfo[m][0] == 2:
                plotsToToggle.append(plotInfo[m][1])
        if len(plotsToToggle) > 0:
            SetActivePlots(tuple(plotsToToggle))
            HideActivePlots()

        #
        # Now handle adding new <plotTypeName> plots if needed
        #
        if targetState > 0:
            for m in plotInfo.keys():
                if plotInfo[m][0] == 0:
                    AddPlot(plotTypeName, m)
            DrawPlots()

    SetActiveWindow(1)

def ToggleMesh():
    TogglePlot("Mesh")

def ToggleBoundary():
    TogglePlot("Boundary")

###############################################################################
# Function: MinimizePickOutput 
#
# Purpose:  Reduce output generated by pick on stdout to bare minimum for
#           PickLoop function.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def MinimizePickOutput():
    global pa_orig
    SuppressQueryOutputOn()
    pa_orig = GetPickAttributes()
    pa = pa_orig
    pa.displayIncidentElements = 0
    pa.showNodeId = 0
    pa.showTimeStep = 0
    pa.showMeshName = 0
    pa.showZoneId = 0
    pa.displayPickLetter = 1
    SetPickAttributes(pa)

###############################################################################
# Function: UnMinimizePickOutput 
#
# Purpose:  Undue the reduction in pick output made by MinimizePickOutput.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def UnMinimizePickOutput():
    global pa_orig
    SetPickAttributes(pa_orig)
    SuppressQueryOutputOff()

###############################################################################
# Function: PickLoop 
#
# Purpose:  Perform a zone or node pick over a specified tuple of element ids.
#           Also, handle case where user may have added variables to the 
#           PickAttributes to be returned during picking. Report the output in
#           a useful tabular form.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def PickLoop(ids, pickType):
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        SetActiveWindow(win)
        ClearPickPoints()
    ResetPickLetter()
    s = ["","",""]
    MinimizePickOutput()
    npicks = 1
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        SetActiveWindow(win)
        for id in ids:
            if pickType == "zonal":
                PickByZone(id)
            else:
                PickByNode(id)
            tmp = GetPickOutput()
            picks = []
            if win == 1:
                picks = re.findall("diff/(.*): *<(zonal|nodal)> = ([0-9.e+\-]*)\s*",tmp)
                npicks = len(picks)
                for p in range(len(picks)):
                    s[win-1] = s[win-1] + "%s=%s"%(picks[p][0], picks[p][2]) + ";"
            else:
                picks = re.findall("(.*): *<(zonal|nodal)> = ([0-9.e+\-]*)\s*",tmp)
                for p in range(len(picks)):
                    s[win-1] = s[win-1] + "%s"%picks[p][2] + ";"
    dpicks = s[0].split(";")
    lpicks = s[2].split(";")
    rpicks = s[3].split(";")
    result =          "   id    |       var        |       DIFF       |      dbLeft      |      dbRight     \n"
    result = result + "---------|------------------|------------------|------------------|------------------\n"
    k = 0
    for id in ids:
        for p in range(npicks):
            dsplit = dpicks[k].split("=")
            result = result + "% 9d|% 18s|% 18s|% 18s|% 18s\n"%(id,dsplit[0],dsplit[1],lpicks[k],rpicks[k])
            k = k + 1
#    Disabled for now: winds up poorly formatting the message
#    ClientMethod("MessageBoxOk", result)
    print result
    SetActiveWindow(1)
    UnMinimizePickOutput()

def ZPick(zoneIds):
    PickLoop(zoneIds, "zonal")

def NPick(nodeIds):
    PickLoop(nodeIds, "nodal")

###############################################################################
# Function: SyncWindows 
#
# Purpose:  Bring all the windows up to date with contents of the specified
#           source window. This is done by deleting all the other windows and
#           re-cloning them from the source. Although this is costly, it is
#           the only easy way to ensure that all plots, operators, lighting,
#           etc., are consistent.
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def SyncWindows(srcWin):
    global dbr
    global dbl

    #
    # Get List of active plots
    #
    activePlotsList = []
    hiddenPlotsList = []
    SetActiveWindow(srcWin)
    srcPlots = GetPlotList()
    for p in range(srcPlots.GetNumPlots()):
        if srcPlots.GetPlots(p).activeFlag == 1:
            activePlotsList.append(p)
        if srcPlots.GetPlots(p).hiddenFlag == 1:
            hiddenPlotsList.append(p)

    #
    # Delete the old windows so we can re-clone them
    #
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        if win == srcWin:
            continue
        SetActiveWindow(win)
        DeleteWindow()

    #
    # Clone the src window and adjust variable names
    #
    for win in (1,2,3,4):
        if win == 2 and cmfeMode == 0:
            continue
        if win == srcWin:
            continue
        SetActiveWindow(srcWin)
        CloneWindow()
        SetActiveWindow(win)

	# re-set the annotations
        ao = GetAnnotationObject(1)
        if win == 1:
            ReplaceDatabase(dbl)
	    if cmfeMode == 0:
                ao.text = "L-ConnCMFE(R)"
	    else:
                ao.text = " L-PosCMFE(R)"
        elif win == 2:
            ReplaceDatabase(dbr)
	    if cmfeMode == 0:
                ao.text = "       Unused"
            else:
                ao.text = " R-PosCMFE(L)"
	elif win == 3:
            ReplaceDatabase(dbl)
            ao.text = "      Left-db"
        elif win == 4:
            ReplaceDatabase(dbr)
            ao.text = "     Right-db"

	# reset the plot variables
        plots = GetPlotList()
        for p in range(plots.GetNumPlots()):
            pv = plots.GetPlots(p).plotVar
	    if IsNotScalarVarPlotType(plots.GetPlots(p).plotType):
                continue
            theVar = GetDiffVarNames(pv) 
            if win == 1 and pv == theVar[0]:
                print "Warning: Looks like you are not displaying a diff variable in the DIFF window"
            SetActivePlots((p,))
            if win == 1:
                ChangeActivePlotsVar(theVar[1])
            else:
                ChangeActivePlotsVar(theVar[0])
        DrawPlots()
	hiddenPlotsTmp = tuple(hiddenPlotsList)
	if len(hiddenPlotsTmp) > 0:
	    SetActivePlots(tuple(hiddenPlotsList))
	    HideActivePlots()
        SetActivePlots(tuple(activePlotsList))

    SetActiveWindow(srcWin)

###############################################################################
# Function: SyncWins... 
#
# Purpose:  Stubs to register as macros 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def SyncWinsL_R():
    SyncWindows(1)

def SyncWinsR_L():
    SyncWindows(2)

def SyncWinsLeft():
    SyncWindows(3)

def SyncWinsRight():
    SyncWindows(4)

###############################################################################
# Function: CompareMinMaxInfos 
#
# Purpose:  Sorter function for sorting output from DiffSummary 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
###############################################################################
def CompareMinMaxInfos(a1, a2):
    v1min = abs(a1[1])
    v1max = abs(a1[5])
    v2min = abs(a2[1])
    v2max = abs(a2[5])
    v1 = v1min
    if v1max > v1min:
        v1 = v1max
    v2 = v2min
    if v2max > v2min:
        v2 = v2max 
    if v1 < v2:
        return 1
    elif v1 > v2:
        return -1
    else:
        return 0

###############################################################################
# Function: DiffSummary 
#
# Purpose:  Iterate over all variables in diffVars and report differences. 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
# Modifications:
#   Mark C. Miller, Tue Aug 21 10:03:35 PDT 2007
#   Added calls to disable re-draws and then re-enable to accelerate
#
#   Mark C. Miller, Tue Aug 21 11:17:20 PDT 2007
#   Added support for difference summary mode 
#
###############################################################################
def DiffSummary():
    SetActiveWindow(1)
    DisableRedraw()
    SuppressQueryOutputOn()
    diffSummary = []
    for v in diffVars:
        vname = re.search("diff/(.*)",v)
        if vname != None:
            vname = vname.group(1)
        else:
            vname = v
        if diffSummaryOnly == 1:
	    print "Processing variable \"%s\""%v
        AddPlot("Pseudocolor", v)
        DrawPlots()
        Query("MinMax")
        qo = GetQueryOutputString()
        qv = GetQueryOutputValue()
        mininfo = re.search("Min = ([0-9.e+\-]*) \((node|zone) ([0-9]*) (in domain ([0-9]*) at|at())",qo)
        maxinfo = re.search("Max = ([0-9.e+\-]*) \((node|zone) ([0-9]*) (in domain ([0-9]*) at|at())",qo)
#                       val             node|zone             elem-id          dom-id
#                 0     1/5                2/6                  3/7              4/8
        if mininfo != None and maxinfo != None:
            diffSummary.append( \
                (vname[-12:], qv[0], mininfo.group(2), mininfo.group(3), mininfo.group(5), \
                        qv[1], maxinfo.group(2), maxinfo.group(3), maxinfo.group(5)))
        else:
            diffSummary.append((vname[-12:], 0.0, "Unknown", "Unknown", "Unknown", \
                                       0.0, "Unknown", "Unknown", "Unknown"))
        #time.sleep(0.5)
        DeleteActivePlots()
    SuppressQueryOutputOff()
    print "Difference Summary sorted in decreasing difference magnitude...\n"
    print "NOTE: Differences are computed in only single precision"
    print "    var     |max -diff   | max -elem  ; -dom  |max +diff   | max +elem  ;  +dom |"
    print "------------|------------|--------------------|------------|--------------------|"
    diffSummary.sort(CompareMinMaxInfos)
    for k in range(len(diffSummary)):
        if diffSummary[k][1] == 0.0 and diffSummary[k][5] == 0.0:
            print "% 12.12s| NO DIFFERENCES"%diffSummary[k][0]
        else:
            print "% 12.12s|%+12.7f|%4s % 7s;% 7s|%+12.7f|%4s % 7s;% 7s|"%diffSummary[k]
    RedrawWindow()

###############################################################################
# Main program and global variables 
#
# Programmer: Mark C. Miller 
# Date:       Wed Jul 18 18:12:28 PDT 2007 
#
# Modifications:
#   Mark C. Miller, Tue Aug 21 11:17:20 PDT 2007
#   Added support for difference summary mode 
###############################################################################
diffVars = []
dbl = "notset"
dbr = "notset"
mdl = 0
mdr = 0
forcePosCMFE = 0
diffSummaryOnly = 0
cmfeMode = 0
currentTimeState = -1

ProcessCLArgs()
Initialize()
if diffSummaryOnly == 1:
    DiffSummary()
    sys.exit()
