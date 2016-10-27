# @file lassdata.py
# @brief load sensor data from lass. management sensor data
# README:
# MODULE_ARCH:  
# CLASS_ARCH:
# GLOBAL USAGE: 
#standard
#extend
import urllib
import simplejson 
import requests #requests need to load after simplejson
#library
import lib.globalclasses as gc
from lib.const import *
 
##### Code section #####

        
#Spec: one lass site
#How/NeedToKnow:
class Site():
    def __init__(self,site_data):
        self.device_id = site_data['device_id']
        self.gps_lat= float(site_data['gps_lat'])
        self.gps_lon= float(site_data['gps_lon'])
        self.site_name=site_data['SiteName']
        self.sensor_data = {}
        self.sensor_data[site_data['timestamp']] = {'s_t0':site_data['s_t0'],'s_d0':site_data['s_d0'],'s_h0':site_data['s_h0']}
        self.pos_idx = "0@0" #index that used in the map
    def in_area(self,area): #return true if in area. area=[long1,lat1,long2,lat2]
        if self.gps_lat >= area[1] and self.gps_lat <= area[3]:
            if self.gps_lon >= area[0] and self.gps_lon <= area[2]:
                return True
        return False
    def update_his(self,json_data): # history of 2 day
        for feeds in json_data['feeds']:
            try:
                self.sensor_data[feeds['timestamp']] = {'s_t0':float(feeds['temperature']),'s_d0':float(feeds['PM2_5']),'s_h0':float(feeds['humidity'])}
            except:
                print("update_his exception:%s" %(self.device_id) )
    def get_data_bytime(self,ts): # get sensor data by time stamp, ts: datetime
        fmt = '%Y-%m-%dT%H:%M:%SZ'
        ts_str = ts.strftime(fmt)
        if ts_str in self.sensor_data:
            print("get_data_bytime : %s" %(ts_str))
            return self.sensor_data[ts_str]
        return None
    def desc(self,did):
        desc_str = "device_id=%s,gps_lon=%f,gps_lat=%f,SiteName=%s,pos_idx=%s"%(self.device_id,self.gps_lon,self.gps_lat,self.site_name, self.pos_idx)
        return desc_str
#Spec: lass data mgr
#How/NeedToKnow:
class LassDataMgr():
    def __init__(self):
        #private
        #global: these variables allow to direct access from outside.
        self.sites_link={'lass':"http://nrl.iis.sinica.edu.tw/LASS/last-all-lass.json",
                    'airbox':"http://nrl.iis.sinica.edu.tw/LASS/last-all-airbox.json"
                    }

        self.cur_json={} #lass:data, airbox:data. json load from sites_link
        self.his2day_json={} #device_id:data, json load from 2 day history 
        
        self.sites = {} #device_id: Site(), all sites
        self.site_tag = {} #tag_name: [ device_id list ]
        self.load_test_tag()
    #to show the latest submission of all alive devices of the PM25 app: 
    #http://nrl.iis.sinica.edu.tw/LASS/last-all-lass.json
    #to show the latest air quality data imported from TPE AirBox Open Data
    #http://nrl.iis.sinica.edu.tw/LASS/last-all-airbox.json
    #
#{
#  "source": "last-all-lass by IIS-NRL",
#  "feeds": [
#    {
#      "gps_lat": 23.705523,
#      "s_t0": 26.9,
#      "SiteName": "FT3_999",
#      "timestamp": "2016-10-23T20:58:04Z",
#      "gps_lon": 120.547142,
#      "s_d0": 43.0,
#      "s_h0": 85.5,
#      "device_id": "FT3_999"
#    },
#    ...
#  ],
#  "version": "2016-10-23T21:10:02Z",
#  "num_of_records": 49
#}  
    #load from lass original, airbox
    # for test purpose
    def load_test_tag(self):
        self.site_tag['t']=['74DA3895C2B4','74DA3895C214']
    #to show the latest submission of all alive devices of the PM25 app: 
    #http://nrl.iis.sinica.edu.tw/LASS/last-all-lass.json
    #to show the latest air quality data imported from TPE AirBox Open Data
    #http://nrl.iis.sinica.edu.tw/LASS/last-all-airbox.json
    #
    #load from lass original, airbox
    def load_site_list(self):
        for link_key in self.sites_link.keys():
            response =  urllib.request.urlopen(self.sites_link[link_key])
            data = simplejson.load(response)
            self.cur_json[link_key] = data
            for site_data in data['feeds']:
                device_id = site_data['device_id']
                site = Site(site_data)
                self.sites[device_id] = site
                [x,y] = gc.MODEL.map.gps_to_idx([site.gps_lon,site.gps_lat])
                site.pos_idx = "%i@%i" %(x,y)
                #print(site.desc(0))
        #print(data["version"])
#print(data["feeds"])
#print(len(data["feeds"]))
#print(data["feeds"][0]['device_id'])

#show the hourly average of a partucylar PM25 device in the past two days: 
#http://nrl.iis.sinica.edu.tw/LASS/history-hourly.php?device_id=FT1_001
#{
#  "device_id": "FT1_001",
#  "feeds": [
#    {
#      "timestamp": "2016-10-25T00:00:00Z",
#      "temperature": 31.61,
#      "humidity": 81.14,
#      "PM2_5": 7.62,
#      "PM10": 9.15
#    },
#    ...
#      ]
#}
    #load from 2 day history json
    def load_site_history_of_2day(self,device_id):
        json_link = "http://nrl.iis.sinica.edu.tw/LASS/history-hourly.php?device_id=%s" %(device_id)
        response =  urllib.request.urlopen(json_link)
        his_json_data = simplejson.load(response)
        self.his2day_json[device_id] = his_json_data
        site = self.sites[device_id]
        site.update_his(his_json_data)
    #load all devices_id that list by tag_name
    def load_his_by_tag(self,tag_name):
        for device_id in self.site_tag[tag_name]:
            logging.info("loading history json for %s" %(device_id))
            self.load_site_history_of_2day(device_id)
    # find site by area and tag a name
    def tag_site_by_area(self,name,area ): #area = [long1,lat1,long2,lat2]
        for site_key in self.sites:
            site = self.sites[site_key]
            if site.in_area(area):
                if name in self.site_tag:
                    self.site_tag[name].append(site.device_id)
                    #print("%s" %(site.device_id) )
                else:
                    self.site_tag[name]=[site.device_id]
    #by using map_time to get sensor data and update to the map
    def apply_to_map(self,map,map_time,tag_name): #map_time: datetime
        for device_id in self.site_tag[tag_name]:
            site = self.sites[device_id]
            sensor_data = site.get_data_bytime(map_time)
            if sensor_data:
                [x,y] = map.gps_to_idx([site.gps_lon,site.gps_lat])
                pos_idx = "%i@%i" % (x,y)
                #print("apply_to_map pos_idx=%s" %(pos_idx))
                map.poss[pos_idx].pm_set(sensor_data['s_d0'])
                
    def save_csv(self,tag_name,pathname):
        header="timestamp,device_Id, SiteName, gps_lon , gps_lat, PM2_5, PM10, temperature, humidity\n"
        str_output = ""
        try:

            for device_id in self.site_tag[tag_name]:
                site = self.sites[device_id]  
                for feeds in self.his2day_json[device_id]['feeds']:
#timestamp,device_Id, SiteName, gps_lon , gps_lat, PM2_5, PM10, temperature, humidity
                    #ts_format = "yyyy-MM-dd HH:mm:ss" #2016-10-25T00:00:00Z
                    ts_format = feeds['timestamp'].replace('T',' ').replace('Z','')
                    str_output +="%s,%s,%s,%f,%f,%f,%f,%f,%f\n" %(ts_format,site.device_id, site.site_name, site.gps_lon , site.gps_lat, feeds['PM2_5'], feeds['PM10'], feeds['temperature'], feeds['humidity'])
        except :
            print("load history from %s have problem!" %(device_id))
        with open(pathname, "w") as text_file:
            text_file.write("%s%s" % (header,str_output))

    def desc(self,did):    
        print("LASS data - All SiteName" )
        for data_key in self.cur_json.keys():
            print("data_key=%s,count=%i" % (data_key,len(self.cur_json[data_key]['feeds'])))
            for site in self.cur_json[data_key]['feeds']:
                print(site['SiteName'])
        for tag_key in self.site_tag:
            print("tag %s count=%i, include:\n%s" % (tag_key,len(self.site_tag[tag_key]),self.site_tag[tag_key]))
        for data_key in self.his2day_json.keys():
            for record in self.his2day_json[data_key]['feeds']:
                print("%s@%s" %(record['PM2_5'],record['timestamp']))