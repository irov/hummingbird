#!/bin/bash

if [ "$1" = "dev" ]; then
  # ./hb_grid --config debug.json
  echo "Starting debug server"
elif [ -z "$1" ] || [ "$1" == "prod" ]; then
  # ./hb_grid --config prod.json
  echo "Starting production server"
else
  echo "Unknown argument: $1"
  echo "Usage: $0 [dev|prod]"
  exit 1
fi
# ./hb_grid --config prod.json


