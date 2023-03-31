import json
import os
import matplotlib.pyplot as plt
from build import TargetDir, LabPrefix


def run(target: TargetDir, n_size: int = 1, k: int = 0, ignore: bool = False, lab_name: LabPrefix = LabPrefix.DEFAULT):
    libs_env = 'LD_LIBRARY_PATH="$PWD/FW_1.3.1_Lin64/lib"'
    path = f'{libs_env} ./{target.value}/{lab_name} {n_size} {k}'
    result = os.popen(path).read()
    numbers, timing = result.split('\n')[:2]
    if not ignore:
        print(f'{path} {timing}')
    # if not ignore:
    #     print(result)
    return numbers, int(timing)


def get_n_variants(config: dict, target: TargetDir):
    n_1, n_2 = config[target.name]['1000'], config[target.name]['5000']
    delta = int((n_2 - n_1) / 10)
    n_variants = [n_1 + delta * i for i in range(10)]
    return n_variants


def plt_save(target: TargetDir, n_variants: list, results: dict):
    for i in results[target.name]:
        print(f'{results[target.name][i]=}\n{n_variants=}')
        plt.plot(n_variants, results[target.name][i], label=f"parall {i}")

    plt.xlabel('N')
    plt.ylabel('Exec ms')
    plt.legend()
    plt.savefig(f'./assets/{target.name}_results.png')
    plt.clf()


def main():
    k = range(1, 17)

    with open('../lab1/assets/n_config.json', 'r') as f:
        config = json.load(f)

    results = {}
    try:
        for target in TargetDir:
            expected = ''
            n_variants = get_n_variants(config, target)
            results['N'] = list(n_variants)
            print(f'{target=} {n_variants=}')
            results[target.name] = {}
            for n_threads in k:
                results[target.name][n_threads] = []

            for n in n_variants:
                for n_threads in k:
                    # prepend run
                    for i in range(3):
                        run(target, 100, n_threads, True)

                    numbers, timing = run(target, n, n_threads, False)
                    results[target.name][n_threads].append(timing)

                    if n_threads == 1:
                        print('verification init')
                        expected = numbers
                    else:
                        print('verified' if numbers == expected else 'failed verification')
                        print(timing)
            plt_save(target, n_variants, results)
    except KeyboardInterrupt:
        pass

    with open('./assets/results.json', 'w') as f:
        json.dump(results, f)


if __name__ == '__main__':
    main()
