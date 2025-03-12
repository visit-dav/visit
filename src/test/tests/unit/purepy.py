# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Pure python CLI methods
#
#  Tests:      Any CLI methods implemented in pure python
#
#  Defect ID:  none
#
#  Programmer: Mark C. Miller, Sun Jan 10 10:24:59 PST 2021
#
# ----------------------------------------------------------------------------

# Basic cases
def get_active_window():
    TestSection("GetActiveWindow")

    SetWindowLayout(4) # creates 2x2 arrangement of 4 windows numbered (1,2,3,4)
    SetActiveWindow(2)
    DeleteWindow()     # After, active window is 1 and available windows are (1,3,4)
    SetActiveWindow(3)
    aw = GetActiveWindow()
    TestValueEQ("active_window_id_a",aw,3)
    DeleteWindow()     # After, active window is 1 and available windows are (1,4)
    aw = GetActiveWindow()
    TestValueEQ("active_window_id_b",aw,1)
    SetActiveWindow(4)
    aw = GetActiveWindow()
    TestValueEQ("active_window_id_c",aw,4)
    
def main():
    get_active_window()

main()
Exit()
