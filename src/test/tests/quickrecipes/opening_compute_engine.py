# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def opening_a_compute_engine1():
 
  try:
    # opening a compute engine 1 {
    # Open a local, parallel compute engine before opening a database
    # Use 4 processors on 2 nodes
    OpenComputeEngine("localhost", ("-np", "4", "-nn", "2"))
    OpenDatabase("~juanita/silo/stuff/multi_ucd3d.silo")
    # opening a compute engine 1 }
    TestValueEQ('opening a compute engine 1 error message',GetLastError(),'')
    TestPOA('opening a compute engine 1 exceptions')
  except:
    TestFOA('opening a compute engine 1 exception', LINE())
    pass
  vqr_cleanup()
  CloseComputeEngine(GetLocalHostName())

def opening_a_compute_engine2():
 
  try:
    # opening a compute engine 2 {
    # Use the "srun" job launcher, the "batch" partition, the "mybank" bank,
    # 72 processors on 2 nodes and a time limit of 1 hour
    OpenComputeEngine("localhost",
        ("-l", "srun", "-p", "batch", "-b", "mybank",
         "-np", "72", "-nn", "2", "-t", "1:00:00"))
    # opening a compute engine 2 }
    TestValueEQ('opening a compute 2 engine error message',GetLastError(),'')
    TestPOA('opening a compute engine 2 exceptions')
  except:
    TestFOA('opening a compute engine 2 exception', LINE())
    pass
  CloseComputeEngine("localhost")

CloseComputeEngine(GetLocalHostName())
opening_a_compute_engine1()
opening_a_compute_engine2()

Exit()
