#include <QApplication>

#include "app/MainWindow.h"

int main(int argc, char** argv)
{
  // 启用 High DPI Scaling 支持，确保在高分屏下界面显示正常
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  QApplication app(argc, argv);

  MainWindow w;
  w.resize(1200, 800);
  w.show();

  return app.exec();
}
