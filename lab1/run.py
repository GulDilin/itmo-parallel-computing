import json
import os
import matplotlib.pyplot as plt
from build import TargetDir, LabPrefix
import argparse


def run(target: TargetDir, n_size: int = 1, k: int = 0, ignore: bool = False):
    lab_name = LabPrefix.NO_PARALLEL if k == 0 else f'{LabPrefix.PARALLEL}-{k}'
    path = f'./{target.value}/{lab_name} {n_size}'
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
    k = [1, 2, 4, 16]

    with open('./assets/n_config.json', 'r') as f:
        config = json.load(f)

    results = {}
    verification = {}
    targets = [t for t in TargetDir if t.name in args.targets]
    try:
        for target in targets:
            n_variants = args.n_variants or get_n_variants(config, target)
            print(f'{target=} {n_variants=}')
            results[target.name] = {}
            for n_threads in [0, *k]:
                if args.k_variants and n_threads not in args.k_variants:
                    continue
                results[target.name][n_threads] = []

            for n in n_variants:
                if not args.k_variants or 0 in args.k_variants:
                    numbers, timing = run(target, n, 0, False)
                    results[target.name][0].append(timing)
                    verification[n] = numbers

                for n_threads in k:
                    if args.k_variants and n_threads not in args.k_variants:
                        continue
                    # prepend run
                    for i in range(3):
                        run(target, 100, n_threads, True)

                    numbers, timing = run(target, n, n_threads, False)
                    results[target.name][n_threads].append(timing)

                    is_verified = numbers == verification[n]
                    print('verified' if is_verified else 'failed verification')
                    print(timing)
            plt_save(target, n_variants, results)
    except KeyboardInterrupt:
        pass

    with open('./assets/results.json', 'w') as f:
        json.dump(results, f)
    with open('./assets/verification.json', 'w') as f:
        json.dump(verification, f)


if __name__ == '__main__':
    target_vals = [t.name for t in TargetDir]
    k_variants = (0, 1, 2, 4, 16)
    parser = argparse.ArgumentParser()
    parser.add_argument('--target', '-t', dest="targets", nargs="*", default=target_vals, choices=target_vals)
    parser.add_argument('--n-threads', '-k', dest="k_variants", type=int, nargs="*", default=None, choices=k_variants)
    parser.add_argument('--n-variants', dest="n_variants", type=int, nargs="*", default=None)
    args = parser.parse_args()
    main(args)
