# 基于 OpenCascade（OCC）做一个简单 CAD 的面试项目大纲（可落地）

> 目标：做一个**能演示、能讲清架构、能扩展**的“MiniCAD”。重点不是功能堆满，而是：**清晰的数据模型 + 命令系统 + 交互/显示链路 + 可扩展建模能力**。

---

## 1. 面试定位与最小可用范围（MVP）

### 1.1 项目一句话
- **MiniCAD**：基于 OCC 的桌面 CAD 原型，支持基本几何创建、选择/变换、布尔运算、STEP 导入导出、撤销重做与简单标注。

### 1.2 MVP（建议 1～2 周内能做出来并可演示）
- **视图与导航**
  - 3D 视图（旋转/平移/缩放，FitAll）
  - 坐标轴/网格（可选）
- **创建基础体**
  - 盒体 Box、圆柱 Cylinder、球体 Sphere（参数化输入）
- **选择与高亮**
  - 单选/框选（至少单选）
  - 高亮（Hover/Selected）与属性面板显示（名称、类型、体积/面积可选）
- **变换**
  - 平移/旋转/缩放（先做平移即可）
- **布尔运算**
  - Union / Cut / Common（至少 Cut 或 Union 之一）
- **文件能力**
  - STEP 导入（读）
  - STEP 导出（写）
- **撤销重做**
  - 以“命令模式”实现 Undo/Redo（至少覆盖创建/删除/变换/布尔）

> 面试时的关键点：你能把“**UI 交互 → 选择/命令 → 数据模型 → OCC 建模 → 显示更新**”整条链路讲清楚，并且有 Undo/Redo 的工程化设计。

---

## 2. 技术栈建议

### 2.1 推荐组合（最常见、资料多）
- **语言**：C++
- **GUI**：Qt 6（或 Qt 5）
- **内核**：OpenCascade（OCCT）
- **构建**：CMake

### 2.2 架构
- OCC 本质是：**几何/拓扑内核 + 显示 AIS/V3d 框架 + 交换格式（STEP/IGES）**
- “模型（TopoDS_Shape）”与“显示（AIS_Shape）”分离：这是你架构设计的基础。

---

## 3. 总体架构

### 3.1 模块划分（推荐）
- **App/UI 层**
  - 主窗口、工具栏、属性面板、命令面板（参数输入）
- **Viewport/渲染层（OCC Visualization）**
  - `V3d_Viewer` / `V3d_View`
  - `AIS_InteractiveContext`（选择、高亮、显示对象管理）
- **Document/数据模型层（核心）**
  - 文档 `Document`：保存“设计树/对象列表/参数/关系”
  - 对象 `CadObject`：id、name、type、参数、`TopoDS_Shape`
  - 显示对象缓存：`Handle(AIS_Shape)`（与数据对象关联）
- **命令系统（Undo/Redo）**
  - `ICommand`：`Do()` / `Undo()` / `Redo()`（或 `Do()` + `Undo()`）
  - `CommandManager`：命令栈
- **建模服务层（封装 OCC 算法）**
  - `ModelingService`：创建体、布尔、倒角/圆角（后续扩展）
- **IO 层**
  - STEP 导入导出（与文档对象互转）
- **选取/交互层**
  - 鼠标事件 → Context 选择 → 映射回 `CadObject`
  - 交互模式：Select / Transform / Create（状态机或工具系统）

### 3.2 关键数据流（必须能解释）
- **创建**：UI 参数 → Command → ModelingService 生成 `TopoDS_Shape` → Document 添加对象 → 生成 `AIS_Shape` → `AIS_InteractiveContext::Display`
- **选择**：鼠标点击 → `AIS_InteractiveContext` 选中 `AIS_InteractiveObject` → 通过绑定表找到 `CadObjectId` → 属性面板刷新
- **更新**：对象 shape 变了 → 更新/替换 AIS → `Context::Redisplay`
- **撤销**：Command Undo → Document 回滚 + Context 同步（Display/Remove/Redisplay）

---

## 4. OCC 关键能力点

### 4.1 你会用到的核心概念
- **拓扑（TopoDS_...）**：`TopoDS_Shape` / `TopoDS_Face` / `TopoDS_Edge` …
- **建模算法（BRepAlgoAPI / BRepPrimAPI）**
  - `BRepPrimAPI_MakeBox / MakeCylinder / MakeSphere`
  - `BRepAlgoAPI_Cut / Fuse / Common`
- **几何计算**
  - 体积/面积：`GProp_GProps` + `BRepGProp::VolumeProperties / SurfaceProperties`
- **显示与交互**
  - `AIS_Shape`：把 `TopoDS_Shape` 包装成可交互显示对象
  - `AIS_InteractiveContext`：选择/高亮/管理显示对象
  - `V3d_View`：相机控制、FitAll
- **数据结构/命名**
  - 后续可以引入 OCAF（TDocStd / TDF / TNaming），但 MVP 不必一开始上。

---

## 5. 工程结构（目录级）

```
minicad/
  CMakeLists.txt
  external/                 # 可选：子模块或第三方脚本
  src/
    app/
      MainWindow.*
      AppController.*       # 连接 UI 与命令/文档
    viewport/
      OccViewportWidget.*   # Qt Widget + OCC View/Context 封装
      OccViewController.*   # 视图导航/拾取封装
    document/
      Document.*
      CadObject.*
      ObjectRegistry.*      # id -> object，object -> AIS 映射
    commands/
      ICommand.*
      CommandManager.*
      CreatePrimitiveCommand.*
      TransformCommand.*
      BooleanCommand.*
      DeleteCommand.*
    modeling/
      ModelingService.*
    io/
      StepImporter.*
      StepExporter.*
    utils/
      IdGenerator.*
      Logger.*
  assets/
  docs/
    design.md               # 设计文档
```

---

## 6. 里程碑

### 里程碑 A（第 1～2 天）：跑通 OCC + Qt 视口
- Qt 窗口里显示空 3D 视图
- 旋转/平移/缩放，FitAll

### 里程碑 B（第 3～5 天）：创建与显示基础体 + 选择
- Box/Cylinder/Sphere 创建并显示
- 单选高亮 + 属性面板（显示名称/类型）

### 里程碑 C（第 6～8 天）：命令系统 + Undo/Redo
- 把“创建/删除/变换”全部命令化
- Undo/Redo 能稳定工作（演示关键）

### 里程碑 D（第 9～12 天）：布尔运算 + STEP 导入导出
- Union/Cut/Common 至少 1～2 个
- STEP 读写打通（导入后变成文档对象或作为单对象）

### 里程碑 E（第 13～14 天）：打磨与可展示性
- 工具栏/快捷键
- 状态栏提示（当前模式、选择数量）
- 保存/打开工程（可选：先用 JSON 存参数与引用 STEP）
---

## 8. 重点

- **问：怎么组织数据模型？为什么不用直接操作 AIS？**  
  - AIS 是显示对象，不适合作为业务数据源；我用 Document 管 `TopoDS_Shape` 与参数，AIS 只是视图缓存，便于 Undo/Redo 与 IO。

- **问：Undo/Redo 怎么做？**  
  - 命令模式，每个命令记录“变更前后”的对象快照或 delta（如 transform 矩阵、shape 替换前后的引用），Undo 负责反向应用并刷新 Context。

- **问：布尔失败/结果异常怎么办？**  
  - 先检查输入形体有效性（`BRepCheck_Analyzer`），必要时修复（ShapeFix_*），布尔后再验证；UI 侧给出错误信息并保留原对象。

- **问：STEP 导入怎么映射到你的对象？**  
  -导入得到一个或多个 `TopoDS_Shape`，为每个 shape 创建 `CadObject`（保留来源文件/层级名），并生成对应 AIS 显示对象。

---

## 9. 可选

- **测量工具**：点到点距离/边长/角度
- **基本草图（2D）+ 拉伸**：Sketch（平面上画线/圆）→ Extrude
- **倒角/圆角**：`BRepFilletAPI_MakeFillet` / `BRepFilletAPI_MakeChamfer`
- **简单约束（轻量）**：只做水平/垂直/相等等少量约束
- **对象树（Design Tree）**：显示操作历史/特征树

---

