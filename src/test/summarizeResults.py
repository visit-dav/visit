#!/bin/env python
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
"""
file: summarizeResults.py
description: prints a summary of test results contained in results.json
author: Kathleen Biagas 
date: Tue Feb 11 08:57:54 PST 2014
"""
# ----------------------------------------------------------------------------
#  Modifications:
#
# ----------------------------------------------------------------------------
import os
import json

if (os.path.isfile("results.json")):
    full = json.load(open("results.json"))
    for r in full["results"]:
        if "status" in r:    
            print("%s:  %s/%s"%(r["status"],r["category"],r["base"]))
            if r["status"] != "succeeded":
                for s in r["details"]["sections"]:
                    for c in s["cases"]:
                        print("    %s:  %s"%(c["status"],c["name"]))
else:
    print("results.json does not exist.")

exit()
