import os
import shutil
from enum import Enum


class TargetDir(str, Enum):
    # GCC = 'target-gcc'
    CLANG = 'target-clang'
    # TCC = 'target-tcc'


class LabPrefix(str, Enum):
    DEFAULT = 'lab-4'
    NO_PARALLEL = 'lab-4-no-parallel'
    ADDITIONAL = 'lab-4-add'


def clear():
    for v in TargetDir:
        try:
            shutil.rmtree(f'./{v.value}')
        except FileNotFoundError:
            pass
        os.mkdir(v.value)


def build() -> None:
    for target in TargetDir:
        os.system(
            (
                f'{target.name.lower()} -std=gnu11 -O3 -Wall -Werror main.c '
                f'-o {target.value}/{LabPrefix.NO_PARALLEL} -lm -lgomp'
            )
        )
        os.system(
            (
                f'{target.name.lower()} -std=gnu11 -O1 -Wall -Werror -fopenmp main.c '
                f'-o {target.value}/{LabPrefix.DEFAULT} -lm -lgomp'
            )
        )
        os.system(
            (
                f'{target.name.lower()} -std=gnu11 -O3 -Wall -Werror -fopenmp calc_additional_time.c '
                f'-o {target.value}/{LabPrefix.ADDITIONAL} -lm -lgomp'
            )
        )


def main():
    clear()
    build()


if __name__ == '__main__':
    main()
