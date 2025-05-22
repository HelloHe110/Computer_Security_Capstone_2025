#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<fcntl.h>

void awesome() {
	char buf[16];
	printf("Input something awesome: ");
	memset(buf, 0, sizeof(buf));
	int n = read(0, buf, 256);

	//remove trailing newlines
	while(buf[n - 1] == '\n' || buf[n - 1] == '\r') {
		buf[n - 1] = '\0';
		n--;
	}

	printf("CSC2025 and %s are awesome!!! Let's give you another overflow :)\n", buf);
	printf("Input: ");

	memset(buf, 0, sizeof(buf));
	read(0, buf, 256);

	return;
}

int main() {

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	printf("=====================\n");
	printf("**** Hard ROP !!! ****\n");
	printf("=====================\n");
	
	printf("Let's try to break this with every protection on, but give you more chances :)\n");
	
	while(1) {
		awesome();
	}
	
	return 0;
}
