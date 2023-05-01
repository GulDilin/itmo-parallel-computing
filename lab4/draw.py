import json
import argparse
from build import TargetDir
from run import plt_save


def main(args):
    with open(args.results_path, 'r') as f:
        results = json.load(f)

    for target in TargetDir:
        n_variants = results['N']
        plt_save(target, n_variants, results, postfix=args.postfix)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--result-path',
        dest="results_path",
        type=str,
        default='./assets/results.json'
    )
    parser.add_argument(
        '--postfix',
        dest="postfix",
        type=str,
        default='results'
    )
    args = parser.parse_args()
    main(args)
