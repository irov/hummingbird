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

timestamp=$(date +%s)
file=$(find ./bin/docker/ -name hb_grid)
mkdir -p ./docker_tmp/

cp "$file" ./docker_tmp/
DEBUG_CONFIG=$( jq -n '{
  "mark": '"${timestamp}"',
  "name": "hb_grid",
  "max_thread": 4,
  "grid_uri": "0.0.0.0",
  "grid_port": 5555,
  "cache_uri": "redis-dev",
  "cache_port": 6379,
  "db_host": "mongo-dev",
  "db_port": 27017,
  "log_uri": "127.0.0.1",
  "log_port": 24224
}')
echo $DEBUG_CONFIG > ./docker_tmp/debug.json

PROD_CONFIG=$( jq -n '{
  "mark": '"${timestamp}"',
  "name": "hb_grid",
  "max_thread": 4,
  "grid_uri": "0.0.0.0",
  "grid_port": 5555,
  "cache_uri": "127.0.0.1",
  "cache_port": 6379,
  "db_host": "127.0.0.1",
  "db_port": 27017,
  "log_uri": "127.0.0.1",
  "log_port": 24224
}')
echo $PROD_CONFIG > ./docker_tmp/prod.json

cat << EOF > ./docker_tmp/docker-entrypoint.sh
#!/bin/bash

if [ "\$1" = 'dev' ]; then
  ./hb_grid --config debug.json
elif [ -z "\$1" ] || [ "\$1" = 'prod' ]; then
  ./hb_grid --config prod.json
else
  echo "Unknown argument: \$1"
  echo "Usage: \$0 [dev|prod]"
  exit 1
fi
# ./hb_grid --config prod.json
EOF

echoMsg "=============== TEMP DIR CREATED ==============="

docker build -f Dockerfile.debug -t hummingfab:v1.debug .

# if rm -r ./docker_tmp; then
#   echoMsg "=============== TEMP DIR DELTED ==============="
# else
#   echoErr "!!!!!!!!!!! tmp dir was not delete !!!!!!!!"
# fi

