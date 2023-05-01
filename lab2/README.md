# Лабораторная работа 2

## Исследование эффективности параллельных библиотек для C-программ

### Установка библиотеки AMD Framewave

1. Скачать https://sourceforge.net/projects/framewave/files/framewave-releases/Framewave%201.3.1/FW_1.3.1_Lin64.tar.gz/download

2. Распаковать tar gz
```
tar -xf FW_1.3.1_Lin64.tar.gz
```

3. Перейти в директорию и создать ссылки
```shell
cd FW_1.3.1_Lin64/lib

ln -sf ./libfwBase.so.1.3.1 libfwBase.so
ln -sf ./libfwSignal.so.1.3.1 libfwSignal.so
ln -sf ./libfwImage.so.1.3.1 libfwImage.so
ln -sf ./libfwJPEG.so.1.3.1 libfwJPEG.so
ln -sf ./libfwVideo.so.1.3.1 libfwVideo.so

ln -sf ./libfwBase.so.1.3.1 libfwBase.so.1
ln -sf ./libfwSignal.so.1.3.1 libfwSignal.so.1
ln -sf ./libfwImage.so.1.3.1 libfwImage.so.1
ln -sf ./libfwJPEG.so.1.3.1 libfwJPEG.so.1
ln -sf ./libfwVideo.so.1.3.1 libfwVideo.so.1
```

4. При компиляции выставить ключи
```shell
g++ -m64 -c -IFW_1.3.1_Lin64 test.cpp
clang -m64 -c -IFW_1.3.1_Lin64 main.c

clang -LFW_1.3.1_Lin64/lib -o lab2 main.c -lm -lfwSignal -lfwBase
export LD_LIBRARY_PATH="$PWD/FW_1.3.1_Lin64/lib"
```

### Проведение эксперимента

1. Установить компилятор *clang*
```
sudo apt-get update
sudo apt-get install -y clang
```
2. Установить `Python-3.8 +`
```
sudo apt-get install -y python3 python3-pip
pip install -r requirements.txt
```
3. Сборка эксперементальных билдов
```
python build.py
```

4. Подборка входных данных по времени выполнения для эксперементальных запуск
```
python compute_n_for_run.py
```

5. Запуск эксперимента и генерация графиков
```
python run.py
```
