import os
from contextlib import contextmanager
import shutil
from enum import Enum


class TargetDir(str, Enum):
    CLANG = 'target-clang'


class LabPrefix(str, Enum):
    DEFAULT = 'lab-2'
    ADDITIONAL = 'calc_additional_time'


LIBS_DIR = 'FW_1.3.1_Lin64'


@contextmanager
def cwd(path):
    owd = os.getcwd()
    os.chdir(path)
    try:
        yield
    except Exception as e:
        raise e
    finally:
        os.chdir(owd)


def clear():
    for v in TargetDir:
        try:
            shutil.rmtree(f'./{v.value}')
        except FileNotFoundError:
            pass
        os.mkdir(v.value)


def link_libs():
    if not os.path.exists(LIBS_DIR):
        os.system(f'tar -xf {LIBS_DIR}.tar.gz')
    with cwd(f'./{LIBS_DIR}/lib'):
        files = [f for f in os.listdir('.') if f.endswith('.so.1.3.1')]
        for file in files:
            name = file.split('.', 2)[0]
            os.system(f'ln -sf ./{file} {name}.so')
            os.system(f'ln -sf ./{file} {name}.so.1')


def build() -> None:
    os.system(
        (
            f'clang -m64 -L{LIBS_DIR}/lib -Wall -Werror '
            f'-o {TargetDir.CLANG}/{LabPrefix.DEFAULT} main.c '
            '-lm -lfwSignal -lfwBase'
        )
    )
    os.system(
        (
            f'clang -m64 -L{LIBS_DIR}/lib -Wall -Werror '
            f'-o {TargetDir.CLANG}/{LabPrefix.ADDITIONAL} '
            f'{LabPrefix.ADDITIONAL}.c -lm -lfwSignal -lfwBase'
        )
    )


def main():
    clear()
    link_libs()
    build()


if __name__ == '__main__':
    main()
