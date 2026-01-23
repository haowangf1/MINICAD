#pragma once

#include "ICommand.h"

#include <QString>

#include "model/SceneObject.h"

class Document;
class OccViewportWidget;

// Import STEP file as a SceneObject into Document (Undo/Redo-able).
class ImportStepCommand final : public ICommand
{
public:
  ImportStepCommand(Document* doc, OccViewportWidget* viewport, QString filePath);

  QString title() const override;
  QString error() const override { return m_error; }
  bool execute() override;
  void undo() override;
  void redo() override;

private:
  Document* m_doc = nullptr;               // non-owning
  OccViewportWidget* m_viewport = nullptr; // non-owning
  QString m_filePath;

  unsigned long long m_id = 0;
  SceneObject m_obj; // persisted for redo
  QString m_error;
};

