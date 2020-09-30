# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests: Ensures VisIt and/or the test harness is in the correct "mode".
#
#  Mark C. Miller, Mon Sep 28 16:24:44 PDT 2020
# ----------------------------------------------------------------------------
import json
import sys

#
# Obtain mode info direct from command-line args instead of using
# any of the logic in the test harness. This ensures we compare
# actual mode requested in command-line to behavior
#
def ModeKeys():
    clargs = json.loads(TestEnv.params["clargs"])
    for i in range(len(clargs)):
        if clargs[i] == '-m':
            return clargs[i+1].split(',')
        elif clargs[i][0:7] == '--mode=':
            return clargs[i][8:].split(',')
    return ('serial',)

#
# Confirm all mode strings
#
def AllModeKeysRecognized():
    knownModeKeys = json.loads(TestEnv.params["mode_keys"])
    for m in ModeKeys():
        if m not in knownModeKeys:
            return False
    return True

#
# Check that engine matches the specified mode
#
def EngineMatchesMode():
    pa = GetProcessAttributes("engine")
    if 'parallel' in ModeKeys():
        if not pa.isParallel:
            return False
        if len(pa.pids) < 2:
            return False
        return True
    else:
        if pa.isParallel:
            return False
        if len(pa.pids) > 1:
            return False
        return True

#
# Check that Silo data path matches its mode
#
def SiloDataPathMatchesMode():
    if 'pdb' in ModeKeys():
        if 'silo_pdb_test_data' not in silo_data_path(''):
            return False
    else:
        if 'silo_hdf5_test_data' not in silo_data_path(''):
            return False
    return True

#
# Ensure rendering mode matches mode
#
def ScalableSettingMatchesMode():
    ra = GetRenderingAttributes()
    if 'scalable' in ModeKeys():
        if ra.scalableActivationMode != ra.Always:
            return False
    else:
        if ra.scalableActivationMode != ra.Never:
            return False
    return True

def IcetClargMatchesMode():
    if 'icet' in ModeKeys():
        if '-icet' not in sys.argv:
            return False
    return True

def AllowdynamicClargMatchesMode():
    if 'dlb' in ModeKeys():
        if '-allowdynamic' not in sys.argv:
            return False
    return True

AssertTrue("All mode strings recognized", AllModeKeysRecognized())
AssertTrue("Engine matches mode", EngineMatchesMode())
AssertTrue("Silo data path matches mode", SiloDataPathMatchesMode())
AssertTrue("Scalable setting matches mode", ScalableSettingMatchesMode())
AssertTrue("Icet clarg matches mode", IcetClargMatchesMode())
AssertTrue("Allowdynamic clarg matches mode", AllowdynamicClargMatchesMode())

Exit()
