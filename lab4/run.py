import json
import os
import argparse
from typing import Optional
import matplotlib.pyplot as plt
from build import TargetDir, LabPrefix

#  SPEC FOR SET_NUM_THREADS https://www.openmp.org/spec-html/5.0/openmpse50.html#x289-20540006.2
def run(
    target: TargetDir,
    n_size: int = 1,
    k: int = 0,
    ignore: bool = False,
    lab_name: LabPrefix = LabPrefix.DEFAULT,
    sort_threads: int = 2,
):
    env = ''
    if k == 0:
        lab_name = LabPrefix.NO_PARALLEL
    else:
        env = f'OMP_NUM_THREADS=2,{k} '
    path = f'{env}./{target.value}/{lab_name} {n_size} {k} {sort_threads}'
    result = os.popen(path).read()
    print(path)
    print(result)
    print()
    r = result.rsplit('\n')
    r = [it for it in r if not it.startswith('PROGRESS')]
    timing = r[-2]
    numbers_s = r[:-2]
    benchmarks = []
    bench_start_id = r.index('BENCHMARK')
    if bench_start_id:
        numbers_s = r[:bench_start_id]
        benchmarks_s = r[bench_start_id + 1:]
        benchmarks = benchmarks_s[:benchmarks_s.index('')]

    numbers_s = [it for it in numbers_s if it]
    numbers = ''.join(numbers_s)
    if not ignore:
        print(f'{path} {timing}')
    return numbers, float(timing), benchmarks


def get_n_variants(config: dict, target: TargetDir):
    n_1, n_2 = config[target.name]['1000'], config[target.name]['5000']
    delta = int((n_2 - n_1) / 10)
    n_variants = [n_1 + delta * i for i in range(10)]
    return n_variants


def plt_save(target: TargetDir, n_variants: list, results: dict, postfix: str = 'results', ylim: Optional[list] = None):
    for i in results[target.name]:
        print(f'{results[target.name][i]=}\n{n_variants=}')
        plt.plot(n_variants, results[target.name][i], label=f"parall {i}")

    if ylim:
        plt.ylim(ylim)
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

            for n_threads in k:
                benchmarks_total = []
                for n in n_variants:
                    numbers, timing, benchmarks = run(target, n, n_threads, False, sort_threads=args.sort_threads)
                    benchmarks_total.append(benchmarks)
                    results[target.name][n_threads].append(timing)

                    is_verified = args.local_config or numbers == verification.get(str(n), None)
                    if args.local_config and n_threads == 0:
                        verification[n] = numbers
                    print('verified' if is_verified else 'failed verification')
                    print(timing)
                print(f'BENCHMARK RESULTS: {n_threads}')
                for col in range(len(benchmarks_total[0])):
                    print(','.join([benchmarks_total[row][col] for row in range(len(benchmarks_total))]))
                print()

            plt_save(target, n_variants, results)
    except KeyboardInterrupt:
        pass

    with open('./assets/results.json', 'w') as f:
        json.dump(results, f)
    if args.local_config:
        with open('./assets/verification.json', 'w') as f:
            json.dump(verification, f)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    # k_variants = range(0, 17)
    k_variants = [2, 3, 4, 8, 16]
    parser.add_argument('--n-threads', '-k', dest="k_variants", type=int, nargs="*", default=k_variants)
    parser.add_argument('--n-variants', dest="n_variants", type=int, nargs="*", default=None)
    parser.add_argument('--n-start', dest="n_start", type=int, default=None)
    parser.add_argument('--n-end', dest="n_end", type=int, default=None)
    parser.add_argument('--n-amount', dest="n_amount", type=int, default=10)
    parser.add_argument('--sort-threads', dest="sort_threads", type=int, default=2)
    parser.add_argument('--local-config', dest="local_config", action="store_true", default=False)
    args = parser.parse_args()
    if args.n_start and args.n_end:
        args.n_variants = range(
            args.n_start,
            args.n_end,
            int((args.n_end - args.n_start) / args.n_amount)
        )
    main(args)
