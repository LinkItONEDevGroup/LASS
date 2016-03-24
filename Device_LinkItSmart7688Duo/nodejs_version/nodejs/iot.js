var global = //全域設定值與變數
{
    //設定值
    deviceId: "LANMA_001",//your device id
    lat : 25.1309489,//固定值
    lng : 121.494209,//固定值
    mqttserver:"gpssensor.ddns.net",//your mqtt server ex:"localhost" or "gpssensor.ddns.net"
    mqttpath:"LASS/Test/PM25",//ex: "LASS/Test/PM25"
}

setup();

function setup(){//初始化
    global.str="";
    global.gpsstr="|gps_lat="+global.lat+"|gps_lon="+global.lng;
    global.ntpsync = new NTPSync();
    global.mqttConnect = onMQTTConnect; 
    global.mqtt = new MQTT(global);//.mqttserver);
    global.serialCallback = loop;
    global.serial = new Serial(global);//由Serial收到訊息時放至.str帶有換行才呼叫.serialCallback
}
function loop(){//每當收到感測訊息
    //取系統時間
    var sTime =  new Date(Date.now()).toISOString();
    var sDate = sTime.substr(0,10);
    var sTime = sTime.substr(11,8);
    //組出完整MQTT訊息
    var sFullData = "|ver_format=3|fmt_opt=0|app=PM25|ver_app=0.7.13|device=LinkIt7688Duo"+ 
        "|device_id=" + global.deviceId + global.gpsstr +
        "|gps_fix=0|gps_num=1|gps_alt=1"
        + "|date=" + sDate+ "|time=" + sTime+global.str;
    console.log(sFullData);
    //發送MQTT
    global.mqtt.send(global.mqttpath,sFullData);//LASS/Test/PM25
    global.mqtt.send(global.mqttpath+"/"+global.deviceId,sFullData);//LASS/Test/PM25/your_device_id
    //global.mqtt.send('Lanma',sFullData);//LASS/Test/PM25
}

function onMQTTConnect(){
    global.mqtt.subscribe(global.mqttpath+"/"+global.deviceId+"/C",onMQTTSubscribeControlMessage);
}
function onMQTTSubscribeControlMessage(message){
    global.gpsstr = message;
}

/////////
function Serial(para){//Srial Helper. para需要有para.str變數與para.serialCallback之Function
	//this.mraa = require('mraa');
	//console.log('MRAA Version: ' + this.mraa.getVersion()); //print out the mraa version in IoT XDK console//Intel® Edison & Intel® Galileo
	//this.u = new this.mraa.Uart(0); //Default
	//var serialPath = this.u.getDevicePath();

    var serialPath = ('/dev/ttyS0'); //7688 Duo的MCU/MPU通訊埠
	var SerialPort = require("serialport").SerialPort;
	this.serialPort = new SerialPort(serialPath, {
		baudrate: 57600//7688 Duo的MCU/MPU通訊埠之傳輸速率
	});
    this.para = para;
	this.serialPort.on("open",function() {//通訊埠開啟
		console.log("Connected to "+serialPath);
		this.serialPort.on("data", function (data) {//收到資料時
            var sMessage = data.toString();//注意data並非String
            if(sMessage.indexOf("\r\n") <0){//判斷若無換行則先記錄
                if(this.para.str==undefined)
                    this.para.str = "";
                this.para.str+=sMessage;
            }
            else{//有換行則取代掉換行且執行Callback，然後清空資料
                if(this.para.str==undefined)
                    this.para.str = "";
                this.para.str+=sMessage.replace("\r\n","");//去掉換行
                console.log(this.para.str);
                if(this.para.serialCallback!=undefined)
                    this.para.serialCallback(this.para.str);//執行Callback
                this.para.str="";

            }
        }.bind(this));
	}.bind(this));
}
//////
function MQTT(para){//MQTT Helper
    this.strHost = para.mqttserver;//strHost;
    this.mqttConnect=para.mqttConnect;
    this.connected = false;
	this.mqtt    = require('mqtt');
	this.client  = this.mqtt.connect('mqtt://'+this.strHost);
    this.callback={};
    console.log("mqtt start to connect to "+this.strHost);

	this.a=1;
	this.client.on('connect', function() {
      console.log("mqtt connected to "+this.strHost);
      this.connected = true;
      if(this.mqttConnect!=undefined)
        this.mqttConnect();
	}.bind(this));
    this.send = function(strPath,strPayload){
        if(this.connected)
            this.client.publish(strPath, strPayload);
    }.bind(this);
    this.subscribe = function(strPath,callback){
        console.log("Subscribe:"+strPath);
        if(this.connected){
            this.client.subscribe(strPath);
            this.callback[strPath]=callback;
        }
    }.bind(this);
    this.client.on('message', function (topic, message) {
        // message is Buffer 
        console.log("OnMessage:"+topic +":"+message.toString());
        //this.client.end();
        if(this.callback[topic]!=undefined)
            this.callback[topic](message);
    }.bind(this));
}

//////
Date.prototype.sysdatetime = function() {//yyyy-mm-dd hh:mm:ss
    var yyyy = this.getFullYear().toString();
    var mm = (this.getMonth()+1).toString(); // getMonth() is zero-based
    var dd  = this.getDate().toString();
    return yyyy +"-"+ (mm[1]?mm:"0"+mm[0]) +"-"+ (dd[1]?dd:"0"+dd[0])+" "+this.toLocaleTimeString();
};

function NTPSync(){//使用NTP校正系統時間
   var ntpClient = require('ntp-client');
    ntpClient.getNetworkTime("tick.stdtime.gov.tw", 123, function(err, date) {
        if(err) {
            console.error(err);
            return;
        }
        console.log("Sync NTP time : ");
        console.log(date);
        var exec = require('child_process').exec;
        var cmd = 'date -s "'+date.sysdatetime()+'"';//利用Command設定取得之ntp時間
        exec(cmd, function(error, stdout, stderr) {
        });

    });
 
}
