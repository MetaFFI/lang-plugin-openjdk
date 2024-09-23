from genericpath import isdir
import platform
from typing import List, Tuple, Dict
import glob
import os
import subprocess


def get_files(win_metaffi_home: str, ubuntu_metaffi_home: str) -> Tuple[Dict[str, str], Dict[str, str]]:
	# get all files from $METAFFI_HOME/go - the installed dir of this project recursively
	# don't continue recursively if the directory starts with '__'
	
	pluginname = 'openjdk'
	
	win_metaffi_home = win_metaffi_home.replace('\\', '/')
	ubuntu_metaffi_home = ubuntu_metaffi_home.replace('\\', '/')

	win_files = {}
	for file in glob.glob(win_metaffi_home + f'/{pluginname}/**', recursive=True):		
		if os.path.isfile(file) and '__' not in file:
			file = file.replace('\\', '/')
			win_files[file.removeprefix(win_metaffi_home+f'/{pluginname}/')] = file

	assert len(win_files) > 0, f'No files found in {win_metaffi_home}/{pluginname}'

	ubuntu_files = {}
	for file in glob.glob(ubuntu_metaffi_home + f'/{pluginname}/**', recursive=True):
		if os.path.isfile(file) and '__' not in file:
			file = file.replace('\\', '/')
			ubuntu_files[file.removeprefix(ubuntu_metaffi_home+f'/{pluginname}/')] = file

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
			print(f"""OpenJDK 21 is required\nfound:{output_str}{stderr_str}""")
			return False
	except Exception as e:
		print("java is not installed")
		return False
		

def print_prerequisites():
	print("""Prerequisites:\n\tOpenJDK 21""")