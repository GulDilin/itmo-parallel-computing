import os
import shutil
from enum import Enum
import argparse
import platform

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


def build(args) -> None:
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
        print(platform.system())
        suffix = '.exe' if platform.system() == 'Windows' else ''
        if (args.mode == 'CUDA'):
            os.system(
                (
                    rf'{target.name.lower()} '
                    rf' --cuda-path="{args.cuda_path}" '
                    rf'-L"{args.cuda_path}\lib\x64" '
                    rf'-I"{args.cuda_path}\include" '
                    'main.c '
                    f'-o {target.value}/{LabPrefix.DEFAULT}{suffix} '
                    '-lOpenCL'
                )
            )
        else:
            os.system(
                (
                    rf'{target.name.lower()} '
                    rf'-L"{args.ocl_path}\lib" '
                    rf'-I"{args.ocl_path}\include" '
                    'main.c '
                    f'-o {target.value}/{LabPrefix.DEFAULT}{suffix} '
                    '-lOpenCL'
                )
            )


def main():
    CUDA_PATH = r'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1'
    OPENCL_SDK_PATH = r'D:\tool\OpenCL'
    parser = argparse.ArgumentParser()
    parser.add_argument('--mode', '-m', dest="mode", type=str, choices=('CUDA', 'AMD'), required=True)
    parser.add_argument('--cuda-path', dest="cuda_path", type=str, default=CUDA_PATH)
    parser.add_argument('--ocl-path', dest="ocl_path", type=str, default=OPENCL_SDK_PATH)
    args = parser.parse_args()

    clear()
    build(args)


if __name__ == '__main__':
    main()
