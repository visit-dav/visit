#!/bin/sh
#
# Cron driver script to run visit's test suite
#

# Change directories to the working directory.
cd /usr/WS1/visit/test_trunk/nightly_cron

# Get the latest version of the test script
# Kinda chicken-egg scenario since the first thing we do is git-update, but
# we need the latest script before that happens.

rm -f regressiontest
wget https://raw.githubusercontent.com/visit-dav/visit/develop/src/tools/dev/scripts/regressiontest

# Run the test suite.
chmod 755 regressiontest
./regressiontest

