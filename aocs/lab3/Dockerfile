FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y openssh-server python3 python3-pip && \
    mkdir /var/run/sshd

RUN echo 'root:root' | chpasswd
RUN service ssh start

EXPOSE 22

CMD ["/usr/sbin/sshd", "-D"]
