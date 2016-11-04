import urllib
import requests
import simplejson
import wget

def test_json(): 
    response =  urllib.request.urlopen("http://nrl.iis.sinica.edu.tw/LASS/last-all-lass.json")
    #print(response.read())
    #data = simplejson.load(response)
    print( data)
    #print(data["version"])
    #print(data["feeds"])
    #print(len(data["feeds"]))
    #print(data["feeds"][0]['device_id'])
    
def test_vincenty():
    from vincenty import vincenty
    boston = (42.3541165, -71.0693514) #緯度，經度
    newyork = (40.7791472, -73.9680804)
    print(vincenty(boston, newyork))
    
    p1=(34,117)
    p2=(32,108)
    print(vincenty(p1, p2))
#test_vincenty()
def test_urldownload():
    url= 'http://opendata.cwb.gov.tw/datadownload?dataid=O-A0001-001'
    out_file = "../output/O-A0001-001"
    wget.download(url,out = out_file)
test_urldownload()