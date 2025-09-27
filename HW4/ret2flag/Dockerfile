FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd ret2flag
RUN	mkdir ret2flag

ADD ret2flag ./ret2flag 

RUN chown -R ret2flag ./ret2flag
ENV TZ="Asia/Taipei"

# In Server, there will be a flag in environment variable
USER ret2flag
CMD	ncat -c ./ret2flag/ret2flag -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

