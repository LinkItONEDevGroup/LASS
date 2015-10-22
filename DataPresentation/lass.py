#
# README: please install the library before use
#       :     Require Libraries:
#                MQTT https://pypi.python.org/pypi/paho-mqtt
#                matplotlib http://matplotlib.org/
#                    It's easiler if you just install http://continuum.io/
#                simplekml http://simplekml.readthedocs.org/
#                numpy:
# Features:
#    Plot sensor data in realtime.
#    Support device_id filter.
#    CLI(Command line interface) support
#    User setting can be adjust through CLI
#    Support export to KML
#        Use google map map to show it. refer: https://support.google.com/mymaps/answer/3024836?hl=en
#    Auto log data, save/load data
#    Data filter by datetime
# Author:
#    Wuulong, Created 28/06/2015
#    https://github.com/LinkItONEDevGroup/LASS
# Arch:
#     Classes:
#        Setting:
#        SensorPlot:
#        Device:
#        Devices:
#        SensorData: one record
#        ExportKml:
#        LassCli ( CliSetting, CliExport, CliData )
#        MonitorThread:
#        FakeDataGenerator:
import random
import threading
import datetime
import time
import re
#cli
import cmd
#kml
import simplekml
import paho.mqtt.client as mqtt
#plot
import matplotlib.pyplot as plt
import numpy as np
import json
import argparse
import sys
import math

data_log_file = None
data_file = None
VERSION = "0.7.5"

global datetime_format_def


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")
    #topic="Sensors/#"
    client.subscribe(sEtting.mqtt_topic, qos=0)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    #payload_hex = ''.join(format(str(x), '02x') for x in msg.payload)
    payload_str = str(msg.payload)
    console_str = datetime.datetime.now().strftime("%X") + "|" +msg.topic+ payload_str[2:-1]
    print(console_str)

    sensor_data = dEvices.add(str(payload_str[1:]))
    if sensor_data:
        if sEtting.log_enabled:
            global data_log_file
            if data_log_file==None:
                data_log_file = open("lass_data_" + datetime.datetime.now().strftime("%Y%m%d") + ".log", 'w+')
            data_log_file.write(console_str + "\n")

            global data_file
            if data_file==None:
                data_file = open("lass_data_" + datetime.datetime.now().strftime("%Y%m%d") + ".raw", 'w+')
            data_file.write(sensor_data.raw + "\n")

        if sEtting.plot_enabled:
            sensorPlot.plot(1)


# the setting for the program
class Setting:
    def __init__(self):
        #system general setting
        self.debug_enable=0 #Default:0, 0: debug disable , 1: debug enable
        self.plot_cnt=90 #Default:90,  the value count in plotter, if 10 seconds for 1 value, about 15 min.
        #self.mqtt_topic="LASS/#"   #REPLACE: to your sensor topic
        #self.mqtt_topic="LASS/Test/+"  #REPLACE: to your sensor topic, it do not subscribe device id's channel
        self.mqtt_topic="LASS/Test/+"  #Default: LASS/Test/+ , REPLACE: to your sensor topic, it do not subscribe device id's channel
        self.filter_par_type=2 #Default: 0, 0: no filer, 1: filter device_id, 2: filter ver_format
        self.device_id="YOUR_DEVICE_NAME" #Default: YOUR_DEVICE_NAME, REPLACE: to your device id
        self.ver_format=3 #Default 3,: filter parameter when filter_par_type=2
        self.kml_export_type=0 #Default:0, default kml export type. name = deviceid_localtime
        self.plot_enabled=0 #Default:0, 0: realtime plot not active, 1: active plot
        self.plot_save=1 #Default:1, 0: show plot realtime, 1:plot to file
        self.log_enabled=1 #Default:1, 0: not auto save receive data in log format, 1: auto save receive data in log format
        self.auto_monitor=1 #Default:1,0: not auto start monitor command, 1: auto start monitor commmand
        # plot, kml marker's color only apply to 1 sensor, this is the sensor id
        #0: battery level, 1: battery charging, 2: ground speed ( Km/hour )
        #10: dust sensor, 11: UIdust sensor, 12: sound sensor
        self.sensor_cur=0   #Default:0,REPLACE: to your interest current sensor


# Sensor plot funcition
class SensorPlot:
    def __init__(self):
        self.first=1 # first run

    def init(self):
        try:
            self.fig=None
            self.ax=None
        except:
            pass
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111)
    def plot(self,plot_id):
        device_id = sEtting.device_id #"LASS-Example0"
        if self.first:
            self.init()
            plt.title(sEtting.mqtt_topic + ' Sensor data')
            plt.ylabel('Sensor value')
            plt.xlabel("Data sensing time")

        if plot_id>0:
            #FIXME error handler
            if device_id in dEvices.devs:
                x, y = dEvices.devs[device_id].get_values(sEtting.plot_cnt,plot_id)
                self.first=0
            else:
                print("plot device:" + device_id + " not exist!")
                return
        else:
            x, y = dEvices.get_values(sEtting.plot_cnt) #FIXME
            self.first=0
            # draw and show it
            #self.fig.canvas.draw()
            #plt.show(block=False)

        if len(x)<=0 or not x :
            print("data count=0, ignore plot. Maybe device id is wrong")
        plt.gcf().autofmt_xdate()
        (self.li, )= self.ax.plot(x, y)
        self.li.set_xdata(x)
        self.li.set_ydata(y)
        self.fig.canvas.draw()
        if sEtting.plot_save:
            plt.savefig("lass_" + str(plot_id) + ".png")
        else:
            plt.show(block=False)




#one device
class Device():
    def __init__(self,dev_id):
        self.id = dev_id
        self.app = ""
        self.sensor_datas = []
        self.datas=[]
    def add_data(self,sensor_data):
        if len(self.sensor_datas):
            #need init
            self.app = sensor_data.app
        self.sensor_datas.append(sensor_data)

    def fake_gen(self,case_id):
        if case_id=="case1":
            data = random.randint(0, 99)
            self.datas.append(data)

    def get_last_records(self,record_cnt):
        if len(self.sensor_datas)>=record_cnt:
            record_need=record_cnt
        else:
            record_need = len(self.sensor_datas)
        return self.sensor_datas[-record_need:]

    def get_values(self,latest_cnt,type_id):
        #print("in sensor.get_values")
        values_x = []
        values_y = []
        bnext=0
        value_y_prev=0
        for data in self.sensor_datas:
            if data.valid==1 and data.filter_out==False:
                if type_id==1: # get current sensor value
                    value_x = data.datatime
                    values = data.get_values("")
                    value_y = float(values[sEtting.sensor_cur])
                    values_x.append(value_x)
                    values_y.append(value_y)
                if type_id==2: # get current sensor value diff
                    value_x = data.datatime
                    values = data.get_values("")
                    value_y = float(values[sEtting.sensor_cur])
                    if bnext==0: #init
                        bnext=1
                    else:
                        values_x.append(value_x)
                        values_y.append(value_y - value_y_prev)
                    value_y_prev = value_y

        return (values_x[-latest_cnt:], values_y[-latest_cnt:])

    def desc(self):
        print("dev_id=" + self.id + "," + "app=" + self.app )
        for sensor_data in self.sensor_datas:
            sensor_data.desc()
        #print("\tfake_data:" + str(self.datas))


#Spec: Sensor data sets
class Devices:
    def __init__(self):
        self.datas=[] # maintain all data records
        self.devs={} # maintain all current devices

    def reset(self):
        self.datas=[]

    def add(self,payload):
        sensor_data = SensorData(payload)
        if sensor_data.valid==1:
            self.datas.append(sensor_data)
            device_id = sensor_data.value_dict["device_id"]
            if not (device_id in self.devs):
                #sensor.desc()
                self.devs[device_id]=Device(device_id)
                #print("add sensor_id=" + device_id)
            self.devs[device_id].add_data(sensor_data)
            return sensor_data
        else:
            return None
        #print(sensor_data.get_values_str())
        #self.desc()

    def add_device(self,dev): # sensor is Sensor()
        self.devs.append(dev)

    def get_values(self,latest_cnt):
        values_x = []
        values_y = []
        for data in self.datas:
            if data.valid==1 and data.filter_out==False:
                value_x = data.datatime
                values = data.get_values("")
                value_y = float(values[sEtting.sensor_cur])
                values_x.append(value_x)
                values_y.append(value_y)

        return (values_x[-latest_cnt:], values_y[-latest_cnt:])
    #clear filter_out flag

    def filter_clear(self):
        for data in self.datas:
            data.filter_out=False
    #the data not in the range of datetime(start), datetime(end) filter_out=True
    #previous filter be cancel automatically
    def filter_datetime(self, start, end):
        for data in self.datas:
            data.filter_out = False
            if data.datatime < start or data.datatime > end:
                data.filter_out=True

    def desc(self):
        print("All data count=" + str(len(self.datas)))
        print("The data here only include non-filter out data!")
        data_cnt =0
        for data in self.datas:
            if not data.filter_out:
                data_cnt = data_cnt+1
                print(data.raw)
        print("In range data count=" + str(data_cnt))
        print("----- devices information -----")
        print("devices count=" + str(len(self.devs)))
        for dev in self.devs.values():
            dev.desc()

#Spec: 1 Sensor data
class SensorData:
    def __init__(self,payload):
        #example payload
        #V0.1-V0.2
        #|device_id=LASS-Wuulong|time=2474079|device=LinkItONE|values=14|gps=$GPGGA,103106.000,2448.0291,N,12059.5732,E,1,4,5.89,29.9,M,15.0,M,,*63
        #V0.3
        #08:01:25|LASS/Test/EXAMPLE_APP b'|app=EXAMPLE_APP|device_id=LASS-Wuulong|tick=11569371|date=7/7/15|time=15:18:32|device=LinkItONE|values=0,100,1|gps=$GPGGA,151832.001,2446.5223,N,12050.1608,E,0,0,,-0.0,M,14.9,M,,*64\r'

        self.raw = payload
        self.valid=0 # only use data when valid=1
        self.localtime=datetime.datetime.now()

        # parameters valid after data_processing
        self.value_dict={} # value is string type
        self.datatime=0
        self.app=""
        self.gps_x=0.0
        self.gps_y=0.0
        self.gps_z=0.0
        self.gps_lon=0.0
        self.gps_lat=0.0
        self.gps_alt=0.0
        self.sensor_types=[]
        self.sensor_values=[]
        self.ver_format=0 #int type
        self.ver_app="0.0"   #string type
        self.data_process()
        self.check_valid()
        self.filter_out=False #False: user need this data, True: user don't need this data for now

    def desc(self):
        print("datatime=" + str(self.datatime) + ",filter_out=" + str(self.filter_out) + ",values=" + str(self.get_values("")) )
    #although csv head is the same for every record, it still good to be allocate here.
    def get_csvhead(self):
        #|ver_format=1|app=HELLO_APP|ver_app=0.6|device_id=LASS-Hello|tick=13072946|date=1/8/15|time=16:0:10|device=LinkItONE
#ver_format,app,ver_app,device_id,tick,datetime,device,value0,gps_x,gps_y,gps_z
        csv_head = "ver_format,app,ver_app,device_id,tick,datetime,device"
        csv_head = csv_head + ",gps_lat,gps_lon,gps_alt"
        for type in self.sensor_types:
            csv_head = csv_head +","+type
        if sEtting.debug_enable:
            print "csv_head %s" % csv_head
        return csv_head

    def get_csv(self):
#b'|app=EXAMPLE_APP|device_id=LASS-Example|tick=49484274|date=15/7/15|time=12:15:18|device=LinkItONE|values=47.00,100.00,1.00,856.27,544.52|gps=$GPGGA,121518.000,2447.9863,N,12059.5843,E,1,8,1.53,40.2,M,15.0,M,,*6B\r'
        ret_str = ""
        try:
            ret_str = self.value_dict["ver_format"] + "," + self.app + "," + self.value_dict["device_id"] + "," + self.value_dict["ver_app"] + "," + self.value_dict["tick"] + "," + self.value_dict["date"] + " " + self.value_dict["time"]+ "," + self.value_dict["device"]
            ret_str = ret_str + "," + str(self.gps_lat) + "," + str(self.gps_lon) + "," + str(self.gps_alt)
            for data in self.sensor_values:
                ret_str = ret_str + "," + data
            if sEtting.debug_enable:
                print "csv_str %s" % ret_str
        except :
            print( "[GET_CSV] Oops!  Export get un-expcected data, maybe it's old version's data")
        return ret_str

    def get_jsonhead(self):
        head = 'lass_callback({"type":"FeatureCollection","metadata":{"generated":1395197681000,"url":"https://github.com/LinkItONEDevGroup/LASS","title":"LASS Sensors data","status":200,"api":"0.7.1","count":3},"features":['
        return head

    def get_json(self):
#{"type":"Feature","properties":{"data_d":7.0,"ver_format":2,"fmt_opt":0,"app":"EXAMPLE_APP2","ver_app":"0.7.1","device_id":"LASS-DUST-LJ","tick":160839412,"date":"2015-10-15","time":"06:26:14","device":"LinkItONE","data-0":16100.00,"data-1":100.00,"data-2":1.00,"data-3":0.00,"data-D":9.00,"gps-lat":25.023487,"gps-lon":121.370950,"gps-fix":0,"gps-num":0,"gps-alt":13},"geometry":{"type":"Point","coordinates":[121.370950,25.023487,8.7]},"id":""}
        ret_str = ""
        try:
            str_head = '{"type":"Feature","properties":{'
            str_fmt1 = '"ver_format":%s,"fmt_opt":%s,"app":"%s","ver_app":"%s","device_id":"%s","tick":%s,"date":"%s","time":"%s","device":"%s","gps_lat":%s,"gps_lon":%s,"gps_fix":%s,"gps_num":%s,"gps_alt":%s'
            str_base = str_fmt1 % (self.value_dict["ver_format"],self.value_dict["fmt_opt"],self.value_dict["app"],self.value_dict["ver_app"],self.value_dict["device_id"],self.value_dict["tick"],self.value_dict["date"],self.value_dict["time"],self.value_dict["device"],self.gps_lat,self.gps_lon,self.value_dict["gps_fix"],self.value_dict["gps_num"],self.gps_alt)
            str_fmt2 = '%s%s,%s},"geometry":{"type":"Point","coordinates":[%s,%s,%s]}'
            ret_str = str_fmt2 % (str_head,str_base,self.get_values_str("json"), self.gps_lat, self.gps_lon, self.gps_alt)
            if sEtting.debug_enable:
                print "json_str %s" % ret_str
        except :
            print( "[GET_JSON] Oops!  Export get un-expcected data, maybe it's old version's data")
        return ret_str

    def get_jsontail(self):
        #|ver_format=1|app=HELLO_APP|ver_app=0.6|device_id=LASS-Hello|tick=13072946|date=1/8/15|time=16:0:10|device=LinkItONE
#ver_format,app,ver_app,device_id,tick,datetime,device,value0,gps_x,gps_y,gps_z
        tail = '],"bbox":[-179.463,-60.7674,-2.9,178.4321,67.0311,609.13]});'
        return tail

    #parse the data and form the related variables.
    def data_process(self):
        global datetime_format_def
        if sEtting.debug_enable:
            print("[SensorData] raw=" + self.raw)
        cols=self.raw.split("|")
        for col in cols:
            if sEtting.debug_enable:
                print("col:" + col)
            pars = col.split("=")
            if len(pars)>=2:
                self.value_dict[pars[0]] = pars[1]
        #setup values
        try:
            self.parse_ver()
            if self.ver_app == "0.6.6":
                datetime_format_def = '%d/%m/%y %H:%M:%S'
            elif self.ver_app == "0.7.0":
                datetime_format_def = '%Y-%m-%d %H:%M:%S'
            else:
                datetime_format_def = '%Y-%m-%d %H:%M:%S'
            sEtting.device_id = self.value_dict["device_id"]
            self.parse_gps()
            self.parse_datatime()
            self.parse_app()
            #self.app = self.value_dict["app"]
            if self.ver_app in ["0.6.6" "0.7.0"]:
                self.parse_values()
            if self.ver_format==3:
                self.parse_data()
                pass
            self.valid=1
        except:
            print( "[DATA_PROCESS] Oops!  Data parser get un-expcected data when data_process")
        #self.gps_x = 24.780495 + float(self.value_dict["values"])/10000
        #self.gps_y = 120.979692 + float(self.value_dict["values"])/10000

    def parse_gps(self):
        try:
            if self.ver_app == "0.6.6":
                gps_str = self.value_dict["gps"]
                gps_cols=gps_str.split(",")
                y = float(gps_cols[4])/100
                x = float(gps_cols[2])/100
                z = float(gps_cols[9])

                y_m = (y -int(y))/60*100*100
                y_s = (y_m -int(y_m))*100

                x_m = (x -int(x))/60*100*100
                x_s = (x_m -int(x_m))*100

                self.gps_x = int(x) + float(int(x_m))/100 + float(x_s)/10000
                self.gps_y = int(y) + float(int(y_m))/100 + float(y_s)/10000
                self.gps_z = z
            elif self.ver_app == "0.7.0":
                gps_loc = self.value_dict["gps-loc"].replace("type", "\"type\"").replace("coordinates", "\"coordinates\"")
                gps_fix = self.value_dict["gps-fix"]
                gps_num = self.value_dict["gps-num"]
                gps_alt = self.value_dict["gps-alt"]
                gps_coor = list(json.loads(gps_loc)["coordinates"])

                r = 6371000 + float(gps_alt)
                self.gps_x = r*math.cos(float(gps_coor[0]))*math.cos(float(gps_coor[1]))
                self.gps_y = r*math.cos(float(gps_coor[0]))*math.sin(float(gps_coor[1]))
                self.gps_z = r*math.sin(float(gps_coor[0]))
            else:
                gps_lat = self.value_dict["gps_lat"]
                gps_lon = self.value_dict["gps_lon"]
                gps_alt = self.value_dict["gps_alt"]
                #gps_fix = self.value_dict["gps_fix"]
                #gps_num = self.value_dict["gps_num"]

                ''' transform from (lat,lon,alt) to (x,y,z), not use currently
                #r = 6378137.0 + float(gps_alt)
                #self.gps_x = r*math.cos(float(gps_lat))*math.cos(float(gps_lon))
                #self.gps_y = r*math.cos(float(gps_lat))*math.sin(float(gps_lon))
                #self.gps_z = r*math.sin(float(gps_lat))
                '''

                self.gps_lat = self.gps_to_map(float(gps_lat))
                self.gps_lon = self.gps_to_map(float(gps_lon))
                self.gps_alt = self.gps_to_map(float(gps_alt))

            if sEtting.debug_enable:
                print("[parse_gps] gps_lat=" + str(self.gps_lat) + ",gps_lon=" + str(self.gps_lon)+ ",gps_alt=" + str(self.gps_alt))
        except:
            print("[PARSE_GPS] Oops!  Data parser get un-expcected data when parse_gps")

    def parse_datatime(self):
        date_str = self.value_dict["date"]
        time_str = self.value_dict["time"]
        self.datatime = datetime.datetime.strptime( date_str + " " + time_str, datetime_format_def)

    def parse_app(self):
        self.app = self.value_dict["app"]

    def parse_values(self):
        self.sensor_values = self.value_dict["values"].split(",")

    def parse_data(self):
        data_string = self.get_values_str()
        data_cols = data_string.split(",")
        for col in data_cols:
            pair = col.split("=")
            if len(pair) >= 2:
                self.sensor_types.append(pair[0])
                self.sensor_values.append(pair[1])
        if sEtting.debug_enable:
            print '[parse_data sensor_types] [%s]' % ', '.join(map(str, self.sensor_types))
            print '[parse_data sensor_values] [%s]' % ', '.join(map(str, self.sensor_values))

    def parse_ver(self):
        try:
            self.ver_format=int(self.value_dict["ver_format"])
            self.ver_app = self.value_dict["ver_app"]
        except :
            pass

    #check if data valid and apply filter
    def check_valid(self):
        if sEtting.filter_par_type==1:
            if self.value_dict["device_id"]==sEtting.device_id:
                self.valid=1
            else:
                self.valid=0
        if sEtting.filter_par_type==2:
            if self.value_dict["ver_format"]==str(sEtting.ver_format):
                self.valid=1
            else:
                self.valid=0

    #transfer gps value to google map format
    def gps_to_map(self,x):
        x_m = (x -int(x))/60*100*100
        x_s = (x_m -int(x_m))*100
        gps_x = int(x) + float(int(x_m))/100 + float(x_s)/10000
        return gps_x

    def get_values_str(self,output_type=""): # currently return "" if not valid. The return type is string
        values=""
        if self.valid!=1:
            return values

        sensors_cnt=0
        if self.ver_format ==3:
            for key in self.value_dict.keys():
                if key.startswith("s_"):
                    if sensors_cnt==0:
                        if output_type=="json":
                            values = '"' + key + '"' + ":" + self.value_dict[key]
                        else:
                            values = key + "=" + self.value_dict[key]
                    else:
                        if output_type=="json":
                            values = values + "," + '"' + key + '"' + ":" + self.value_dict[key]
                        else:
                            values = values + "," + key + "=" + self.value_dict[key]

                    sensors_cnt=sensors_cnt+1
        else:
            print ("ver_format:" + str(self.ver_format))

        return values
    def get_values(self,output_type=""):
        #default, return all values as list
        #values=self.value_dict["values"].split(",")
        values = self.get_values_str()
        return values


#Spec: export KML
class ExportKml:
    def __init__(self):
        self.kml = simplekml.Kml()
    #values is the sensor data list
    # datavalue is string
    def add_point1(self,point_name, gps_lat,gps_lon, gps_alt, datavalue, app):
        pnt = self.kml.newpoint(name=point_name, coords=[(gps_lon, gps_lat, gps_alt)])
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/green-dot.png
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/orange-dot.png
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/yellow-dot.png
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/red-dot.png
        pnt.style.iconstyle.icon.href = self.value_to_icon(datavalue, app)
        cols = datavalue.split(",")
        for col in cols:
            pars = col.split("=")
            if len(pars)>=2:
                pnt.extendeddata.newdata(pars[0],pars[1])

    #app customize
    #the logic related to value and the icon by app name
    def value_to_icon(self,datavalue,app):
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/orange-dot.png
        icon_list=["http://www.google.com/intl/en_us/mapfiles/ms/icons/red-dot.png",
                   "http://www.google.com/intl/en_us/mapfiles/ms/icons/yellow-dot.png",
                   "http://www.google.com/intl/en_us/mapfiles/ms/icons/green-dot.png"]

        icon_url = ""
        if app=="EXAMPLE_APP":
            values = datavalue.split(",")
            value = float(values[sEtting.sensor_cur]) #sound sensor
            if value>5000:
                icon_url = icon_list[0]
            else:
                if value>2000:
                    icon_url = icon_list[1]
                else:
                    icon_url = icon_list[2]
        elif app=="EXAMPLE_APP1":
            values = datavalue.split(",")
            value = float(values[sEtting.sensor_cur]) #sound sensor
            if value>100:
                icon_url = icon_list[0]
            else:
                if value>50:
                    icon_url = icon_list[1]
                else:
                    icon_url = icon_list[2]

        else:
            return icon_list[2]
        return icon_url
    def export(self,filename):
        self.kml.save(filename)

#LASS CLI i
class LassCli(cmd.Cmd):
    """LASS cli."""

    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = 'LASS>'
        self.cli_export = CliExport()
        self.cli_sEtting = CliSetting()
        self.cli_data = CliData()
        self.monitor_thread = None
        if sEtting.auto_monitor:
            self.do_monitor("")

############ cli maintain ####################
    def do_quit(self, line):
        """quit"""
        if self.monitor_thread:
            self.monitor_thread.exit=True
            time.sleep(1)
        if data_log_file:
            data_log_file.close()
        if data_file:
            data_file.close()
        return True

    def do_monitor(self, line):
        """ monitor sensor data
         """
        if self.monitor_thread==None:
            cLient.connect("gpssensor.ddns.net", 1883, 60)
            self.monitor_thread = MonitorThread()
            self.monitor_thread.start()
        else:
            print("MQTT client already exist! Don't start again")


    def do_stop(self,line):
        """ stop monitor sensor data.
        stop
        ex: stop"""
        if self.monitor_thread:
            self.monitor_thread.exit=True
        time.sleep(2)
        self.monitor_thread = None

    def do_savedata(self,line):
        """ save raw data.
        savedata [filename]
        ex: savedata lass.raw"""
        pars=line.split()
        filename = "lass.raw"
        if len(pars)==1:
            filename = pars[0]
        f_raw = open(filename, 'w')
        for data in dEvices.datas:
            line1 = data.raw
            print(line1)
            f_raw.write(line1 + "\n")
        f_raw.close()

    def do_loaddata(self,line):
        """ load raw data.
        loaddata [filename]
        ex: loaddata lass.raw"""
        pars=line.split()
        filename = "lass.raw"
        if len(pars)==1:
            filename = pars[0]
        f_raw = open(filename, 'r')
        dEvices.reset()
        for line1 in f_raw:
            dEvices.add(str(line1))
        f_raw.close()

    def do_about(self, line):
        """ About LASS
        LASS version """
        print("----- LASS V" + str(VERSION) + " -----")

############ ethmgr sub cmd ####################
    def do_export(self,line):
        """export : export functions
        export sub command"""
        self.cli_export.prompt = self.prompt[:-1]+':export>'
        self.cli_export.cmdloop()

############ setting sub cmd ####################
    def do_setting(self,line):
        """setting functions
        setting sub command"""
        self.cli_sEtting.prompt = self.prompt[:-1]+':setting>'
        self.cli_sEtting.cmdloop()

############ data sub cmd ####################
    def do_data(self,line):
        """data related functions
        data sub command"""

        self.cli_data.prompt = self.prompt[:-1]+':data>'
        self.cli_data.cmdloop()
############ setting sub cmd ####################
class CliSetting(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)

    def do_topic(self, line):
        """ Setting for MQTT topic
        topic [topic_name]
        ex: topic Sensors/#"""
        pars=line.split()
        topic = "Sensors/#"
        if len(pars)==1:
            topic = pars[0]
        sEtting.mqtt_topic = topic

    def do_deviceid(self, line):
        """ Setting for MQTT device_id
        deviceid [deviceid]
        ex: deviceid LASS-Example"""
        pars=line.split()
        deviceid = "LASS-Example"
        if len(pars)==1:
            deviceid = pars[0]
        sEtting.device_id = deviceid

    def do_ver_format(self, line):
        """ Setting for filter ver_format
        ver_format [ver_format]
        ex: ver_format 3"""
        pars=line.split()
        ver_format = 3
        if len(pars)==1:
            deviceid = pars[0]
        sEtting.ver_format = int(ver_format)

    def do_debug(self, line):
         sEtting.debug_enable = 1 if sEtting.debug_enable is 0 else 0

    def do_sensor_curr(self,line):
        """ Setting for current monitor sensor position, pos range: 0 - sensor_cnt-1
        sensor_curr [sensor_pos]
        ex: sensor_curr 4"""
        pars=line.split()
        sensor_curr = 4
        if len(pars)==1:
            sensor_curr = int(pars[0])
            sEtting.sensor_cur = sensor_curr
            sensorPlot.first=1
            sensorPlot.plot(1)

    def do_plot_save(self,line):
        """ setting plot with real time display or save to file
        plot_save 0/1 # 0: plot with real time 1: save plot
        ex: plot_save 1"""
        pars=line.split()
        if len(pars)==1:
            plot_save = int(pars[0])
            sEtting.plot_save = plot_save

    def do_plot_enabled(self,line):
        """ setting plot enable or disable
        plot_enabled 0/1 # 0: disable plot 1: enable plot
        ex: plot_enabled 1"""
        pars=line.split()
        if len(pars)==1:
            plot_enabled = int(pars[0])
            sEtting.plot_enabled = plot_enabled

    def do_filter_par_type(self,line):
        """ setting filter par type
        filter_par_type 0/1/2 # 0: disable filter 1: filter device_id, 2: filter ver_format
        ex: filter_par_type 1"""
        pars=line.split()
        if len(pars)==1:
            filter_par_type = int(pars[0])
            sEtting.filter_par_type = filter_par_type

    def do_plot_cnt(self,line):
        """ Setup the samples count in the plot
        plot_cnt [samples_count] #
        ex: plot_cnt 100"""
        pars=line.split()
        if len(pars)==1:
            plot_cnt = int(pars[0])
            sEtting.plot_cnt = plot_cnt

    def do_show(self, line):
        """ Show current setting
        ex: show """
        print("Topic=%s\nDeviceId=%s\nsensor_cur=%i\nplot_enabled=%i\nplot_save=%i,\nfilter_par_type=%i,\nplot_count=%i\nver_format=%i" % (sEtting.mqtt_topic,sEtting.device_id,sEtting.sensor_cur,sEtting.plot_enabled,sEtting.plot_save,sEtting.filter_par_type,sEtting.plot_cnt,sEtting.ver_format))

    def do_quit(self, line):
        """quit"""
        return True


############ export sub cmd ####################
class CliExport(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
    def do_kml(self, line):
        """ export to KML.
        kml [filename]
        ex: kml lass.kml"""
        pars=line.split()
        filename = "lass.kml"
        if len(pars)==1:
            filename = pars[0]
        ekml = ExportKml()
        for data in dEvices.datas:
            if sEtting.kml_export_type==0:
                #pnt = ekml.add_point1(sEtting.device_id + "_" + data.datatime.strftime("%X"),data.gps_x,data.gps_y,data.gps_z,"sensor_value",data.get_values_str(), data.app)
                ekml.add_point1(sEtting.device_id + "_" + data.datatime.strftime("%X"),data.gps_lat,data.gps_lon,data.gps_alt,data.get_values(), data.app)
        ekml.export(filename)

    def do_csv(self,line):
        """ export to CSV for later on analyzer
        R is good good that you can use read.table("lass.csv",sep = ",",header = TRUE) to import your data
        csv [filename]
        ex: kml lass.csv"""
        pars=line.split()
        filename = "lass.csv"
        if len(pars)==1:
            filename = pars[0]
        csv_file = open(filename, 'w')
        #csv_file.write(csv_str + "\n")
        bnext=0
        for data in dEvices.datas:
            if bnext==0:
                csv_head = data.get_csvhead()
                csv_file.write(csv_head + "\n")
                bnext=1
            csv_str = data.get_csv()
            csv_file.write(csv_str + "\n")
        csv_file.close()

    def do_json(self,line):
        """ export to GeoJSON for html display
        json [filename] [each_device_record_cnt=10000]
        ex: json lass.json 1"""
        pars=line.split()
        filename = "lass.json"
        record_cnt = 10000
        if len(pars)>=1:
            filename = pars[0]
        if len(pars)>=2:
            record_cnt = int(pars[1])
        json_file = open(filename, 'w')
        bnext=0

        for dev in dEvices.devs.values():
            sensors_data = dev.get_last_records(record_cnt)
            for data in sensors_data:
                if bnext==0:
                    json_head = data.get_jsonhead()
                    json_file.write(json_head + "\n")
                    bnext=1
                json_str = data.get_json()
                json_file.write(json_str + "},\n")
        json_tail = data.get_jsontail()
        json_file.write(json_tail + "\n")
        json_file.close()

    def do_quit(self, line):
        """quit"""
        return True

############ data sub cmd ####################
class CliData(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)

    def do_desc(self,line):
        """ Show current sensor data.
        desc
        ex: desc"""
        dEvices.desc()

    def do_filter_by_datetime(self,line):
        """ filter data, only keep data in the datetime range, please input the same date time format.
        filter_by_datetime [datetime_start] [datetime_end]
        ex: filter_by_datetime 30/6/15 1:0:0,1/7/15 23:00:00"""
        pars=line.split(",")
        try:
            if len(pars)==2:
                datestr_start = pars[0]
                datestr_end = pars[1]
                datetime_start = datetime.datetime.strptime( datestr_start, datetime_format_def)
                datetime_end = datetime.datetime.strptime( datestr_end, datetime_format_def)
                dEvices.filter_datetime(datetime_start,datetime_end)
                print("data range from start" + str(datetime_start) + "to end=" + str(datetime_end))
            else:
                print("Parameters count should be 2!")
        except ValueError:
            print ("User input with un-expected data format!")

    def do_gps_to_map(self,line):
        """ transform gps data value to google map format
        gps_to_map [Latitude,gps_lat],[Longitude, gps_lon]
        ex: gps_to_map 25.024037,121.368875"""

        pars=line.split(",")
        if len(pars)==2:
            y = float(pars[0])
            x = float(pars[1])

            y_m = (y -int(y))/60*100*100
            y_s = (y_m -int(y_m))*100

            x_m = (x -int(x))/60*100*100
            x_s = (x_m -int(x_m))*100

            gps_x = int(x) + float(int(x_m))/100 + float(x_s)/10000
            gps_y = int(y) + float(int(y_m))/100 + float(y_s)/10000

            print("google map format(Latitude,Longitude,altitude): %f,%f" %(gps_y,gps_x))

    def do_fake_gen(self,line):
        """ Fake data generator for analyze purpose
        fake_gen [case_name]
        ex: fake_gen 1"""
        pars=line.split(",")
        if len(pars)==1:
            fAker.gen_by_case(pars[0])
            dEvices.desc()

    def do_server_import(self,line):
        """ Import data from server's log
        server_import [server_data_filename]
        ex: server_import server.data"""
        pars=line.split()
        filename = "server.data"
        if len(pars)==1:
            filename = pars[0]

        f_raw = open(filename, 'r')
        #LASS/Test/EXAMPLE_APP |ver_format=1|app=EXAMPLE_APP|ver_app=0.6.2|device_id=LASS-Example0|tick=239775|date=6/1/80|time=0:3:39|device=LinkItONE|values=100.00,1.00,50.00,0.62,856.27|gps=$GPGGA,000339.000,2447.9796,N,12059.5357,E,0,0,,135.0,M,15.0,M,,*46
        lines = f_raw.readlines()
        dEvices.reset()
        for line in lines:
            m = re.search('(\S+)\s(.+)', line)
            line_raw = m.group(2)
            dEvices.add(str(line_raw))
        f_raw.close()
        #lines_str = "\n".join(lines)

        #m.group(0)

        #for m1  in m.group(1):

        #    print("raw:"+ m1)
            #
        pass

    def do_plot(self,line):
        """ plot selected data
        plot [plot_id]  #0: all data, 1: one sensor data, 2: one sensor diff data
        ex: plot 1"""
        pars=line.split(",")
        if len(pars)==1:
            sensorPlot.plot(int(pars[0]))

    def do_quit(self, line):
        """quit"""
        return True

#MQTT thread
class MonitorThread(threading.Thread):
    def __init__(self, wait=0.01):
        threading.Thread.__init__(self)
        self.event = threading.Event()
        self.wait = wait
        self.exit = False

    def set_ts(self, ts):
        self.wait = ts

    def do_function(self):
        # Blocking call that processes network traffic, dispatches callbacks and
        # handles reconnecting.
        # Other loop*() functions are available that give a threaded interface and a
        # manual interface.
        #cLient.loop_forever()
        cLient.loop(1.0,100)

    def run(self):
        while 1:
            if self.exit:
                break
            self.do_function()
            self.event.wait(self.wait)

#the parameters for fiter, analyze purpose
class AnalyzePar():
    def __init__(self):
        #valid mean positive list, invalid_mean nagtive list
        self.v_devices=[] #device_id
        self.iv_devices=[]
    #if exist(v), need in v list
    #then can't in iv list
    def device_ok(self,dev):
        ret = True
        if len(self.v_devices)>0:
            if dev in self.v_devices:
                return True
            else:
                return False
        else:
            if dev in self.iv_devices:
                return False
            else:
                return True


#fake data generation, for analyze purpose
class FakeDataGenerator():
    def __init__(self):
        pass
    def gen_value(self, record_id, gen_type):
        value = 0
        if gen_type==0:
            value = record_id + random.randint(0,3)/10.0
        return value
    def gen_by_case(self,case_id):
        data_cnt = 10
        device_cnt=1
        if case_id=="1":
            #sample_str = "b'|app=EXAMPLE_APP|device_id=LASS-Example|tick=49484274|date=15/7/15|time=12:15:18|device=LinkItONE|values=47.00,100.00,1.00,856.27,544.52|gps=$GPGGA,121518.000,2447.9863,N,12059.5843,E,1,8,1.53,40.2,M,15.0,M,,*6B\r'"
            app = "EXAMPLE_APP"
            device_id="LASS-Example"
            value_str = "47.00,100.00,1.00,856.27,544.52"
            date_str="15/7/15"
            time_str="12:15:18"

            for i in range(0,data_cnt): #data count
                device_id="LASS-Example" + str(random.randint(0,device_cnt-1)) #device cnt
                sample_str = "b'|app=" + app + "|device_id=" + device_id + "|tick=49484274|date=" + date_str + "|time=" + time_str + "|device=LinkItONE|values=" + value_str + "|gps=$GPGGA,121518.000,2447.9863,N,12059.5843,E,1,8,1.53,40.2,M,15.0,M,,*6B\r'"
                sensor_data = dEvices.add(sample_str)
                sensor_data.datatime = sensor_data.datatime + datetime.timedelta(minutes=1*i) #days, seconds, microseconds, minutes, hours, weeks
                sensor_data.values[sEtting.sensor_cur]= self.gen_value(i,0)

def init_config():
    global sEtting
    global dEvices
    global sensorPlot
    global aNalyzePar

    sEtting = Setting()
    dEvices = Devices()
    sensorPlot = SensorPlot()
    aNalyzePar = AnalyzePar()

def main():
    global cLient
    init_config()

    print("----- LASS V" + str(VERSION) + " -----")
    parser = argparse.ArgumentParser()
    parser.add_argument('--version', action='store_true', default=None, help="Get current LASS.PY Version.")
    parser.add_argument('--testtopic', action='store_true', default=None, help="Test through the Topic: LASS/Test/+."
                                                               "\n device_id=LASS-Example")
    parser.add_argument('--lasscmd',action='store_true', default=None, help="LASS commandline interface.")
    parser.add_argument('--testdev',action='store_true', default=None, help="Run offline unit test.")
    args = parser.parse_args()

    if(args.version is not None):
        print("----- LASS.PY V" + str(VERSION) + " -----")

    if(len(sys.argv) == 1) or (args.testtopic is not None):
        sEtting.auto_monitor = 1
        #fAker = FakeDataGenerator()
        cLient = mqtt.Client()
        cLient.on_connect = on_connect
        cLient.on_message = on_message
        cLient.loop_start
        #client.connect("gpssensor.ddns.net", 1883, 60)
        #client.loop_forever()
        LassCli().cmdloop()

    if args.lasscmd is not None:
        sEtting.auto_monitor = 0
        #fAker = FakeDataGenerator()
        cLient = mqtt.Client()
        cLient.on_connect = on_connect
        cLient.on_message = on_message
        cLient.loop_start
        #client.connect("gpssensor.ddns.net", 1883, 60)
        #client.loop_forever()
        LassCli().cmdloop()


    if args.testdev is not None:
        if VERSION == "0.7.5":
            ##APP_VERSION == "0.7.5" ver_format=3
            test_log = "LASS/Test/PM25 |ver_format=3|fmt_opt=0|app=PM25|ver_app=0.7.5|device_id=FT1_003|" \
                       "tick=5447049|date=2015-10-18|time=06:26:25|device=LinkItONE|" \
                       "s_0=459.00|s_1=66.00|s_2=1.00|s_3=0.00|s_d0=33.00|s_t0=22.60|s_h0=83.50|" \
                       "gps_lat=25.025452|gps_lon=121.371092|gps_fix=1|gps_num=9|gps_alt=3"
        elif VERSION == "0.7.0":
            test_log = "LASS/Test/MAPS |ver_format=1|fmt_opt=0|app=MAPS|ver_app=0.7.0|device_id=LASS-MAPS-LJ|" \
                    "tick=421323065|date=2015-10-10|time=06:54:42|device=LinkItONE|" \
                    "values=37249.00,100.00,1.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,1007.83,26.70,81.20,6.00,0.00,0.00,0.00,0.00,0.00,0.00|" \
                    "data-0=37249.00|data-1=100.00|data-2=1.00|data-3=0.00|data-B=1007.83|data-T=26.70|data-H=81.20|data-L=6.00|" \
                    "gps-loc={ type:\"Point\",  coordinates: [25.024463 , 121.368752 ] }|gps-fix=1|gps-num=6|gps-alt=3"
        elif VERSION == "0.6.6":
            test_log = "LASS/Test/MAPS |ver_format=1|fmt_opt=0|app=MAPS|ver_app=0.6.6|device_id=LASS-MAPS-LJ|" \
                   "tick=71787795|date=1/10/15|time=0:52:48|device=LinkItONE|" \
                   "values=6397.00,100.00,1.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,1010.88,33.00,99.90,119.00,0.00,0.00,0.00,0.00,0.00,0.00|" \
                   "gps=$GPGGA,005248.009,3024.2268,S,13818.7933,E,0,0,,-2001.4,M,12.7,M,,*44"
        elif VERSION == "0.7.1":
            test_log = "LASS/Test/MAPS |ver_format=2|fmt_opt=0|app=MAPS|ver_app=0.7.1|device_id=LASS-MAPS-LJ|" \
                   "tick=8521980|date=2015-10-19|time=06:29:41|device=LinkItONE|" \
                   "data-0=679.00|data-1=100.00|data-2=1.00|data-3=0.00|data-B=1001.64|data-T=26.90|data-H=72.70|data-L=24.00|" \
                   "gps-lat=25.040351|gps-lon=121.387630|gps-fix=0|gps-num=0|gps-alt=1"
        else:
            test_log = ""
            print "Not support this version: %s " % (VERSION)
        
        if test_log:
            dEvices.add(test_log)

if __name__ == "__main__":
    main()







