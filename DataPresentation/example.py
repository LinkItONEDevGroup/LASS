#
# README: please install the library before use
#       :     Require Libraries:
#                MQTT https://pypi.python.org/pypi/paho-mqtt
#                matlabplotlib
# Features:
#    Plot sensor data in realtime.
#    Support device_id filter.
#    User configuable by modify the setting in the code.
# Author:
#    Wuulong, Created 28/06/2015 
#    https://github.com/LinkItONEDevGroup/LASS
import paho.mqtt.client as mqtt
#plot
import matplotlib.pyplot as plt
import datetime

VERSION=0.01

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
    sensor_datas.add(msg.payload)
    sensor_plot.plot()
    
# the setting for the program
class Setting:
    def __init__(self):
        #system general setting
        self.debug_enable=0 #0: debug disable , 1: debug enable
        self.plot_cnt=90 # the value count in plotter, if 10 seconds for 1 value, about 15 min.
        
        self.mqtt_topic="Sensors/SoundSensor"  #REPLACE: to your sensor topic
        self.device_id="LASS-Example"
        self.filter_deviceid_enable=0 # the filter make you focus on this device_id
    
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
        
        self.data_process()
        self.check_valid()
            
    def data_process(self):
        cols=self.raw.split("|")
        for col in cols:
            if setting.debug_enable:
                print("col:" + col)
            pars = col.split("=")
            if len(pars)>=2:
                self.value_dict[pars[0]] = pars[1]
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

print("----- LASS V" + str(VERSION) + " -----")

setting = Setting()            
sensor_datas = SensorDatas()   
sensor_plot = SensorPlot()     
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("gpssensor.ddns.net", 1883, 60)

 
# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()





