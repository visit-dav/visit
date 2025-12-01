#!/bin/sh

# Change directories to the working directory.
cd /usr/workspace/visit/test_trunk/nightly_cron/

# Get the latest version of the test script
rm -f regressiontest_dane
wget https://raw.githubusercontent.com/visit-dav/visit/develop/src/tools/dev/scripts/regressiontest_dane

# Run the test suite.
chmod 755 regressiontest_dane
./regressiontest_dane
