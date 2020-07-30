# Simple TCP Server
### Простой многопоточный кроссплатформенный TCP Сервер

# Требования

* Git
* CMake

# Сборка

```shell
git clone https://github.com/PatifonKakao/Simple_TCP_Server 
cd Simple_TCP_Server
```

## Windows (Visual Studio)
* Измените версию Visual Studio в vs.bat
* Запустите vs.bat
* Соберите решение
* Назначьте запускаемые проекты: server_start и client_start.
Или запустите их из папки ..\output_64\bin\Debug\

## Linux

```shell
mkdir build
cd build
cmake ..
make
gnome-terminal -e './bin/server_start'
gnome-terminal -e './bin/client_start'
```
