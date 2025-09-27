FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd secure_random
RUN	mkdir secure_random

ADD secure_random ./secure_random 

RUN chown -R secure_random ./secure_random
ENV TZ="Asia/Taipei"

# In Server, there will be a flag in environment variable
USER secure_random
CMD	ncat -c ./secure_random/secure_random -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

