class move_avg:
	def __init__(self, bufmax):
		self.buf = []
		self.bufmax = bufmax
        
	def add(self, val):
		self.buf.append(val)

		if len(self.buf) > self.bufmax:
			del self.buf[0]
        
	def get(self):
		if len(self.buf) > 0:
			avg = sum(self.buf)/len(self.buf)
		else:
			avg = 0
        
		return avg

