#Importing modules
import json
import urllib2
import urllib
import sys
import hashlib
import subprocess
import os

#You can put the version info somewhere in your program
currentVersion = 1.0

#MD5 check helper function
def md5(fileName):
    """Compute md5 hash of the specified file"""
    m = hashlib.md5()
    try:
        fd = open(fileName,"rb")
    except IOError:
        print "Reading file has problem:", filename
        return
    x = fd.read()
    fd.close()
    m.update(x)
    return m.hexdigest()

#Start of Program
print '[LASS-OTA]OTA Begin: Retrieve Version Info...'
#Update info URL(this must in code or somewhere you store it) 
url = 'https://dl.dropboxusercontent.com/u/15099413/version.info'

# This takes a python object and dumps it to a string which is a JSON representation of that object
data = json.load(urllib2.urlopen(url))
netversion = float(data['VERSION'])

#Check Versions
print '[LASS-OTA]Check Versions...'
need_update=0
if netversion > currentVersion:
	print '[LASS-OTA]Need Update!\nOld Version:',currentVersion,'====>>>> New Version:',data['VERSION']
	print '[LASS-OTA]Start Download New Firmware:',data['linkurl']
	print '[LASS-OTA]Start Download New Firmware:',data['linkurl2']
	need_update=1

elif netversion == currentVersion:
	print '[LASS-OTA]You Have the Correct Version(Maybe)'
	sys.exit(0)

#Download
downloaded=0
if need_update:
	cmd='curl '+data['linkurl']+' -o smart7688.hex'
	print cmd
	subprocess.call(cmd, shell=True)
	cmd='curl '+data['linkurl2']+' -o main_new.py'
	print cmd
	subprocess.call(cmd, shell=True)
	downloaded = 1

#Check Download MD5 
MD5checked=0
if downloaded:
	md5checksum = md5("smart7688.hex")
	print "[LASS-OTA]Downloaded file's MD5 is %s With File Name %s" % (md5checksum, "smart7688.hex")
	if data['MD5']==md5checksum:
		MD5checked = 1
		print '[LASS-OTA]MD5 Pattern Matched!...'
	else:
		print '[LASS-OTA]MD5 Dismatch!...Abort'
		sys.exit(0)

if downloaded and MD5checked:
	md5checksum = md5("main_new.py")
	print "[LASS-OTA]Downloaded file's MD5 is %s With File Name %s" % (md5checksum, "main_new.py")
	if data['MD5_2']==md5checksum:
		MD5checked = 1
		print '[LASS-OTA]MD5 Pattern Matched!...'
	else:
		print '[LASS-OTA]MD5 Dismatch!...Abort'
		sys.exit(0)


#Burnning Hex in ATMEGA32U2
retcode =0
if  MD5checked:
	cmd='avrdude -c linuxgpio -C /etc/avrdude.conf -p m32u4 -U flash:w:smart7688.hex -Uflash:w:$1 $2'
	retcode = subprocess.call(cmd, shell=True)
	if retcode != 1: 
		print '[LASS-OTA]Avrdude Failed...'
		sys.exit(retcode)
	else:
		print '[LASS-OTA]Avrdude Success...'
		os.remove("smart7688.hex")



#Restart Python Process and Reboot
if retcode:
	command = "killall python main.py"
	process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
	print '[LASS-OTA]Remove Main.py'
	os.remove("main.py")
	print '[LASS-OTA]Rename Main_new.py'
	os.rename("main_new.py","main.py")
	print '[LASS-OTA]OTA Completed Rebootting System....'
	os.system("reboot")
