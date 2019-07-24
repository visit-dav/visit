# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_encoding.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/09/2010
 description:
    Unit tests for movie encoding helpers.

"""

import unittest
import os
import sys
import glob

from visit_utils import encoding
from visit_utils.common import VisItException

from os.path import join as pjoin

iframes_dir = pjoin(os.path.split(__file__)[0],"_data")
iframes = pjoin(iframes_dir,"wave.movie.%04d.png")
iframes_short_a = pjoin(iframes_dir,"wave.movie.%03d.png")
iframes_short_b = pjoin(iframes_dir,"wave.movie.%d.png")
iframes_stereo  = pjoin(iframes_dir,"wave.movie.stereo.%04d.png")
output_dir  = pjoin(os.path.split(__file__)[0],"_output")

def lst_slnks():
    return glob.glob(pjoin(iframes_dir,"_encode.lnk.*"))

def clean_slnks():
    slnks = lst_slnks()
    for slnk in slnks:
        os.remove(slnk)

def check_encoded_file(path):
    if os.path.isfile(path):
        # make sure the file isn't empty
        st = os.stat(path)
        return st.st_size > 0
    return False

class TestEncoding(unittest.TestCase):
    def setUp(self):
        if not os.path.exists(output_dir):
            os.mkdir(output_dir)
    def test_encoders(self):
        encoders = encoding.encoders()
        if len(encoders) > 0:
            self.assertTrue("mpg" in encoders)
            self.assertTrue("wmv" in encoders)
    def test_ffmpeg_encoders(self):
        for enc in ["wmv","mpg","divx","mov","swf","mp4","avi"]:
            if enc in encoding.encoders():
                ofile = pjoin(output_dir,"wave.movie.%s" % enc)
                encoding.encode(iframes,ofile)
                self.assertTrue(check_encoded_file(ofile))
                ofile = pjoin(output_dir,"wave.movie.slow.%s" % enc)
                encoding.encode(iframes,ofile,2)
                self.assertTrue(check_encoded_file(ofile))
    def test_sm(self):
        if "sm" in encoding.encoders():
            ofile = pjoin(output_dir,"wave.movie.sm")
            encoding.encode(iframes,ofile)
            self.assertTrue(check_encoded_file(ofile))
            clean_slnks()
            ofile = pjoin(output_dir,"wave.movie.slow.sm")
            encoding.encode(iframes,ofile,2)
            self.assertEqual(0,len(lst_slnks()))
            self.assertTrue(check_encoded_file(ofile))
    def test_unsupported(self):
            self.assertRaises(VisItException, encoding.encode, iframes,"wave.movie.bad_ext")
    def test_sm_stereo(self):
        if "sm" in encoding.encoders():
            clean_slnks()
            ofile = pjoin(output_dir,"wave.movie.stereo.sm")
            encoding.encode(iframes_stereo,ofile,stereo=True)
            self.assertEqual(0,len(lst_slnks()))
            self.assertTrue(check_encoded_file(ofile))
            clean_slnks()
            ofile = pjoin(output_dir,"wave.movie.stereo.slow.sm")
            encoding.encode(iframes_stereo,ofile,2,stereo=True)
            self.assertEqual(0,len(lst_slnks()))
            self.assertTrue(check_encoded_file(ofile))
    def test_stereo_uneven_frames_error(self):
            self.assertRaises(VisItException, encoding.encode, iframes,
                                                                pjoin(output_dir,"wave.movie.stereo.bad.sm"),
                                                                stereo=True)
    def test_extract(self):
        if "mpg" in encoding.encoders():
            eframes = pjoin(output_dir,"extract_out_%04d.png")
            encoding.encode(iframes,pjoin(output_dir,"wave.movie.mpg"))
            encoding.extract(pjoin(output_dir,"wave.movie.mpg"),eframes)
            ofile = pjoin(output_dir,"wave.movie.extract.and.reencode.mpg")
            encoding.encode(eframes,ofile)
            self.assertTrue(check_encoded_file(ofile))
    def test_pre_lr_stereo(self):
        if "divx" in encoding.encoders():
            iframes = pjoin(iframes_dir,"noise.stereo.left.right.1080p.%04d.png")
            ofile = pjoin(output_dir,"noise.movie.stereo.pre.left.right.avi")
            encoding.encode(iframes,ofile,etype="divx")
            self.assertTrue(check_encoded_file(ofile))
    def test_short_symlinks(self):
        if "mpg" in encoding.encoders():
            clean_slnks()
            ofile = pjoin(output_dir,"wave.movie.test.seq.pattern.03d.mpg")
            encoding.encode(iframes_short_a,ofile,3)
            self.assertEqual(0,len(lst_slnks()))
            self.assertTrue(check_encoded_file(ofile))
            clean_slnks()
            ofile = pjoin(output_dir,"wave.movie.test.seq.pattern.d.mpg")
            encoding.encode(iframes_short_b,ofile,5)
            self.assertEqual(0,len(lst_slnks()))
            self.assertTrue(check_encoded_file(ofile))
    def test_ffmpeg_input_frame_rate(self):
        for enc in ["wmv","mpg","divx","mov","swf","mp4"]:
            if enc in encoding.encoders():
                clean_slnks()
                ofile = pjoin(output_dir,"wave.movie.input_frame_rate.%s" % enc)
                encoding.encode(iframes,ofile,input_frame_rate=5)
                self.assertEqual(0,len(lst_slnks()))
                self.assertTrue(check_encoded_file(ofile))
    def test_ffmpeg_input_and_output_frame_rate(self):
        for enc in ["wmv","mov"]:
            if enc in encoding.encoders():
                clean_slnks()
                ofile = pjoin(output_dir,"wave.movie.input_and_output_frame_rate.%s" % enc)
                encoding.encode(iframes,ofile,input_frame_rate=5,output_frame_rate=30)
                self.assertEqual(0,len(lst_slnks()))
                self.assertTrue(check_encoded_file(ofile))
    def test_ffmpeg_reencode_new_format(self):
        encoders =  encoding.encoders() 
        if "mpg" in encoders and "wmv" in encoders:
            clean_slnks()
            ofile_src = pjoin(output_dir,"wave.movie.reencode.src.mpg")
            ofile_des = pjoin(output_dir,"wave.movie.reencode.src.wmv")
            encoding.encode(iframes,ofile_src)
            encoding.encode(ofile_src,ofile_des)
            self.assertEqual(0,len(lst_slnks()))
            self.assertTrue(check_encoded_file(ofile_src))
            self.assertTrue(check_encoded_file(ofile_des))


if __name__ == '__main__':
    unittest.main()


