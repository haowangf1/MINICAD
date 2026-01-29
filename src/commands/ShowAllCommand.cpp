#include "ShowAllCommand.h"

#include "model/Document.h"
#include "viewport/OccViewportWidget.h"

ShowAllCommand::ShowAllCommand(Document* doc, OccViewportWidget* viewport)
  : m_doc(doc)
  , m_viewport(viewport)
{
}

QString ShowAllCommand::title() const
{
  return "ShowAll";
}

bool ShowAllCommand::execute()
{
  m_error.clear();
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    m_error = "Internal error: null doc/viewport";
    return false;
  }

  // Capture ids for undo/redo (best-effort: we re-hide these ids on undo).
  m_ids = m_doc->allObjectIds();

  // Show (display) every document object.
  for (const auto id : m_ids)
  {
    (void)m_viewport->displayDocumentObject(id);
  }

  // Keep selection as-is (usually empty after hide/delete commands).
  return true;
}

void ShowAllCommand::undo()
{
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    return;
  }

  // Best-effort: revert by hiding (removing display) for the same set of ids.
  for (const auto id : m_ids)
  {
    (void)m_viewport->removeDocumentObject(id);
  }
  m_doc->setSelection({});
}

void ShowAllCommand::redo()
{
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    return;
  }

  for (const auto id : m_ids)
  {
    (void)m_viewport->displayDocumentObject(id);
  }
}

