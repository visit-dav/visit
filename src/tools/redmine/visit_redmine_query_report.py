#!/usr/bin/env python
# file: visit_redmine_query_report.py
# author: Cyrus Harrison (cyrush@llnl.gov)
# purpose:
#  Generates issue report pdfs from VisIt redmine queries.
#  See http://www.visitusers.org/index.php?title=Redmine_Reports for more info.
#

import pyrmine,sys,os,datetime,logging
import ho.pisa as pisa


class VisItIssue(pyrmine.Issue):
    """
    Extents standard issue class to provide VisIt specific html output.
    """
    def __init__(self,data,conn):
        pyrmine.Issue.__init__(self,data,conn)
    def to_html(self):
        """
        Creates an html representation of a VisIt issue.
        """
        i = self.data._asdict()
        res = "<h1># %s (%s) %s</h1>\n" % (i["id"],i["tracker"],i["subject"])
        res += "<table><tr><td>\n"
        for key in ["status","author","created","updated","priority","assigned_to"]:
            res += "<b>%s</b>: %s<br>\n" % (key,self.__format_blank(i[key]))
        res+="</td><td>\n"
        if i["tracker"] == "Feature":
            for key in ["expected_use","impact"]:
                res += "<b>%s</b>: %s<br>\n" % (key,self.__format_blank(i[key]))
        elif i["tracker"] == "Bug":
            for key in ["likelihood","severity","found_in_version"]:
                res += "<b>%s</b>: %s<br>\n" % (key,self.__format_blank(i[key]))
        for key in ["os","support_group","estimated_time"]:
                res += "<b>%s</b>: %s<br>\n" % (key,self.__format_blank(i[key]))
        res +="</td></tr><table>\n"
        res +="<p><b>Description:</b><br>\n"
        res += i["description"].replace("\n","<br>") + "</p>\n<p>"
        updates = self.fetch_updates()
        if len(updates) == 0:
            res += "<b>(No Updates)</b><br>\n"
        else:
            if len(updates) == 1:
                res += "<b>History: (1 Update)</b><br>\n"
            else:
                res += "<b>History: (%d Updates)</b><br>\n" % len(updates)
            for u in updates:
                res += "<blockquote>\n<b>%s</b> (%s)<br>" % (u["author"],u["date"])
                res += "%s\n</blockquote>\n" % u["content"]
        res += "</p><hr>\n"
        return res
    def __format_blank(self,txt):
        """
        Returns (unset) to indicate a blank field value.
        """
        if txt.strip() == "":
            return "(unset)"
        return txt

class VisItReportGenerator(object):
    """
    Creates html/pdf reports from redmine queries.
    """
    def __init__(self):
        base_url = 'https://visitbugs.ornl.gov/'
        print "[connecting to redmine instance @ %s]" % base_url
        self.conn = pyrmine.Connection(base_url);
        self.conn.login()
    def generate_query_report(self,query_id,obase):
        obase = os.path.abspath(obase)
        ohtml = obase + ".html"
        opdf  = obase + ".pdf"
        print "[fetching issues]"
        fields,issues = self.conn.fetch_issues("visit",query_id,VisItIssue)
        print "[generating %s]" % ohtml
        fhtml = open(ohtml,"w")
        fhtml.write(self.__generate_html_start())
        for i in issues:
            fhtml.write(i.to_html())
        fhtml.write(self.__generate_html_end())
        fhtml.close()
        print "[generating %s]" % opdf
        pdf = pisa.CreatePDF( open(ohtml, "rb"),open(opdf, "wb"))
    def __generate_html_start(self):
        """
        Generates the start of html doc, creates style sheet & header.
        """
        res = """
        <html>
        <style>
            @page {
            size: letter;
            margin: 1cm;
            @frame footer {
                -pdf-frame-content: footerContent;
                bottom: .5cm;
                margin-left: .5cm;
                margin-right: .5cm;
                height: .5cm;
                }
            }
            h1 {
            font-size: 135%;
            }

            td,
            p {
            font-size: 130%;
            }
            li {
                font-size: 125%;
            }
        </style>
        <body>
        <table><tr><td>
        <h2> VisIt Redmine issue report </h2>
        </td><td align="right">
        <h5> Generated:
        """
        res += str(datetime.datetime.now())
        res += "</h5>\n</td></tr>\n</table>\n<hr>\n"
        return res

    def __generate_html_end(self):
        """
        Generates the end of html doc, creates page # footer.
        """
        return """
        <div id="footerContent" align="right">
            Page #<pdf:pagenumber>
        </div>
        </body>
        </html>
        """


def parse_args():
    if len(sys.argv) < 3:
        print "usage: visit_redmine_query_report.py [query_id] [output_base]"
        sys.exit(-1)
    qid, ofile = sys.argv[1:3]
    return qid,ofile

if __name__ == "__main__":
    logging.basicConfig(level=logging.ERROR)
    query_id, obase = parse_args()
    vrq = VisItReportGenerator()
    vrq.generate_query_report(query_id,obase)



