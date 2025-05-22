#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void putFlag() {
	char *flag = getenv("FLAG");
	if (flag == NULL) {
		puts("CSC2025{SAMPLE_FLAG}");
	} else {
		puts(flag);
	}
	return;
}

int main() {

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	int8_t len;
	char password[256];
	printf("=============================\n");
	printf("**A Simple Password Checker**\n");
	printf("=============================\n");
	printf("Type Your Password: ");

	fgets(password, 256, stdin);

	len = strlen(password);

	if (len < 0) {
		printf("You have such bad password, we decide to give you a hint for creating your password!\n");
		putFlag();
	} else {
		printf("Good Password\n");
	}

	return 0;
}
