#!/usr/bin/env python

"""
File:    DocBlock.py
Author:  Cyrus Harrison
Created: February 24, 2008
Purpose: Provides classes that translate VisIt's documenation blocks to 
         doxygen parsable directives. 
"""

import re;

def doxy_star_pad(val,sep="\n"):
    res = ""
    for v in val.split("\n"):
        res += " * " + v + sep
    return res

def doxy_line(tag,val):
    if val !="":
        return doxy_star_pad("\\" + tag + " " + val.strip())
    else:
        return ""

class Section(object):
    def __init__(self,name,text):
        self.name = name
        self.text = text
        self.tag  = 0
        self.__filter_text()
    def __filter_text(self):
        res = ""
        lines = self.text.split("\n")
        for l in lines:
            lst = l.strip()
            if lst != "//" and lst.find("// ******") < 0 :
                if lst[:2] == "//":
                    lst = lst[2:].strip()
            else:
                lst ="\n"
            res += lst + "\n"
        self.text = res
    def __str__(self):
        return "Section:%s\n%s" % (self.name,self.text)

class DocBlock(object):
    def __init__(self,block_text,lopen,lclose):
        self.lopen  = lopen
        self.lclose = lclose
        self.gtag   = None
        self.__extract_sections(block_text)
        self.__parse()
    def set_group_tag(self,gtag):
        self.gtag = gtag
    def check(self):
        missing = []
        if self.type == "":
            missing.append("type")
        if self.name == "":
            missing.append("name")
        if self.date == "":
            missing.append("date")
        if self.authors == "":
            missing.append("author")
        if self.summary == "":
            missing.append("summary")
        if self.notes == "":
            missing.append("notes")
        if len(self.mods) == 0:
            missing.append("mods")
        if len(missing) >0:
            print "[Missing Sections: %s]" % str(missing)
    def __extract_sections(self,text):
        # tokenize by sections Name: (text)
        # (assumes sections have first letter capitalized, second uncap)
        self.sections = {}
        res = re.compile(r'[A-Z][a-z][a-zA-Z]*:\s').search(text)
        while not res is None:
            sname = text[res.start():res.end()][:-2]
            text = text[res.end():]
            res = re.compile(r'[A-Z][a-z][a-zA-Z]*:\s').search(text)
            if res is None:
                stext = text
            else:
                stext = text[:res.start()]
            stext = self.__sanitize(stext)
            self.sections[sname] = Section(sname,stext)
    def __has_section(self,key):
        return key in self.sections.keys()
    def __get_section_text(self,key):
        if key in self.sections.keys():
            sect = self.sections[key]
            sect.used = 1
            return sect.text.strip()
        else:
            return ""
    def __sanitize(self,txt):
        #handle problematic chars
        txt = txt.replace("<","&lt;")
        txt = txt.replace(">","&gt;")
        return txt
    def __parse(self):
        self.type ="<unknown>"
        self.name =""
        if self.__has_section("Class"):
            self.type = "class"
            self.name = self.__get_section_text("Class")
        elif self.__has_section("Method"):
            self.type = "method"
            self.name = self.__get_section_text("Method")
        elif self.__has_section("Function"):
            self.type = "function"
            self.name = self.__get_section_text("Function")
        if self.type == "<unknown>":
            return
        self.date     = self.__get_section_text("Creation")
        self.summary  = self.__get_section_text("Purpose")
        self.notes    = self.__get_section_text("Note")
        self.notes   += self.__get_section_text("Notes")
        self.authors  = self.__get_section_text("Programmer")
        self.__parse_args()
        self.__parse_mods()
    def __parse_args(self):
        self.args = []
        if self.__has_section("Arguments"):
            args_lines  = self.__get_section_text("Arguments").split("\n")
            args_lines  = [ l.strip() for l in args_lines if l.strip() != ""]
            for l in args_lines:
                aname, atxt = self.__parse_arg_line(l)
                if aname !="":
                    self.args.append(Section(aname,atxt))
                elif atxt != "":
                    pass
    def __parse_arg_line(self,aline):
        # if first token has [wspace] > 2 ,
        aname = ""
        atxt  = ""
        idx = aline.find(" ")
        if idx >=0 and idx < len(aline):
            if aline[idx+1] == " ": #first token is a var name
                aname = aline[:idx]
                atxt  = aline[idx:].strip()
            else: # extended description
                atxt  = aline
        return aname, atxt
        
    def __parse_mods(self):
        self.mods =[]
        mods_key = "Modifications"
        if mods_key in self.sections.keys():
            # split by double newline
            mods = self.sections[mods_key].text.strip().split("\n\n")
            for m in mods:
                lines = [ l.strip() for l in m.split("\n")]
                mod_ttl =""
                mod_txt =""
                for l in lines:
                    if l != "" and mod_ttl == "":
                        mod_ttl = l
                    elif l != "":
                        mod_txt += l +" "
                if mod_txt != "":
                    self.mods.append(Section(mod_ttl,mod_txt))
    def doxygenate(self):
        res  = ""
        if self.type =="<unknown>":
            return res
        res += self.__doxy_header()
        res += self.__doxy_date()
        res += self.__doxy_authors()
        res += self.__doxy_summary()
        res += self.__doxy_notes()
        res += self.__doxy_args()
        res += self.__doxy_mods()
        res += self.__doxy_footer()
        return res
    def __doxy_header(self):
        res = ""
        if self.type == "class":
            if not self.gtag is None:
                res += "/** \\ingroup group_%s*/\n" % self.gtag
            res += "/** @{*/\n"
            res += "/**\n"
            res += doxy_line("class",self.name)
        else:
            res += "/**\n"
        return res
    def __doxy_footer(self):
        res = " */\n"
        if self.type == "class":
            res += "/** @}*/\n"
        return res
    def __doxy_date(self):
        return doxy_star_pad("\\created " + self.date)
    def __doxy_authors(self):
        if self.authors.find(" and ") >=0:
            return doxy_star_pad("\\programmers " + self.authors)
        elif self.authors != "":
            return doxy_star_pad("\\programmer " + self.authors)
        else:
            return ""
    def __doxy_summary(self):
        if self.summary != "":
            res =  " *\n"
            res += doxy_star_pad(self.summary)
            res += " *\n"
            return res
        else:
            return ""
    def __doxy_notes(self):
        if self.notes =="":
            return ""
        return doxy_star_pad("\\notes " + self.notes)
    def __doxy_args(self):
        res =""
        if len(self.args) > 0:
            for a in self.args:
                res += doxy_line("param", a.name + " " + a.text)
        return res
    def __doxy_mods(self):
        res =""
        if len(self.mods) > 0:
            res += " * \\mods\n"
            for m in self.mods:
                res += doxy_star_pad(" - " + m.name + "<br>" + m.text)
        return res

    