#pragma once

#include <QObject>

#include <memory>
#include <vector>

#include "ICommand.h"

// Manages command history for Undo/Redo.
class CommandManager final : public QObject
{
  Q_OBJECT

public:
  explicit CommandManager(QObject* parent = nullptr);

  bool canUndo() const;
  bool canRedo() const;

  QString undoTitle() const;
  QString redoTitle() const;

  // Executes a command and pushes it into undo stack on success.
  // Clears redo stack on any new command.
  bool doCommand(std::unique_ptr<ICommand> cmd, QString* outError = nullptr);

public slots:
  void undo();
  void redo();

signals:
  void stackChanged();

private:
  std::vector<std::unique_ptr<ICommand>> m_undo;
  std::vector<std::unique_ptr<ICommand>> m_redo;
};

