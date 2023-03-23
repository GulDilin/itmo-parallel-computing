# Лабораторная работа 2

## Автоматическое распараллеливание программ

### Проведение эксперимента

1. Установить компиляторы *gcc*, *clang*, *tcc*
```
sudo apt-get update
sudo apt-get install -y gcc clang tcc
```
2. Установить `Python-3.10`
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