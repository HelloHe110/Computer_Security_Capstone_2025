FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd simple_shell
RUN	mkdir simple_shell

ADD simple_shell ./simple_shell 

# Flag is in /flag.txt
ADD ./flag.txt /flag.txt


RUN chown -R simple_shell ./simple_shell
ENV TZ="Asia/Taipei"

USER simple_shell
CMD	ncat -c ./simple_shell/simple_shell -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

