#!/bin/bash

terminalColorClear='\033[0m'
terminalColorError='\033[1;31m'
terminalColorMessage='\033[1;33m'

mkdir -p ./bin/docker

id=$(docker create hummingfab:builder)
docker cp $id:/hummingfab/bin/ ./bin/docker

file=$(find ./bin/docker/ -name hb_grid)

echoMsg() {
  echo -e "${terminalColorMessage}$1${terminalColorClear}"
}
echoErr() {
  echo -e "${terminalColorError}$1${terminalColorClear}"
}

mkdir -p ./docker_tmp/
cp "$file" ./docker_tmp/
echoMsg "=============== TEMP DIR CREATED ==============="

docker build -f Dockerfile.debug -t hummingfab:debug .

if rm -r ./docker_tmp; then
  echoMsg "=============== DELTED TMP DIR ==============="
else
  echoErr "tmp dir was not delete"
fi

