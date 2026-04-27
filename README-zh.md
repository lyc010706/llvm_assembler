## 1、项目介绍

欢迎来到openEuler社区的LLVM项目！本仓库是[llvm-project](https://github.com/llvm/llvm-project)下游仓库。

此仓库包含LLVM项目的源代码，LLVM基础设施是一个用于构建高度优化的编译器、优化器和运行时环境的工具包。

LLVM项目有多个组件。该项目的核心组件被称为“LLVM”，它包含处理中间表示及将其转换为目标文件所需的所有工具（包括汇编器、反汇编器、位码分析器和为位码优化器）、库和头文件。

类C语言使用Clang前端，该组件将C、C++、Objective-C和Objective-C++代码编译成LLVM位码，并使用LLVM将其转换为二进制目标文件。

其他组件包括：C++标准库（libc++）、LLD链接器等。

## 2、构建指导

首先通过git下载源码，然后通过build.sh脚本一键式构建LLVM。构建方式有`直接命令行构建`和`容器化构建`两种。

### 2.1、直接命令行构建

推荐使用openEuler操作系统进行构建，如果您使用其他操作系统，建议使用容器化构建方式。

首先确保系统安装了依赖软件包，可以用如下命令安装。
```
yum install -y gcc g++ make cmake openssl-devel python3 \
python3-setuptools python-wheel texinfo binutils-devel libatomic
```
然后可以通过 `./build.sh -h`查看当前工程支持的构建选项。通过命令行执行一键式构建，例如：
```
./build.sh -r -b release -X X86 -j 8
```

### 2.2、容器化构建

为了解决由于开发环境差异导致的构建失败和构建产物二进制差异问题，openEuler LLVM项目提供了容器化构建方法。得益于[openEuler容器镜像项目](https://gitee.com/openeuler/openeuler-docker-images)，提前制作了[llvm-build-deps容器镜像](https://gitee.com/openeuler/openeuler-docker-images/tree/master/llvm-build-deps)。开发者可以通过`build.sh`脚本的`-C`选项启用容器化构建，例如：
```
./build.sh -C -r -b release -X X86 -j 8   // 添加了-C选项
```

相关依赖：
* 开发环境需要正确安装了docker应用。
* 用户加入了docker用户组，使得`build.sh`脚本执行docker命令时不需要再加`sudo`命令。可以通过如下命令将当前用户加入docker用户组。
```
sudo usermod -aG docker ${USER}
```
注意：第一次执行容器化构建时，脚本会自动从镜像仓库拉取llvm-build-deps容器镜像。

## 3、贡献指导

1、Fork 本仓库  
2、新建 Feat_xxx 分支  
3、提交代码    
4、新建 Pull Request   

## 4、讨论与求助

### 4.1、上游社区
* 加入[discourse论坛](https://discourse.llvm.org/)，提出或参与问题、RFC等交流。

* 社区参与者的[行为规范](https://llvm.org/docs/CodeOfConduct.html).

### 4.1、openEuler社区Compiler SIG
几种方式：
* 订阅[Compiler SIG邮件列表](https://mailweb.openeuler.org/postorius/lists/compiler@openeuler.org/)
* 在[openEuler论坛](https://forum.openeuler.org/?locale=zh_CN)发帖讨论。
* 微信交流群：请先添加微信名`Compiler_Assistant`
