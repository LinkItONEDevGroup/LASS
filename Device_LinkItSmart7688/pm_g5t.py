import mraa
import time

from multiprocessing import Queue,Process

import move_avge

NUM_INCOME_BYTE = 32
CHAR_PRELIM     = 0x42
NUM_DATA_BYTE   = 30
CHECK_BYTE      = 30
CF_PM1_BYTE     = 4
CF_PM25_BYTE    = 6
CF_PM10_BYTE    = 8
PM1_BYTE        = 10
PM25_BYTE       = 12
PM10_BYTE       = 14
Tmp_BYTE       	= 24
RH_BYTE		= 26

class sensor(Process):
	def __init__(self, q):
		Process.__init__(self)
		self.q = q

		self.u=mraa.Uart(0)
		self.u.setBaudRate(9600)
		self.u.setMode(8, mraa.UART_PARITY_NONE, 1)
		self.u.setFlowcontrol(False, False)

		self.cfpm1_0_avg = move_avge.move_avg(1)		
		self.cfpm2_5_avg = move_avge.move_avg(1)		
		self.cfpm10_avg = move_avge.move_avg(1)		
		self.pm1_0_avg = move_avge.move_avg(1)		
		self.pm2_5_avg = move_avge.move_avg(1)		
		self.pm10_avg = move_avge.move_avg(1)		
		self.tmp_avg = move_avge.move_avg(1)		
		self.rh_avg = move_avge.move_avg(1)		

	def data_log(self, dstr):
		bytedata = bytearray(dstr)
		if self.checksum(dstr) is True:
			CF_PM1_0 = bytedata[CF_PM1_BYTE]*256 + bytedata[CF_PM1_BYTE]
			CF_PM2_5 = bytedata[CF_PM25_BYTE]*256 + bytedata[CF_PM25_BYTE]
			CF_PM10 = bytedata[CF_PM10_BYTE]*256 + bytedata[CF_PM10_BYTE]
			PM1_0 = bytedata[PM1_BYTE]*256 + bytedata[PM1_BYTE+1]
			PM2_5 = bytedata[PM25_BYTE]*256 + bytedata[PM25_BYTE+1]
			PM10 = bytedata[PM10_BYTE]*256 + bytedata[PM10_BYTE+1]
			Tmp = (bytedata[Tmp_BYTE]*256 + bytedata[Tmp_BYTE+1])/10.0
			RH = (bytedata[RH_BYTE]*256 + bytedata[RH_BYTE+1])/10.0
	
			self.cfpm1_0_avg.add(CF_PM1_0)
			self.cfpm2_5_avg.add(CF_PM2_5)
			self.cfpm10_avg.add(CF_PM10)
			self.pm1_0_avg.add(PM1_0)
			self.pm2_5_avg.add(PM2_5)
			self.pm10_avg.add(PM10)
			self.tmp_avg.add(Tmp)
			self.rh_avg.add(RH)
		else:
			return


	def checksum(self, dstr):
		bytedata = bytearray(dstr)
		calcsum = 0
		for i in range(0,NUM_DATA_BYTE,1):
			calcsum = calcsum + bytedata[i]
		exptsum = bytedata[CHECK_BYTE] * 256 + bytedata[CHECK_BYTE+1]
		if calcsum==exptsum:
			return True
		else:
			return False


	def get_data(self):
		CF_PM1_0 = self.cfpm1_0_avg.get()
		CF_PM2_5 = self.cfpm2_5_avg.get()
		CF_PM10 = self.cfpm10_avg.get()
		PM1_0 = self.pm1_0_avg.get()
		PM2_5 = self.pm2_5_avg.get()
		PM10 = self.pm10_avg.get()
		Tmp = self.tmp_avg.get()
		RH = self.rh_avg.get()

		ret = {	'CFPM1.0': CF_PM1_0,
			'CFPM2.5': CF_PM2_5,
			'CFPM10': CF_PM10,
			'PM1.0': PM1_0,
			'PM2.5': PM2_5,
			'PM10': PM10,
			'Tmp': Tmp,
			'RH': RH
			}

		return ret

	def run(self):
		while True:
			if self.u.dataAvailable():
				time.sleep(0.05)
				getstr = self.u.readStr(NUM_INCOME_BYTE)

				if len(getstr) == NUM_INCOME_BYTE:
					self.data_log(getstr)

					g = self.get_data()
					self.q.put(g)
				time.sleep(5)
					


if __name__ == '__main__':

	q = Queue(maxsize=5)
	p = sensor(q)
	p.start()


	while True:
		print('air: '+ str(q.get()))

