all: ret2flag
.PHONY: homework docker-build docker-rmi docker-run

ret2flag: ret2flag.c
	gcc -fstack-protector-all -w -g -o $@ $<

docker-build:
	docker build -t ret2flag .

docker-rmi:
	docker rmi -f ret2flag

docker-run:
	docker run -d -p 8881:8888 ret2flag

homework: ret2flag
	cp ret2flag ret2flag.c ../homework

clean:
	rm -f ret2flag

