# @file app.py
# @brief The main module to maintain whole app
# README: Application wide management unit
# MODULE_ARCH:  ExecuteCmd, globalclasses_init, globallist_init
# CLASS_ARCH: FastApp
# GLOBAL USAGE: execmd_par

#standard
import logging
import copy
from datetime import datetime,timedelta
#extend
import simpy
#library
import codes.model as md
import codes.ui as ui
import lib.globalclasses as gc
from lib.const import *

##### Code section #####
#Spec: simulation control, log managment, setting load/save
#How/NeedToKnow:
class LApp:
    def __init__(self):
        self.init_log()
        logging.info("%s version: v%s" %(LSIM_TITLE,LSIM_VERSION))
        pass
    def init_log(self):
        # set up logging to file - see previous section for more details
        logging.basicConfig(level=logging.INFO,
                            format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                            datefmt='%m-%d %H:%M',
                            filename='output/sim.log',
                            filemode='a')
        # define a Handler which writes INFO messages or higher to the sys.stderr
        console = logging.StreamHandler()
        console.setLevel(logging.INFO) #logging.INFO
        # set a format which is simpler for console use
        formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
        # tell the handler to use this format
        console.setFormatter(formatter)
        # add the handler to the root logger
        logging.getLogger('').addHandler(console)
        
        # Now, we can log to the root logger, or any other logger. First the root...
        #logging.info('Logger initialed')
    
    def load_setting(self):
        pass
    def save_setting(self):
        pass
    def simrun(self, v_until): #v_until>=1
        """current debug command"""
        gc.MODEL.entity_setup()
        
        mm = md.ModelMonitor()
        logging.info("Simulation start!\nSimulation Descriptor:\n%s" %(gc.MODEL.get_desc_str() ) )
        logging.info(gc.MODEL.map.desc())
        
        map_diff = 0.0
        map_diff1 = 0.0
        avg_map_diff =0.0
        zero_count=0
        for i in range(1,v_until):
            gc.MODEL.env.run(until=i)
            gc.MODEL.dt_end = gc.MODEL.dt_start + timedelta(hours=i)
            gc.MODEL.map.timestamp = gc.MODEL.dt_end
            # this step sim end, house keeping
            if gc.MODEL.map_prev:
                map_diff1 =  gc.MODEL.map_evaluation(gc.MODEL.map, gc.MODEL.map_prev, gc.LASSDATA)
                map_diff += map_diff1
                if map_diff1 == 0.0:
                    zero_count+=1
            gc.MODEL.map.sum_pm_total()
            mm.mon_step(gc.MODEL.map.pm_total,map_diff1)
            gc.UI.save_esri(gc.MODEL.map,"test")
            #backup this step
            gc.MODEL.map_prev = copy.deepcopy( gc.MODEL.map)
        if v_until > 1:
            avg_map_diff = map_diff / (v_until-1-zero_count)
        
        logging.info(gc.MODEL.desc() )
        logging.info("pm_total history: %s" % (mm.pm_total))
        logging.info("pm_map_diff history: %s"%(mm.pm_map_diffs))
        logging.info("pm_map_diff average: %f"%(avg_map_diff))
        gc.UI.test(mm.pm_map_diffs)
    
