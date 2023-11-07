# Docker

## Config
* *0.0.0.0* - INADDR_ANY (only ipv4)
* *127.0.0.1* - INADDR_LOOPBACK (only)
* *::1* - IN6ADDR_LOOPBACK (only)
* *localhost* - system dns (127.0.0.1 and ::1)

## Geting Started
Если нужно замонтировать папку логов для fluentd в docker-compose.yml раскомментировать volumes и указать host-путь, у которого есть разрешения на запись.
```
chmod o+w -R you_path_to_log
```

## Cheetsheet

Build image
```
docker build -f Dockerfile.build -t hummingfab:build .
```

Run and connect to container
```
docker run -it --rm hummingfab:build
```

* `docker login` принимает логин только в нижнем регистре [TakiMoysha -> takimoysha].

## Help

- `docker rmi $(docker images --filter dangling=true -q)` - удалить <none> изображения
- `docker run -d --name mongo-dev -p 27017:27017 --rm mongo` - запустить монгодб в фоне, под именем mongo-dev, с пробросом порта 27017, с автоудалением после завершения
- `docker run --name redis-dev -p 6379:6379 -d redis` - запустить редиc

- `docker run --name mongo-dev -p 27017:27017 --network dev-network --memory=512m -d mongo`
- `docker run --name redis-dev -p 6379:6379 --network dev-network --memory=512m -d redis`
- `docker run --name hb_grid`

- `docker inspect <image_id>` - посмотреть инфу по изображению
- `docker tag <username>/<image_name>:<tag> <new_username>/<new_image_name>:<new_tag>` - изменить репозиторий, имя, тег

Production
- `sudo docker run --rm --name hb_grid \
    -v /home/admin/workspace/tmp/:/hummingfab/tmp/ \
    --add-host=mongo-dev:172.17.0.1 \
    --add-host=redis-dev:172.17.0.1 \
    -it -p 5555:5555 \
    -d --security-opt seccomp=unconfined \
    takimoysha/hummingfab:v1.debug
    `

## Exaples

```
docker build -f Dockerfile.debug -v /usr/bin/python:/usr/bin/python -t hummingfab:debug .
```

- f - файл сборки
- v - для сборки проекта нужен python, [локальный_python]:[python_в_docker]
- t - тег
