#!/usr/bin/env python
#
# file: Connection.py
# author: Cyrus Harrison <cyrush@llnl.gov>
# created: 6/1/2010
# purpose:
#  Provides a 'Connection' class that interacts with a redmine instance to
#  extract results from redmine queries.
#

import urllib2,urllib,csv,getpass,warnings
from collections import namedtuple

from Issue import *

try:
    import pyPdf
except:
    print "Warning: pyrmine requires the 'pyPdf' ",
    print "module for full pdf functionality."

class Connection(object):
    def __init__(self,base_url):
        """
        Creates a redmine connection object to redmine instance at the given
        url.
        """
        self.urls = {}
        if base_url[-1] == "/":
            base_url = base_url[:-1]
        self.urls["base"] = base_url
        self.urls["login"] = "%s/login/" % base_url
        self.opener  = urllib2.build_opener(urllib2.HTTPCookieProcessor())
    def login(self,uname=None,passwd=None):
        """
        Login handshake.
        If username & passwd are not given this function asks for them via
        stdout/stdin.
        """
        if uname is None:
            uname = raw_input("username:")
        if passwd is None:
            passwd = getpass.getpass("password:")
        f = self.opener.open(self.urls["login"])
        data = f.read()
        f.close()
        split_key = '<input name="authenticity_token" type="hidden" value="'
        data = data.split(split_key)[1]
        atok= data.split('" />')[0]
        params = dict(username=uname,
                      password=passwd,
                      authenticity_token=atok)
        params = urllib.urlencode(params)
        f = self.opener.open(self.urls["login"], params)
        data = f.readlines()
        f.close()
    def open_base_url(self,url):
        """
        Constructs and opens an url relative to the base of this connection.
        """
        url = "%s/%s" % (self.urls["base"],url)
        return self.opener.open(url)
    def open_project_url(self,project,url):
        """
        Constructs and opens a project url relative to the base of this
        connection.
        """
        url = "%s/projects/%s/%s" % (self.urls["base"] ,project)
        return self.opener.open(url)
    def fetch_issues(self,project,query_id=-1,iclass=Issue):
        """
        Executes a query and returns a set of Issues holding
        the results.
        You can specify which class is used to wrap returned issues via 'iclass'.
        """
        issues_url = "%s/projects/%s/issues.csv" % (self.urls["base"] ,project)
        if int(query_id) >= 0:
            params = {}
            params['query_id'] = str(query_id)
            issues_url += "?" + urllib.urlencode(params)
        print "[executing query: %s]" % issues_url
        f = self.opener.open(issues_url)
        csv_reader = csv.reader(f)
        issues = [ row for row in csv_reader]
        fields = [self.__format_field_name(val) for val in issues[0]]
        issues = issues[1:]
        print "[query returned %d issues]" % len(issues)
        IssueTuple = namedtuple("Issue",fields)
        issues = [iclass(IssueTuple(*i),self) for i in issues]
        return fields,issues
    def save_query_pdf(self,project,query_id,output_file):
        """
        Collects pdfs of all issues returned by a query and combines them into
        a single output pdf.
        """
        fields,issues = self.fetch_issues(project,query_id)
        nissues = len(issues)
        if nissues == 0:
            print "[query returned no issues -",
            print " skipping creation of '%s']" % output_file
            return
        # try to ingore some deprecation warnings from pyPdf
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            opdf = pyPdf.PdfFileWriter()
            for i in issues:
                print "[downloading issue %s]" % i.id
                idata = i.fetch_pdf_buffer()
                ipdf = pyPdf.PdfFileReader(idata)
                for p in range(ipdf.numPages):
                    opdf.addPage(ipdf.getPage(p))
            print "[creating %s]" % output_file
            opdf.write(file(output_file,"wb"))
    def __format_field_name(self,name):
        """
        Helper that makes sure field names comply w/ rules required for
        creating a 'namedtuple' object.
        """
        name = name.lower().replace(" ","_")
        if name == "#":
            name = "id"
        name = name.replace("%","percent")
        return name




