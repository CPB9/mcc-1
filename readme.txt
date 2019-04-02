Windows

1. Скачать и установить meson: https://github.com/mesonbuild/meson/releases
1.1 установить python 3
1.2 установить пакеты python:
    pip install meson
    pip install colorama
2. Скачать и установить qt
3*. В переменную среды PATH добавить путь к папке qt (Например, C:\libs\Qt\5.11.2\msvc2017_64\bin) (Можно и не настраивать переменную среды (см. далее))
4. Запустить командную строку с переменными окружения VS x64 (ВНИМАНИЕ!!! обратить внимание, что в консоли появится запись об x64, а не x86)
Это можно сделать 2мя способами:
4.1 создать ярлык на командную строку и прописать в его свойствах: %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
4.2 непосредественно запустить vcvars64.bat из директории установки VS
5*. Если не выполнялся п.3: В открой консоли настроить окружение qt (например: C:\libs\Qt\5.11.2\msvc2017_64\bin\qtenv2.bat)
6. Перейти в директорию исходников mcc
7. Создать подкаталог build: mkdir build
8. Перейти в build: cd build

Для сборки с помощью VS:
9. Создать проект сборки VS: meson.exe --backend=vs2017 ..
9.1. Создать проект сборки VS: meson.exe --backend=vs2017 --buildtype=release ..
10. Открыть проект mcc.sln
11. Rebuild all
12. Запустить

Во время работы с VS:
1. Для применения изменений meson.build файлов необходимо собирать проект REGEN
В этом случае может возникнуть проблема вроде: error MSB8036: The Windows SDK version 8.1 was not found. Install the required version of Windows SDK or change the SDK version in the project property pages or by right-clicking the solution and selecting "Retarget solution".
Для решения проблемы нужно клинуть меню(или тоже самое через клик по солюшену): Project -> Retarget Solution. В появившемся диалоге выбрать последнюю версию Windows SDK.
После этого обычным образом пересобрать солюшен.


Для сборки с помощью Ninja:
4. Скачать и Установить Ninja: https://github.com/ninja-build/ninja/releases
9. Создать проект сборки: meson.exe --backend=ninja ..
9.1. Для сборки релиза:  meson.exe --backend=ninja --buildtype=release ..
10. ninja
11. запустить bin/mcc.exe


Для записи видео:
1. скачать ffmpeg: https://ffmpeg.zeranoe.com/builds/
2. указать путь к папке c ffmpeg.exe в settings.ini (по умолчанию, МПУ ищет ffmpeg в ffmpeg/bin/ffmpeg относительно корня МПУ)

Для формирования архива исходников:
1. установить gitpython: pip install gitpython
2. выполнить: pack-sources.py modules-opensource.json


Linux

1. Установить средства meson:
    sudo apt install python python3 python3-pip
    pip3 install meson
2. Устновить средства сборки:
    sudo apt install g++ ninja-build
3. Установить пакеты, требуемые для сборки проекта
    sudo apt install qt5-default libqt5svg5-dev qtpositioning5-dev libqt5serialport5-dev qttools5-dev-tools uuid-dev pkg-config libudev-dev libssl-dev
4. Перейти в директорию исходников mcc
5. Создать подкаталог build: mkdir build
6. Перейти в build: cd build
7. meson --backend=ninja --buildtype=debug ..
8. ninja

Настройка QtCreator

