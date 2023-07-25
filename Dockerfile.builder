# wip
# FROM alpine:latest 

# work
FROM debian:latest 

# for deploy server
# FROM debian:buster 

## install dependencies
# RUN apk update
# RUN apk add build-base
# RUN apk add libexecinfo libexecinfo-dev
# RUN apk add snappy
RUN apt-get update && apt-get install -y \
  build-essential \
  libsnappy-dev \
  libssl-dev \
  libz-dev \
  openssl \
  python3 \
  cmake \
  git 

## optional soft for debugging 
# RUN apt-get install -y tree

## hb_grid build
# COPY . /hummingfab/
RUN git clone https://github.com/irov/hummingbird /hummingfab

RUN chown root -R /hummingfab
WORKDIR /hummingfab/build/downloads/
# CMD ["chmod", "+x", "./downloads_unix.bash"]
RUN bash ./downloads_unix.bash
RUN echo "========== DOWNLOAD UNIX ============="

WORKDIR /hummingfab/build/unix/
# CMD ["chmod", "+x", "./build_dependencies_debug.bash"]
RUN bash ./build_dependencies_debug.bash
RUN echo "========== BUID DEPENDENCIES ============="
# CMD ["chmod", "+x", "./build_solution_debug.bash"]
RUN bash ./build_solution_debug.bash
RUN echo "========== BUID SOLUTION ============="

# WORKDIR /hummingfab/
# RUN tree -d
