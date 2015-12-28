from pyfirmata import Arduino, util
from time import sleep
import threading
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish


cLient=None
monitor_thread=None
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

def do_monitor():
    """ monitor sensor data
     """
    global monitor_thread
    global client
    if monitor_thread==None:
        cLient.connect("gpssensor.ddns.net", 1883, 60)
        monitor_thread = MonitorThread()
        monitor_thread.start()
    else:
        print("MQTT client already exist! Don't start again")

def do_send():
    #client = new mqtt.MqttClient("tcp://gpssensor.ddns.net:1883", "LASS-Wuuong");
    #client.connect();
    #MqttMessage message = new MqttMessage();
    #message.setPayload("A single message".getBytes());
    #client.publish("LASS/Test/LinkItSmart7688-Test", message);
    #client.disconnect();
    publish.single("LASS/Test/LinkItSmart7688-Test", "LASS-wuulong", hostname="gpssensor.ddns.net")

def board_led():
    board.digital[13].write(1)
    sleep(0.5)
    board.digital[13].write(0)
    sleep(0.5)

def board_d4():
    global button
    iterator = util.Iterator(board)
    iterator.start()
    sleep(1)
    button = board.get_pin('d:4:i')
    sleep(1)
    button.enable_reporting()
    sleep(1)
    
def board_analog():
    global button
    it = util.Iterator(board)
    it.start()
    sleep(1)
    button = board.get_pin('a:0:i')
    sleep(1)
    button.enable_reporting()
    sleep(1)    
    
    
    #board.analog[0].enable_reporting()
    #print("A0=%f" % (board.analog[0].read()))
  

board = Arduino('/dev/ttyS0')
print "LASS Starting"
cLient = mqtt.Client()
cLient.on_connect = on_connect
cLient.on_message = on_message
cLient.loop_start
#do_monitor()        
#do_send()       
board_analog()
#board_d4()
while True:
    #board_led()
    #board_d4()
    global button
    print "Button state: %s" % button.read()
    sleep(0.5)   