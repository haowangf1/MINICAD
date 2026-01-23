#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QMenuBar>
#include <QToolBar>

#include "viewport/OccViewportWidget.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setWindowTitle("MiniCAD");

  m_viewport = new OccViewportWidget(this);
  setCentralWidget(m_viewport);

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

