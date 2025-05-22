FROM ubuntu:24.04

RUN	apt-get update
RUN	apt-get upgrade -y
RUN	apt-get install ncat tzdata -y
RUN	useradd password_checker
RUN	mkdir password_checker

ADD password_checker ./password_checker 

RUN chown -R password_checker ./password_checker
ENV TZ="Asia/Taipei"

# In Server, there will be a flag in environment variable
USER password_checker
CMD	ncat -c ./password_checker/password_checker -tkl4 0.0.0.0 8888 2>&1
EXPOSE	8888

