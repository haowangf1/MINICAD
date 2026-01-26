#pragma once

#include "ICommand.h"
#include <QString>
#include "model/SceneObject.h"

class Document;
class OccViewportWidget;

class DelCommand final : public ICommand
{
public:
  DelCommand(Document* doc, OccViewportWidget* viewport);

  QString title() const override;
  QString error() const override { return m_error; }
  bool execute() override;
  void undo() override;
  void redo() override;

private:
  Document* m_doc = nullptr;               // non-owning
  OccViewportWidget* m_viewport = nullptr; // non-owning

  unsigned long long m_id = 0;
  SceneObject m_obj; // persisted for redo
  QString m_error;
};

