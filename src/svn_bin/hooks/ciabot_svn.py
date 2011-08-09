#!/usr/bin/env python
#
# This is a CIA client script for Subversion repositories, written in python.
# It generates commit messages using CIA's XML format, and can deliver them
# using either XML-RPC or email. See below for usage and cuztomization
# information.
#
# --------------------------------------------------------------------------
#
# Copyright (c) 2004-2007, Micah Dowty
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
# This script is cleaner and much more featureful than the shell
# script version, but won't work on systems without Python.
#
# To use the CIA bot in your Subversion repository...
#
# 1. Customize the parameters below
#
# 2. This script should be called from your repository's post-commit
#    hook with the repository and revision as arguments. For example,
#    you could copy this script into your repository's "hooks" directory
#    and add something like the following to the "post-commit" script,
#    also in the repository's "hooks" directory:
#
#      REPOS="$1"
#      REV="$2"
#      $REPOS/hooks/ciabot_svn.py "$REPOS" "$REV" &
#
#    Or, if you have multiple project hosted, you can add each
#    project's name to the commandline in that project's post-commit
#    hook:
#
#      $REPOS/hooks/ciabot_svn.py "$REPOS" "$REV" "ProjectName" &
#
############# There are some parameters for this script that you can customize:

class config:
    # Replace this with your project's name, or always provide a project
    # name on the commandline.
    #
    # NOTE: This shouldn't be a long description of your project. Ideally
    #       it is a short identifier with no spaces, punctuation, or
    #       unnecessary capitalization. This will be used in URLs related
    #       to your project, as an internal identifier, and in IRC messages.
    #       If you want a longer name shown for your project on the web
    #       interface, please use the "title" metadata key rather than
    #       putting that here.
    #
    project = "VisIt"

    # Subversion's normal directory hierarchy is powerful enough that
    # it doesn't have special methods of specifying modules, tags, or
    # branches like CVS does.  Most projects do use a naming
    # convention though that works similarly to CVS's modules, tags,
    # and branches.
    #
    # This is a list of regular expressions that are tested against
    # paths in the order specified. If a regex matches, the 'branch'
    # and 'module' groups are stored and the matching section of the
    # path is removed.
    #
    # Several common directory structure styles are below as defaults.
    # Uncomment the ones you're using, or add your own regexes.
    # Whitespace in the each regex are ignored.

    pathRegexes = [
       r"^ trunk/           (?P<module>[^/]+)/ ",
       r"^ (branches|tags)/ (?P<branch>[^/]+)/ ",
#       r"^ (branches|tags)/ (?P<module>[^/]+)/ (?P<branch>[^/]+)/ ",
    ]

    # If your repository is accessible over the web, put its base URL here
    # and 'uri' attributes will be given to all <file> elements. This means
    # that in CIA's online message viewer, each file in the tree will link
    # directly to the file in your repository.
    repositoryURI = "http://portal.nersc.gov/svn/visit/"

    # If your repository is accessible over the web via a tool like ViewVC 
    # that allows viewing information about a full revision, put a format string
    # for its URL here. You can specify various substitution keys in the Python
    # syntax: "%(project)s" is replaced by the project name, and likewise
    # "%(revision)s" and "%(author)s" are replaced by the revision / author.
    # The resulting URI is added to the data sent to CIA. After this, in CIA's
    # online message viewer, the commit will link directly to the corresponding
    # revision page.
    revisionURI = None
    # Example (works for ViewVC as used by SourceForge.net):
    #revisionURI = "https://svn.sourceforge.net/viewcvs.cgi/%(project)s?view=rev&rev=%(revision)s"

    # This can be the http:// URI of the CIA server to deliver commits over
    # XML-RPC, or it can be an email address to deliver using SMTP. The
    # default here should work for most people. If you need to use e-mail
    # instead, you can replace this with "cia@cia.navi.cx"
    server = "http://cia.navi.cx"

    # The SMTP server to use, only used if the CIA server above is an
    # email address.
    smtpServer = "localhost"

    # The 'from' address to use. If you're delivering commits via email, set
    # this to the address you would normally send email from on this host.
    fromAddress = "fogal1@localhost"

    # When nonzero, print the message to stdout instead of delivering it to CIA.
    debug = 0


############# Normally the rest of this won't need modification

import sys, os, re, urllib, getopt

class File:
    """A file in a Subversion repository. According to our current
    configuration, this may have a module, branch, and URI in addition
    to a path."""

    # Map svn's status letters to our action names
    actionMap = {
        'U': 'modify',
        'A': 'add',
        'D': 'remove',
        }

    def __init__(self, fullPath, status=None):
        self.fullPath = fullPath
        self.path = fullPath
        self.action = self.actionMap.get(status)

    def getURI(self, repo):
        """Get the URI of this file, given the repository's URI. This
        encodes the full path and joins it to the given URI."""
        quotedPath = urllib.quote(self.fullPath)
        if quotedPath[0] == '/':
            quotedPath = quotedPath[1:]
        if repo[-1] != '/':
            repo = repo + '/'
        return repo + quotedPath

    def makeTag(self, config):
        """Return an XML tag for this file, using the given config"""
        attrs = {}

        if config.repositoryURI is not None:
            attrs['uri'] = self.getURI(config.repositoryURI)

        if self.action:
            attrs['action'] = self.action

        attrString = ''.join([' %s="%s"' % (key, escapeToXml(value,1))
                              for key, value in attrs.items()])
        return "<file%s>%s</file>" % (attrString, escapeToXml(self.path))


class SvnClient:
    """A CIA client for Subversion repositories. Uses svnlook to
    gather information"""
    name = 'Python Subversion client for CIA'
    version = '1.20'

    def __init__(self, repository, revision, config):
        self.repository = repository
        self.revision = revision
        self.config = config

    def deliver(self, message):
        if config.debug:
            print message
        else:
            server = self.config.server
            if server.startswith('http:') or server.startswith('https:'):
                # Deliver over XML-RPC
                import xmlrpclib
                xmlrpclib.ServerProxy(server).hub.deliver(message)
            else:
                # Deliver over email
                import smtplib
                smtp = smtplib.SMTP(self.config.smtpServer)
                smtp.sendmail(self.config.fromAddress, server,
                              "From: %s\r\nTo: %s\r\n"
                              "Subject: DeliverXML\r\n\r\n%s" %
                              (self.config.fromAddress, server, message))

    def main(self):
        self.collectData()
        self.deliver("<message>" +
                     self.makeGeneratorTag() +
                     self.makeSourceTag() +
                     self.makeBodyTag() +
                     "</message>")

    def makeAttrTags(self, *names):
        """Given zero or more attribute names, generate XML elements for
           those attributes only if they exist and are non-None.
           """
        s = ''
        for name in names:
            if hasattr(self, name):
                v = getattr(self, name)
                if v is not None:
                    # Recent Pythons don't need this, but Python 2.1
                    # at least can't convert other types directly
                    # to Unicode. We have to take an intermediate step.
                    if type(v) not in (type(''), type(u'')):
                        v = str(v)
                    
                    s += "<%s>%s</%s>" % (name, escapeToXml(v), name)
        return s

    def makeGeneratorTag(self):
        return "<generator>%s</generator>" % self.makeAttrTags(
            'name',
            'version',
            )

    def makeSourceTag(self):
        return "<source>%s</source>" % self.makeAttrTags(
            'project',
            'module',
            'branch',
            )

    def makeBodyTag(self):
        return "<body><commit>%s%s</commit></body>" % (
            self.makeAttrTags(
            'revision',
            'author',
            'log',
            'diffLines',
            'url',
            ),
            self.makeFileTags(),
            )

    def makeFileTags(self):
        """Return XML tags for our file list"""
        return "<files>%s</files>" % ''.join([file.makeTag(self.config)
                                              for file in self.files])

    def svnlook(self, command):
        """Run the given svnlook command on our current repository and
        revision, returning all output"""
        # We have to set LC_ALL to force svnlook to give us UTF-8 output,
        # then we explicitly slurp that into a unicode object.
        return unicode(os.popen(
            'LC_ALL="en_US.UTF-8" svnlook %s -r "%s" "%s"' %
            (command, self.revision, self.repository)).read(),
            'utf-8', 'replace')

    def collectData(self):
        self.author = self.svnlook('author').strip()
        self.project = self.config.project
        self.log = self.svnlook('log')
        self.diffLines = len(self.svnlook('diff').split('\n'))
        self.files = self.collectFiles()
        if self.config.revisionURI is not None:
            self.url = self.config.revisionURI % self.__dict__
        else:
            self.url = None

    def collectFiles(self):
        # Extract all the files from the output of 'svnlook changed'
        files = []
        for line in self.svnlook('changed').split('\n'):
            path = line[2:].strip()
            if path:
                status = line[0]
                files.append(File(path, status))

        # Try each of our several regexes. To be applied, the same
        # regex must mach every file under consideration and they must
        # all return the same results. If we find one matching regex,
        # or we try all regexes without a match, we're done.
        matchDict = None
        for regex in self.config.pathRegexes:
            matchDict = matchAgainstFiles(regex, files)
            if matchDict is not None:
                self.__dict__.update(matchDict)
                break

        return files


def matchAgainstFiles(regex, files):
    """Try matching a regex against all File objects in the provided list.
       If the regex returns the same matches for every file, the matches
       are returned in a dict and the matched portions are filtered out.
       If not, returns None.
       """
    prevMatchDict = None
    compiled = re.compile(regex, re.VERBOSE)
    for f in files:

        match = compiled.match(f.fullPath)
        if not match:
            # Give up, it must match every file
            return None

        matchDict = match.groupdict()
        if prevMatchDict is not None and prevMatchDict != matchDict:
            # Give up, we got conflicting matches
            return None

        prevMatchDict = matchDict

    # If we got this far, the regex matched every file with
    # the same results.  Now filter the matched portion out of
    # each file and store the matches we found.
    for f in files:
        f.path = compiled.sub('', f.fullPath)
    return prevMatchDict


def escapeToXml(text, isAttrib=0):
    text = unicode(text)
    text = text.replace("&", "&amp;")
    text = text.replace("<", "&lt;")
    text = text.replace(">", "&gt;")
    if isAttrib == 1:
        text = text.replace("'", "&apos;")
        text = text.replace("\"", "&quot;")
    return text


def usage():
    """Print a short usage description of this script and exit"""
    sys.stderr.write("Usage: %s [OPTIONS] REPOS-PATH REVISION [PROJECTNAME]\n" %
                      sys.argv[0])


def version():
    """Print out the version of this script"""
    sys.stderr.write("%s %s\n" % (sys.argv[0], SvnClient.version))


def main():
    try:
        options = [ "version" ]
        for key in config.__dict__:
            if not key.startswith("_"):
                options.append(key + "=");
        opts, args = getopt.getopt(sys.argv[1:], "", options)
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    
    for o, a in opts:
        if o == "--version":
            version()
            sys.exit()
        else:
            # Everything else maps straight to a config key. Just have
            # to remove the "--" prefix from the option name.
            config.__dict__[o[2:]] = a

    # Print a usage message when not enough parameters are provided.
    if not len(args) in (2,3):
        sys.stderr.write("%s: incorrect number of arguments\n" % sys.argv[0])
        usage();
        sys.exit(2);

    # If a project name was provided, override the default project name.
    if len(args) == 3:
        config.project = args[2]

    # Go do the real work.
    SvnClient(args[0], args[1], config).main()


if __name__ == "__main__":
    main()

### The End ###
