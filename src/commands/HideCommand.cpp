#include "HideCommand.h"

#include "model/Document.h"
#include "viewport/OccViewportWidget.h"

HideCommand::HideCommand(Document* doc, OccViewportWidget* viewport)
  : m_doc(doc)
  , m_viewport(viewport)
{
}

QString HideCommand::title() const
{
  return "Hide";
}

bool HideCommand::execute()
{
  m_error.clear();
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    m_error = "Internal error: null doc/viewport";
    return false;
  }

  const auto& sel = m_doc->selection();
  if (sel.empty())
  {
    m_error = "No selection";
    return false;
  }

  m_ids = sel;
  for (const auto id : m_ids)
  {
    // Hide in viewport only.
    (void)m_viewport->removeDocumentObject(id);
  }

  // Clear selection after hide (consistent with delete behavior).
  m_doc->setSelection({});
  return true;
}

void HideCommand::undo()
{
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    return;
  }

  for (const auto id : m_ids)
  {
    (void)m_viewport->displayDocumentObject(id);
  }
  if (!m_ids.empty())
  {
    m_doc->setSelection(m_ids);
  }
}

void HideCommand::redo()
{
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    return;
  }

  for (const auto id : m_ids)
  {
    (void)m_viewport->removeDocumentObject(id);
  }
  m_doc->setSelection({});
}

