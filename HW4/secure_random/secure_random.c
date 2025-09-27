#include<stdlib.h>
#include<stdio.h>
#include<time.h>

unsigned int long_secure_random() {
	srand(time(NULL));
	unsigned int r[100];
	for (int i = 0; i < 100; i ++) {
		r[i] = rand() % 32323;
	}
	for (int i = 1; i < 100; i ++) {
		r[i] = r[i] * r[i - 1] * r[i - 1] * r[i - 1] + r[i] * r[i - 1] * r[i - 1] * 3 + r[i] * r[i - 1] * 2 + r[i];
	}
	return r[99];
}

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

	unsigned int val;

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	printf("Try to break my unbreakable secure random\n");
	scanf("%u", &val);

	if (val == long_secure_random()) {
		printf("You succeed, here are your flag\n");
		putFlag();
	} else {
		printf("You failed, there are no flag\n");
	}

	return 0;
}
