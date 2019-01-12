def MatChecker (matname, meshname):
  # We want to get a SIL, but the only thing we have access to is the
  # materials object.  So add a Filled Boundary plot.
  AddPlot("FilledBoundary", matname)
  sil = SILRestriction()
  mats = sil.SetsInCategory(matname)
  matnames=[]
  for i in range(len(mats)):
     matnames = matnames + [sil.SetName(mats[i])]
  DeleteAllPlots()
  
  # We are going to want to look at the zones on the material interface only.
  # Set up some constructs that will allow us to do this.
  nmats_defn = "nmats(%s)" %(matname)
  DefineScalarExpression("nmats", nmats_defn)
  t = ThresholdAttributes()
  t.variable = "nmats"
  t.lbound = 1.5
  SetDefaultOperatorOptions(t)
  
  # Now go over each material and collect the information.
  report=""
  for i in range(len(mats)):
    # Start off by calculating the volume fraction times the volume for all
    # zones.  This will be stored in "prod".
    matvf_defn = "matvf(%s, %s)" %(matname, matnames[i])
    volume_defn = "volume(%s)" %(meshname)
    prod_name = "prod_%s" %(matname)
    prod_defn = "%s * %s" %(matvf_defn, volume_defn)
    print "Prod = %s" %(prod_defn)
    DefineScalarExpression(prod_name, prod_defn)
    AddPlot("Pseudocolor", prod_name)
    sil = SILRestriction()
    sil.TurnOnAll()
    SetPlotSILRestriction(sil)
    DrawPlots()
    Query("Variable Sum")
    prod = GetQueryOutputValue()
    
    # Now get the same information at the zones on the material interface.
    AddOperator("Threshold")
    Query("Variable Sum")
    prod_interface = GetQueryOutputValue()
    RemoveLastOperator()

    # Now calculate the volume using our material interface reconstruction
    # algorithm.  This will be stored in "vol".
    sil.TurnOffAll()
    sil.TurnOnSet(mats[i])
    SetPlotSILRestriction(sil)
    Query("Volume")
    vol = GetQueryOutputValue()
    
    # Now get the same information at the zones on the material interface.
    AddOperator("Threshold")
    Query("Volume")
    vol_interface = GetQueryOutputValue()
    RemoveLastOperator()

    # Now print out the statistics.
    report += "For material %s:\n" %(matnames[i])
    report += "\tOver the whole problem\n"
    report += "\t\tProduct of \"material volume fraction\" * volume = %f\n" %(prod)
    report += "\t\tVolume using material interface reconstruction = %f\n" %(vol)
    if (vol != 0.):
        report += "\t\tRatio = %f\n" %(prod/vol)
    report += "\tConsidering only zones along the material interface\n"
    report += "\t\tProduct of \"material volume fraction\" * volume = %f\n" %(prod_interface)
    report += "\t\tVolume using material interface reconstruction = %f\n" %(vol_interface)
    if (vol_interface != 0.):
        report += "\t\tRatio = %f\n" %(prod_interface/vol_interface)
    DeleteAllPlots()
    DeleteExpression(prod_name)
  print report

