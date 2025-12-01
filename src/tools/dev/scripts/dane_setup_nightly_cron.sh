#!/bin/sh

# copy current scripts to shared exe location in /usr/workspace/visit

cp dane_cron_script_run_test_suite.sh dane_cron_script_change_perms.sh /usr/workspace/visit/test_trunk/nightly_cron/

# Set the permissions so that others may access the test directory.
chgrp -R visit /usr/workspace/visit/test_trunk/nightly_cron/
chmod -R g+rwX,o+rX /usr/workspace/visit/test_trunk/nightly_cron/

# install crontab
crontab dane_crontab

