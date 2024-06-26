import os
import time
import sys


def hello_world() -> None:
	print('Hello World, from Python3')


def returns_an_error() -> None:
	print('going to throw exception')
	raise Exception('Error')


def div_integers(x: int, y: int) -> float:
	return x / y


def join_strings(arr) -> str:
	print(arr)
	res = ','.join(arr)
	return res


five_seconds = 5


def wait_a_bit(secs: int):
	print(f'"waiting" {secs} seconds')
	return None


class testmap:
	def __init__(self):
		self.curdict = dict()
		self.name = 'name1'
	
	def set(self, k: str, v):
		self.curdict[k] = v
	
	def get(self, k: str):
		return self.curdict[k]
	
	def contains(self, k: str):
		return k in self.curdict


def call_callback_add(add_function):
	print(f'Calling add_function {add_function}', file=sys.stderr)
	
	try:
		res = add_function(1, 2)
	except Exception as e:
		print(f'add_function threw an Exception:\n{e}', file=sys.stderr)
		raise

	print(f'returned from callback')
	res = res[0]
	if res != 3:
		print(f'expected 3, got: {res}', file=sys.stderr)
		raise Exception(f'expected 3, got: {res}')
