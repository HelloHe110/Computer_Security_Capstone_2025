#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<fcntl.h>

struct User {
	char password[16];
	char username[16];
};

struct User user;
struct User admin;
bool is_admin;
bool is_login;

void init() {
	strcpy(admin.username, "admin");
	int fd = open("/dev/urandom", O_RDONLY);
	memset(admin.password, 0, sizeof(admin.password));
	read(fd, admin.password, 8);
	close(fd);
	is_admin = false;
	is_login = false;
}

void read_string(char *buf, int sz) {
	char *ret = fgets(buf, sz, stdin);
	if (ret == NULL) {
		return;
	}
	int len = strlen(buf);
	while(buf[len - 1] == '\n' || buf[len - 1] == '\r') {
		buf[len - 1] = '\0';
		len--;
	}
}

void login_user() {
	char username[32];
	char password[32];
	
	printf("Enter your username: \n> ");
	read_string(username, sizeof(struct User));
	printf("Enter your password: \n> ");
	read_string(password, sizeof(struct User));
	
	if (strcmp(username, admin.username) == 0 && strcmp(password, admin.password) == 0) {
		is_admin = true;
		is_login = true;
		printf("Login successful as admin\n");
	} else if (strcmp(username, user.username) == 0 && strcmp(password, user.password) == 0) {
		is_admin = false;
		is_login = true;
		printf("Login successful as user\n");
	} else {
		printf("Login failed\n");
	}
}

void register_user() {
	char username[32];
	char password[32];
	
	printf("Enter your username: \n> ");
	read_string(username, sizeof(struct User));
	printf("Enter your password: \n> ");
	read_string(password, sizeof(struct User));
	
	strcpy(user.username, username);
	strcpy(user.password, password);
	printf("User registered successfully\n");
}

void exec_program() {
	char command[128];
	if (is_admin && is_login) {
		printf("Enter your command: \n> ");
		read_string(command, 128);
		system(command);
	} else {
		printf("You are not logged in as admin\n");
	}
}


int main() {

	init();
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	printf("====================================\n");
	printf("**** Welcome to my simple shell ****\n");
	printf("====================================\n");

	int s;
	
	while(1) {
		printf("Enter Your Action\n> ");
		int opt;
		char opt_string[4];
		fgets(opt_string, sizeof(opt_string), stdin);
		sscanf(opt_string, "%d", &opt);
		if (opt_string[0] == '\r' || opt_string[0] == '\n') {
			continue;
		}
		
		switch (opt) {
			case 1:
				printf("You choose to login\n");
				login_user();
				break;
				
			case 2:
				printf("You choose to register\n");
				register_user();
				break;
				
			case 3:
				printf("You try to exec\n");
				exec_program();
				break;
			case 4:
				printf("You choose to exit\n");
				exit(0);
				break;
		}
	}






	return 0;
}
