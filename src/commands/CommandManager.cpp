#include "CommandManager.h"

#include "ICommand.h"

CommandManager::CommandManager(QObject* parent)
  : QObject(parent)
{
}

bool CommandManager::canUndo() const
{
  return !m_undo.empty();
}

bool CommandManager::canRedo() const
{
  return !m_redo.empty();
}

QString CommandManager::undoTitle() const
{
  return canUndo() ? m_undo.back()->title() : QString();
}

QString CommandManager::redoTitle() const
{
  return canRedo() ? m_redo.back()->title() : QString();
}

bool CommandManager::doCommand(std::unique_ptr<ICommand> cmd, QString* outError)
{
  if (!cmd)
  {
    if (outError) { *outError = "Null command"; }
    return false;
  }

  if (!cmd->execute())
  {
    if (outError) { *outError = cmd->error(); }
    return false;
  }
  //CommandManager 存储执行得命令以执行Undo redo
  m_undo.push_back(std::move(cmd));
  m_redo.clear();
  emit stackChanged();
  return true;
}

void CommandManager::undo()
{
  if (!canUndo())
  {
    return;
  }

  std::unique_ptr<ICommand> cmd = std::move(m_undo.back());
  m_undo.pop_back();
  cmd->undo();
  m_redo.push_back(std::move(cmd));
  emit stackChanged();
}

void CommandManager::redo()
{
  if (!canRedo())
  {
    return;
  }

  std::unique_ptr<ICommand> cmd = std::move(m_redo.back());
  m_redo.pop_back();
  cmd->redo();
  m_undo.push_back(std::move(cmd));
  emit stackChanged();
}

