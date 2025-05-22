all: hard_rop
.PHONY: homework docker-build docker-rmi docker-run

hard_rop: hard_rop.c
	gcc -pie -fpie -fstack-protector-all -w -g -z now -o $@ $<

docker-build:
	docker build -t hard_rop .

docker-rmi:
	docker rmi -f hard_rop

docker-run:
	docker run -d -p 8881:8888 hard_rop

homework: hard_rop
	cp hard_rop hard_rop.c ../homework

clean:
	rm -f hard_rop

