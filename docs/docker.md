# Docker

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

## Exaples

```
docker build -f Dockerfile.debug -v /usr/bin/python:/usr/bin/python -t hummingfab:debug .
```

- f - правило сборки
- v - для сборки проекта нужен python, [локальный_python]:[python_в_docker]
- t -
