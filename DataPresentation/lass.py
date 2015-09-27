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


VERSION="0.6.5"
data_log_file=None
data_file=None
datetime_format_def = '%d/%m/%y %H:%M:%S'
    
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
    payload_hex = ''.join(format(x, '02x') for x in msg.payload)
    payload_str = str(msg.payload)
    console_str = datetime.datetime.now().strftime("%X") + "|" +msg.topic+ payload_str[2:-1]
    print(console_str)

                       
    sensor_data = dEvices.add(str(msg.payload))
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
        self.debug_enable=0 #0: debug disable , 1: debug enable
        self.plot_cnt=90 # the value count in plotter, if 10 seconds for 1 value, about 15 min.
        
        #self.mqtt_topic="LASS/#"   #REPLACE: to your sensor topic
        
        #self.mqtt_topic="LASS/Test/+"  #REPLACE: to your sensor topic, it do not subscribe device id's channel
        self.mqtt_topic="LASS/Test/+"  #REPLACE: to your sensor topic, it do not subscribe device id's channel
        
        
        self.device_id="LASS-Example"

        self.filter_deviceid_enable=0 # the filter make you focus on this device_id
        
        self.kml_export_type=0 # default kml export type. name = deviceid_localtime
        self.plot_enabled=0 # 0: realtime plot not active, 1: active plot
        self.plot_save=1 # 0: show plot realtime, 1:plot to file
        self.log_enabled=1 # 0: not auto save receive data in log format, 1: auto save receive data in log format
        self.auto_monitor=1 #0: not auto start monitor command, 1: auto start monitor commmand
        # plot, kml marker's color only apply to 1 sensor, this is the sensor id
        #0: battery level, 1: battery charging, 2: ground speed ( Km/hour ) 
        #10: dust sensor, 11: UIdust sensor, 12: sound sensor 

        self.sensor_cur=0   #REPLACE: to your interest current sensor
        
    
# Sensor plot funcition    
class SensorPlot:
    def __init__(self):
        self.first=1 # first run

    def init(self):
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
            else:
                return 
        else:
            x, y = dEvices.get_values(sEtting.plot_cnt,0) #FIXME
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
        self.values=[]
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
        csv_head = csv_head + ",gps_x,gps_y,gps_z"

        for i in range(0,len(self.values)):
            csv_head = csv_head + ",value" + str(i)
        return csv_head
    
    def get_csv(self):
#b'|app=EXAMPLE_APP|device_id=LASS-Example|tick=49484274|date=15/7/15|time=12:15:18|device=LinkItONE|values=47.00,100.00,1.00,856.27,544.52|gps=$GPGGA,121518.000,2447.9863,N,12059.5843,E,1,8,1.53,40.2,M,15.0,M,,*6B\r'        
        ret_str = ""
        try:
            ret_str=  self.value_dict["ver_format"] + "," + self.app + "," + self.value_dict["device_id"] + "," + self.value_dict["ver_app"] + "," + self.value_dict["tick"]+ "," + self.value_dict["date"] + " " + self.value_dict["time"]+ "," + self.value_dict["device"] 
            ret_str = ret_str + "," + str(self.gps_x) + "," + str(self.gps_y) + "," + str(self.gps_z)
    
            for value in self.values:
                ret_str = ret_str + "," + str(value)
        except :
            print( "Oops!  Export get un-expcected data, maybe it's old version's data")
        return ret_str
    #parse the data and form the related variables.        
    def data_process(self):
        #print("raw=" + self.raw)
        cols=self.raw.split("|")
        for col in cols:
            if sEtting.debug_enable:
                print("col:" + col)
            pars = col.split("=")
            if len(pars)>=2:
                self.value_dict[pars[0]] = pars[1]
        #setup values
        try:
            self.parse_gps()
            self.parse_datatime()
            self.app = self.value_dict["app"]
            self.parse_values()
            self.parse_ver()
        except :
            print( "Oops!  Data parser get un-expcected data")
        #self.gps_x = 24.780495 + float(self.value_dict["values"])/10000
        #self.gps_y = 120.979692 + float(self.value_dict["values"])/10000
    def parse_gps(self):
        if self.valid==0:
            return 
        try:
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
    
            print("gps_x=" + str(self.gps_x) + ",gps_y=" + str(self.gps_y)+ ",gps_z=" + str(self.gps_z))
        except :
            print( "Oops!  Data parser get un-expcected data")

    def parse_datatime(self):
        date_str = self.value_dict["date"]
        time_str = self.value_dict["time"]
        self.datatime = datetime.datetime.strptime( date_str + " " + time_str, datetime_format_def)
    def parse_app(self):
        self.app = self.value_dict["app"]
    def parse_values(self):
        self.values=self.value_dict["values"].split(",")
    def parse_ver(self):
        try:
            self.ver_format=int(self.value_dict["ver_format"])
            self.ver_app= self.value_dict["ver_app"]
        except :
            pass
    #check if data valid and apply filter
    def check_valid(self):
        if sEtting.filter_deviceid_enable==1:
            if  self.value_dict["device_id"]==sEtting.device_id:
                self.valid=1
            else:
                self.valid=0
        else:
            self.valid=0
    def get_values_str(self): # currently return "" if not valid. The return type is string
        if self.valid!=1:
            return ""
        return self.value_dict["values"]
    def get_values(self,ap_sub_type):
        #default, return all values as list
        #values=self.value_dict["values"].split(",")
        values = self.values
        if self.app=="SOME_APNAME":
            #customize here
            return values
        else:
            return values
        

#Spec: export KML
class ExportKml:
    def __init__(self):
        self.kml = simplekml.Kml()
    #values is the sensor data list
    # datavalue is string
    def add_point1(self,point_name, coord_x,coord_y, coord_z, dataname, datavalue, app):
        pnt = self.kml.newpoint(name=point_name, coords=[(coord_y, coord_x, coord_z)])
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/green-dot.png 
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/orange-dot.png 
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/yellow-dot.png 
        #http://www.google.com/intl/en_us/mapfiles/ms/icons/red-dot.png
        pnt.style.iconstyle.icon.href = self.value_to_icon(datavalue, app)
        pnt.extendeddata.newdata(dataname,datavalue)
    
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
    def do_sensor_curr(self,line):
        """ Setting for current monitor sensor position, pos range: 0 - sensor_cnt-1
        sensor_curr [sensor_pos]
        ex: sensor_curr 4"""
        pars=line.split()
        sensor_curr = 4
        if len(pars)==1:
            sensor_curr = int(pars[0])
            sEtting.sensor_cur = sensor_curr
            sensorPlot.plot(1)
    def do_plot_save(self,line):
        """ setting plot with real time display or save to file 
        plot_save 0/1 # 0: plot with real time 1: save plot
        ex: plot_save 1"""
        pars=line.split()

        if len(pars)==1:
            plot_save = int(pars[0])
            sEtting.plot_save = plot_save

    def do_show(self, line):
        """ Show current setting
        ex: show """        
        print("Topic=%s\nDeviceId=%s" % (sEtting.mqtt_topic,sEtting.device_id))

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
                pnt = ekml.add_point1(sEtting.device_id + "_" + data.datatime.strftime("%X"),data.gps_x,data.gps_y,data.gps_z,"sensor_value",data.get_values_str(), data.app)
                
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
        
print("----- LASS V" + str(VERSION) + " -----")


sEtting = Setting()            
dEvices = Devices()   
sensorPlot = SensorPlot() 
aNalyzePar= AnalyzePar()   

fAker = FakeDataGenerator() 
cLient = mqtt.Client()
cLient.on_connect = on_connect
cLient.on_message = on_message

#client.connect("gpssensor.ddns.net", 1883, 60) 
#client.loop_forever()
LassCli().cmdloop()





