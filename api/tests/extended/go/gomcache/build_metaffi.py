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
		
	all_stdout = output.stdout
	
	# if the return code is not zero, raise an exception
	return str(all_stdout).strip()


def main():
	
	os.chdir(os.path.dirname(os.path.abspath(__file__)))
	
	run_command('go mod init gomcache')
	run_command('go get github.com/OrlovEvgeny/go-mcache@v0.0.0-20200121124330-1a8195b34f3a')
	
	gopath = run_command("go env GOPATH")
	run_command(f'metaffi -c --idl github.com/OrlovEvgeny/go-mcache@v0.0.0-20200121124330-1a8195b34f3a/mcache.go -g')


if __name__ == '__main__':
	main()
	
	