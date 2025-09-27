FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd simple_rop
RUN	mkdir simple_rop

ADD simple_rop ./simple_rop 

# Flag is in /flag.txt
ADD ./flag.txt /flag.txt


RUN chown -R simple_rop ./simple_rop
ENV TZ="Asia/Taipei"

USER simple_rop
CMD	ncat -c ./simple_rop/simple_rop -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

