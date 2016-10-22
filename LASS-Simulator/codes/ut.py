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
    def test_001_setting_signature(self):
        print("\nThe expected unit test environment is")
        print("1. XXX")
        self.assertEqual(gc.SETTING["SIGNATURE"],'LASS-SIM')        
