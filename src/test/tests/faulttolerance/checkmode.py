# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests: Ensures VisIt and/or the test harness is in the correct "mode".
#
#  Mark C. Miller, Mon Sep 28 16:24:44 PDT 2020
#
# Modifications:
#    Mark C. Miller, Mon Jan 11 10:32:17 PST 2021
#    Replace AssertTrue() with TestValueEQ(..., True)
# ----------------------------------------------------------------------------
import json
import sys

#
# Obtain mode info directly from top-level command-line args passed to test
# suite harness and stored to "clargs" member of TestEnv.params instead of
# using any of the alternative logic in the test harness. This ensures we
# compare with actual mode(s) requested on command-line.
#
def GetModeKeysFromClArgs():
    clargs = json.loads(TestEnv.params["clargs"])
    for i in range(len(clargs)):
        if clargs[i] == '-m':
            return clargs[i+1].split(',')
        elif clargs[i][0:7] == '--mode=':
            return clargs[i][8:].split(',')
    return ('serial',)

#
# Ensure all mode keys are compatible
#
def AllModeKeysCompatible():
    if 'serial' in activeModeKeys and 'parallel' in activeModeKeys:
        return False
    if 'pdb' in activeModeKeys and 'hdf5' in activeModeKeys:
        return False
    if 'icet' in activeModeKeys and 'parallel' not in activeModeKeys:
        return False
    return True

#
# Check that engine matches the specified mode
#
def EngineMatchesMode():
    pa = GetProcessAttributes("engine")
    if 'parallel' in activeModeKeys:
        if pa.isParallel:
            if len(pa.pids) > 1:
                return True
    else:
        if not pa.isParallel:
            if len(pa.pids) == 1:
                return True
    return False

#
# Check that Silo data path matches its mode
#
def SiloDataPathMatchesMode():
    if 'pdb' in activeModeKeys:
        if 'silo_pdb_test_data' in silo_data_path(''):
            return True
    else:
        if 'silo_hdf5_test_data' in silo_data_path(''):
            return True
    return False

#
# Ensure rendering mode matches mode
#
def ScalableSettingMatchesMode():
    ra = GetRenderingAttributes()
    if 'scalable' in activeModeKeys:
        if ra.scalableActivationMode == ra.Always:
            return True
    else:
        if ra.scalableActivationMode == ra.Never:
            return True
    return False

def IcetClargMatchesMode():
    if 'icet' in activeModeKeys and '-icet' not in sys.argv:
        return False
    if '-icet' in sys.argv and 'icet' not in activeModeKeys:
        return False
    return True

def AllowdynamicClargMatchesMode():
    if 'dlb' in activeModeKeys and '-allowdynamic' not in sys.argv:
        return False
    if '-allowdynamic' in sys.argv and 'dlb' not in activeModeKeys:
        return False
    return True

#
# Capture clargs and known mode keys from raw data entries stored
# to TestEnv.params. Do this once, now, instead of each time we
# need to interrogate their contents in the above functions.
#
activeModeKeys = GetModeKeysFromClArgs()

TestValueEQ("All mode strings compatible", AllModeKeysCompatible(), True)
TestValueEQ("Engine matches mode", EngineMatchesMode(), True)
TestValueEQ("Silo data path matches mode", SiloDataPathMatchesMode(), True)
TestValueEQ("Scalable setting matches mode", ScalableSettingMatchesMode(), True)
TestValueEQ("Icet clarg matches mode", IcetClargMatchesMode(), True)
TestValueEQ("Allowdynamic clarg matches mode", AllowdynamicClargMatchesMode(), True)

Exit()
