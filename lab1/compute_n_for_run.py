import os
from build import TargetDir
from run import run
import json


def find_n_for_exec_time(target: TargetDir, exec_time: int):
    threshold_ms = 100
    n_start = 200
    n_end = 0
    n = n_start
    time = 0
    while True:
        _, time = run(target, n_start)
        if time > exec_time:
            n_end = n_start
            n_start = int(n_start / 2)
            break
        n_start *= 2

    while abs(exec_time - time) > threshold_ms:
        n = int((n_start + n_end) / 2)
        _, time = run(target, n)
        if time > exec_time:
            n_end = n
        else:
            n_start = n

    print(f'{n=} {time=}')
    return n


def main():
    results = {}
    try:
        for target in [TargetDir.TCC]:
            results[target.name] = {}
            results[target.name]['1000'] = find_n_for_exec_time(target, 1000)
            results[target.name]['5000'] = find_n_for_exec_time(target, 5000)
    except KeyboardInterrupt:
        pass

    try:
        os.makedirs('assets')
    except FileExistsError:
        pass
    with open('./assets/n_config.json', 'w') as f:
        json.dump(results, f, indent=2)


if __name__ == '__main__':
    main()
