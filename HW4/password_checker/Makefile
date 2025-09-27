all: password_checker
.PHONY: homework docker-build docker-rmi docker-run

password_checker: password_checker.c
	gcc -pie -fpie -fstack-protector-all -w -g -z now -o $@ $<

docker-build:
	docker build -t password_checker .

docker-rmi:
	docker rmi -f password_checker

docker-run:
	docker run -d -p 8881:8888 password_checker

homework: password_checker
	cp password_checker password_checker.c ../homework

clean:
	rm -f password_checker

