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
        """Start simulation"""
        gc.GAP.simrun(15)
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
        

    def do_test(self,line):
        """current debug command"""
        xml_template = """
        <?xml version="1.0" encoding="UTF-8"?>
<MapStacks xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.wldelft.nl/fews/PI" xsi:schemaLocation="http://www.wldelft.nl/fews/PI http://fews.wldelft.nl/schemas/version1.0/pi-schemas/pi_mapstacks.xsd" version="1.2">
    <geoDatum>WGS 1984</geoDatum>
    <timeZone>8.0</timeZone>
    <mapStack>
        <locationId>%s</locationId>
        <parameterId>Rainfall</parameterId>
        <timeStep unit="hour" multiplier="1"/>
        <startDate date="2015-06-03" time="02:00:00"/>
        <endDate date="2015-06-03" time="08:00:00"/>
        <file>
            <ascii file="C1550_Grid????.???"/>
        </file>
    </mapStack>
</MapStacks>
        """ %("abc")
        print(xml_template)
    