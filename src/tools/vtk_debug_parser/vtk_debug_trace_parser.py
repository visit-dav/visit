#!/usr/bin/env python

# ----------------------------------------------------------------------------
#  Script that parses vtk messages from VisIt's engine debug logs and
#  provides an xml report with possible vtk leaks.
#  Usage example:
#   (run visit with -vtk-debug to generate an engine debug log)
#   vtk_debug_trace_parser.py engine_ser.1.log trace_output.xml
#   firefox trace_output.xml
#
# ----------------------------------------------------------------------------

import sys
import os

class trace_set(object):
    "A collection of trace objects"
    def __init__(self,fname):
        "Constructs a trace set"
        self._parse_file_(fname)

    def to_xml(self):
        "Dumps trace set to xml string"
        res = "<trace_set>\n"
        for t in self.traces.values():
                res += t.to_xml()
        res += "</trace_set>\n"
        return res

    def size(self):
        "Returns the number of trace objects"
        return len(self.traces)

    def _parse_file_(self,fname):
        "Parses a visit debug for vtk events"
        self.traces = {}
        addy_reuse = {}
        f = open(fname)
        lines = f.readlines()
        f.close()
        for line in lines:
            if self._valid_line_(line):
                oaddy,otype,oevnt = self._parse_line_(line.strip())
                if not self._check_filter_(oevnt):
                    # address reuse logic:
                    # checks # of times an address has rcved a
                    # Destructing! message, and increments a reuse
                    # counter. The unique key for an object is:
                    #  address-reuse_number
                    # Example:
                    #  0x90d8300-0
                    if not oaddy in addy_reuse.keys():
                        addy_reuse[oaddy]=0;
                    oaddy_u = oaddy + "-%d" % addy_reuse[oaddy]
                    if not oaddy_u in self.traces.keys():
                        trace = trace_object(oaddy_u,otype)
                        self.traces[oaddy_u] = trace

                    self.traces[oaddy_u].add_event(oevnt)
                    if oevnt == "Destructing!":
                        addy_reuse[oaddy] += 1
        return self.traces

    def _valid_line_(self,txt):
        "Checks if a line is a debug message we care about"
        if  txt[:3] == "vtk" and txt.find(" (") >0 and txt.find("):") >0:
            return True
        return False

    def _parse_line_(self,txt):
        "Extracts info from a debug message"
        tok = txt.strip().split()
        otype = tok[0]
        oaddy = tok[1][1:-2]
        oevnt = txt[txt.find(":")+1:].strip()
        return oaddy,otype,oevnt

    def _check_filter_(self,txt):
        "Filters common messages"
        if not txt.find("location = ") >=0:
            return False
        return True

class trace_object(object):
    "Holds debug messages for a unique vtk object."
    def __init__(self,oaddy,otype):
        "Creates a trace object"
        self.addy   = oaddy
        self.type   = otype
        self.ref_count = "unknown"
        self.events = []

    def check_event(self,txt):
        "Checks if this object contains this event"
        if txt in self.events:
            return True
        return False

    def add_event(self,txt):
        "Adds a new event"
        self.events.append(txt)
        rc_str ="ReferenceCount ="
        rc_idx = txt.find(rc_str);
        if rc_idx > 0:
            rc = txt[rc_idx + len(rc_str)+1:]
            self.ref_count = int(rc)

    def __str__(self):
        "Creates a human readable rep of this trace object"
        res = "%s::%s\n" % ( self.addy , self.type )
        for e in self.events:
            res += " e:%s\n" % e
        return res

    def to_xml(self):
        "Dumps trace object to xml string"
        res  = "<trace_object>\n"
        res += " <addy>%s</addy>\n" % self.addy
        res += " <type>%s</type>\n" % self.type
        res += " <events>\n"
        for e in self.events:
            res += "  <event>%s</event>\n" % e
        res += " </events>\n"
        res += "</trace_object>\n"
        return res

class leak_set(object):
    "Parses trace events to find non-destructed objects."
    def __init__(self,trace_set):
        "Constructs a leak set from a trace set"
        self.leaks = {}
        self.by_type = {}
        for k,t in trace_set.traces.items():
            if not t.check_event("Destructing!"):
                if not t.type in self.by_type.keys():
                    self.by_type[t.type] =1;
                else:
                    self.by_type[t.type] +=1;
                self.leaks[k] = t
    def to_xml(self):
        "Dump leaks to xml string"
        res = "<leak_set>\n"
        for k,v in self.by_type.items():
            res += "<type_summary><type>%s</type><count>%d</count></type_summary>\n" % (k,v)
        for t in self.leaks.values():
            res +=" <leak>\n  "
            res +=" <addy>%s</addy>\n " % t.addy
            res +=" <type>%s</type>\n " % t.type
            res +=" <ref_count>" + str(t.ref_count) + "</ref_count>\n "
            res +=" </leak>\n"
        res += "</leak_set>\n"
        return res

    def size(self):
        "Return the # of leaks"
        return len(self.leaks)


def output_header():
    "Generates a valid xml output header with correct style sheet location"
    style_file  = os.path.dirname(os.path.abspath(sys.argv[0]))
    style_file += "/vtk_debug_trace_style.xsl"
    res  = '<?xml version="1.0"?>\n'
    res += '<?xml-stylesheet type="text/xsl" href="%s"?>\n' % style_file
    res += '<root>\n'
    return res;

def output_footer():
    "Generates our xml output footer"
    return '</root>\n'


def process(fname_in,fname_out):
    "Creates trace and leak set and saves them as xml an output file"
    #create a trace set
    ts = trace_set(fname_in)
    #create a leak set
    ls = leak_set(ts)

    # open output file, dump header and trace set
    fout = open(fname_out,"w")
    fout.write(output_header())
    fout.write(ts.to_xml())
    # if we have leaks dump these
    if ls.size() >0:
        fout.write(ls.to_xml())
    # write footer
    fout.write(output_footer())
    fout.close()


def main():
    if len(sys.argv) < 3:
        print "usage: vtk_debug_trace_parser.py [engine_log] [output_file]"
        sys.exit(-1)
    engine_log  = sys.argv[1]
    output_file = sys.argv[2]
    process(engine_log,output_file)


if __name__ == "__main__":
    main()



