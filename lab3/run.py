import json
import os
import argparse
import matplotlib.pyplot as plt
from build import TargetDir, LabPrefix


def run(
    target: TargetDir,
    n_size: int = 1,
    k: int = 0,
    ignore: bool = False,
    lab_name: LabPrefix = LabPrefix.DEFAULT
):
    env = ''
    if k == 0:
        lab_name = LabPrefix.NO_PARALLEL
    else:
        env = f'OMP_NUM_THREADS={k} OMP_DYNAMIC=FALSE '
    path = f'{env}./{target.value}/{lab_name} {n_size} {k}'
    result = os.popen(path).read()
    numbers, timing = result.split('\n')[:2]
    if not ignore:
        print(f'{path} {timing}')
    return numbers, int(timing)


def get_n_variants(config: dict, target: TargetDir):
    n_1, n_2 = config[target.name]['1000'], config[target.name]['5000']
    delta = int((n_2 - n_1) / 10)
    n_variants = [n_1 + delta * i for i in range(10)]
    return n_variants


def plt_save(target: TargetDir, n_variants: list, results: dict, postfix: str = 'results'):
    for i in results[target.name]:
        print(f'{results[target.name][i]=}\n{n_variants=}')
        plt.plot(n_variants, results[target.name][i], label=f"parall {i}")

    plt.xlabel('N')
    plt.ylabel('Exec ms')
    plt.legend()
    plt.savefig(f'./assets/{target.name}_{postfix}.png')
    plt.clf()


def main(args):
    k = args.k_variants

    config_path = './assets/n_config.json' if args.local_config else '../lab1/assets/n_config.json'
    with open(config_path, 'r') as f:
        config = json.load(f)
    with open('../lab1/assets/verification.json', 'r') as f:
        verification = json.load(f)

    results = {}
    try:
        for target in TargetDir:
            n_variants = args.n_variants or get_n_variants(config, target)
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

                    is_verified = args.local_config or numbers == verification.get(str(n), None)
                    print('verified' if is_verified else 'failed verification')
                    print(timing)

            plt_save(target, n_variants, results)
    except KeyboardInterrupt:
        pass

    with open('./assets/results.json', 'w') as f:
        json.dump(results, f)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    # k_variants = range(0, 17)
    k_variants = [0, 1, 2, 3, 4, 8, 16, 32]
    parser.add_argument('--n-threads', '-k', dest="k_variants", type=int, nargs="*", default=k_variants)
    parser.add_argument('--n-variants', dest="n_variants", type=int, nargs="*", default=None)
    parser.add_argument('--n-start', dest="n_start", type=int, default=None)
    parser.add_argument('--n-end', dest="n_end", type=int, default=None)
    parser.add_argument('--n-amount', dest="n_amount", type=int, default=10)
    parser.add_argument('--local-config', dest="local_config", action="store_true", default=False)
    args = parser.parse_args()
    if args.n_start and args.n_end:
        args.n_variants = range(
            args.n_start,
            args.n_end,
            int((args.n_end - args.n_start) / args.n_amount)
        )
    main(args)
