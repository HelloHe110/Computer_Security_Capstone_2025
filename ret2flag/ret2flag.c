#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<fcntl.h>

void putFlag() {
	char *flag = getenv("FLAG");
	if (flag == NULL) {
		puts("CSC2025{SAMPLE_FLAG}");
	} else {
		puts(flag);
	}
	return;
}


char password[16];

void try () {
	char buf[16];
	for (int i = 0; i < 5; i ++) {
		memset(buf, 0, sizeof(buf));
		read(0, buf, 64);
		printf("Your password is %s", buf);
		if (strcmp(buf, password) == 0) {
			puts("Password is correct");
			putFlag();
		} else {
			puts("Password is incorrect. Let's try again");
		}
	}
}

int main() {

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, password, sizeof(password));
	close(fd);
	
	puts("Here is another password checker, you got 5 chances to try");
	
	try();
	

	return 0;
}
