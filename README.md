# 民航订票系统

一个基于 Linux 环境、使用 C 语言开发的命令行民航订票系统。

## 📖 项目简介

本项目是一个控制台程序，模拟民航订票的核心业务流程。用户可以查询航班、预订机票、退订机票，并查看自己的订单信息。项目采用 C 语言编写，通过 CMake 构建，适合作为 C 语言课程设计或练手项目。

## ✨ 功能特性

- [x] 航班信息查询（按出发地/目的地/时间）
- [x] 用户注册与登录
- [x] 机票预订
- [x] 机票退订
- [x] 订单管理
- [x] 管理员后台
- [x] 数据持久化到文件/数据库

## 🛠️ 技术栈

- **语言**：C (C99)
- **IDE**：CLion
- **构建工具**：CMake
- **运行环境**：Linux / WSL / MinGW (Windows)
- **编译器**：GCC

## 📂 项目结构

```text
民航订票系统/
├── src/                # 源代码目录
│   ├── main.c          # 程序入口
│   ├── introduce.c     # 系统介绍/欢迎界面
│   ├── message.c       # 消息提示模块
│   └── order_system.c  # 订单/订票核心逻辑
├── include/            # 头文件目录
│   ├── introduce.h
│   ├── message.h
│   └── order_system.h
├── docs/               # 文档与截图
├── CMakeLists.txt      # CMake 构建脚本
├── .gitignore          # Git 忽略配置
└── README.md           # 本文件
