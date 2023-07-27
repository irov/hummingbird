#!/bin/bash

terminalColorClear='\033[0m'
terminalColorError='\033[1;31m'
terminalColorMessage='\033[1;33m'

echoMsg() {
  echo -e "${terminalColorMessage}$1${terminalColorClear}"
}
echoErr() {
  echo -e "${terminalColorError}$1${terminalColorClear}"
}


mkdir -p ./bin/docker
id=$(docker create hummingfab:builder)
docker cp $id:/hummingfab/bin/ ./bin/docker

file=$(find ./bin/docker/ -name hb_grid)
mkdir -p ./docker_tmp/

cp "$file" ./docker_tmp/
JSON_STRING=$( jq -n '{
  "name": "hb_grid",
  "max_thread": 1,
  "grid_uri": "127.0.0.1",
  "grid_port": 5555,
  "cache_uri": "127.0.0.1",
  "cache_port": 6379,
  "db_host": "127.0.0.1",
  "db_port": 27017,
  "log_uri": "127.0.0.1",
  "log_port": 5044
}')
echo $JSON_STRING >> ./docker_tmp/debug.json
echoMsg "=============== TEMP DIR CREATED ==============="

docker build -f Dockerfile.debug -t hummingfab:debug .

if rm -r ./docker_tmp; then
  echoMsg "=============== TEMP DIR DELTED ==============="
else
  echoErr "!!!!!!!!!!! tmp dir was not delete !!!!!!!!"
fi

