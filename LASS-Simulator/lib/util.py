# @file util.py
# @brief application wide share functions
# README:
# MODULE_ARCH:  
# CLASS_ARCH:
# GLOBAL USAGE: 
#standard
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
#test_dis_diff()
            
            
