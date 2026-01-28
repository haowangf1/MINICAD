#include "DelCommand.h"

#include "model/Document.h"
#include "viewport/OccViewportWidget.h"

DelCommand::DelCommand(Document* doc, OccViewportWidget* viewport)
  : m_doc(doc)
  , m_viewport(viewport)
{

}

QString DelCommand::title() const
{
    return "Del";
}

bool DelCommand::execute()
{
  m_error.clear();
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    m_error = "Internal error: null doc/viewport";
    return false;
  }

  // Spec (Milestone C): delete command reads target from Document selection.
  const auto& sel = m_doc->selection();
  if (sel.empty())
  {
    m_error = "No selection";
    return false;
  }

  // Minimal implementation: delete the first selected object.
  const unsigned long long id = sel.front();
  const auto objOpt = m_doc->getObject(id);
  if (!objOpt.has_value())
  {
    m_error = "Selected object not found in document";
    return false;
  }

  // Backup for undo/redo.
  m_id = id;
  m_obj = *objOpt;

  // Remove from viewport (might already be absent; ignore return).
  (void)m_viewport->removeDocumentObject(m_id);

  // Remove from document.
  if (!m_doc->removeObject(m_id))
  {
    m_error = "Failed to remove object from document";
    // Best-effort restore display if document still has it.
    (void)m_viewport->displayDocumentObject(m_id);
    return false;
  }

  // Clear selection after deletion (as described in the milestone).
  m_doc->setSelection({});
  return true;
}

void DelCommand::undo()
{
  if (m_doc == nullptr || m_viewport == nullptr || m_id == 0)
  {
    return;
  }

  // 确保恢复的是同一个 id
  m_obj.id = m_id;

  m_doc->addObject(m_obj);
  if (!m_viewport->displayDocumentObject(m_id))
  {
    // 最起码别让数据和显示不一致
    (void)m_doc->removeObject(m_id);
    return;
  }

  m_doc->setSelection({m_id});
}

void DelCommand::redo()
{
  if (m_doc == nullptr || m_viewport == nullptr || m_id == 0)
  {
    return;
  }
 
  // Re-apply deletion: remove display, remove from document, clear selection.
  (void)m_viewport->removeDocumentObject(m_id);
  (void)m_doc->removeObject(m_id);
  m_doc->setSelection({});
}
