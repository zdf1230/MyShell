#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>

#define Max_Name_Len 256
#define Max_Path_Len 1024
#define Maxn 100

char *argv[Maxn];
int len;
int fd[2];

void type_prompt() {
	char hostname[Max_Name_Len];
	char pathname[Max_Path_Len];
	struct passwd *pwd;

	pwd = getpwuid(getuid());
	gethostname(hostname, Max_Name_Len);
	printf("[MyShell] %s@%s:", pwd -> pw_name, hostname);
	getcwd(pathname, Max_Path_Len);
	if (strncmp(pathname, pwd -> pw_dir, strlen(pwd -> pw_dir)) == 0) {
		printf("~%s", pathname + strlen(pwd -> pw_dir));
	}
	else {
		printf("%s", pathname);
	}
	if (geteuid() == 0) {
		printf("#\n");
	}
	else {
		printf("$\n");
	}
	
}

void read_command()
{
	char ch = ' ';
	int i = 0;
	memset(argv, 0, sizeof(argv));
	
	while (ch != '\n') {
		char s[Maxn]; // the command length should be less than Maxn
		scanf("%s%c", s, &ch);
		argv[i] = (char*)malloc(sizeof(s));
		strcpy(argv[i++], s);
	}
	len = i;
}

void RunOutterFunction(int offset) 
{
	int ret;
	ret = execvp(argv[offset], argv + offset);
	if (ret < 0) {
		printf("command not found\n");
	}
}

int RunInnerFunction()
{
	struct passwd *pwd;
	char *cd_path = NULL;
	if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) // exit
		exit(0);
	if (strcmp(argv[0], "cd") == 0) {
		pwd = getpwuid(getuid());
		if (argv[1] == NULL) { // cd
			cd_path = malloc(strlen(pwd -> pw_dir));
			strcpy(cd_path, pwd -> pw_dir);
		}
		else if (argv[1][0] == '~') { // cd ~/...
			cd_path = malloc(strlen(pwd -> pw_dir) + strlen(argv[1]));
			strcpy(cd_path, pwd -> pw_dir);
			strcpy(cd_path + strlen(pwd -> pw_dir), argv[1] + 1);
		}
		else { // normal
			cd_path = malloc(strlen(argv[1] + 1));
			strcpy(cd_path, argv[1]);
		}
		if (chdir(cd_path) != 0) {
			printf("-MyShell: cd: %s:%s\n", cd_path, strerror(errno));
		}
		free(cd_path);
		return 1;
	}
	return 0;
}

void Redirect()
{
	int i;
	for (i = 1; i < len; ++i) 
		if (argv[i] != NULL){
			if (strcmp(argv[i], ">") == 0) {
				int output = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
				dup2(output, 1);
				close(output);
				argv[i] = NULL;
				++i;
				continue;
			}
			if (strcmp(argv[i], ">>") == 0) {
				int output = open(argv[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0666);
				dup2(output, 1);
				close(output);
				argv[i] = NULL;
				++i;
				continue;
			}
			if (strcmp(argv[i], "2>") == 0) {
				int output = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
				dup2(output, 2);
				close(output);
				argv[i] = NULL;
				++i;
				continue;
			}
			if (strcmp(argv[i], "2>>") == 0) {
				int output = open(argv[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0666);
				dup2(output, 2);
				close(output);
				argv[i] = NULL;
				++i;
				continue;
			}
			if (strcmp(argv[i], "<") == 0) {
				int input = open(argv[i + 1], O_CREAT | O_RDONLY, 0666);
				dup2(input, 0);
				close(input);
				argv[i] = NULL;
				++i;
				continue;
			}
	}
}

int hasPipe()
{
	int i;
	for (i = 1; i < len; ++i)
		if (argv[i] != NULL && strcmp(argv[i], "|") == 0) {
			argv[i] = NULL;
			return i + 1;
		}
	return 0;
}

void Pipe(int x)
{
	int pid2;
	if (x == 0) return;
	if ((pid2 = fork()) == 0) {
		//sub process
		close(fd[1]);
		dup2(fd[0], 0);
		RunOutterFunction(x);
	}
	else {
		//parent process
		close(fd[1]);
		waitpid(pid2, NULL, 0);
	}
}

int main()
{
	int pid, flag;
	while (1) {
		type_prompt();
		read_command();
		if (RunInnerFunction()) 
			continue;
		if (flag = hasPipe())
			pipe(fd);
		if ((pid = fork()) == 0) {
			//sub process
			if (flag) { 
				close(fd[0]);
				dup2(fd[1], 1);
			}
			Redirect();
			RunOutterFunction(0);
			break;
		}
		else if (pid > 0) {
			//parent process
			Pipe(flag);
			waitpid(pid, NULL, 0);
		}
		else {
			printf("fork failed.");
			break;
		}
	}
	return 0;
}
