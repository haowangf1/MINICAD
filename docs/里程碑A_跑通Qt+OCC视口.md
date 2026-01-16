# 里程碑 A：跑通 Qt + OCCT 视口（VSCode + CMake，Windows）

这一阶段目标：**能编译运行**，出现一个 Qt 主窗口，中央是 OCC 3D 视口；支持：
- 鼠标左键拖动：旋转
- 鼠标中键拖动：平移
- 滚轮：缩放
- 按 `F`：FitAll
---

## A.1 准备依赖

- Visual Studio 2022（C++ 桌面开发）
- VSCode + CMake Tools 插件
- Qt（Qt6 优先；也兼容 Qt5）
- OCCT（OpenCascade，建议先用预编译包）

---

## A.1.1 Qt 安装（Windows，推荐 Qt6 + MSVC 2022 x64）

### 安装步骤

1) ** VS2022 的 C++ 工具链**  
   - 打开 “Visual Studio Installer”  
   - 确保安装了 workload：`Desktop development with C++`

2) **下载并运行 Qt 官方安装器（Qt Online Installer / Maintenance Tool）**  
   - 下载地址：[Qt Online Installer（OSS）](https://www.qt.io/development/download-qt-installer-oss)
   - 去 Qt 官网下载对应的 Windows 安装器（通常叫 “Qt Online Installer”）  
   - 登录/注册 Qt 账号后继续安装




