from genericpath import isdir
import platform
from typing import List, Tuple, Dict
import glob
import os
import subprocess


def get_files(win_metaffi_home: str, ubuntu_metaffi_home: str) -> Tuple[Dict[str, str], Dict[str, str]]:
	# get all files from $METAFFI_HOME/go - the installed dir of this project recursively
	# don't continue recursively if the directory starts with '__'
	
	pluginname = 'jvm'
	
	win_metaffi_home = win_metaffi_home.replace('\\', '/')+f'/{pluginname}/'
	ubuntu_metaffi_home = ubuntu_metaffi_home.replace('\\', '/')+f'/{pluginname}/'

	win_files = {
		'xllr.jvm.dll': win_metaffi_home + 'xllr.jvm.dll',
		'metaffi.api.jar': win_metaffi_home + 'metaffi.api.jar',
		'boost_filesystem-vc143-mt-gd-x64-1_87.dll': win_metaffi_home + 'boost_filesystem-vc143-mt-gd-x64-1_87.dll'
	}
	
	# for each absolute path in the value of win_files, check if the file exists
	for key, value in win_files.items():
		if not os.path.isfile(value):
			raise FileNotFoundError(f'{value} not found - cannot build the installer')
	
	ubuntu_files = {
		'xllr.jvm.so': ubuntu_metaffi_home + 'xllr.jvm.so',
		'metaffi.api.jar': ubuntu_metaffi_home + 'metaffi.api.jar',
		'libboost_filesystem.so.1.87.0': ubuntu_metaffi_home + 'libboost_filesystem.so.1.87.0'
	}
	
	# for each absolute path in the value of ubuntu_files, check if the file exists
	for key, value in ubuntu_files.items():
		if not os.path.isfile(value):
			raise FileNotFoundError(f'{value} not found - cannot build the installer')
	

	assert len(ubuntu_files) > 0, f'No files found in {ubuntu_metaffi_home}/{pluginname}'

	# * copy the api tests
	current_script_dir = os.path.dirname(os.path.abspath(__file__))
	api_tests_files = glob.glob(f'{current_script_dir}/api/tests/**', recursive=True)
	for file in api_tests_files:
		if '__pycache__' in file:
			continue

		if os.path.isfile(file):
			target = file.replace('\\', '/').removeprefix(current_script_dir.replace('\\', '/')+'/api/')
			win_files[target] = file
			ubuntu_files[target] = file

	# * uninstaller
	win_files['uninstall_plugin.py'] = os.path.dirname(os.path.abspath(__file__))+'/uninstall_plugin.py'
	ubuntu_files['uninstall_plugin.py'] = os.path.dirname(os.path.abspath(__file__))+'/uninstall_plugin.py'

	return win_files, ubuntu_files


def setup_environment():
	if platform.system() == 'Windows':
		from pycrosskit.envariables import SysEnv
		import os
		# add $JAVA_HOME/bin/server to PATH
		java_home = os.environ['JAVA_HOME']
		SysEnv().set('PATH', f'{java_home}/bin/server;{os.environ["PATH"]}')
	else:
		from pycrosskit.envariables import SysEnv
		import os
		
		# Fail if JAVA_HOME is not set
		if "JAVA_HOME" not in os.environ:
			raise EnvironmentError("JAVA_HOME is not set. Cannot configure LD_LIBRARY_PATH.")
		
		java_home = os.environ["JAVA_HOME"]
		ld_lib = os.environ.get("LD_LIBRARY_PATH", "")
		paths = ld_lib.split(":") if ld_lib else []
		
		if f"{java_home}/lib/server" not in paths:
			new_ld = f"{java_home}/lib/server:{ld_lib}" if ld_lib else f"{java_home}/lib/server"
			SysEnv().set("LD_LIBRARY_PATH", new_ld)


def check_prerequisites() -> bool:
	try:
		# Run the 'java -version' command and capture its output
		completed_process = subprocess.run(['java', '-version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
		output_str = completed_process.stdout
		stderr_str = completed_process.stderr
		if completed_process.returncode != 0:
			print("java is not installed")
			return False

		# Check if the output contains the desired version string
		if 'version "21.' in output_str.lower() or 'version "21.' in stderr_str.lower():
			return True
		else:
			print(f"""JVM 21 is required\nfound:{output_str}{stderr_str}""")
			return False
	except Exception as e:
		print("java is not installed")
		return False

def print_prerequisites():
	pass


def get_version():
	return '0.3.0'
