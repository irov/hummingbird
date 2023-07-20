# Docker

Build image

```
docker build -f Dockerfile.debug -v /usr/bin/python:/usr/bin/python -t hummingfab:debug .
```

- f - правило сборки
- v - для сборки проекта нужен python, [локальный_python]:[python_в_docker]
- t -
