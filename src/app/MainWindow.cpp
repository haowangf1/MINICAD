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

#include "viewport/OccViewportWidget.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setWindowTitle("MiniCAD");

  m_viewport = new OccViewportWidget(this);
  setCentralWidget(m_viewport);

  // File menu
  auto* fileMenu = menuBar()->addMenu(tr("File"));
  auto* importStepAction = new QAction(tr("Import STEP..."), this);
  connect(importStepAction, &QAction::triggered, this, &MainWindow::onImportStep);
  fileMenu->addAction(importStepAction);

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
  connect(createBoxAction, &QAction::triggered, m_viewport, &OccViewportWidget::addBox);
  createMenu->addAction(createBoxAction);

  auto* createSphereAction = new QAction(tr("Sphere"), this);
  connect(createSphereAction, &QAction::triggered, m_viewport, &OccViewportWidget::addSphere);
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

  if (!m_viewport->importStep(filePath))
  {
    QMessageBox::warning(this, tr("Import STEP"),
                         tr("Failed to import STEP file:\n%1").arg(filePath));
  }
}
