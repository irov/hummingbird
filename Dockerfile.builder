FROM debian:latest 

ENV DEBUG=True

## install dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  libsnappy-dev \
  libssl-dev \
  libz-dev \
  openssl \
  python3 \
  cmake \
  git 

## optional for debugging 
# RUN apt-get install -y tree

## hb_grid build, 
## copy files - because developer has to define build version
COPY . /hummingfab/

WORKDIR /hummingfab/build/downloads/
RUN bash ./downloads_unix.bash
RUN echo "========== DOWNLOAD UNIX ============="

WORKDIR /hummingfab/build/unix/
RUN bash ./build_dependencies_debug.bash
RUN echo "========== BUID DEPENDENCIES ============="
RUN bash ./build_solution_debug.bash
RUN echo "========== BUID SOLUTION ============="

