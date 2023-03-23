# Параллельное программирование

Лабораторные работы по предмету "Параллельное программирование" для 1 курса магистратуры СППО Университета ИТМО

[Методические указания](./instructions.pdf)

[Отчёт по лабоработной 1](./report-lab1.pdf)

[Отчёт по лабоработной 2](./report-lab2.pdf)


# Лабораторная №1: Автоматическое распараллеливание программ #
**Вариант A = 280; map=[4,6]; merge=4; sort=4**

1. На компьютере с многоядерным процессором установить ОС Linux и компилятор GCC версии не ниже 4.7.2. При невозможности установить Linux или отсутствии компьютера с многоядерным процессором можно выполнять лабораторную работу на виртуальной машине. Минимальное количество ядер при использовании виртуальной машины - 2.

2. На языке Cи написать консольную программу lab1.c, решающую задачу, указанную в п.IV (см. ниже). В программе нельзя исполь- зовать библиотечные функции сортировки, выполнения матричных операций и расчёта статистических величин. В программе нельзя использовать библиотечные функции, отсутствующие в стандарт- ных заголовочных файлах stdio.h, stdlib.h, math.h, sys/time.h. Задача должна решаться 100 раз с разными начальными значениями ге- нератора случайных чисел (ГСЧ). Структура программы примерно следующая:
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
int main(int argc, char* argv[])
{
    int i, N;
    struct timeval T1, T2;
    long delta_ms;
    N = atoi(argv[1]); /* N равен первому параметру командной строки */
    gettimeofday(&T1, NULL); /* запомнить текущее время T1 */
    for (i=0; i<100; i++) /* 100 экспериментов */
    {
        srand(i);  /* инициализировать начальное значение ГСЧ   */
        /* Заполнить массив исходных данных размером N */
        /* Решить поставленную задачу, заполнить массив с результатами
        /* Отсортировать массив с результатами указанным методом */
    }
    gettimeofday(&T2, NULL);   /* запомнить текущее время T2 */
    delta_ms =  1000*(T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\nN=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 - T1 */
    return 0;
}
```

3. Скомпилировать написанную программу без использования авто- матического распараллеливания с помощью следующей команды: `/home/user/gcc -O3 -Wall -Werror -o lab1-seq lab1.c`

4. Скомпилировать написанную программу, используя встроенное в gcc средство автоматического распараллеливания Graphite с помо- щью следующей команды `/home/user/gcc -O3 -Wall -Werror -floop- parallelize-all -ftree-parallelize-loops=K lab1.c -o lab1-par-K` (пере- менной K поочерёдно присвоить хотя бы 4 значения: 1, меньше количества ядер, равное количеству физических ядер и больше ко- личества физических ядер).

5. В результате получится одна нераспараллеленная программа и че- тыре или более распараллеленных.

6. Закрыть все работающие в операционной системе прикладные про- граммы (включая Winamp, uTorrent, браузеры и Skype), чтобы они не влияли на результаты последующих экспериментов. При ис- пользовании ноутбука необходимо иметь постоянное подключение к сети питания, на время проведения эксперимента.

7. Запускать файл lab1-seq из командной строки, увеличивая значения N до значения N1, при котором время выполнения превысит 0.01 с. Подобным образом найти значение N=N2, при котором время выполнения превысит 5 с.

8. Используя найденные значения N1 и N2, выполнить следующие эксперименты (для автоматизации проведения экспериментов рекомендуется написать скрипт):
    - запускать lab1-seq для значений `N = N1, N1+∆, N1+2∆, N1+3∆,..., N2` и записывать получающиеся значения времени `delta_ms(N)` в функцию `seq(N)`;
    - запускать lab1-par-K для значений `N = N1, N1+∆, N1+2∆, N1+3∆,..., N2` и записывать получающиеся значения времени `delta_ms(N)` в функцию `par − K(N)`;
    - значение ∆ выбрать так: `∆ = (N2 − N1)/10`.

9. Провести верификацию значения X. Добавить в конец цикла вывод значения X и изменить количество экспериментов на 5. Сравнить значения X для распараллеленной программы и не распараллеленной.

10. Написать отчёт о проделанной работе.

11. Подготовиться к устным вопросам на защите.

12. Найти вычислительную сложность алгоритма до и после распарал- леливания, сравнить полученные результаты.

13. Необязательное задание No1 (для получения оценки «четыре» и «пять»). Провести аналогичные описанным экс- перименты, используя вместо gcc компилятор Solaris Studio (или любой другой на своё усмотрение). При компиля- ции следует использовать следующие опции для автома- тического распараллеливания: `solarisstudio -cc -O3 - xautopar -xloopinfo lab1.c`.

14. Необязательное задание No2 (для получения оценки «пять»). Это задание выполняется только после выполнения предыдущего пункта. Провести аналогичные описанным эксперименты, исполь- зуя вместо gcc компилятор Intel ICC (или любой другой на своё усмотрение). В ICC следует при компиляции использовать следу- ющие опции для автоматического распараллеливания: `icc -parallel -par-threashold0 -par-num-threads=K -o lab1-icc-par-K lab1.c`.



# Лабораторная №2: Исследование эффективности параллельных библиотек для C-программ #

1. В исходном коде программы, полученной в результате выполнения лабораторной работы №1, нужно на этапах Map и Merge все циклы
с вызовами математических функций заменить их векторными ана- логами из библиотеки «AMD Framewave» (http://framewave. sourceforge.net). При выборе конкретной Framewave-функции необходимо убедиться, что она помечена как `MT` (Multi-Threaded), т.е. распараллеленная. Полный перечень доступных функций на- ходится по ссылке: http://framewave.sourceforge.net/Manual/fw_section_060.html#fw_section_060. Например, Framewave-функция `min` в списке поддерживаемых технологий имеет только `SSE2`, но не `MT`.

    ***Примечание:*** выбор библиотеки Framewave не является обязатель- ным, можно использовать любую другую параллельную библиоте- ку, если в ней нужные функции распараллелены, так, например, можно использовать ATLAS (для этой библиотеки необходимо вы- ключить троттлинг и энергосбережение, а также разобраться с ме- ханизмом изменения числа потоков) или Intel Integrated Performance Primitives.

2. Добавить в начало программы вызов Framewave-функции `SetNumThreads(M)` для установки количества создаваемых парал- лельной библиотекой потоков, задействуемых при выполнении рас- параллеленных Framewave-функций. Нужное число M следует уста- навливать из параметра командной строки `(argv)` для удобства ав- томатизации экспериментов.

    ***Примечание:*** При использовании Intel IPP функцию SetNumThreads(M) не нужно использовать. Необходимо компилировать программу под разное количество потоков.

3. Скомпилировать программу, не применяя опции автоматического распараллеливания, использованные в лабораторной работе №1. Провести эксперименты с полученной программой для тех же значений N1 и N2, которые использовались в лабораторной работе No1, при M = 1, 2, . . . , K, где K – количество процессоров (ядер) на экспериментальном стенде.

4. Сравнить полученные результаты с результатами лабораторной ра- боты No1: на графиках показать, как изменилось время выполне- ния программы, параллельное ускорение и параллельная эффективность.

5. Написать отчёт о проделанной работе.

6. Подготовиться к устным вопросам на защите.

7. **Необязательное задание №1** (для получения оценки «четыре» и «пять»). Исследовать параллельное ускорение для различных зна- чений M > K, т.е. оценить накладные расходы при создании чрез- мерного большого количества потоков. Для иллюстрации того, что программа действительно распараллелилась, привести график за- грузки процессора (ядер) во время выполнения программы при N = N2 для всех использованных M. Для получения графика можно как написать скрипт, так и просто сделать скриншот дис- петчера задач, указав на скриншоте моменты начала и окончания эксперимента (в отчёте нужно привести текст скрипта или назва- ние использованного диспетчера).

8. **Необязательное задание №2** (для получения оценки «пять»). Это задание выполняется только после выполнения предыдущего пунк- та. Используя закон Амдала, рассчитать коэффициент распаралле- ливания для всех экспериментов и привести его на графиках. Про- комментировать полученные результаты.
