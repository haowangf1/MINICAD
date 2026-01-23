#include "CreatePrimitiveCommand.h"

#include "model/Document.h"
#include "viewport/OccViewportWidget.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

namespace
{
static int g_boxCounter = 0;
static int g_sphereCounter = 0;
}

CreatePrimitiveCommand::CreatePrimitiveCommand(Document* doc, OccViewportWidget* viewport, BoxParams params)
  : m_doc(doc)
  , m_viewport(viewport)
  , m_type(PrimitiveType::Box)
  , m_box(params)
{
}

CreatePrimitiveCommand::CreatePrimitiveCommand(Document* doc, OccViewportWidget* viewport, SphereParams params)
  : m_doc(doc)
  , m_viewport(viewport)
  , m_type(PrimitiveType::Sphere)
  , m_sphere(params)
{
}

QString CreatePrimitiveCommand::title() const
{
  return (m_type == PrimitiveType::Box) ? "Create Box" : "Create Sphere";
}

bool CreatePrimitiveCommand::buildObjectOnce()
{
  if (m_built)
  {
    return true;
  }

  m_obj = SceneObject{};
  m_obj.id = 0;
  m_obj.trsf = gp_Trsf(); // identity

  if (m_type == PrimitiveType::Box)
  {
    m_obj.type = "Box";
    m_obj.name = QString("Box%1").arg(++g_boxCounter);
    m_obj.shape = BRepPrimAPI_MakeBox(m_box.dx, m_box.dy, m_box.dz).Shape();
  }
  else
  {
    m_obj.type = "Sphere";
    m_obj.name = QString("Sphere%1").arg(++g_sphereCounter);
    m_obj.shape = BRepPrimAPI_MakeSphere(m_sphere.r).Shape();
  }

  if (m_obj.shape.IsNull())
  {
    m_error = "Failed to build primitive shape";
    return false;
  }

  m_built = true;
  return true;
}

bool CreatePrimitiveCommand::execute()
{
  m_error.clear();
  if (m_doc == nullptr || m_viewport == nullptr)
  {
    m_error = "Internal error: null doc/viewport";
    return false;
  }

  // First-time execution: build and add into Document, capturing stable id.
  if (m_id == 0)
  {
    if (!buildObjectOnce())
    {
      return false;
    }

    m_id = m_doc->addObject(m_obj);
    const auto objOpt = m_doc->getObject(m_id);
    if (!objOpt.has_value())
    {
      m_error = "Failed to add object into document";
      m_id = 0;
      return false;
    }
    m_obj = *objOpt;
  }
  else
  {
    // If execute() is called again (e.g. default redo), ensure object is in document.
    m_doc->addObject(m_obj);
  }

  if (!m_viewport->displayDocumentObject(m_id))
  {
    m_error = "Failed to display object";
    (void)m_doc->removeObject(m_id);
    m_id = 0;
    return false;
  }

  return true;
}

void CreatePrimitiveCommand::undo()
{
  if (m_doc == nullptr || m_viewport == nullptr || m_id == 0)
  {
    return;
  }

  (void)m_viewport->removeDocumentObject(m_id);
  (void)m_doc->removeObject(m_id);
}

void CreatePrimitiveCommand::redo()
{
  if (m_doc == nullptr || m_viewport == nullptr || m_id == 0)
  {
    return;
  }

  m_doc->addObject(m_obj);
  (void)m_viewport->displayDocumentObject(m_id);
}

