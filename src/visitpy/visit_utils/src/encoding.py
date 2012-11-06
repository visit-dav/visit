#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
"""
 file: encoding.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    Interface to encode image file sequences into movie files.

 examples:
    from visit_utils.encoding import *
    print encoders()
    encode("input.%04d.png","output.mpg")
    encode("input.%04d.png","output.wmv",fdup=5)
    encode("input.%04d.png","output.sm")
    encode("input.%04d.png","output.sm",stereo=True)
    extract("movie.mpg","output%04d.png")

"""


import os
import subprocess
import glob

from os.path import join as pjoin

from common import VisItException, sexe

__all__ = [ 'encoders','encode','extract']

def encoders():
    """
    Lists available encoders.
    """
    res = []
    if not ffmpeg_bin() is None:
        res.extend(["mpg","wmv","avi","mov","swf","mp4","divx"])
    if not img2sm_bin() is None:
        res.append("sm")
    return res

def extract(ifile,opattern):
    """
    Extracts a sequence of images from a a movie.
    Requires ipattern to use printf style # format like "file%04d.png".
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd = "%s -i %s -f image2 %s" % (enc_bin,ifile,opattern)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to extract frames.")

def encode(ipattern,ofile,fdup=None,etype=None,stereo=False):
    """
    Encodes a sequence of images into a movie.
    Requires ipattern to use printf style # format like "file%04d.png".
    """
    ipattern = os.path.abspath(ipattern)
    ofile = os.path.abspath(ofile)
    if etype is None:
        # get from ofile
        base,ext = os.path.splitext(ofile)
        if ext[0] == '.':
            ext = ext[1:]
        etype = ext
    if not etype in encoders():
        raise VisItException("Unsupported encoder: %s" % etype)
    patterns = {"full": ipattern}
    lnks = []
    if not fdup is None and not stereo:
       patterns,lnks = gen_symlinks(ipattern,fdup)
    if stereo:
       patterns,lnks = gen_symlinks_stereo(ipattern,fdup)
    encode_patterns(patterns,ofile,etype,stereo)
    if len(lnks) > 0:
       clean_symlinks(lnks)

def encode_patterns(patterns,ofile,etype,stereo):
    """
    Helper which dispatches encoding.

    Handles mono / stereo logic.
    """
    outputs = []
    if stereo:
        obase, oext = os.path.splitext(ofile)
        ofile_s = ofile
        ofile_r = obase + ".left"  + oext
        ofile_l = obase + ".right" + oext
        outputs = [ (patterns["stereo"], ofile_s,True),
                    (patterns["left"],   ofile_l,False),
                    (patterns["right"],  ofile_r,False)]
    else:
        outputs = [ (patterns["full"],ofile,False)]
    for out in outputs:
        print "[encoding: %s]" % out[1]
        if etype == "sm":
            encode_sm(out[0],out[1],stereo=out[2])
        elif etype == "mpg" or etype == "mpeg1":
            encode_mpeg1(out[0],out[1])
        elif etype == "wmv":
            encode_wmv(out[0],out[1])
        elif etype == "avi":
            encode_avi(out[0],out[1])
        elif etype == "mov":
            encode_mov(out[0],out[1])
        elif etype == "swf":
            encode_swf(out[0],out[1])
        elif etype == "mp4":
            encode_mp4(out[0],out[1])
        elif etype == "divx":
            encode_divx(out[0],out[1])


def encode_sm(ipattern,ofile,stereo=False):
    """
    Creates a 'streaming movie' or sm file.
    """
    enc_bin = img2sm_bin()
    if not enc_bin is None:
        cmd = "%s -rle" % enc_bin
        if stereo:
            cmd += " -stereo"
        cmd += " %s %s " % (ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("img2sm not found: Unable to encode streaming movie.")

def encode_mpeg1(ipattern,ofile):
    """
    Creates a mpeg1 video file using ffmpeg.
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd =  "echo y | %s -f image2 -i %s -qmin 1 -qmax 2 -an -vcodec mpeg1video "
        cmd += "-mbd -rd -trellis 2 -cmp 2 -subcmp 2 -pass 1/2 "
        cmd += "-b 18000000 -r 24 %s"
        cmd =  cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to encode mpeg.")

def encode_wmv(ipattern,ofile):
    """
    Creates a wmv video file using ffmpeg.
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd =  "echo y | %s -f image2 -i %s -qmin 1 -qmax 2 -g 100 -an -vcodec msmpeg4v2 "
        cmd += "-mbd -rd -flags +mv4+aic -trellis 2 -cmp 2 -subcmp 2 -pass 1/2 "
        cmd += "-b 18000000 -r 30 %s"
        cmd =  cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to encode wmv.")

def encode_swf(ipattern,ofile):
    """
    Creates a swf (flash) video file using ffmpeg.
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd =  "echo y | %s -f image2 -i %s -qmin 1 -qmax 2 -g 100 -an -vcodec flv "
        cmd += "-mbd -rd -flags +mv4+aic -trellis 2 -cmp 2 -subcmp 2 -pass 1/2 "
        cmd += "-b 18000000 -r 30 -f swf %s"
        cmd =  cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to encode swf.")


def encode_avi(ipattern,ofile):
     """
     Creates an avi video file (mjpeg) using ffmpeg.
     """
     enc_bin = ffmpeg_bin()
     if not ffmpeg_bin is None:
        cmd =  "echo y | %s -f image2 -i %s -vcodec mjpeg -qscale 1 -an %s "
        cmd =  cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
     else:
        raise VisItException("ffmpeg not found: Unable to encode avi.")


def encode_divx(ipattern,ofile):
    """
    Creates divx avi video file (mpeg4) using ffmpeg.
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd  = "echo y | %s -f image2 -i %s -vcodec mpeg4 -qscale 1 -f avi "
        cmd += "-vtag DX50 -an %s "
        cmd  = cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to encode divx avi.")


def encode_mov(ipattern,ofile):
    """
    Creates a mov video file (mpeg4) using ffmpeg.
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd =  "echo y | %s -f image2 -i %s -qmin 1 -qmax 2 -g 100 -an -vcodec mpeg4 "
        cmd += "-mbd -rd -flags +mv4+aic -trellis 2 -cmp 2 -subcmp 2 -pass 1/2 "
        cmd += "-an -b 18000000 -f mov -r 30 %s"
        cmd =  cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to encode mov.")

def encode_mp4(ipattern,ofile):
    """
    Creates a mp4 video file (mpeg4) using ffmpeg.
    """
    enc_bin = ffmpeg_bin()
    if not ffmpeg_bin is None:
        cmd =  "echo y | %s -f image2 -i %s -qmin 1 -qmax 2 -g 100 -an -vcodec mpeg4 "
        cmd += "-mbd -rd -flags +mv4+aic -trellis 2 -cmp 2 -subcmp 2 -pass 1/2 "
        cmd += "-an -b 18000000 -f mp4 %s"
        cmd =  cmd % (enc_bin,ipattern,ofile)
        sexe(cmd)
    else:
        raise VisItException("ffmpeg not found: Unable to encode mp4.")


#
# Symlink gen & cleanup
#

def gen_symlinks(ipattern,fdup):
    """
    Creates duplicate symlinks to help change effective frame rate
    of encoded movies.
    """
    idir, ifile = os.path.split(ipattern)
    pattern = pjoin(idir,"_encode.lnk.%s" % ifile)
    ifs = glob.glob(ipattern.replace("%04d","*"))
    ifs.sort()
    lnks = []
    lnk_cnt = 0
    for f in ifs:
        for i in range(fdup):
            lnk = pattern  % lnk_cnt
            os.symlink(f,lnk)
            lnks.append(lnk)
            lnk_cnt +=1
    res = {"full":pattern }
    return res, lnks

def gen_symlinks_stereo(ipattern,fdup=None):
    """
    Creates duplicate symlinks to help change effective frame rate
    of encoded movies.

    Handles stereo case.
    """
    if fdup is None:
        fdup = 1
    idir, ifile = os.path.split(ipattern)
    pattern_s   = pjoin(idir,"_encode.lnk.%s" % ifile)
    rbase, rext = os.path.splitext(pattern_s)
    pattern_l   = rbase + ".left"  + rext
    pattern_r   = rbase + ".right" + rext
    ifs = glob.glob(ipattern.replace("%04d","*"))
    ifs.sort()
    ifs_l = [ ifs[i] for i in xrange(len(ifs)) if i == 0 or i % 2 == 0]
    ifs_r = [ ifs[i] for i in xrange(len(ifs)) if i %  2 == 1]
    cnt_l = len(ifs_l)
    cnt_r = len(ifs_r)
    # num left frames  & num right frames should match
    if cnt_l != cnt_r:
        msg = "Different number of left (%d) and right (%d) frames." % (cnt_l,cnt_r)
        raise VisItException("encode stereo error: %s" % msg)
    lnks = []
    lnk_cnt = 0
    frm_cnt = 0
    for i in xrange(cnt_l):
        for j in range(fdup):
            in_l = ifs_l[i]
            in_r = ifs_r[i]
            lnk_0 = pattern_s %  lnk_cnt
            lnk_1 = pattern_s % (lnk_cnt + 1)
            lnk_l = pattern_l % frm_cnt
            lnk_r = pattern_r % frm_cnt
            os.symlink(in_l,lnk_0)
            os.symlink(in_r,lnk_1)
            os.symlink(in_l,lnk_l)
            os.symlink(in_r,lnk_r)
            lnks.extend([lnk_0,lnk_1,lnk_l,lnk_r])
            lnk_cnt +=2
            frm_cnt +=1
    res  = {"stereo": pattern_s,
            "left":   pattern_l,
            "right":  pattern_r}
    return res, lnks

def clean_symlinks(lnks):
    """
    Helper to removes symlinks created via gen_symlinks().
    """
    for lnk in lnks:
        os.remove(lnk)

#
# Helpers to find encoding binaries
#


def img2sm_bin():
    """
    Returns path to the 'img2sm' script, or None if this script is not available.
    """
    res = sexe("which img2sm",ret_output=True)[1].strip()
    if os.path.exists(res):
        return res
    if os.environ.has_key('SYS_TYPE'):
        res= pjoin("/usr/gapps/asciviz/blockbuster/latest",os.environ["SYS_TYPE"],"bin/img2sm")
        if os.path.exists(res):
            return res
    return None

def ffmpeg_bin():
    """
    Returns path to the 'ffmpeg' binary, or None if this binary is not available.
    """
    res = sexe("which ffmpeg",ret_output=True)[1].strip()
    if os.path.exists(res):
        return res
    else:
        return None
