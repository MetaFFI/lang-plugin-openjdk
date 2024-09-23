import importlib
import sys
import subprocess

def ensure_package(package_name, pip_package_name=None):
	try:
		importlib.import_module(package_name)
	except ImportError:
		import subprocess
		import sys
		print(f"Installing {package_name}...")
		
		if pip_package_name is None:
			pip_package_name = package_name
			
		subprocess.check_call([sys.executable, "-m", "pip", "install", pip_package_name])
		
		print(f"{package_name} installed successfully!")

ensure_package("shutil")

if __name__ == "__main__":
    	# delete the plugins directory
	import shutil
	import os
	
	# get the path to the plugins directory
	metaffi_home = os.getenv('METAFFI_HOME')
	assert metaffi_home is not None, 'METAFFI_HOME is not set'
	
	plugins_dir = os.path.join(metaffi_home, 'openjdk')
	
	# remove the plugins directory
	shutil.rmtree(plugins_dir)


		