import json
import argparse
from build import TargetDir
from run import plt_save


def calc_v(k, target, results):
    return [1 / it for it in results[target][str(k)]]


def main(args):
    with open(args.results_path, 'r') as f:
        results = json.load(f)

    results_acc = {}
    for target in args.targets:
        v_1 = calc_v(args.default_key, target, results)
        results_acc[target] = {}
        for k in results[target]:
            v_p = calc_v(k, target, results)
            results_acc[target][k] = [v_p[i]/v_1[i] for i in range(len(v_1))]
        n_variants = results['N']
        plt_save(TargetDir[target], n_variants, results_acc, postfix='acc')


if __name__ == '__main__':
    target_vals = [t.name for t in TargetDir]
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--target', '-t',
        dest="targets",
        nargs="*",
        default=target_vals,
        choices=target_vals
    )
    parser.add_argument(
        '--result-path',
        dest="results_path",
        type=str,
        default='./assets/results.json'
    )
    parser.add_argument(
        '--default-key',
        dest="default_key",
        type=str,
        default=1
    )
    args = parser.parse_args()
    main(args)
