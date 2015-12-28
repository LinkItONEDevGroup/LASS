from pyfirmata import Arduino, util
from time import sleep
import threading
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish


cLient=None
ctrl=None

#msg example
#LASS/Test/PM25 |ver_format=3|fmt_opt=0|app=PM25|ver_app=0.7.6|device_id=FT1_007|tick=582358|date=2080-01-06|time=00:09:21|device=LinkItONE|s_0=54.00|s_1=100.00|s_2=1.00|s_3=0.00|s_d0=12.00|s_t0=27.90|s_h0=54.50|gps_lat=24.480128|gps_lon=120.595609|gps_fix=0|gps_num=0|gps_alt=13
class Setting:
    def __init__(self):
        #system general setting
        self.debug_enable=0 #Default:0, 0: debug disable , 1: debug enable
        #sensor setting
        self.ver_format=3
        self.fmt_opt=1
        self.app="LinkItSmart7688-Test"
        self.ver_app="0.0.2"
        self.device_id="LASS-TST_007"
        self.device="LinkItSmart7688Duo"
    def get_packstr(self):
        packstr = "|ver_format=%i|fmt_opt=%i|app=%s|ver_app=%s|device_id=%s|device=%s" % (self.ver_format,self.fmt_opt,self.app,self.ver_app,self.device_id,self.device)
        return packstr

class SensorValue:
    def __init__(self):
        self.tick=0
        self.datestr="2016-01-01"
        self.timestr="01:01:01"
        self.gps_fix=0
        self.gps_num=0
        self.gps_alt=0
        self.gps_lat=24.480128
        self.gps_lon=120.595609
        self.value={}     
        # logic
        self.sensing_cnt = 0
    
    #sensing data before this function
    def get_packstr(self):
        setting_fix = sEtting.get_packstr()
        packstr_fix = "|tick=%i|date=%s|time=%s|gps_fix=%i|gps_num=%i|gps_alt=%i|gps_lat=%f|gps_lon=%f" % (self.tick,self.datestr,self.timestr,self.gps_fix,self.gps_num,self.gps_alt,self.gps_lat,self.gps_lon)
        packstr_sensor="|s0=%.2f|s1=%.4f" %(self.value["s0"],self.value["s1"])
        return "%s%s%s" %(setting_fix,packstr_fix,packstr_sensor)
    def sensing(self):
        global ctrl
        self.value["s0"]=float(self.sensing_cnt)
        self.value["s1"]=float(ctrl.button.read())
        self.sensing_cnt=self.sensing_cnt+1
        pass
class Controller:
    def __init__(self):

        self.monitor_thread=None
        self.button = None
        self.board = Arduino('/dev/ttyS0')


    def mqtt_monitor(self):
        """ monitor sensor data
         """
        global cLient
        if self.monitor_thread==None:
            cLient.connect("gpssensor.ddns.net", 1883, 60)
            self.monitor_thread = MonitorThread()
            self.monitor_thread.start()
        else:
            print("MQTT client already exist! Don't start again")
    
    def mqtt_send(self,payload):
        #client = new mqtt.MqttClient("tcp://gpssensor.ddns.net:1883", "LASS-Wuuong");
        #client.connect();
        #MqttMessage message = new MqttMessage();
        #message.setPayload("A single message".getBytes());
        #client.publish("LASS/Test/LinkItSmart7688-Test", message);
        #client.disconnect();
        publish.single("LASS/Test/LinkItSmart7688-Test", payload, hostname="gpssensor.ddns.net")
    
    def board_led_blink(self):
        self.board.digital[13].write(1)
        sleep(0.5)
        self.board.digital[13].write(0)
        sleep(0.5)
    
    def board_d_reporting(self):
        iterator = util.Iterator(self.board)
        iterator.start()
        sleep(1)
        self.button = self.board.get_pin('d:4:i')
        sleep(1)
        self.button.enable_reporting()
        sleep(1)
        
    def board_a_reporting(self):
        it = util.Iterator(self.board)
        it.start()
        sleep(1)
        self.button = self.board.get_pin('a:0:i')
        sleep(1)
        self.button.enable_reporting()
        sleep(1)    
    
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


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")
    #topic="Sensors/#"
    client.subscribe("LASS/Test/#", qos=0)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    #payload_hex = ''.join(format(str(x), '02x') for x in msg.payload)
    payload_str = str(msg.payload)
    console_str = msg.topic+ "|" +  payload_str[1:-1]
    print(console_str)
    
    
    #board.analog[0].enable_reporting()
    #print("A0=%f" % (board.analog[0].read()))

def init_config():
    global sEtting
    global sEnsorValue
    global cLient
    sEtting = Setting()
    sEnsorValue = SensorValue()
    cLient = mqtt.Client()
    cLient.on_connect = on_connect
    cLient.on_message = on_message
    cLient.loop_start
      
def main():
    global ctrl
    print "LASS Starting"
    init_config()
    ctrl = Controller()    
    ctrl.mqtt_monitor()        
    #do_send()       
    ctrl.board_a_reporting()
    #board_d4()
    while True:
        #board_led()
        #board_d4()
        print "Button state: %s" % ctrl.button.read()
        sEnsorValue.sensing()
        packinfo ="PackInfo=%s|" %(sEnsorValue.get_packstr()) 
        print packinfo
        ctrl.mqtt_send(packinfo)
        sleep(5)   
    
if __name__ == "__main__":
    main()
    