all: simple_rtos
.PHONY: homework docker-build docker-rmi docker-run

simple_rtos: simple_rtos.c
	gcc -no-pie -fstack-protector-all -w -g -z relro -o $@ $<

docker-build:
	docker build -t simple_rtos .

docker-rmi:
	docker rmi -f simple_rtos

docker-run:
	docker run -d -p 8881:8888 simple_rtos

homework: simple_rtos
	cp simple_rtos simple_rtos.c ../homework

clean:
	rm -f simple_rtos

