#! /usr/bin/env python
# -*- coding: utf-8 -*-

# @file ut.py
# @brief The main unit test program of whole project
# README: organize the unit tests in the number range
#    refer UTGeneral functions
#    print the suggested procedure in the console
#    print the suggested check procedure in the console
#    support current supported important features
#    this unit test include in the release procedure
# MODULE_ARCH:  
# CLASS_ARCH: UTGeneral
# GLOBAL USAGE: 

#standard
import unittest
#homemake
import lib.globalclasses as gc
from lib.const import *



##### Unit test section ####
#the test ID provide the order of testes. 
class UTGeneral(unittest.TestCase):
    #local
    #ID:0-99
    def test_01_setting_signature(self):
        print("\nThe expected unit test environment is")
        print("1. TBD")
        self.assertEqual(gc.SETTING["SIGNATURE"],'LASS-SIM')        
    def test_02_check_library(self):
        #check external library that need to be installed
        import simpy
        from configobj import ConfigObj
        import urllib
        import simplejson 
        import requests 
        from vincenty import vincenty
        import matplotlib
        import numpy
        import pygrib 
    def test_03_check_dir_exist(self):
        pass
    def test_04_check_grib(self):
        import pygrib # import pygrib interface to grib_api
        grbs = pygrib.open('include/M-A0060-000.grb2')
        print("grbs[:4] count=%i" %(len(grbs[:4])))
        
    def test_11_loadjson(self):
        gc.LASSDATA.load_site_list()
        print("LASS sites count = %i" % (len(gc.LASSDATA.sites)))
        self.assertTrue(len(gc.LASSDATA.sites)>0)