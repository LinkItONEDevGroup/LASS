# @file util.py
# @brief application wide share functions
# README:
# MODULE_ARCH:  
# CLASS_ARCH:
# GLOBAL USAGE: 
#standard
import glob
#extend
from vincenty import vincenty
#library
#import lib.globalclasses as gc
#from lib.const import *

##### Code section #####
#Spec: 
#How/NeedToKnow:
def distance_by_geo(lat1,long1,lat2,long2):
    #boston = (42.3541165, -71.0693514) #lat,long
    #newyork = (40.7791472, -73.9680804)
    pos1 = (lat1,long1)
    pos2 = (lat2,long2)
    return vincenty(pos1, pos2)

#test
# measure the differece on geo distance
def test_dis_diff():
    lat = [22.0,25.0]
    long = [120.0,122.0]
    str1 = ""
    for y in range(0,9):
        for x in range(0,9):
            lat_y1 = lat[0] + (lat[1]-lat[0])/10*y
            long_x1 = long[0] + (long[1]-long[0])/10*x

            lat_y2 = lat[0] + (lat[1]-lat[0])/10*(y)
            long_x2 = long[0] + (long[1]-long[0])/10*(x+1)

            lat_y3 = lat[0] + (lat[1]-lat[0])/10*(y+1)
            long_x3 = long[0] + (long[1]-long[0])/10*(x)
            
            x_diff = distance_by_geo(lat_y1,long_x1,lat_y2,long_x2)
            y_diff = distance_by_geo(lat_y1,long_x1,lat_y3,long_x3)
            str1 += "%f,%f\t" %(x_diff, y_diff)
        str1 += "\n"   
    print(str1)
def str_to_int(num_str):
    return int(num_str.replace(',',''))
    #"12,345" to int, no error handling
def reencode(file,src_codepage):
    for line in file:
        #yield line.decode('windows-1250').encode('utf-8') 
        yield line.decode(src_codepage).encode('utf-8') 
def filefrom_big5_to_utf8(src_path,dest_path):
    s = open(src_path,"rb").read().decode("big5").encode("utf8")
    open(dest_path,"wb").write(s)
def filefrom_utf16_to_utf8(src_path,dest_path):
    s = open(src_path,"rb").read().decode("utf16").encode("utf8")
    open(dest_path,"wb").write(s)
#search filename in the src_path tree, merge all to dest_path
#convert utf16 to utf8
def merge_same_filename_to_single(src_path,dest_path, filename):
    files = []
    for pathname in glob.iglob("%s%s%s" %(src_path,"/**/" , filename ), recursive=True):
        files.append(pathname)
    
    concat = ""
    for file in files:
        if concat=="":
            dest = open(dest_path + "/" + filename,"wb+")
        concat = open(file,"rb").read().decode("utf16").encode("utf8")
        dest.write(concat)

#test_dis_diff()
            
            
