import platform
import typing
import unittest
import metaffi
import metaffi.metaffi_runtime
import metaffi.metaffi_module
import metaffi.metaffi_types

runtime: metaffi.metaffi_runtime.MetaFFIRuntime = None


def init():
	global runtime
	runtime = metaffi.metaffi_runtime.MetaFFIRuntime('go')


def fini():
	global runtime
	runtime.release_runtime_plugin()


class GoMCache:
	def __init__(self):
		global runtime
		if platform.system() == 'Windows':
			module = runtime.load_module('mcache_MetaFFIGuest.dll')
		elif platform.system() == 'Linux':
			module = runtime.load_module('mcache_MetaFFIGuest.so')
		else:
			raise Exception(f'Unsupported platform {platform.system()}')
		
		# get INFINITY to use in "set method"
		infinity_getter = module.load('global=TTL_FOREVER,getter', None, [metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type)])
		self.infinity = infinity_getter()[0]
		
		# load constructor
		new_gomcache = module.load('callable=New', None, [metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)])
		self.instance = new_gomcache()[0]
		
		# load methods
		self.plen = module.load('callable=CacheDriver.Len,instance_required',
			[metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)],
			[metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_array_type)])
		
		self.pset = module.load('callable=CacheDriver.Set,instance_required',
			[metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
			metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type),
			metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type),
			metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type)],
			[metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)])
		
		self.pget = module.load('callable=CacheDriver.Get,instance_required',
			[metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
			metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type),
			metaffi.metaffi_types.new_metaffi_type_with_alias(metaffi.metaffi_types.MetaFFITypes.metaffi_bool_type)])
		
	def __len__(self):
		return self.plen(self.instance)[0]
	
	def set(self, key: str, val):
		err = self.pset(self.instance, key, val, self.infinity)
		if err[0] is not None:
			raise Exception(f'Failed with error: {err[0]}')
		
	def get(self, key: str) -> typing.Tuple[typing.Any, bool]:
		return self.pget(self.instance, key)


class GoMCacheTest(unittest.TestCase):
	
	@classmethod
	def setUpClass(cls):
		init()
	
	@classmethod
	def tearDownClass(cls):
		fini()
	
	def test_mcache(self):
		global runtime
		
		# get field
		m = GoMCache()
		m.set('myinteger', 101)
		
		if len(m) != 1:
			self.fail(f'len should be 1, but it returned {len(m)}')
		
		val, is_found = m.get('myinteger')
		
		if not is_found:
			self.fail('did not find the key myinteger after settings it')
		
		if val != 101:
			self.fail("val expected to be 101, while it is" + str(val))
		
