#include "ImportStepCommand.h"

#include "model/Document.h"
#include "viewport/OccViewportWidget.h"

ImportStepCommand::ImportStepCommand(Document* doc, OccViewportWidget* viewport, QString filePath)
  : m_doc(doc)
  , m_viewport(viewport)
  , m_filePath(std::move(filePath))
{
}

QString ImportStepCommand::title() const
{
  return "Import STEP";
}

bool ImportStepCommand::execute()
{
  m_error.clear();
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    m_error = "Internal error: null doc/viewport";
    return false;
  }

  // First execution: parse STEP and create an object.
  if (m_id == 0)
  {
    SceneObject obj;
    QString err;
    if (!m_viewport->importStepToObject(m_filePath, obj, &err))
    {
      m_error = err.isEmpty() ? "Failed to import STEP" : err;
      return false;
    }

    // Store into Document (assign stable id).
    //导入step得具体命令执行，构建SceneObject包含Toposhape，并存到Document
    m_id = m_doc->addObject(obj);
    m_obj = *m_doc->getObject(m_id); // persist full object for redo
  }
  else
  {
    // Redo path should call redo(), but execute() might be called by default redo() too.
    m_doc->addObject(m_obj);
  }

  // Display it (and build id <-> AIS mapping in viewport).
  if (!m_viewport->displayDocumentObject(m_id))
  {
    m_error = "Failed to display imported object";
    // Rollback document insert on failure.
    m_doc->removeObject(m_id);
    m_id = 0;
    return false;
  }

  return true;
}

void ImportStepCommand::undo()
{
  //由命令自己定义undo redo的具体行为
  if (m_doc == nullptr || m_viewport == nullptr || m_id == 0)
  {
    return;
  }

  (void)m_viewport->removeDocumentObject(m_id);
  (void)m_doc->removeObject(m_id);
}

void ImportStepCommand::redo()
{
  if (m_doc == nullptr || m_viewport == nullptr || m_id == 0)
  {
    return;
  }

  // Restore into document using the same stable id.
  m_doc->addObject(m_obj);
  (void)m_viewport->displayDocumentObject(m_id);
}

