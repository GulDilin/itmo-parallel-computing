## Build

1. Install OpenCL
You can choose each variant

- OpenCL-SDK
https://github.com/KhronosGroup/OpenCL-SDK

- CUDA
https://developer.nvidia.com/cuda-toolkit

- AMD ROCm (only linux)
https://rocmdocs.amd.com/en/latest/deploy/linux/install_overview.html

2. Build with installation path

CUDA
```
python build.py -m CUDA --cuda-path "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1"
```

AMD + OpenCL SDK
```
python build.py -m AMD --cuda-path "path\to\OpenCL-SDK"
```
