import os
import matplotlib.pyplot as plt
from build import TargetDir, LabPrefix


def run(target: TargetDir, n_size: int = 1):
    libs_env = 'LD_LIBRARY_PATH="$PWD/FW_1.3.1_Lin64/lib"'
    path = f'{libs_env} ./{target.value}/{LabPrefix.ADDITIONAL} {n_size}'
    result = os.popen(path).read()
    timing = result
    print(f'{timing}')
    return int(timing)


def plt_save(target: TargetDir, n_variants: list, results: dict):
    print(f'{results[target.name]=}\n{n_variants=}')
    plt.plot(n_variants, results[target.name], label=f"{target.name}")

    plt.xlabel('N')
    plt.ylabel('Exec ms')
    plt.legend()
    plt.savefig(f'./assets/{target.name}_additional_results.png')
    plt.clf()


def main():
    results = {}
    n_variants = list(range(1, 100))
    try:
        for target in TargetDir:
            print(f'{target=} {n_variants=}')
            results[target.name] = []

            for n_threads in n_variants:
                timing = run(target, n_threads)
                results[target.name].append(timing)
            plt_save(target, n_variants, results)
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()
