#pragma once

#include "ICommand.h"

#include <QString>
#include <vector>

class Document;
class OccViewportWidget;

// Hide currently selected objects (Undo/Redo-able).
// Implementation: remove from viewport only; keep objects in Document.
class HideCommand final : public ICommand
{
public:
  HideCommand(Document* doc, OccViewportWidget* viewport);

  QString title() const override;
  QString error() const override { return m_error; }
  bool execute() override;
  void undo() override;
  void redo() override;

private:
  Document* m_doc = nullptr;               // non-owning
  OccViewportWidget* m_viewport = nullptr; // non-owning

  std::vector<unsigned long long> m_ids;
  QString m_error;
};

