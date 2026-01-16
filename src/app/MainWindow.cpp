#include "MainWindow.h"

#include <QAction>
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

