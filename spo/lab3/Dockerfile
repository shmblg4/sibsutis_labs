FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    clang \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

CMD ["/bin/bash"]