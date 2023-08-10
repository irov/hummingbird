# hummingbird
fast game server

[![Build status](https://ci.appveyor.com/api/projects/status/mc9j9w88oh24n0ve?svg=true)](https://ci.appveyor.com/project/irov/hummingbird)
[![Build Status](https://travis-ci.org/irov/hummingbird.svg?branch=master)](https://travis-ci.org/irov/hummingbird)
[![APM](https://img.shields.io/apm/l/vim-mode)](https://en.wikipedia.org/wiki/MIT_License)
[![C](https://img.shields.io/badge/language-C-red.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

## Windows [Sandbox]

Для начала работы нужно скачать исходники библиотек зависимостей

`build\downloads\downloads.bat`
    
Дальше мы можем собрать например дебаг для Visual Studio 16 2019

`\build\msvc16\build_dependencies_debug.bat.bat`

и сформировать солюшен

`\build\msvc16\make_solution_debug.bat`
    
который мы сможем найти в папке

`\solutions\solution_msvc16\Debug\hummingbird_win32.sln`

Удачи в сборке, и использованию!

### MongoDB
https://www.mongodb.com/download-center/community
### Redis
https://github.com/dmajkic/redis/downloads

### Docker
More in ./docker.md
```
docker-compose up
```

[![CodeFactor](https://www.codefactor.io/repository/github/irov/hummingbird/badge)](https://www.codefactor.io/repository/github/irov/hummingbird) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/21af5c85dcc1429bb380225862f5cc73)](https://www.codacy.com/manual/irov13/hummingbird?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=irov/hummingbird&amp;utm_campaign=Badge_Grade)
