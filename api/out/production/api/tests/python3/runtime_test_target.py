import time
import sys

def hello_world()->None:
	print('Hello World, from Python3')


def returns_an_error()->None:
	print('going to throw exception')
	raise Exception('Error')


def div_integers(x:int, y:int)->float:
	return x/ y


def join_strings(arr)->str:
	res = ','.join(arr)
	return res

five_seconds = 5
def wait_a_bit(secs : int):
	time.sleep(secs)
	return None

class testmap:
	def __init__(self):
		self.curdict = dict()
		self.name = 'name1'


	def set(self, k: str, v):
		self.curdict[k] = v


	def get(self, k:str):
		return self.curdict[k]
		

	def contains(self, k:str):
		return k in self.curdict
