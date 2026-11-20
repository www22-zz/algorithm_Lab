# 整数向量 BFS（integer_vector）

本目录包含可独立构建的 Ascend NPU 整数向量 BFS，来自 `bfs_cannlab_8192` 版本。

## 文件

- `bfs_benchmark_integer_vector.asc`：CPU 对照、NPU kernel 和主程序。
- `benchmark_graph.hpp`：节点规模与固定种子的图生成。
- `benchmark_parallel.hpp`：CPU 线程分工、计时和正确性核对。
- `CMakeLists.txt`：仅构建整数向量目标，无需其他方法源码。
- `run_integer_vector.sh`：编译、重复运行、核对成功标记并保存日志。

两个 `.hpp` 是项目自定义头文件，必须与 `.asc` 一起保留。

## 环境与运行

目标环境为 Ascend 910B3 / CANN 9.0（含 ASC 编译器和 CMake 包）、CMake >= 3.16。
编译参数沿用原版 `--npu-arch=dav-2201`。本机无 CANN/NPU，本次独立构建配置仍需在目标机器验证。

```bash
git clone https://github.com/www22-zz/algorithm_Lab.git
cd algorithm_Lab/integer_vector
source /home/developer/Ascend/cann-9.0.0/set_env.sh
bash run_integer_vector.sh
```

`set_env.sh` 请按实际安装路径调整。默认 CPU 多线程为 16 线程，完整运行 5 轮。
例如，显式指定 16 线程、只运行 1 轮：

```bash
bash run_integer_vector.sh 16 1
```

只构建后单独执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target bfs_benchmark_integer_vector -j2
BFS_CPU_THREADS=16 ./build/bfs_benchmark_integer_vector
```

脚本生成 `results_integer_vector_时间戳_PID/`，包含每轮日志、线程/轮数配置和文件校验值。
任一程序执行或正确性检查失败即停止。

## 算法与测量

- N=64、128、256、512、1024、2048、4096、8192；图为环加随机无向边，额外加边概率 10%。
- CPU 与 NPU 都逐层计算完整的整数 `frontier × A`，再过滤已访问节点、更新距离与下一前沿。
- NPU 每 block 负责 32 个目标，源节点按 64 个一批搬运，用广播、向量乘法和树形加法归约实现。
- 每个规模 CPU 单线程/多线程分别预热，交替测量 10 次取均值；NPU 预热一次，测量 5 次取均值。
- CPU 多线程保留原来的每层 `std::async` 实现，计入任务启动与等待开销；没有改为固定线程池。
- `NPU-SubmitSync` 是 Host 提交到同步的累计耗时，不是纯设备计算时间。
- `NPU-E2E` 包含图/状态上传、逐层提交同步、前沿回传判空、最终距离回传；不含图生成和基础设备内存分配。
- 每次 NPU 运行都与 CPU 单线程距离结果核对，全部通过后打印 `All BFS tests completed. All correctness checks passed.`。
