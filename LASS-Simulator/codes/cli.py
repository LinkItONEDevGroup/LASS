# @file cli.py
# @brief cli of whole tool
# README: Command line interface
# MODULE_ARCH:  
# CLASS_ARCH:
# GLOBAL USAGE: 
#standard
import cmd
#extend
#library
import lib.globalclasses as gc
from lib.const import *


##### Code section #####
#Spec: local variable maintain, about, user commands, test commands
#How/NeedToKnow:
class Cli(cmd.Cmd):  
    """Simple command processor example."""    
    def __init__(self,stdout=None):
        cmd.Cmd.__init__(self)
        self.prompt = 'FastCLI> '
        pass
############ cli maintain ####################        
    def do_set(self,line):
        """set scli variable, can be new or update.
        set var_name var_value
        ex: set mac_addr 001122334455"""

        pars=line.split()
        if len(pars)==2:
            var = pars[0]
            value = pars[1]
        else:
            return 
        
        if var in ('dev_console_display','log_level_file','log_level_console'):
            value = int(value)
            
        gc.GAP.user_vars[var] = value
        # dynamic apply
        # these variable need to work out, log_level_file, log_level_console

    def do_show(self,line):
        """show simcli variables, if miss variable name, show all
        show variable_name
        system variables list:
            ;log level definition, DEBUG=10,INFO=20,WARNING=30,ERROR=40,CRITICAL=50
            log_level_console=20     #the console message log level
            log_level_file=40        #file message log level
            ;device console real time display
            dev_console_display=1    #(0) don't display (1) display
        ex: show mac_addr """
        for var in gc.GAP.user_vars.keys():
            print("%s=%s" % ( var , gc.GAP.user_vars[var]))

############ top command ####################                      
    #def do_test1(self, line):
    #    """current debug command"""
    #    self.cli_ebm.do_init("")
        
    def do_about(self, line):
        """About this software"""
        print("%s version: v%s" %(LSIM_TITLE,LSIM_VERSION))
    def do_quit(self, line):
        """quit"""
        return True
############ top command ####################                      
    def do_simrun(self, line):
        """Start simulation
        simrun [ until ]
        ; until: how many time unit simulation should run, default 15 hours
        """
        pars=line.split()
        until = 15
        if len(pars)==1:
            until = int(pars[0])
        else:
            return        
        gc.GAP.simrun(until)
    def do_save_esri_xml(self,line):
        """Save map to RSRI ASCII xml format
        save_esri_xml [ name ] 
        ; name: export file name prefix
        ex: save_esri_xml sim
        ESRI export naming rule:
          name.xml
        """
        pars=line.split()
        name = ""
        if len(pars)==1:
            name = pars[0]
        else:
            return
        
        gc.UI.save_esri_xml(gc.MODEL,name)
    def do_save_esri(self,line):
        """Save map to RSRI ASCII format
        save_esri [ name ] 
        ; name: export file name prefix
        ex: save_esri map
        ESRI export naming rule:
          name_timestamp.asc  (ex: abc_20160708210000.asc) 
        """
        pars=line.split()
        name = ""
        if len(pars)==1:
            name = pars[0]
        else:
            return
        
        gc.UI.save_esri(gc.MODEL.map,name)
        
    def do_loadlass(self,line):
        """load current lass data"""
        lassdata = gc.LASSDATA
        lassdata.load_site_list()
        
        lassdata.tag_site_by_area('default',gc.MODEL.corners)#[24.0, 120.0 ,25.0,121.0])
        #lassdata.load_site_history_of_2day('FT1_001')
        lassdata.load_his_by_tag('default')
        #lassdata.desc(0)
        #lassdata.save_csv('default','output/lass_his.csv')
    def do_test(self,line):
        """current debug command"""
        #gc.ENVDATA.load_car_density()
        #gc.ENVDATA.load_population_count()
        #gc.ENVDATA.load_fixed_pollution_srcs("include/%s" % (gc.SETTING["IN_FIX_POLLUTION_SRC_DIR"]))
        #gc.ENVDATA.load_cwb_weather_curr()
        #gc.ENVDATA.load_cwb_weather_gfs("include/%s" %(gc.SETTING["IN_CWB_WEATHER_GFS"]))
        gc.ENVDATA.load_all()
        gc.ENVDATA.desc(0)