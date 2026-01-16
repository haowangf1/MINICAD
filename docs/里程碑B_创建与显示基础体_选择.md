# 里程碑 B：创建与显示基础体 + 选择（Qt Widgets + OCCT，Windows）

目标（对应 `CAD_OCC_项目大纲.md`）：
- 创建并显示：**Box / Cylinder / Sphere**
- **单选高亮** + 右侧（或下方）**属性面板**：显示“名称/类型”（先做最小可用）

---

## B.0 前置条件
- 里程碑 A 已完成：有 `MainWindow`，中央是 `OccViewportWidget`，已能旋转/平移/缩放
- 已有 OCCT 交互上下文：`AIS_InteractiveContext`（当前在 `OccViewportWidget::initOcc()` 里创建）

---

## B.1 推荐的代码结构（最少改动版本）
为了后续扩展（多物体、属性、删除/隐藏等），建议把“场景操作”封到 `OccViewportWidget`，`MainWindow` 只负责 UI。

- **`MainWindow` 负责**
  - 菜单/工具栏：Create Box / Cylinder / Sphere
  - 属性面板（`QDockWidget` + `QFormLayout` 或 `QTreeWidget`）
  - 接收视口的“选中变化”信号，刷新属性面板

- **`OccViewportWidget` 负责**
  - 提供接口：`addBox()` / `addCylinder()` / `addSphere()`（内部创建 TopoDS_Shape + AIS_Shape 并 Display）
  - 处理选择：鼠标点击时调用 `m_context->Select(...)`，并把选中的对象信息通过 Qt 信号抛给 `MainWindow`

---

## B.2 创建并显示基础体（Box/Cylinder/Sphere）
关键 OCCT API：
- 建模：`BRepPrimAPI_MakeBox` / `BRepPrimAPI_MakeCylinder` / `BRepPrimAPI_MakeSphere`
- 显示：`AIS_Shape`
- 上屏：`m_context->Display(ais, Standard_True)`（或先 false，最后统一 `m_view->Redraw()`）

最小示例（放到 `OccViewportWidget` 的某个接口里）：

```cpp
#include <BRepPrimAPI_MakeBox.hxx>
#include <AIS_Shape.hxx>

void OccViewportWidget::addBox(double dx, double dy, double dz)
{
  TopoDS_Shape shape = BRepPrimAPI_MakeBox(dx, dy, dz).Shape();
  Handle(AIS_Shape) ais = new AIS_Shape(shape);
  m_context->Display(ais, Standard_True);
  m_view->FitAll();
}
```

建议：创建时给每个对象一个“名字”（先用自增编号），放在你自己的映射表里：
- `std::unordered_map<Handle(AIS_InteractiveObject), QString>` 或
- 用 OCCT 的 `TDataStd_Name`（后面引入 OCAF 再升级）

---

## B.3 单选高亮（选择）
### 选择需要什么
OCCT 的选择/高亮在 `AIS_InteractiveContext` 里完成，典型流程：
- 鼠标移动：`MoveTo(x, y, m_view, Standard_True)`（用于预高亮/检测）
- 鼠标点击：`Select(true)` 或框选 `Select(x1,y1,x2,y2, view, true)`
- 获取选中：`m_context->HasSelectedShape()` / `m_context->SelectedInteractive()` / 迭代 `InitSelected()` / `MoreSelected()` / `NextSelected()`

### 和里程碑 A 的“左键旋转”冲突怎么处理（两种做法）
- **做法 1（推荐）**：左键点击=选择；**Alt+左键拖动**=旋转（更像 DCC/CAD）
- **做法 2**：左键拖动=旋转不变；**单击不拖动**=选择（需要你判断“按下到抬起位移是否小于阈值”）

最小可用：先实现“单击选择”，旋转可改为 Alt+左键拖动。

---

## B.4 属性面板（名称/类型）
### UI 建议
在 `MainWindow` 添加一个 `QDockWidget`（右侧）：
- 字段：Name / Type
- 先用 `QLabel` 或 `QLineEdit(只读)` 显示

### 数据从哪来
`OccViewportWidget` 在选择变化时发一个 Qt 信号给 `MainWindow`：
- `selectedChanged(QString name, QString type)`
- 未选中时发空字符串/或发一个 `cleared()` 信号

类型判断（最小版本）：
- 在你创建对象时把 `type` 存进映射表（例如 `"Box"`/`"Cylinder"`/`"Sphere"`）

---

## B.5 验收清单（完成标准）
- 点击菜单/工具栏能创建 Box/Cylinder/Sphere，并出现在视口中
- 单击某个实体会高亮（取消时不高亮）
- 属性面板能显示：**Name + Type**

---

## B.6 常见坑（提前避雷）
- **坐标/单位**：OCCT 默认无单位，自己约定 mm；创建尺寸不要太小（比如 Box 100×80×60）
- **选择没反应**：要确保 `m_view` 已绑定窗口且 `m_context` 存在；点击时要调用 `Select`；移动时可加 `MoveTo` 增强体验
- **Qt 与 OCC 重绘**：对象 Display 后通常需要 `m_view->Redraw()`（或 `Display(..., Standard_True)` 自动触发）


在 OccViewportWidget 增加 addBox()（创建 TopoDS_Shape + AIS_Shape 并显示）
在 MainWindow 增加 Create Box 菜单/工具栏 action 并连接到 addBox()
增加 addCylinder()/addSphere() 并补齐对应 action
实现单击选择/高亮，并从 OccViewportWidget 发出 selectedChanged 信号
在 MainWindow 增加属性面板 Dock，并在选中变化时显示 Name/Type

