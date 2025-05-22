all: secure_random
.PHONY: homework docker-build docker-rmi docker-run

secure_random: secure_random.c
	gcc -pie -fpie -fstack-protector-all -w -g -z now -o $@ $<

docker-build:
	docker build -t secure_random .

docker-rmi:
	docker rmi -f secure_random

docker-run:
	docker run -d -p 8881:8888 secure_random

homework: secure_random
	cp secure_random secure_random.c ../homework

clean:
	rm -f secure_random

