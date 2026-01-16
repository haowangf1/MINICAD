#pragma once

#include <QMainWindow>

class OccViewportWidget;

class MainWindow final : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);

private:
  OccViewportWidget* m_viewport = nullptr;
};

