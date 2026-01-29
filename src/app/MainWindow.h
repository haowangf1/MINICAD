#pragma once

#include <QMainWindow>

class OccViewportWidget;
class QLineEdit;
class Document;
class CommandManager;

class MainWindow final : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);

private slots:
  void onSelectionInfoChanged(const QString& name, const QString& type);
  void onImportStep();
  void onCreateBox();
  void onCreateSphere();
  void onDeleteSelected();
  void onHideSelected();
  void onShowAll();
  void onBoolBrepCommon();
  void updateUndoRedoActions();

private:
  OccViewportWidget* m_viewport = nullptr;
  QLineEdit* m_nameField = nullptr;
  QLineEdit* m_typeField = nullptr;

  Document* m_doc = nullptr;
  CommandManager* m_cmdMgr = nullptr;

  QAction* m_undoAction = nullptr;
  QAction* m_redoAction = nullptr;
  QAction* m_DelAction = nullptr;
  QAction* m_BoolBrepCommonAction = nullptr;
};

