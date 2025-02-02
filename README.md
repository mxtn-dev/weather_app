# Погода

#### Описание домашнего задания:

Написать программу, скачивающую с помощью libcurl и разбирающую с помощью произвольной сторонней библиотеки для JSON в C текущие погодные данные с API wttr.in для заданного аргументом командной строки города.

#### Требования:

- Выбрана библиотека для работы с JSON в C.
- Создано консольное приложение, принимающее аргументом командной строки название города (например, Moscow).
- Приложение выводит на экран прогноз погоды на текущий день: текстовое описание погоды, направление и скорость ветра, диапазон температуры.
- Приложение корректно обрабатывает ошибочно заданную локацию и сетевые ошибки.
- Код компилируется без предупреждений с ключами компилятора -Wall -Wextra -Wpedantic -std=c23.

Устанавливаем бибилиотеки в Ubuntu:

```
sudo apt install libcurl4-openssl-dev libcjson-dev
```
Проверяем компилятор:

```
gcc --version
```

Используем 23 стандарт:

```
CFLAGS = -Wall -Wextra -Wpedantic -std=c23 -O2
```