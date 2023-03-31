import json
import argparse
from build import TargetDir
from run import plt_save, get_n_variants


def calc_v(k, target: str, results: dict):
    return [1 / it for it in results[target][str(k)]]


def main(args):
    config_path = '../lab1/assets/n_config.json'
    with open(config_path, 'r') as f:
        config = json.load(f)
    with open('./assets/results.json', 'r') as f:
        results = json.load(f)

    results_acc = {}
    for target in args.targets:
        v_1 = calc_v(1, target, results)
        results_acc[target] = {}
        for k in results[target]:
            v_p = calc_v(k, target, results)
            results_acc[target][k] = [v_p[i]/v_1[i] for i in range(len(v_1))]
        n_variants = get_n_variants(config, TargetDir[target])
        plt_save(TargetDir[target], n_variants, results_acc, postfix='acc')


if __name__ == '__main__':
    target_vals = [t.name for t in TargetDir]
    parser = argparse.ArgumentParser()
    parser.add_argument('--target', '-t', dest="targets", nargs="*", default=target_vals, choices=target_vals)
    args = parser.parse_args()
    main(args)
