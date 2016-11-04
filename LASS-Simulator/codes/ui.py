# @file ui.py
# @brief 
# README: User interface related, import/export
# MODULE_ARCH:  
# CLASS_ARCH:
# GLOBAL USAGE: 
#standard

#extend
import numpy as np
import matplotlib.pyplot as plt

#load json
import urllib
import requests
import simplejson


#library
import lib.globalclasses as gc
from lib.const import *

##### Code section #####
#Spec: export esri, plot
#How/NeedToKnow:
class UserInterface():
    def __init__(self):
        #private
        #global: these variables allow to direct access from outside.
        pass
    def save_esri_xml(self,model,name): #model(Model)      
        start_date=model.dt_start.strftime("%Y-%m-%d") #"2015-06-03"
        start_time=model.dt_start.strftime("%H:%M:%S")#"02:00:00"
        end_date=model.dt_end.strftime("%Y-%m-%d") #"2015-06-03"
        end_time=model.dt_end.strftime("%H:%M:%S")#"02:00:00"

        xml_template = """<?xml version="1.0" encoding="UTF-8"?>
<MapStacks xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.wldelft.nl/fews/PI" xsi:schemaLocation="http://www.wldelft.nl/fews/PI http://fews.wldelft.nl/schemas/version1.0/pi-schemas/pi_mapstacks.xsd" version="1.2">
    <geoDatum>WGS 1984</geoDatum>
    <timeZone>8.0</timeZone>
    <mapStack>
        <locationId>%s</locationId>
        <parameterId>PM25 Value</parameterId>
        <timeStep unit="minute" multiplier="1"/>
        <startDate date="%s" time="%s"/>
        <endDate date="%s" time="%s"/>
        <file>
            <ascii file="%s_??????????????.asc"/>
        </file>
    </mapStack>
</MapStacks>
        """ %(name,start_date, start_time,end_date, end_time, name)
        pathname = "output/%s.xml" % (name) 
        with open(pathname, "w") as text_file:
            text_file.write("%s" % (xml_template))
#<?xml version="1.0" encoding="UTF-8"?>
#<MapStacks xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.wldelft.nl/fews/PI" xsi:schemaLocation="http://www.wldelft.nl/fews/PI http://fews.wldelft.nl/schemas/version1.0/pi-schemas/pi_mapstacks.xsd" version="1.2">
#    <geoDatum>WGS 1984</geoDatum>
#    <timeZone>8.0</timeZone>
#    <mapStack>
#        <locationId>C1550_Grid</locationId>
#        <parameterId>Rainfall</parameterId>
#        <timeStep unit="hour" multiplier="1"/>
#        <startDate date="2015-06-03" time="02:00:00"/>
#        <endDate date="2015-06-03" time="08:00:00"/>
#        <file>
#            <ascii file="C1550_Grid????.???"/>
#        </file>
#    </mapStack>
#</MapStacks>        

    #format refer: http://resources.esri.com/help/9.3/arcgisengine/java/GP_ToolRef/spatial_analyst_tools/esri_ascii_raster_format.htm
    # export 2 file with esri format
    # name_timestamp.asc, name.xml
    def save_esri(self,map,name): #map(Map)      
#ncols 43
#nrows 16
#xllcenter 120.13125
#yllcenter 23.38125
#cellsize 0.0125
#NODATA_value -999.0

        header = "ncols %i\nnrows %i\nxllcorner %f\nyllcorner %f\ncellsize %f\nnodata_value %f\n" %(map.x_max,map.y_max,map.x0,map.y0,map.cellsize,map.nodata_value)
        str_value=""
        for y in range(0,map.y_max):
            for x in range(0,map.x_max):
                pos_idx = "%i@%i" % (x,y) 
                str_value += "%f\t" % (map.poss[pos_idx].pm_value)
            str_value  += "\n"
        timest = map.timestamp.strftime("%Y%m%d%H%M%S")#"20160101120000"
        pathname = "output/%s_%s.asc" % (name,timest) 
        with open(pathname, "w") as text_file:
            text_file.write("%s\n%s" % (header,str_value))

    def load_lass(self): 
        pass
    #download environement data that can easy be updated. 
    def download_env_data(self):
        url_lists = [['http://opendata.cwb.gov.tw/datadownload?dataid=O-A0001-001','IN_EPA_WEATHER_CURRENT']] #[ [url,file_dest], []]
        for url_list in url_lists:
            pathname = gc.SETTING[url_list[1]]
            
    def test_json(self):
        response =  urllib.request.urlopen("http://nrl.iis.sinica.edu.tw/LASS/last-all-lass.json")
        data = simplejson.load(response)
        print(data)
        
    def test(self,data):
        line, = plt.plot(data)
        plt.show()
