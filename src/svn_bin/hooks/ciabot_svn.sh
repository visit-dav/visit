#!/bin/sh
#
# This is a CIA bot client script for Subversion repositories,
# delivering via email. See below for usage and customization information.
#
# --------------------------------------------------------------------------
#
# Copyright (c) 2004-2005, Micah Dowty
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * The name of the author may not be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# --------------------------------------------------------------------------
#
# See http://cia.navi.cx for more information on what the CIA bot
# is and how it works, and links to other client scripts.
#
# To use the CIA bot in your Subversion repository...
#
# 1. Customize the parameters below, specifically the ones under
#    the "Project information" section
#
# 2. This script should be called from your repository's post-commit
#    hook with the repository and revision as arguments. For example,
#    you could copy this script into your repository's "hooks" directory
#    and add something like the following to the "post-commit" script,
#    also in the repository's "hooks" directory:
#
#      REPOS="$1"
#      REV="$2"
#      $REPOS/hooks/ciabot_svn.sh "$REPOS" "$REV"&
#
# ------ There are some parameters for this script that you can customize:

# Project information
#
# NOTE: This shouldn't be a long description of your project. Ideally
#       it is a short identifier with no spaces, punctuation, or
#       unnecessary capitalization. This will be used in URLs related
#       to your project, as an internal identifier, and in IRC messages.
#       If you want a longer name shown for your project on the web
#       interface, please use the "title" metadata key rather than
#       putting that here.
#
project_name="VisIt"
return_address="tfogal@alumni.unh.edu"

# System
sendmail_command="/usr/sbin/sendmail -t"

############# Below this line you shouldn't have to change anything

# Script arguments
REPOS="$1"
REV="$2"

# The email address CIA lives at
cia_address="cia@cia.navi.cx"

author=`svnlook author -r "$REV" "$REPOS" | sed 's/\&/\&amp;/g;s/</\&lt;/g;s/>/\&gt;/g'`
log=`svnlook log -r "$REV" "$REPOS" | sed 's/\&/\&amp;/g;s/</\&lt;/g;s/>/\&gt;/g'`
diff_lines=`svnlook diff -r "$REV" "$REPOS" | wc -l`
for file in `svnlook changed -r "$REV" "$REPOS" | cut -c 3- | sed 's/\&/\&amp;/g;s/</\&lt;/g;s/>/\&gt;/g'`; do
    files="$files<file>$file</file>"
done

# Send an email with the final XML message
(cat <<EOF
From: $return_address
To: $cia_address
Subject: DeliverXML

<message>
    <generator>
        <name>Subversion CIA Bot client shell script</name>
        <version>1.1</version>
    </generator>
    <source>
        <project>$project_name</project>
    </source>
    <body>
        <commit>
            <revision>$REV</revision>
            <author>$author</author>
            <files>$files</files>
            <log>$log</log>
            <diffLines>$diff_lines</diffLines>
        </commit>
    </body>
</message>
EOF
) | $sendmail_command

### The End ###
