## 1. Project Introduction

Welcome to the LLVM project in the openEuler community! This warehouse is the downstream warehouse of [llvm-project](https://github.com/llvm/llvm-project).

This repository contains the source code for LLVM, a toolkit for the construction of highly optimized compilers, optimizers, and run-time environments.

The LLVM project has multiple components. The core of the project is itself called "LLVM". This contains all of the tools, libraries, and header files needed to process intermediate representations and convert them into object files. Tools include an assembler, disassembler, bitcode analyzer, and bitcode optimizer.

C-like languages use the [Clang](https://clang.llvm.org/) frontend. This component compiles C, C++, Objective-C, and Objective-C++ code into LLVM bitcode -- and from there into object files, using LLVM.

Other components include: the [libc++ C++ standard library](https://libcxx.llvm.org/), the [LLD linker](https://lld.llvm.org/), and more.

## 2. Construction Guide

You can use `git` to download the source code, and then use the `build.sh` script to build the LLVM in one-click mode. There are two build modes: `build with command line` and `build with container`.

### 2.1. Build with command line directly

You are advised to use the `openEuler` for building. If you use other operating systems, you are advised to use the containerized building mode.

Ensure that the dependency software packages are installed. You can run the following command to install the software packages:

` ` `
yum install -y gcc g++ make cmake openssl-devel python3 \
python3-setuptools python-wheel texinfo binutils-devel libatomic
` ` `

You can run the `./build.sh -h` command to view the build options supported by the current project. Run the following command to perform a one-click build:

` ` `
./build.sh -r -b release -X X86 -j 8
` ` `

### 2.2 Build with container

The openEuler LLVM project provides a containerized building mode to solve the problems of build failures and binary differences of build products caused by development environment differences. Thanks to the [openEuler container image project](https://gitee.com/openeuler/openeuler-docker-images), the [llvm-build-deps container image](https://gitee.com/openeuler/openeuler-docker-images/tree/master/llvm-build-deps) is created in advance. Developers can enable containerized builds using the `-C` option of the `build.sh` script. For example:

` ` `
./build.sh -C -r -b release -X X86 -j 8 // added -C option
` ` `

Dependency:
* The Docker application must be correctly installed in the development environment.
* The user is added to the docker user group so that the `sudo` command is not required when the `build.sh` script executes the docker command. You can run the following command to add the current user to the docker user group:

` ` `
sudo usermod -aG docker ${USER}
` ` `

Note: When you perform a containerized build for the first time, the script automatically pulls the `llvm-build-deps container image` from the image repository.

## 3. Contribution guidance

1. Fork This Warehouse
2. Create the Feat_xxx branch.
3. Submit the code.
4. Create a Pull Request.

## 4. Discussion and help-seeking

### 4.1 Upstream Community
* Join the [Discourse Forum](https://discourse.llvm.org/) .

* [Code of Conduct](https://llvm.org/docs/CodeOfConduct.html) for Community Participants.

### 4.1. Compiler SIG of the openEuler community
There are several ways:
* Subscribe to the [Compiler SIG mailing list](https://mailweb.openeuler.org/postorius/lists/compiler@openeuler.org/)
* Post a discussion at the [openEuler forum](https://forum.openeuler.org/?locale=zh_CN).
* WeChat communication group: Please add the WeChat name `Compiler_Assistant` first.