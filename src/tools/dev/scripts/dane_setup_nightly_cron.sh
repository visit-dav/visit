#!/bin/sh

# copy current scripts to shared exe location in /usr/WS1/visit

cp dane_cron_script_run_test_suite.sh dane_cron_script_change_perms.sh /usr/WS1/visit/test_trunk/nightly_cron/

# Set the permissions so that others may access the test directory.
chgrp -R visit /usr/WS1/visit/test_trunk/nightly_cron/
chmod -R g+rwX,o+rX /usr/WS1/visit/test_trunk/nightly_cron/

# install crontab
crontab dane_crontab

