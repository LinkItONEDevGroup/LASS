#! /usr/bin/env python3

from http.server import BaseHTTPRequestHandler
from http.server import HTTPServer
#from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer

import os, sys, urllib
import requests
import json

######  全域變數建議區 (放此處較易理解) #####
####################################################

HELPER_NAME = "基本的Helper"
HELPER_PORT = 50099
my_device = "none"

####################################################


class CmdHandler(BaseHTTPRequestHandler):
    """
    This class handles HTTP GET requests sent from  Scratch2.
    """
  
    def do_GET(self):
        """
        process HTTP GET requests
        """

        # skip over the first / . example:  /poll -> poll  
        cmd = self.path[1:]
        
        # create a command list .  
        cmd_list = cmd.split('/',1)
        

        s = "不回傳資料"

        global my_device             
        

        ###### 處理Scratch送出的命令
        ###### 若需回應Scratch的Poll命令，再把文字存在變數s ##
        ##############################################################
        if cmd_list[0] == "lass" :           
            my_device = "http://nrl.iis.sinica.edu.tw/LASS/last.php?device_id=" + cmd_list[1]
            #print(my_device)
        if (cmd_list[0] == "poll") and ( my_device != "none"):   
            with urllib.request.urlopen(my_device) as response:
                 s = response.read()
                 j = json.loads(s.decode('utf-8'))
                 s =  "device " + str(j['device_id']) + "\r\n"
                 s += "pm " + str(j['s_d0']) + "\r\n"
                 s += "temp " + str(j['s_t0']) + "\r\n"
                 s += "hum " + str(j['s_h0']) + "\r\n"
                 #print (s)
       
        if cmd_list[0] != "poll" :
            print(cmd_list[0])
       
      
        
        #############################################################
        self.send_resp(s)


    def send_resp(self, response):
        """
        This method sends Scratch an HTTP response to an HTTP GET command.
        """

        crlf = "\r\n"
        http_response = "HTTP/1.1 200 OK" + crlf
        http_response += "Content-Type: text/html; charset=ISO-8859-1" + crlf
        http_response += "Content-Length" + str(len(response)) + crlf
        http_response += "Access-Control-Allow-Origin: *" + crlf
        http_response += crlf

        if response != '不回傳資料':
             http_response += str(response) + crlf
            #print(http_response)
           
            # send it out the door to Scratch
           
        self.wfile.write(http_response.encode('utf-8'))
        

def start_server():
    """
       This function populates class variables with essential data and
       instantiates the HTTP Server
    """
    

    try:
        server = HTTPServer(('localhost', HELPER_PORT ), CmdHandler)
        print ('啟動<' + HELPER_NAME + '>伺服程式!(port ' + str(HELPER_PORT) + ')')
        print ('要退出請按 <Ctrl-C> \n')
        print ('請執行Scrath2(記得要開啟對應的s2e檔案!)')
    except Exception:
        print ('HTTP Socket may already be in use - restart Scratch')
        raise

    try:
        global my_device
        my_device = "none"
        #start the server
        server.serve_forever()
    except KeyboardInterrupt:
        print ('\n\n退出程式……\n')
        sys.exit()
        
if __name__ == "__main__":

        start_server()
