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
    for i in range(len(clArgs)):
        if clArgs[i] == '-m':
            return clArgs[i+1].split(',')
        elif clArgs[i][0:7] == '--mode=':
            return clArgs[i][8:].split(',')
    return ('serial',)

#
# Confirm all mode strings
#
def AllModeKeysRecognized():
    for m in ModeKeys():
        if m not in knownModeKeys:
            return False
    return True

#
# Ensure no mutually exclusive modes
#
def NoMutuallyExclusiveModes():
    if 'pdb' in ModeKeys() and 'hdf5' in ModeKeys():
        return False
    if 'icet' in ModeKeys() and 'parallel' not in ModeKeys():
        return False
    return True

#
# Check that engine matches the specified mode
#
def EngineMatchesMode():
    pa = GetProcessAttributes("engine")
    if 'parallel' in ModeKeys():
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
    if 'pdb' in ModeKeys():
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
    if 'scalable' in ModeKeys():
        if ra.scalableActivationMode == ra.Always:
            return True
    else:
        if ra.scalableActivationMode == ra.Never:
            return True
    return False

def IcetClargMatchesMode():
    if 'icet' in ModeKeys():
        if '-icet' in sys.argv:
            return True
    return False

def AllowdynamicClargMatchesMode():
    if 'dlb' in ModeKeys():
        if '-allowdynamic' in sys.argv:
            return True
    return False

clArgs = json.loads(TestEnv.params["clargs"])
knownModeKeys = json.loads(TestEnv.params["mode_keys"])

AssertTrue("All mode strings recognized", AllModeKeysRecognized())
AssertTrue("No mutually exclusive modes", NoMutuallyExclusiveModes())
AssertTrue("Engine matches mode", EngineMatchesMode())
AssertTrue("Silo data path matches mode", SiloDataPathMatchesMode())
AssertTrue("Scalable setting matches mode", ScalableSettingMatchesMode())
AssertTrue("Icet clarg matches mode", IcetClargMatchesMode())
AssertTrue("Allowdynamic clarg matches mode", AllowdynamicClargMatchesMode())

Exit()
