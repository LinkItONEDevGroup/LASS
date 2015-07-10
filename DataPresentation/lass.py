#
# README: please install the library before use
#       :     Require Libraries:
#                MQTT https://pypi.python.org/pypi/paho-mqtt
#                matplotlib http://matplotlib.org/
#                    It's easiler if you just install http://continuum.io/
#                simplekml http://simplekml.readthedocs.org/
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
import paho.mqtt.client as mqtt
import threading
#plot
import matplotlib.pyplot as plt
import datetime
#kml
import simplekml
#cli
import cmd


VERSION=0.3
data_log_file=None
datetime_format_def = '%d/%m/%y %H:%M:%S'
    
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")
    #topic="Sensors/#"
    client.subscribe(setting.mqtt_topic, qos=0)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    console_str = datetime.datetime.now().strftime("%X") + "|" +msg.topic+" "+str(msg.payload)
    print(console_str)
    if setting.log_enabled:
        global data_log_file
        if data_log_file==None:
            data_log_file = open("lass_data_" + datetime.datetime.now().strftime("%Y%m%d") + ".log", 'w+')
        data_log_file.write(console_str + "\n")
                            
    sensor_datas.add(str(msg.payload))
    if setting.plot_enabled:
        sensor_plot.plot()

        
# the setting for the program
class Setting:
    def __init__(self):
        #system general setting
        self.debug_enable=0 #0: debug disable , 1: debug enable
        self.plot_cnt=90 # the value count in plotter, if 10 seconds for 1 value, about 15 min.
        
        self.mqtt_topic="LASS/#"  #REPLACE: to your sensor topic
        self.device_id="LASS-Example"
        self.filter_deviceid_enable=0 # the filter make you focus on this device_id
        
        self.kml_export_type=0 # default kml export type. name = deviceid_localtime
        self.plot_enabled=0 # 0: realtime plot not active, 1: active plot
        self.log_enabled=1 # 0: not auto save receive data, 1: auto save receive data
    
# Sensor plot funcition    
class SensorPlot:
    def __init__(self):
        self.first=1 # first run

    def init(self):
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111)
    def plot(self):
        x, y = sensor_datas.get_values(setting.plot_cnt)

        if self.first:
            self.init()
            plt.title(setting.mqtt_topic + ' Sensor data')
            plt.ylabel('Sensor value')
            plt.xlabel("Data Receive time")
            plt.gcf().autofmt_xdate()
            (self.li, )= self.ax.plot(x, y)
            
            # draw and show it
            self.fig.canvas.draw()
            plt.show(block=False)
        else:
            self.li.set_xdata(x)
            self.li.set_ydata(y)
            self.fig.canvas.draw()
               
#Spec: Sensor data sets
class SensorDatas:
    def __init__(self):
        self.datas=[]
    def reset(self):
        self.datas=[]
    def add(self,payload):
        sensor_data = SensorData(payload)
        self.datas.append(sensor_data)
        #print(sensor_data.get_values_str())
        #self.desc()
    def get_values(self,latest_cnt):
        values_x = []
        values_y = []
        for data in self.datas:
            if data.valid==1 and data.filter_out==False:
                value_x = data.datatime
                values = data.get_values("")
                value_y = float(values[0])
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
        
        self.data_process()
        self.check_valid()
        self.filter_out=False #False: user need this data, True: user don't need this data for now
    
    #parse the data and form the related variables.        
    def data_process(self):
        #print("raw=" + self.raw)
        cols=self.raw.split("|")
        for col in cols:
            if setting.debug_enable:
                print("col:" + col)
            pars = col.split("=")
            if len(pars)>=2:
                self.value_dict[pars[0]] = pars[1]
        #setup values
        try:
            self.parse_gps()
            self.parse_datatime()
        except ValueError:
            print( "Oops!  Data parser get un-expcected dta")
        #self.gps_x = 24.780495 + float(self.value_dict["values"])/10000
        #self.gps_y = 120.979692 + float(self.value_dict["values"])/10000
    def parse_gps(self):
        gps_str = self.value_dict["gps"]
        gps_cols=gps_str.split(",")
        self.gps_y = float(gps_cols[4])/100
        self.gps_x = float(gps_cols[2])/100
    def parse_datatime(self):
        date_str = self.value_dict["date"]
        time_str = self.value_dict["time"]
        self.datatime = datetime.datetime.strptime( date_str + " " + time_str, datetime_format_def)
    def parse_app(self):
        self.app = self.value_dict["app"]
    #check if data valid and apply filter
    def check_valid(self):
        if setting.filter_deviceid_enable==1:
            if  self.value_dict["device_id"]==setting.device_id:
                self.valid=1
            else:
                self.valid=0
        else:
            self.valid=1
    def get_values_str(self): # currently return "" if not valid. The return type is string
        if self.valid!=1:
            return ""
        return self.value_dict["values"]
    def get_values(self,ap_sub_type):
        #default, return all values as list
        values=self.value_dict["values"].split(",")
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
    def add_point1(self,point_name, coord_x,coord_y, dataname, datavalue):
        pnt = self.kml.newpoint(name=point_name, coords=[(coord_y, coord_x)])
        pnt.extendeddata.newdata(dataname,datavalue)
    
    def export(self,filename):
        self.kml.save(filename)

#LASS CLI i
class LassCli(cmd.Cmd):
    """LASS cli."""
    
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = 'LASS>'
        self.cli_export = CliExport()
        self.cli_setting = CliSetting()
        self.cli_data = CliData()
        self.monitor_thread = None
        #self.do_monitor("")

############ cli maintain ####################        
    def do_quit(self, line):
        """quit"""
        if self.monitor_thread:
            self.monitor_thread.exit=True
        if data_log_file:
            data_log_file.close()
        return True
    
    def do_monitor(self, line):
        """ monitor sensor data
         """
        if self.monitor_thread==None:
            client.connect("gpssensor.ddns.net", 1883, 60) 
            self.monitor_thread = MonitorThread()
            self.monitor_thread.start()
        else:
            print("MQTT client already exist! Don't start again")

    def do_savedata(self,line):
        """ save raw data.
        savedata [filename]
        ex: savedata lass.raw"""
        pars=line.split()
        filename = "lass.raw"
        if len(pars)==1:
            filename = pars[0]
        
        f_raw = open(filename, 'w')
        for data in sensor_datas.datas:
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
        sensor_datas.reset()
        for line1 in f_raw:
            sensor_datas.add(str(line1))
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
    
        self.cli_setting.prompt = self.prompt[:-1]+':setting>'
        self.cli_setting.cmdloop()
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
        
        setting.mqtt_topic = topic
    def do_deviceid(self, line):
        """ Setting for MQTT device_id
        deviceid [deviceid]
        ex: deviceid LASS-Example"""
        pars=line.split()
        deviceid = "LASS-Example"
        if len(pars)==1:
            deviceid = pars[0]
        
        setting.device_id = deviceid

    def do_show(self, line):
        """ Show current setting
        ex: show """        
        print("Topic=%s\nDeviceId=%s" % (setting.mqtt_topic,setting.device_id))

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
        for data in sensor_datas.datas:
            if setting.kml_export_type==0:
                ekml.add_point1(setting.device_id + "_" + data.localtime.strftime("%X"),data.gps_x,data.gps_y,"sensor_value",data.get_values_str())
        ekml.export(filename)

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
        sensor_datas.desc()
            
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
                print("start=" + str(datetime_start) )
                print("end=" + str(datetime_end) )
            else:
                print("Parameters count should be 2!")
        except ValueError:
            print ("User input with un-expected data format!")
        sensor_datas.filter_datetime(datetime_start,datetime_end)
        
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
        #client.loop_forever()
        client.loop(1.0,100)
    def run(self):
        while 1:
            if self.exit:
                break
            self.do_function()            
            self.event.wait(self.wait)

        
print("----- LASS V" + str(VERSION) + " -----")


setting = Setting()            
sensor_datas = SensorDatas()   
sensor_plot = SensorPlot()     
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

#client.connect("gpssensor.ddns.net", 1883, 60) 
#client.loop_forever()
LassCli().cmdloop()
 








