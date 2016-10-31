# @file envdata.py
# @brief evn data collections
# README:
# MODULE_ARCH:  
# CLASS_ARCH:
# GLOBAL USAGE: 
#standard
import csv
import logging
import os
import glob
#extend
#library
import lib.globalclasses as gc
from lib.const import *
from lib.util import *

##### Code section #####

class FixedPollutionSrc(): 
    def __init__(self,fid):
        self.fid=fid
        self.basic=[] #基本資料, 1-columns
        self.fine=[] #裁處資料, multi-columns : [[],[]]
        self.waste=[] #廢棄物申報, multi-columns, multi-columns : [[],[]]
        self.water=[] #水污染申報, multi-columns, multi-columns : [[],[]]
        self.greenhouse=[] #溫室氣體申報, multi-columns : [[],[]]
        self.air_pollution=[] #空氣污染申報, multi-columns : [[],[]]
        self.harmful_air=[] #有害空氣污染申報, multi-columns : [[],[]]
        self.toxic=[] #毒性化學物質申報, multi-columns : [[],[]]
    def desc(self,did):
        logging.info("FixedPollutionSrc ID=%s" %(self.fid))
        logging.info("基本資料=%s" %(self.basic))
        logging.info("裁處資料=%s"%(self.fine))
        logging.info("廢棄物申報=%s"%(self.waste))
        logging.info("水污染申報=%s"%(self.water))
        logging.info("溫室氣體申報=%s"%(self.greenhouse))
        logging.info("空氣污染申報=%s"%(self.air_pollution))
        logging.info("有害空氣污染申報=%s"%(self.harmful_air))
        logging.info("毒性化學物質申報=%s"%(self.toxic))

#基本資料.csv
#管制編號,事業名稱,地址,縣市,鄉鎮,行業別,所屬工業區,列管類別
#B2402442,台中市文山垃圾焚化廠,臺中市南屯區文山里文山南巷５００號,臺中市,南屯區,廢棄物處理業(無害廢棄物處理業),非屬工業區類,空氣污染;水污染;廢棄物;

#裁處資料.csv
#管制編號,裁處時間,管轄縣市,裁處書字號,違反時間,違反法令,裁罰金額,是否訴願,審議機關,訴願結果,陳情結果,違規人名稱,裁處公司名稱
#A3603333,2014/12/29,臺中市,20-103-120022,2014/11/16,空氣污染防制法第20條第1項,NT$100,000,否,,,,台灣電力股份有限公司台中發電廠,台灣電力股份有限公司
#A3603333,2014

#廢棄物申報.csv
#管制編號,申報時段,廢棄物名稱,廢棄物清理方式,申報量,單位,申報狀態
#B2402442,2016年6月,C-0102 鉛及其化合物(總鉛),固化處理,851.69,公噸,
#B2402442,2016年6月,D-1103 焚化爐底渣,物理處理,2437.72,公噸,
#B2402442,2016年6月,D-2003 中間處理後之穩定化產物,掩埋,1190.5060,公噸,

#水污染申報.csv
#管制編號,放流口編號,申報起始日,申報截止日,申報排水量,檢測項目,檢測數值,檢測單位
#L0000624,D01  ,2016/01/01,2016/03/31,951698,ｐＨ值,6.70,
#L0000624,D01  ,2016/01/01,2016/03/31,951698,化學需氧量（mg/L）,56.40,mg/l
#L0000624,D01  ,2016/01/01,2016/03/31,951698,水量(CMH),436,CMH

#溫室氣體申報.csv
#管制編號,申報年度,CO2,CH4,N2O,HFCS,PFCS,SF6,間接排放量,排放總量,自願揭露或法規規定,是否經查證
#B2402442,2011,1522.95,7.97,0.36,0,0,0,140.30,1671.58,自願揭露,否
#B2402442,2010,148125,1016.12,1965.97,0,0,0,110.22,151217.31,自願揭露,否

#空氣污染申報.csv
#管制編號,申報時段,揮發性有機化合物(公噸),粒狀污染物(公噸),硫氧化物(公噸),氮氧化物(公噸),申報狀態
#B2402442,2016年第2季,1.6170,1.1370,1.8080,61.2820,完成申報
#B2402442,2016年第1季,0.6120,0.4670,2.7890,71.9240,核算確認中
#B2402442,2015年第4季,0.5350,0.3390,4.0250,71.2620,完成核算
#B2402442,2014年第4季,0.59,,,,核算確認中
#B2402442,2014年第4季,,1.3710,3.2730,65.5920,完成核算

#有害空氣污染申報.csv
#管制編號,檢測日期,煙道編號,污染物,檢測結果,檢測度量單位,審查狀態／稽查結果
#B2402442,2016/07/19,P002,氯化氫,2.86,ppm,完成審核
#B2402442,2016/01/21,P001,氯化氫,3.12,ppm,完成審核
#B2402442,2016/01/18,P001,鉛及其化合物,0.01,mg/Nm3,完成審核

#毒性化學物質申報.csv
#管制編號,申報時段,申報類別,毒化物列管編號,毒化物中文名稱,毒化物英文名稱,申報量,單位
#L0200633,2016年第3季,製造                  ,05201               ,苯,Benzene,6355.21,公噸
#L0200633,2016年第2季,製造                  ,05201               ,苯,Benzene,6235.76,公噸


#Spec: 
#How/NeedToKnow:
class EnvDataMgr():
    def __init__(self):
        #private
        #global: these variables allow to direct access from outside.
        self.car_density={} # county:[total,car,mobile]
        self.population_count={} # county: population
        self.fixed_pollution_srcs={} # id: FixedPollutionSrc
    def load_all(self):
        self.load_car_density()
        self.load_population_count()
        self.load_fixed_pollution_srcs("include/%s" % (gc.SETTING["IN_FIX_POLLUTION_SRC_DIR"]))
    def load_car_density(self):
#序號,統計區,統計期,總計,總計,總計
#,統計區,統計期,機動車輛登記數(輛),機動車輛登記數(輛),機動車輛登記數(輛)
#,統計區,統計期,總計,汽車,機車
#1,臺北市,2016/03,"1,759,775","800,014","959,761"
# start from line 4
        pathname = "include/%s" %(gc.SETTING["IN_CAR_DENSITY"])
        reader = csv.reader(open(pathname, 'r'))
        self.car_density={}
        for row in reader:
           if row[0].isnumeric()>0:
               self.car_density[row[1]] = [str_to_int(row[3]),str_to_int(row[4]),str_to_int(row[5])]
               #print("type=%s,row=%s,car_density=%s" %(type(row),row,self.car_density[row[1]]))
    def load_population_count(self):
#序號,統計區,統計期,總計
#,,,戶籍人口數(年資料為年度資料)(人)
#1,臺北市,2015/02,"2,705,113"
        src_path = "include/%s" %(gc.SETTING["IN_POPULATION_COUNT"])
        dest_path = "output/%s" %(gc.SETTING["IN_POPULATION_COUNT"])
        filefrom_big5_to_utf8(src_path,dest_path)
        reader = csv.reader(open(dest_path,'r'))
                
        self.population_count={}
        for row in reader:
           if row[0].isnumeric()>0:
               self.population_count[row[1]] = [str_to_int(row[3])]
               #print("row=%s" %(self.population_count[row[1]]))
    def load_fixed_pollution_srcs(self,dir_path):
        #src codepage: utf-16, dest codepage: utf-8
        #scan dir, merge all same filename into output/tmp/file
        #dest_path = "/tmp/%i" % (os.getpid())
        #if not os.path.exists(dest_path):
        #    os.makedirs(dest_path)
        tmp_path = "output/tmp"
        merge_same_filename_to_single(dir_path,tmp_path,"基本資料.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"裁處資料.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"廢棄物申報.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"水污染申報.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"溫室氣體申報.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"空氣污染申報.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"有害空氣污染申報.csv")
        merge_same_filename_to_single(dir_path,tmp_path,"毒性化學物質申報.csv")
        
        #filefrom_utf16_to_utf8(dir_path + "/基本資料.csv" , tmp_path + "/基本資料.csv" )
        #load all files into local variable
        reader = csv.reader(open("%s/%s" % (tmp_path,"基本資料.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = FixedPollutionSrc(fid)
                    self.fixed_pollution_srcs[fid] = fps
                    fps.basic = row
                except:
                    print("Exception: importing 基本資料: fid=%s" %(fid))
            line_pos+=1
               
        #filefrom_utf16_to_utf8(dir_path + "/裁處資料.csv" , tmp_path + "/裁處資料.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"裁處資料.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.fine.append(row)
                except:
                    print("Exception: importing 裁處資料: fid=%s" %(fid))
            line_pos+=1
               
        #filefrom_utf16_to_utf8(dir_path + "/廢棄物申報.csv" , tmp_path + "/廢棄物申報.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"廢棄物申報.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.waste.append(row)
                except:
                    print("Exception: importing 廢棄物申報: fid=%s" %(fid))
            line_pos+=1
               
        #filefrom_utf16_to_utf8(dir_path + "/水污染申報.csv" , tmp_path + "/水污染申報.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"水污染申報.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.water.append(row)
                except:
                    print("Exception: importing 水污染申報: fid=%s" %(fid))
            line_pos+=1
               
        #filefrom_utf16_to_utf8(dir_path + "/溫室氣體申報.csv" , tmp_path + "/溫室氣體申報.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"溫室氣體申報.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.greenhouse.append(row)
                except:
                    print("Exception: importing 溫室氣體申報: fid=%s" %(fid))
            line_pos+=1
        
        #filefrom_utf16_to_utf8(dir_path + "/空氣污染申報.csv" , tmp_path + "/空氣污染申報.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"空氣污染申報.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.air_pollution.append(row)
                except:
                    print("Exception: importing 空氣污染申報: fid=%s" %(fid))
            line_pos+=1
        
        #filefrom_utf16_to_utf8(dir_path + "/有害空氣污染申報.csv" , tmp_path + "/有害空氣污染申報.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"有害空氣污染申報.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.harmful_air.append(row)
                except:
                    print("Exception: importing 有害空氣污染申報: fid=%s" %(fid))
            line_pos+=1
        
        #filefrom_utf16_to_utf8(dir_path + "/毒性化學物質申報.csv" , tmp_path + "/毒性化學物質申報.csv" )
        reader = csv.reader(open("%s/%s" % (tmp_path,"毒性化學物質申報.csv"),'r'))
        line_pos = 1
        for row in reader:
            if line_pos>1: # skip first line
                fid = row[0]
                try:
                    fps = self.fixed_pollution_srcs[fid]
                    fps.toxic.append(row)
                except:
                    print("Exception: importing 毒性化學物質申報: fid=%s" %(fid))
            line_pos+=1
        #build datasets
        pass
    def desc(self,did):
        #logging.info("car_density:")
        #for dict in self.car_density:
        #    logging.info("%s,%s" %(dict,self.car_density[dict]))
        #logging.info("population count:")
        #for dict in self.population_count:
        #    logging.info("%s,%s" %(dict,self.population_count[dict]))
        logging.info("fix_pollution_srcs:")
        for fid in self.fixed_pollution_srcs:
            fps = self.fixed_pollution_srcs[fid]
            fps.desc(0)