import json
import argparse
from build import TargetDir
from run import plt_save


def main(args):
    with open(args.results_path, 'r') as f:
        results = json.load(f)

    for target in TargetDir:
        n_variants = results['N']
        ylim = None
        if args.y_start is not None and args.y_end is not None:
            ylim = [args.y_start, args.y_end]
            print(f'{ylim=}')
        plt_save(target, n_variants, results, postfix=args.postfix, ylim=ylim)


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
    parser.add_argument(
        '--y-start',
        dest="y_start",
        type=int,
    )
    parser.add_argument(
        '--y-end',
        dest="y_end",
        type=int,
    )
    args = parser.parse_args()
    main(args)
