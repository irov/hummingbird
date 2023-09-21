# Docker

## Easy
Скачать готовые сборки и запустить:
```
docker-compose up
```

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

## Help

- `docker rmi $(docker images --filter dangling=true -q)` - удалить <none> изображения
- `docker run -d --name mongo-dev -p 27017:27017 --rm mongo` - запустить монгодб в фоне, под именем mongo-dev, с пробросом порта 27017, с автоудалением после завершения
- `docker run --name redis-dev -p 6379:6379 -d redis` - запустить реди

- `docker run --name mongo-dev -p 27017:27017 --network dev-network -d --rm mongo`
- `docker run --name redis-dev -p 6379:6379 --network dev-network -d redis`
## Exaples

```
docker build -f Dockerfile.debug -v /usr/bin/python:/usr/bin/python -t hummingfab:debug .
```

- f - правило сборки
- v - для сборки проекта нужен python, [локальный_python]:[python_в_docker]
- t -
