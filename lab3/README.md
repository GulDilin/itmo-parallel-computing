# Лабораторная работа 3

## Распараллеливание циклов с помощью технологии OpenMP

### Проведение эксперимента

1. Установить компилятор *clang*
```
sudo apt-get update
sudo apt-get install -y clang
```
2. Установить `Python-3.10`
3. Сборка эксперементальных билдов
```
python build.py
```

4. Запуск эксперимента и генерация графиков
```
python run.py
```

5. Подсчёт времени на накладные расходы
```
python calc_par_time.py
```

6. Создание графика на основе файла с результатами
```
python draw.py

usage: draw.py [-h] [--result-path RESULTS_PATH]

optional arguments:
  -h, --help            show this help message and exit
  --result-path RESULTS_PATH
```

7. Создание графика параллельного ускорения на основе файла с результатами
```
python calc_acc.py

usage: calc_acc.py [-h] [--target [{CLANG} [{CLANG} ...]]] [--result-path RESULTS_PATH]
                   [--default-key DEFAULT_KEY]

optional arguments:
  -h, --help            show this help message and exit
  --target [{CLANG} [{CLANG} ...]], -t [{CLANG} [{CLANG} ...]]
  --result-path RESULTS_PATH
  --default-key DEFAULT_KEY
```
