# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case: expr_loop.py
#
#  Tests: Suspend/Resume GUI Updates to ensure expression list updating in 
#  the GUI is being properly suspended and resumed.
#
#  Mark C. Miller, Wed Dec 18 16:08:48 PST 2024
# ----------------------------------------------------------------------------
import time, os

def time_def_exprs(stem, n):
    t1 = time.time()
    for i in range(n):
        DefineScalarExpression(stem % i, "d*%d" % i)
    t2 = time.time()
    return float(t2-t1)

def del_exprs(stem, n):
    for i in range(n):
        DeleteExpression(stem % i)

def main():
    nexprs = 650
    if os.uname().sysname.lower() == 'darwin':
        nexprs = 200 

    OpenDatabase(silo_data_path("rect2d.silo"))
    OpenGUI()
    time.sleep(5)

    #
    # Check timing without suspending gui
    #
    t1 = time_def_exprs("var%04d", nexprs)
    print(t1)
    n1 = len(Expressions())
    SuspendGUIUpdates()
    del_exprs("var%04d", nexprs)

    #
    # Check timing WITH suspending gui
    #
    SuspendGUIUpdates()
    t2 = time_def_exprs("var%04d", nexprs)
    print(t2)
    ResumeGUIUpdates()

    #
    # Ensure that expressions exist after the suspend/resume step
    #
    n2 = len(Expressions())

    TestValueEQ("Expression count",n1,n2)
    TestValueLE("Expression timing",t2,t1/5)

main()

Exit()
