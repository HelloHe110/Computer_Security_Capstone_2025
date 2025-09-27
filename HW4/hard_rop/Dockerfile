FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd hard_rop
RUN	mkdir hard_rop

ADD hard_rop ./hard_rop 

# Flag is in /flag.txt
ADD ./flag.txt /flag.txt


RUN chown -R hard_rop ./hard_rop
ENV TZ="Asia/Taipei"

USER hard_rop
CMD	ncat -c ./hard_rop/hard_rop -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

