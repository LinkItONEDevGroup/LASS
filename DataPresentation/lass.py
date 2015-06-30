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


VERSION=0.2
    
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
    print(msg.topic+" "+str(msg.payload))
    sensor_datas.add(str(msg.payload))
    #sensor_plot.plot()

        
# the setting for the program
class Setting:
    def __init__(self):
        #system general setting
        self.debug_enable=0 #0: debug disable , 1: debug enable
        self.plot_cnt=90 # the value count in plotter, if 10 seconds for 1 value, about 15 min.
        
        self.mqtt_topic="Sensors/#"#"Sensors/SoundSensor"  #REPLACE: to your sensor topic
        self.device_id="LASS-Example"
        self.filter_deviceid_enable=0 # the filter make you focus on this device_id
        
        self.kml_export_type=0 # default kml export type. name = deviceid_localtime
    
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
               
#Spec: Sensor data default handler
class SensorDatas:
    def __init__(self):
        self.datas=[]
        
    def add(self,payload):
        sensor_data = SensorData(payload)
        self.datas.append(sensor_data)
        print(sensor_data.get_value())
        #self.desc()
    def get_values(self,latest_cnt):
        values_x = []
        values_y = []
        for data in self.datas:
            if data.valid==1:
                value_x = data.localtime
                value_y = float(data.get_value())
                values_x.append(value_x)
                values_y.append(value_y)
            
        return (values_x[-latest_cnt:], values_y[-latest_cnt:])
            
    def desc(self):
        for data in self.datas:
            print(data.raw)

#Spec: Sensor data default handler
class SensorData:
    def __init__(self,payload):
        #example payload 
        #|device_id=LASS-Wuulong|time=2474079|device=LinkItONE|values=14|gps=$GPGGA,103106.000,2448.0291,N,12059.5732,E,1,4,5.89,29.9,M,15.0,M,,*63
        self.raw = payload
        self.valid=0 # only use data when valid=1
        self.localtime=datetime.datetime.now()

        # parameters valid after data_processing
        self.value_dict={} # value is string type
        self.datatime=0
        self.gps_x=0.0
        self.gps_y=0.0
        
        self.data_process()
        self.check_valid()
            
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
        self.parse_gps()
        #self.gps_x = 24.780495 + float(self.value_dict["values"])/10000
        #self.gps_y = 120.979692 + float(self.value_dict["values"])/10000
    def parse_gps(self):
        gps_str = self.value_dict["gps"]
        gps_cols=gps_str.split(",")
        self.gps_y = float(gps_cols[4])/100
        self.gps_x = float(gps_cols[2])/100
    #check if data valid and apply filter
    def check_valid(self):
        if setting.filter_deviceid_enable==1:
            if  self.value_dict["device_id"]==setting.device_id:
                self.valid=1
            else:
                self.valid=0
        else:
            self.valid=1
    def get_value(self): # currently return "" if not valid. The return type is string
        if self.valid!=1:
            return ""
        return self.value_dict["values"]

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
        #self.do_monitor("")

############ cli maintain ####################        
    def do_quit(self, line):
        """quit"""
        return True
    
    def do_monitor(self, line):
        """ monitor sensor data
         """
        client.connect("gpssensor.ddns.net", 1883, 60) 
        monitor_thread = MonitorThread()
        monitor_thread.start()
        
         
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
                ekml.add_point1(setting.device_id + "_" + data.localtime.strftime("%X"),data.gps_x,data.gps_y,"sensor_value",data.get_value())
        ekml.export(filename)
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
        client.loop_forever()
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
 








