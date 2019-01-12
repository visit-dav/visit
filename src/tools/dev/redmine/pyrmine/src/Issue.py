#!/usr/bin/env python
#
# file: base.py
# author: Cyrus Harrison <cyrush@llnl.gov>
# created: 6/1/2010
# purpose:
#  Provides an 'Issue' class that wraps issue data returned by redmine queries.
#

import cStringIO
import xml.etree.ElementTree as etree

from Connection import *

class Issue(object):
    """
    Wraps an issue returned from a redmine query.
    """
    def __init__(self,data,conn):
        self.data = data
        self.conn = conn
    def as_dict(self):
        """
        Returns issue data from namedtuple as a python dictonary.
        """
        return data._asdict()
    def save_pdf(self,output_file):
        """
        Downloads a rendered pdf of an issue to a file.
        """
        buff = self.fetch_pdf_buffer(self.data.id)
        ofile = open(output_file,"wb")
        ofile.write(buff.read())
        f.close()
    def fetch_pdf_buffer(self):
        """
        Downloads a rendered pdf of an issue & wraps this data with a
        StringIO object suitable for use with pyPdf.
        """
        issue_url = "issues/%s.pdf" % self.data.id
        f = self.conn.open_base_url(issue_url)
        return cStringIO.StringIO(f.read())
    def fetch_updates(self):
        """
        Returns a list with the contents of each update pulled from the
        issue's atom feed.
        """
        issue_url = "issues/%s.atom" % self.data.id
        f = self.conn.open_base_url(issue_url)
        root = etree.fromstring(f.read())
        updates = []
        for entry in root.findall('{http://www.w3.org/2005/Atom}entry'):
            update = {}
            for c in entry:
                if c.tag == "{http://www.w3.org/2005/Atom}updated":
                    update["date"] = c.text
                if c.tag == "{http://www.w3.org/2005/Atom}author":
                    update["author"] = c.find("{http://www.w3.org/2005/Atom}name").text
                if c.tag == "{http://www.w3.org/2005/Atom}content":
                    update["content"] = c.text
            updates.append(update)
        return updates
