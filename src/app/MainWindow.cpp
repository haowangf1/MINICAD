#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QKeySequence>

#include <memory>

#include "commands/CommandManager.h"
#include "commands/CreatePrimitiveCommand.h"
#include "commands/DelCommand.h"
#include "commands/HideCommand.h"
#include "commands/ShowAllCommand.h"
#include "commands/ImportStepCommand.h"
#include "commands/BoolBrepCommenCommand.h"
#include "model/Document.h"
#include "viewport/OccViewportWidget.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setWindowTitle("MiniCAD");

  m_doc = new Document(this);
  m_cmdMgr = new CommandManager(this);

  m_viewport = new OccViewportWidget(this);
  m_viewport->setDocument(m_doc);
  setCentralWidget(m_viewport);

  // File menu
  auto* fileMenu = menuBar()->addMenu(tr("File"));
  auto* importStepAction = new QAction(tr("Import STEP..."), this);
  connect(importStepAction, &QAction::triggered, this, &MainWindow::onImportStep);
  //给菜单添加一个功能和命令(QAction)
  fileMenu->addAction(importStepAction);

  // Edit menu (Undo/Redo)
  auto* editMenu = menuBar()->addMenu(tr("Edit"));
  m_undoAction = new QAction(tr("Undo"), this);
  m_undoAction->setShortcut(QKeySequence::Undo); // Ctrl+Z
  connect(m_undoAction, &QAction::triggered, m_cmdMgr, &CommandManager::undo);
  editMenu->addAction(m_undoAction);

  m_redoAction = new QAction(tr("Redo"), this);
  m_redoAction->setShortcut(QKeySequence::Redo); // Ctrl+Y / Ctrl+Shift+Z
  connect(m_redoAction, &QAction::triggered, m_cmdMgr, &CommandManager::redo);
  editMenu->addAction(m_redoAction);

  m_DelAction = new QAction(tr("Delete"), this);
  m_DelAction->setShortcut(QKeySequence::Delete);
  connect(m_DelAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
  editMenu->addAction(m_DelAction);
  
  m_BoolBrepCommonAction = new QAction(tr("BoolBrepCommonAction"), this);
  connect(m_BoolBrepCommonAction, &QAction::triggered, this, &MainWindow::onBoolBrepCommon);
  editMenu->addAction(m_BoolBrepCommonAction);

  connect(m_cmdMgr, &CommandManager::stackChanged, this, &MainWindow::updateUndoRedoActions);
  updateUndoRedoActions();

  // Right-side property panel (Name / Type)
  auto* dock = new QDockWidget(tr("Properties"), this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  auto* dockBody = new QWidget(dock);
  auto* form = new QFormLayout(dockBody);
  m_nameField = new QLineEdit(dockBody);
  m_typeField = new QLineEdit(dockBody);
  m_nameField->setReadOnly(true);
  m_typeField->setReadOnly(true);
  form->addRow(tr("Name"), m_nameField);
  form->addRow(tr("Type"), m_typeField);
  dockBody->setLayout(form);
  dock->setWidget(dockBody);
  addDockWidget(Qt::RightDockWidgetArea, dock);

  connect(m_viewport, &OccViewportWidget::selectionInfoChanged,
          this, &MainWindow::onSelectionInfoChanged);
  connect(m_viewport, &OccViewportWidget::requestDeleteSelected,
          this, &MainWindow::onDeleteSelected);
  connect(m_viewport, &OccViewportWidget::requestHideSelected,
          this, &MainWindow::onHideSelected);
  connect(m_viewport, &OccViewportWidget::requestShowAll,
          this, &MainWindow::onShowAll);

  auto* viewMenu = menuBar()->addMenu(tr("View"));
  auto* fitAllAction = new QAction(tr("Fit All"), this);
  fitAllAction->setShortcut(QKeySequence(Qt::Key_F));
  //在 Qt 里，信号（signal）= 事件通知，槽（slot）= 事件处理函数。
  //connect(A的信号, B的槽) 的意思就是：A 发生某个事件时，自动调用 B 的某个函数。
  connect(fitAllAction, &QAction::triggered, m_viewport, &OccViewportWidget::fitAll);
  viewMenu->addAction(fitAllAction);

  // Render mode submenu (Wireframe / Shaded)
  auto* renderMenu = viewMenu->addMenu(tr("Render Mode"));
  auto* renderGroup = new QActionGroup(this);
  renderGroup->setExclusive(true);

  auto* wireframeAction = new QAction(tr("Wireframe"), this);
  wireframeAction->setCheckable(true);
  connect(wireframeAction, &QAction::triggered, m_viewport, &OccViewportWidget::setWireframe);
  renderGroup->addAction(wireframeAction);
  renderMenu->addAction(wireframeAction);

  auto* shadedAction = new QAction(tr("Shaded"), this);
  shadedAction->setCheckable(true);
  shadedAction->setChecked(true); // default matches context default
  connect(shadedAction, &QAction::triggered, m_viewport, &OccViewportWidget::setShaded);
  renderGroup->addAction(shadedAction);
  renderMenu->addAction(shadedAction);

  auto* toolbar = addToolBar(tr("View"));
  toolbar->addAction(fitAllAction);

  auto* createMenu = menuBar()->addMenu(tr("Create"));
  auto* createBoxAction = new QAction(tr("Box"), this);
  connect(createBoxAction, &QAction::triggered, this, &MainWindow::onCreateBox);
  createMenu->addAction(createBoxAction);

  auto* createSphereAction = new QAction(tr("Sphere"), this);
  connect(createSphereAction, &QAction::triggered, this, &MainWindow::onCreateSphere);
  createMenu->addAction(createSphereAction);

  auto* createToolbar = addToolBar(tr("Create"));
  createToolbar->addAction(createBoxAction);
  createToolbar->addAction(createSphereAction);

}

void MainWindow::onSelectionInfoChanged(const QString& name, const QString& type)
{
  if (m_nameField)
  {
    m_nameField->setText(name);
  }
  if (m_typeField)
  {
    m_typeField->setText(type);
  }
}

void MainWindow::onImportStep()
{
  const QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Import STEP"),
      QString(),
      tr("STEP Files (*.step *.stp);;All Files (*.*)"));

  if (filePath.isEmpty())
  {
    return;
  }

  //主窗口菜单点击Import STEP按钮，对应槽函数，创建一个命令，执行命令后导入文件，然后把命令存到CommandManager待undo  redo
  auto cmd = std::make_unique<ImportStepCommand>(m_doc, m_viewport, filePath);
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("Import STEP"),
                         tr("Failed to import STEP file:\n%1\n\n%2").arg(filePath, err));
  }
}

void MainWindow::onCreateBox()
{
  auto cmd = std::make_unique<CreatePrimitiveCommand>(
      m_doc, m_viewport, CreatePrimitiveCommand::BoxParams{});
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("Create Box"), err);
  }
}

void MainWindow::onCreateSphere()
{
  auto cmd = std::make_unique<CreatePrimitiveCommand>(
      m_doc, m_viewport, CreatePrimitiveCommand::SphereParams{});
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("Create Sphere"), err);
  }
}

void MainWindow::onDeleteSelected()
{
  auto cmd = std::make_unique<DelCommand>(m_doc, m_viewport);
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("Delete"), err);
  }
}

void MainWindow::onHideSelected()
{
  auto cmd = std::make_unique<HideCommand>(m_doc, m_viewport);
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("Hide"), err);
  }
}

void MainWindow::onShowAll()
{
  auto cmd = std::make_unique<ShowAllCommand>(m_doc, m_viewport);
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("Show All"), err);
  }
}

void MainWindow::onBoolBrepCommon()
{
  auto cmd = std::make_unique<BoolBrepCommenCommand>(m_doc, m_viewport);
  QString err;
  if (!m_cmdMgr->doCommand(std::move(cmd), &err))
  {
    QMessageBox::warning(this, tr("BoolBrepCommenCommand"), err);
  }

}

void MainWindow::updateUndoRedoActions()
{
  if (m_cmdMgr == nullptr || m_undoAction == nullptr || m_redoAction == nullptr)
  {
    return;
  }

  const bool canUndo = m_cmdMgr->canUndo();
  const bool canRedo = m_cmdMgr->canRedo();
  m_undoAction->setEnabled(canUndo);
  m_redoAction->setEnabled(canRedo);

  const QString undoTitle = m_cmdMgr->undoTitle();
  const QString redoTitle = m_cmdMgr->redoTitle();
  m_undoAction->setText(undoTitle.isEmpty() ? tr("Undo") : tr("Undo %1").arg(undoTitle));
  m_redoAction->setText(redoTitle.isEmpty() ? tr("Redo") : tr("Redo %1").arg(redoTitle));
}
