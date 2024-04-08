# python script to run unittests for api using subprocess

import subprocess
import os
import sys
from colorama import init, Fore
import platform

# Initialize colorama
init()

# Get the current path of this Python script
current_path = os.path.dirname(os.path.abspath(__file__))


def get_extension_by_platform() -> str:
	if platform.system() == 'Windows':
		return '.dll'
	elif platform.system() == 'Darwin':
		return '.dylib'
	else:
		return '.so'


def run_script(script_path):
	print(f'{Fore.CYAN}Running script: {script_path}{Fore.RESET}')
	
	if script_path.endswith('.py'):
		# Python script
		python_command = 'py' if platform.system() == 'Windows' else 'python3'
		command = [python_command, script_path]
	else:
		raise ValueError(f'Unsupported script file type: {script_path}')
	
	script_dir = os.path.dirname(os.path.abspath(script_path))
	
	process = subprocess.Popen(command, cwd=script_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
	
	while process.poll() is None:
		print(process.stdout.read(), end='')
		print(process.stderr.read(), file=sys.stderr, end='')
	
	if process.returncode != 0:
		raise subprocess.CalledProcessError(process.returncode, command)


def run_unittest(script_path):
	print(f'{Fore.CYAN}Running unittest: {script_path}{Fore.RESET}')
	
	if script_path.endswith('.py'):
		# Python unittest
		python_command = 'py' if platform.system() == 'Windows' else 'python3'
		command = [python_command, '-m', 'unittest', script_path]
	elif script_path.endswith('.java'):
		# Java JUnit test
		junit_jar = os.path.join(current_path, 'junit-platform-console-standalone-1.10.2.jar')
		hamcrest_jar = os.path.join(current_path, 'hamcrest-core-1.3.jar')
		bridge_jar = os.path.join(os.environ['METAFFI_HOME'], 'xllr.openjdk.bridge.jar')
		api_jar = os.path.join(os.environ['METAFFI_HOME'], 'metaffi.api.jar')
		class_name = os.path.splitext(os.path.basename(script_path))[0]
		class_path = f'.{os.pathsep}{junit_jar}{os.pathsep}{hamcrest_jar}{os.pathsep}{bridge_jar}{os.pathsep}{api_jar}'
		
		# Compile the Java source file
		compile_command = ['javac', '-cp', class_path, script_path]
		print(f'{Fore.BLUE}Running - {" ".join(compile_command)}{Fore.RESET}')
		subprocess.run(compile_command, check=True)
		
		# Run the JUnit test
		command = ['java', '-jar', junit_jar, '-cp', class_path, '-c', class_name]
	else:
		raise ValueError(f'Unsupported unittest file type: {script_path}')
	
	script_dir = os.path.dirname(os.path.abspath(script_path))
	
	print(f'{Fore.BLUE}Running - {" ".join(command)}{Fore.RESET}')
	process = subprocess.Popen(command, cwd=script_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
	
	stdout, stderr = process.communicate()
	
	print(stdout, end='')
	print(stderr, file=sys.stderr, end='')
	
	if process.returncode != 0:
		raise subprocess.CalledProcessError(process.returncode, command)
	
	# If it's a Java unittest, delete the compiled .class file
	if script_path.endswith('.java'):
		os.remove(f'{os.path.splitext(script_path)[0]}.class')


# --------------------------------------------

# --------------------------------------------

# sanity tests

# --------------------------------------------

# run openjdk->python3.11 tests

print(f'{Fore.MAGENTA}Testing Sanity OpenJDK -> Python3.11{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

# Define the paths to the scripts to be run
test_sanity_python311_path = os.path.join(current_path, 'sanity', 'APITestPython3.java')

# Run the scripts
run_unittest(test_sanity_python311_path)

print(f'{Fore.MAGENTA}Testing Sanity OpenJDK -> Python3.11{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')

# --------------------------------------------

# run openjdk->Go tests

print(f'{Fore.MAGENTA}Testing Sanity OpenJDK -> Go{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

# Define the paths to the scripts to be run
build_sanity_go_script_path = os.path.join(current_path, 'sanity', 'go', 'build_metaffi.py')
test_sanity_go_path = os.path.join(current_path, 'sanity', 'APITestGo.java')

# Run the scripts
run_script(build_sanity_go_script_path)
run_unittest(test_sanity_go_path)

os.remove(os.path.join(current_path, 'sanity', 'go', f'TestRuntime_MetaFFIGuest{get_extension_by_platform()}'))
print(f'{Fore.MAGENTA}Testing Sanity OpenJDK -> Go{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')

# --------------------------------------------

# extended tests

# --------------------------------------------

# run openjdk->python3.11 tests

print(f'{Fore.MAGENTA}Testing Extended OpenJDK -> Python3.11{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

# Define the path to the unittest script
test_extended_bs4_path = os.path.join(current_path, 'extended', 'python3', 'beautifulsoup', 'BeautifulSoupTest.java')
run_unittest(test_extended_bs4_path)

test_extended_py_collections_path = os.path.join(current_path, 'extended', 'python3', 'collections', 'CollectionsTest.java')
run_unittest(test_extended_py_collections_path)

test_extended_py_complex_primitives_path = os.path.join(current_path, 'extended', 'python3', 'complex-primitives', 'ComplexPrimitivesTest.java')
run_unittest(test_extended_py_complex_primitives_path)

test_extended_numpy_path = os.path.join(current_path, 'extended', 'python3', 'numpy', 'ComplexPrimitivesTest.java')
run_unittest(test_extended_numpy_path)

test_extended_pandas_path = os.path.join(current_path, 'extended', 'python3', 'pandas', 'PandasTest.java')
run_unittest(test_extended_pandas_path)

print(f'{Fore.MAGENTA}Testing Extended OpenJDK -> Python3.11{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')

# --------------------------------------------

# run openjdk->Go tests

print(f'{Fore.MAGENTA}Testing Extended OpenJDK -> Go{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

# Define the paths to the scripts to be run
build_extended_go_bytes_arrays_path = os.path.join(current_path, 'extended', 'go', 'bytearrays', 'build_metaffi.py')
test_extended_go_bytes_arrays_path = os.path.join(current_path, 'extended', 'go', 'bytearrays', 'BytesPrinter.java')
run_script(build_extended_go_bytes_arrays_path)
run_unittest(test_extended_go_bytes_arrays_path)
os.remove(os.path.join(current_path, 'extended', 'go', 'bytearrays', f'BytesPrinter_MetaFFIGuest{get_extension_by_platform()}'))

build_extended_go_bytes_arrays_path = os.path.join(current_path, 'extended', 'go', 'gomcache', 'build_metaffi.py')
test_extended_go_bytes_arrays_path = os.path.join(current_path, 'extended', 'go', 'gomcache', 'GoMCacheTest.java')
run_script(build_extended_go_bytes_arrays_path)
run_unittest(test_extended_go_bytes_arrays_path)
os.remove(os.path.join(current_path, 'extended', 'go', 'gomcache', f'mcache_MetaFFIGuest{get_extension_by_platform()}'))

print(f'{Fore.MAGENTA}Testing Extended OpenJDK -> Go{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')
