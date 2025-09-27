#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<fcntl.h>


void putFlag() {
	char *flag = getenv("FLAG");
	if (flag == NULL) {
		printf("CSC2025{SAMPLE_FLAG}");
	} else {
		printf("%s", flag);
	}
	return;
}

void readline(char *buf, int sz) {
	int n = read(0, buf, sz - 1);
}

void sendline(char *buf) {
	printf(buf);
}

bool should_exit = false;

void running_command(char *buf) {
	int n = strlen(buf);
	if (buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't') {
		puts("Exiting...");
		should_exit = true;
	} 
	usleep(100 * n);
}

void task() {
	char buf[256];
	memset(buf, 0, sizeof(buf));
	printf("Enter your command: \n> ");
	readline(buf, sizeof(buf));
	printf("You entered: ");
	sendline(buf);
	running_command(buf);
}


int main() {

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	puts("=====================");
	puts("**** Simple RTOS ****");
	puts("=====================");
	
	while(1) {
		task();
		if (should_exit) {
			exit(0);
		}
	}






	return 0;
}
