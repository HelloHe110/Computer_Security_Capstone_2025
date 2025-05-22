FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd simple_rtos
RUN	mkdir simple_rtos

ADD simple_rtos ./simple_rtos 

RUN chown -R simple_rtos ./simple_rtos
ENV TZ="Asia/Taipei"

# In Server, there will be a flag in environment variable
USER simple_rtos
CMD	ncat -c ./simple_rtos/simple_rtos -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

