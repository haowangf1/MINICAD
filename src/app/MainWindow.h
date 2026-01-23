#pragma once

#include <QMainWindow>

class OccViewportWidget;
class QLineEdit;

class MainWindow final : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);

private slots:
  void onSelectionInfoChanged(const QString& name, const QString& type);

private:
  OccViewportWidget* m_viewport = nullptr;
  QLineEdit* m_nameField = nullptr;
  QLineEdit* m_typeField = nullptr;
};

