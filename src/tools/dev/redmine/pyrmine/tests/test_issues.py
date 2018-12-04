#!/usr/bin/env python
#
# file: test_issues.py
# author: Cyrus Harrison (cyrush@llnl.gov)
# created: 6/01/2010
#
#
import unittest,os,sys
import pyrmine


class TestIssues(unittest.TestCase):
    def test_csv(self):
        rconn = pyrmine.Connection("http://www.redmine.org/")
        fields,issues = rconn.fetch_issues("redmine")
        self.assertTrue(len(fields) > 0)
        self.assertTrue(len(issues) > 0)

if __name__ == '__main__':
    unittest.main()

