#include "MainWindow.h"

#include <QAction>
#include <QMenuBar>
#include <QToolBar>

#include "viewport/OccViewportWidget.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setWindowTitle("MiniCAD (Qt + OCCT)");

  m_viewport = new OccViewportWidget(this);
  setCentralWidget(m_viewport);

  auto* viewMenu = menuBar()->addMenu(tr("View"));
  auto* fitAllAction = new QAction(tr("Fit All"), this);
  fitAllAction->setShortcut(QKeySequence(Qt::Key_F));
  connect(fitAllAction, &QAction::triggered, m_viewport, &OccViewportWidget::fitAll);
  viewMenu->addAction(fitAllAction);

  auto* toolbar = addToolBar(tr("View"));
  toolbar->addAction(fitAllAction);
}

