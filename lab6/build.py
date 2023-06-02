import os
import shutil
from enum import Enum


class TargetDir(str, Enum):
    # GCC = 'target-gcc'
    CLANG = 'target-clang'
    # TCC = 'target-tcc'


class LabPrefix(str, Enum):
    DEFAULT = 'lab-6'
    # NO_PARALLEL = 'lab-4-no-parallel'
    # ADDITIONAL = 'lab-4-add'


def clear():
    for v in TargetDir:
        try:
            shutil.rmtree(f'./{v.value}')
        except FileNotFoundError:
            pass
        os.mkdir(v.value)


def build() -> None:
    for target in TargetDir:
        # os.system(
        #     (
        #         f'LD_LIBRARY_PATH=/opt/rocm-5.5.0/lib:/opt/rocm-5.5.0/lib64 PATH=$PATH:/opt/rocm-5.5.0/bin:/opt/rocm-5.5.0/opencl/bin {target.name.lower()} main.c '
        #         f'-o {target.value}/{LabPrefix.DEFAULT}.o -I$ROCMOPENCL/include'
        #     )
        # )
        # os.system(
        #     (
        #         'LD_LIBRARY_PATH=/opt/rocm-5.5.0/lib PATH=$PATH:/opt/rocm-5.5.0/bin:/opt/rocm-5.5.0/opencl/lib '
        #         f'{target.name.lower()} -L/opt/rocm-5.5.0/lib main.c '
        #         f'-o {target.value}/{LabPrefix.DEFAULT} -lOpenCL'
        #     )
        # )
                # 'LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH '
        CUDA_PATH = r'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1'
        os.system(
            (
                rf'{target.name.lower()} '
                rf' --cuda-path="{CUDA_PATH}" '
                rf'-L"{CUDA_PATH}\lib\x64" '
                rf'-I"{CUDA_PATH}\include" '
                'main.c '
                f'-o {target.value}/{LabPrefix.DEFAULT}.exe '
                '-lOpenCL'
            )
        )


def main():
    clear()
    build()


if __name__ == '__main__':
    main()