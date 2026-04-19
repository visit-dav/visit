#!/bin/sh
#
# Cron driver script to run visit's test suite
#
# WARNING WARNING WARNING WARNING WARNING WARNING
#
#   If you change this file you will need to manually copy it to the
#   directory where the crontab runs this script. Currently that is
#   /usr/WS1/visit/test_trunk/nightly_cron on the RZ. You should also
#   make sure the permissions are 775.
#
# WARNING WARNING WARNING WARNING WARNING WARNING

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

