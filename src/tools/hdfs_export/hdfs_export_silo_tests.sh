#!/bin/tcsh
rm -rf silo_export.out
tail -f silo_eport.out &
setenv USER miller86
ls ~/silo/4.10.3RC/tests/silo_test_suite_files/*.{silo,visit} | sort | head -n 43 | xargs -n 1 -i /usr/gapps/visit/bin/visit -nowin -cli -s ./hdfs_export_driver.py '{}' silo_root >>& silo_export.out
setenv USER brugger1
ls ~/silo/4.10.3RC/tests/silo_test_suite_files/*.{silo,visit} | sort | tail -n 11 | xargs -n 1 -i /usr/gapps/visit/bin/visit -nowin -cli -s ./hdfs_export_driver.py '{}' silo_root >>& silo_export.out
