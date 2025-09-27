all: simple_rop
.PHONY: homework docker-build docker-rmi docker-run

simple_rop: simple_rop.c
	gcc --static -fno-stack-protector -w -g -o $@ $<

docker-build:
	docker build -t simple_rop .

docker-rmi:
	docker rmi -f simple_rop

docker-run:
	docker run -d -p 8881:8888 simple_rop

homework: simple_rop
	cp simple_rop simple_rop.c ../homework

clean:
	rm -f simple_rop

