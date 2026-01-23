里程碑 C：创建/删除/变换命令化 + Undo/Redo（Qt Widgets + OCCT，Windows）
目标（对应 CAD_OCC_项目大纲.md）：
把“创建/删除/变换”全部命令化
支持稳定的 Undo / Redo（可演示）
为后续功能（导入 STEP、布尔、编辑）留下可扩展结构
C.0 前置条件
里程碑 B 已完成：
MainWindow + OccViewportWidget
视口可旋转/平移/缩放
可创建基础体并显示（至少 Box/Sphere，建议补齐 Cylinder）
单击选择/高亮
右侧属性面板显示 Name/Type（可选中更新）
C.1 推荐的代码结构（最少改动版本）
目标：把“UI”和“业务逻辑/可撤销操作”解耦。
建议新增目录：
src/model/：文档/场景数据模型（Document / SceneModel）
src/commands/：命令接口 + 具体命令 + 命令管理器（Undo/Redo 栈）
src/app/：MainWindow/UI 只负责 QAction、Dock、快捷键
职责划分：
MainWindow 负责
菜单/工具栏：Create / Delete / Transform / Undo / Redo
绑定快捷键：Undo(Ctrl+Z)、Redo(Ctrl+Y 或 Ctrl+Shift+Z)、Delete(Del)
接收“选中变化”信号，刷新属性面板
调用 CommandManager::doCommand(...) 触发命令
OccViewportWidget 负责
依然负责渲染与交互（显示、选择、相机操作）
提供“应用渲染变更”的接口：显示/移除某个对象、更新对象变换、刷新视图
Document/SceneModel 负责
维护场景对象的“真数据源”（而不是 AIS 句柄）
给每个对象分配稳定的 id、name、type，并保存几何与变换
Command 负责
把一次用户操作封装成可撤销单元：execute/undo/redo
C.2 数据模型（Document / SceneModel）怎么设计（最小可用）
定义 SceneObject（示例字段）：
uint64_t id：唯一标识（稳定，不随 redo 改变）
QString name：如 Box1、Sphere2
QString type：如 "Box"/"Cylinder"/"Sphere"
TopoDS_Shape shape：几何（可直接存，也可存参数并懒生成）
gp_Trsf trsf：局部变换（移动/旋转/缩放）
（可选）Handle(AIS_InteractiveObject) ais：渲染句柄（不做持久源，可随时重建）
Document 提供 API（示例）：
uint64_t addObject(SceneObject obj) / removeObject(id)
setTransform(id, gp_Trsf)
optional<SceneObject> getObject(id)
setSelection(vector<id>) / selectedIds()
关键原则：命令只改 Document；渲染由 Document/Viewport 接口同步更新。
C.3 命令系统（Command Pattern）怎么实现
C.3.1 命令接口（ICommand）
最小接口：
QString title() const（可选：用于 Undo 菜单显示）
bool execute()：首次执行
void undo()：撤销
void redo()：重做（一般可调用 execute 的一部分，但要保证 id/name 稳定）
C.3.2 命令管理器（CommandManager）
维护两个栈：
undoStack
redoStack
规则（必须严格遵守）：
doCommand(cmd)：
cmd.execute() 成功 → push 到 undoStack
清空 redoStack（因为历史分叉）
undo()：
pop undoStack → cmd.undo() → push redoStack
redo()：
pop redoStack → cmd.redo() → push undoStack
对 UI 暴露：
canUndo()/canRedo()
undoTitle()/redoTitle()（可选）
C.4 三类核心命令如何实现（必须命令化的三大类）
C.4.1 创建命令：CreatePrimitiveCommand（Box/Cylinder/Sphere）
输入：
primitive 类型 + 参数（Box: dx/dy/dz；Cylinder: r/h；Sphere: r）
初始变换（可选）
命名策略（Box1…）
execute()：
生成几何 TopoDS_Shape（或存参数）
Document 新增对象（分配 id/name/type）
通知视口显示该对象（创建 AIS、Display、必要时 Mesh、默认 Shaded）
（可选）将其设为当前选中
undo()：
从视口移除该对象（Remove/Erase）
Document 删除该对象
清理选择（如果需要）
redo()：
用同一个 id/name/type 再次创建并显示
（建议命令内部保存：id、name、type、shape/参数、trsf）
> 建议：Create 命令必须保证 redo 后对象 id 不变，这样后续引用（属性面板、后续命令）不混乱。
C.4.2 删除命令：DeleteSelectionCommand（删除选中）
输入：
执行时从 Document/Selection 读取选中对象 id（单选/多选都可）
execute()：
将被删对象完整备份到命令内部（vector<SceneObject>）
逐个从视口移除 + 从 Document 删除
清空选择（或保存并更新）
undo()：
将备份对象逐个恢复到 Document
重新创建 AIS 并显示到视口
（可选）恢复删除前的选择
redo()：
再删除同一批 id（或直接复用备份列表）
C.4.3 变换命令：TransformCommand（移动/旋转/缩放）
第一版建议：先做“平移移动”命令（UI 用固定步长按钮），再扩展旋转/缩放。
输入：
vector<uint64_t> targetIds
vector<gp_Trsf> before
vector<gp_Trsf> after
execute()/redo()：
Document 写入 after
视口更新对象变换（对 AIS 设置 local transformation 或 context SetLocation）
Redraw
undo()：
Document 写回 before
视口同步回 before
Redraw
C.5 选择与命令怎么对接（很关键）
命令需要稳定 id；选择来自 AIS_InteractiveContext。
最小可用做法：
在创建 AIS 时，把 AIS_InteractiveObject* → objectId 存到你自己的映射表
点击选择后拿到 SelectedInteractive()（或遍历 Selected），用映射表查出 objectId
Document 记录当前选中 id
Delete/Transform 命令从 Document 读取选中 id
后续可升级做法：
用 OCCT 的 Owner / OCAF（TDF_Label）作为更正规绑定（里程碑后再做）
C.6 UI（MainWindow）需要加哪些东西
新增 Edit 菜单：
Undo（Ctrl+Z）→ CommandManager::undo()
Redo（Ctrl+Y / Ctrl+Shift+Z）→ CommandManager::redo()
Delete（Del）→ doCommand(DeleteSelectionCommand(...))
Create 菜单保持：
Box / Cylinder / Sphere → doCommand(CreatePrimitiveCommand(...))
Transform（可选，第一版先用按钮/菜单测试）：
Move +X / -X / +Y / -Y / +Z / -Z（固定步长，比如 10）
→ doCommand(TransformCommand(selectedIds, before, after))
并在每次命令执行/撤销/重做后：
更新 QAction enabled 状态（canUndo/canRedo）
属性面板刷新（选中对象可能变化/消失/恢复）
C.7 稳定性注意点（确保 Undo/Redo 不崩）
永远以 Document 为真：AIS 句柄只是显示缓存，可销毁重建
redo 不产生新 id：Create/Undo/Redo 反复操作后对象 identity 要稳定
任何新命令都会清空 redo 栈
Delete 命令必须完整备份对象数据（至少 shape + trsf + name/type/id）
变换命令必须保存 before/after（不要只存 delta，避免累计误差/顺序问题）
C.8 建议实现顺序（按最小可用推进）
先实现 CommandManager + ICommand + Undo/Redo QAction（空栈时禁用）
把 Create Box/Sphere/Cylinder 改为 CreatePrimitiveCommand
加 DeleteSelectionCommand（Del）
加 TransformCommand（先做平移）
最后统一整理“选中 id”的存取与 UI 刷新逻辑