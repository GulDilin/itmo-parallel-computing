import os
import shutil
from enum import Enum


class TargetDir(str, Enum):
    # GCC = 'target-gcc'
    CLANG = 'target-clang'
    # TCC = 'target-tcc'


class LabPrefix(str, Enum):
    NO_PARALLEL = 'lab-1-no-parallel'
    PARALLEL = 'lab-1-parallel'


def clear():
    for v in TargetDir:
        try:
            shutil.rmtree(f'./{v.value}')
        except FileNotFoundError:
            pass
        os.mkdir(v.value)


def build_non_parallel() -> None:
    os.system(f'gcc -O3 -Wall -Werror -o {TargetDir.GCC}/{LabPrefix.NO_PARALLEL} main.c -lm')
    os.system(f'clang -O3 -fno-vectorize -fno-slp-vectorize -Wall -Werror -o {TargetDir.CLANG}/{LabPrefix.NO_PARALLEL} main.c -lm')
    os.system(f'tcc -O3 -Wall -Werror -o {TargetDir.TCC}/{LabPrefix.NO_PARALLEL} main.c -lm')


def build_parallel(n_threads: int) -> None:
    os.system(
        (
            "gcc -O3 -Wall -Werror -floop-parallelize-all "
            f"-ftree-parallelize-loops={n_threads} main.c "
            f"-o {TargetDir.GCC}/{LabPrefix.PARALLEL}-{n_threads} -lm"
        )
    )
    os.system(
        (
            f"clang -O3 -Wall -Werror -mllvm -force-vector-width={n_threads} "
            f"main.c -o {TargetDir.CLANG}/{LabPrefix.PARALLEL}-{n_threads} -lm"
        )
    )
    os.system(
        (
            f"tcc -O3 -Wall -Werror -floop-parallelize-all "
            f"-ftree-parallelize-loops={n_threads} "
            f"main.c -o {TargetDir.TCC}/{LabPrefix.PARALLEL}-{n_threads} -lm"
        )
    )


def main():
    clear()
    build_non_parallel()
    k = [1, 2, 4, 16]
    for n_threads in k:
        build_parallel(n_threads)


if __name__ == '__main__':
    main()
