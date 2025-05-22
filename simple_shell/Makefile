all: simple_shell
.PHONY: homework docker-build docker-rmi docker-run

simple_shell: simple_shell.c
	gcc -pie -fpie -fstack-protector-all -w -g -z now -o $@ $<

docker-build:
	docker build -t simple_shell .

docker-rmi:
	docker rmi -f simple_shell

docker-run:
	docker run -d -p 8881:8888 simple_shell

homework: simple_shell
	cp simple_shell simple_shell.c ../homework

clean:
	rm -f simple_shell

