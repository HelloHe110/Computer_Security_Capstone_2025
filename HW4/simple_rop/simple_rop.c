#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<fcntl.h>

int main() {

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	printf("====================\n");
	printf("**** Simple ROP ****\n");
	printf("====================\n");
	
	char buf[16];
	
	
	printf("Let's try to break this with NX protection on!!, here is your stack address %p\n", buf);
	read(0, buf, 512);

	return 0;
}
