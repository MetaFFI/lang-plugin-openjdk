import platform
import os
import subprocess
import shlex


def run_command(command: str):
	
	print(f'{command}')
	
	try:
		command_split = shlex.split(os.path.expanduser(os.path.expandvars(command)))
		output = subprocess.run(command_split, capture_output=True, text=True)
	except subprocess.CalledProcessError as e:
		print(f'Failed running "{command}" with exit code {e.returncode}. Output:\n{str(e.stdout)}{str(e.stderr)}')
		exit(1)
	except FileNotFoundError as e:
		print(f'Failed running {command} with {e.strerror}.\nfile: {e.filename}')
		exit(1)
	
	if output.returncode != 0:
		raise Exception(f'Failed to run {command}. Exit code: {output.returncode}. Output:\n{str(output.stdout).strip()}{str(output.stderr).strip()}')
	
	all_stdout = output.stdout
	
	# if the return code is not zero, raise an exception
	return str(all_stdout).strip()


def main():
	
	os.chdir(os.path.dirname(os.path.abspath(__file__)))
	
	run_command(f'metaffi -c --idl TestRuntime.go -g')


if __name__ == '__main__':
	main()
	
	