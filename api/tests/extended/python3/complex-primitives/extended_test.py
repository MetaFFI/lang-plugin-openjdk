import builtins


class extended_test:
	def __init__(self):
		self._x = 0

	@property
	def x(self) -> int:
		return self._x

	@x.setter
	def x(self, val1: int):
		self._x = val1

	def positional_or_named(self, value: str) -> str:
		print(value)
		return value

	def list_args(self, value='default', *args) -> list:
		print(value)
		res = [value]
		for a in args:
			print(a)
			res.append(a)
		return res

	def dict_args(self, value='default', **named_args) -> list[str]:
		res = [value]
		print(value)
		for k, v in named_args.items():
			res.append(k)
			res.append(v)
			print('{}={}'.format(k, v))
		return res

	def named_only(self, *, named: str) -> str:
		print(named)
		return named

	def positional_only(self, v1: str, v2: str = 'default', /) -> str:
		print(v1 + ' ' + v2)
		return v1 + ' ' + v2

	def arg_positional_arg_named(self, value: str = 'default', *args, **named_args) -> list[str]:

		res = [value]
		print(value)

		for a in args:
			print(a)
			res.append(a)

		for k, v in named_args.items():
			print('{}={}'.format(k, v))
			res.append(k)
			res.append(v)
		return res

