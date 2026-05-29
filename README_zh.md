# 🚀 Learning C++ — AI/ML 开发进阶专题

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20">
  <img src="https://img.shields.io/badge/CMake-3.16+-green.svg" alt="CMake">
  <img src="https://img.shields.io/badge/Platform-Linux-lightgrey.svg" alt="Linux">
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="MIT">
</p>

<p align="center">
  <a href="README.md">English</a> | <b>中文</b>
</p>

一个系统性学习 C++ 进阶知识的实战项目，专注于 AI/大模型开发中常用的 C++ 技术：多线程并发、线程池、设计模式、现代 C++ 特性、元编程、性能优化等。

官方参考文档：https://en.cppreference.com

## 📁 项目结构

```
modern-cpp-forge/
├── CMakeLists.txt              # 项目根构建文件
├── .clang-format               # 代码格式化配置
├── .clang-tidy                 # 静态分析配置
├── common/                     # 公共库（日志、计时器）
│   ├── include/common/
│   │   ├── logger.h            # 线程安全彩色日志系统
│   │   └── timer.h             # 高精度计时器
│   └── src/
│       ├── logger.cpp
│       └── timer.cpp
├── reference/                  # 📖 参考实现（标准答案）
│   ├── concurrency/            # 多线程与并发
│   │   ├── thread_basics.cpp       # 线程创建与管理
│   │   ├── mutex_and_lock.cpp      # 互斥锁与各种锁
│   │   ├── condition_variable.cpp  # 条件变量与生产者-消费者
│   │   ├── thread_pool.cpp         # 线程池完整实现
│   │   ├── atomic_operations.cpp   # 原子操作与无锁编程
│   │   └── async_and_future.cpp    # std::async 与 std::future
│   ├── design_patterns/        # 设计模式
│   │   ├── singleton.cpp           # 单例模式（3种实现）
│   │   ├── factory.cpp             # 工厂模式（注册式工厂）
│   │   ├── observer.cpp            # 观察者模式与事件系统
│   │   └── strategy.cpp            # 策略模式
│   ├── memory_management/      # 内存管理
│   │   ├── smart_pointers.cpp      # 智能指针详解
│   │   ├── move_semantics.cpp      # 移动语义与完美转发
│   │   └── raii.cpp                # RAII 资源管理
│   ├── modern_cpp/             # 现代C++特性
│   │   ├── templates_and_concepts.cpp  # 模板与C++20 Concepts
│   │   ├── lambda_and_functional.cpp   # Lambda与函数式编程
│   │   └── coroutines.cpp             # C++20 协程
│   ├── metaprogramming/        # 元编程
│   │   └── type_traits_and_sfinae.cpp  # 类型萃取与SFINAE
│   └── performance/            # 性能优化
│       └── cache_friendly.cpp      # 缓存友好编程
├── src/                        # ✏️ 你的练习代码（在这里实现）
│   ├── concurrency/
│   ├── design_patterns/
│   ├── memory_management/
│   ├── modern_cpp/
│   ├── metaprogramming/
│   └── performance/
├── tests/                      # 单元测试
├── benchmarks/                 # 性能基准测试
├── docs/                       # 文档
│   └── learning_roadmap.md     # 学习路线图
├── scripts/                    # 工具脚本
│   ├── build.sh                # 构建脚本
│   ├── clean.sh                # 清理脚本
│   └── run.sh                  # 运行脚本
└── .vscode/                    # VS Code 配置
    ├── launch.json             # F5 调试配置
    ├── tasks.json              # 构建任务
    ├── settings.json           # 编辑器设置
    └── extensions.json         # 推荐扩展
```

## 🛠️ 环境要求

- **编译器**：GCC 10+ 或 Clang 12+（需支持 C++20）
- **构建工具**：CMake 3.16+
- **调试器**：GDB
- **操作系统**：Linux（Ubuntu 20.04+）

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/zhanmyz/modern-cpp-forge.git
cd modern-cpp-forge
```

### 2. 构建

```bash
# 方式1：使用脚本（推荐）
chmod +x scripts/*.sh
./scripts/build.sh debug      # Debug 模式（含调试信息和 Sanitizer）
./scripts/build.sh release    # Release 模式（优化性能）

# 方式2：手动 CMake
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel $(nproc)
```

### 3. 运行示例

```bash
# 运行参考代码
./bin/reference/concurrency/thread_basics
./bin/reference/design_patterns/singleton
./bin/reference/performance/cache_friendly

# 运行测试
./bin/tests/test_logger

# 运行基准测试
./bin/benchmarks/bench_containers

# 使用运行脚本
./scripts/run.sh thread_pool
```

### 4. VS Code 调试

1. 打开任意 `.cpp` 文件
2. 按 `F5` 自动编译并调试当前文件
3. 断点、变量查看、调用栈等功能开箱即用

## 📚 学习方法

### 推荐流程

1. **阅读** `reference/` 下的参考代码（含详细英文注释）
2. **理解** 每个示例的核心概念和 AI 项目中的应用
3. **动手** 在 `src/` 对应目录下创建自己的实现文件
4. **对比** 你的实现和参考实现的差异
5. **实验** 修改参数、尝试边界条件、观察行为

### 学习路线图

详见 [docs/learning_roadmap.md](docs/learning_roadmap.md)

建议顺序：**内存管理 → 多线程 → 设计模式 → 现代C++ → 元编程 → 性能优化**

## 🏗️ 项目特性

| 特性         | 说明                                       |
| ------------ | ------------------------------------------ |
| 🎨 彩色日志   | 带时间戳、线程ID、文件行号的彩色终端输出   |
| ⏱️ 计时器     | RAII 自动计时，方便性能测量                |
| 🔧 CMake 构建 | 现代 CMake，每个源文件独立编译为可执行文件 |
| 🐛 调试支持   | AddressSanitizer + UBSanitizer + GDB       |
| 📏 代码规范   | clang-format + clang-tidy 配置             |
| 📊 基准测试   | 内置性能对比框架                           |
| 🧪 单元测试   | 测试框架就绪                               |
| 📖 详细注释   | 每个概念用生活化类比解释                   |

## 🔧 常用命令

```bash
# 构建
./scripts/build.sh debug

# 清理
./scripts/clean.sh

# 格式化代码
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# 静态分析
clang-tidy src/**/*.cpp -- -std=c++20 -I common/include

# 运行所有测试
find build/bin/tests -type f -executable -exec {} \;
```

## 📝 添加新的学习主题

1. 在 `reference/<category>/` 下创建参考实现
2. 在 `src/<category>/` 下创建练习模板
3. 重新构建：`./scripts/build.sh debug`
4. 新文件会自动被 CMake 发现并编译

## License

MIT
