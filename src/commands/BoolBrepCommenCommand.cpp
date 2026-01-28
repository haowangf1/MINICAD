#include "BoolBrepCommenCommand.h"

#include "model/Document.h"
#include "viewport/OccViewportWidget.h"
#include <BRepAlgoAPI_Common.hxx>

BoolBrepCommenCommand::BoolBrepCommenCommand(Document * doc, OccViewportWidget * viewport)
:m_doc(doc),m_viewport(viewport)
{

}

QString BoolBrepCommenCommand::title() const
{
    return "BoolBrepCommen";
}

bool BoolBrepCommenCommand::execute()
{

    m_error.clear();
    if (m_doc == nullptr || m_viewport == nullptr)
    {
        m_error = "Internal error: null doc/viewport";
        return false;
    }

    // Clear previous state (command object may be reused).
    m_objs.clear();
    m_hasResult = false;

    const std::vector<unsigned long long> sel = m_doc->selection();
    std::vector<TopoDS_Shape> shapestack;
    shapestack.reserve(sel.size());

    // Gather selected objects first; do NOT mutate Document/Viewport until we know the boolean succeeds.
    for (const auto id : sel)
    {
        const std::optional<SceneObject> obj = m_doc->getObject(id);
        if (!obj.has_value())
            continue;

        m_objs.push_back(obj.value());
        shapestack.push_back(obj.value().shape);
    }

    if (shapestack.size() < 2)
    {
        m_error = "Failed to bool: need at least 2 selected shapes";
        m_objs.clear();
        return false;
    }

    while (shapestack.size() >= 2)
    {
        const TopoDS_Shape shape1 = shapestack.back();
        shapestack.pop_back();
        const TopoDS_Shape shape2 = shapestack.back();
        shapestack.pop_back();

        BRepAlgoAPI_Common intersect(shape1, shape2);
        intersect.Build();
        if (!intersect.IsDone() || intersect.Shape().IsNull())
        {
            m_error = "BRepAlgoAPI_Common failed";
            m_objs.clear();
            return false;
        }
        shapestack.push_back(intersect.Shape());
    }

    if (shapestack.size() != 1 || shapestack.back().IsNull())
    {
        m_error = "BRepAlgoAPI_Common failed";
        m_objs.clear();
        return false;
    }

    // Remove originals.
    for (const auto& obj : m_objs)
    {
        m_doc->removeObject(obj.id);
        m_viewport->removeDocumentObject(obj.id);
    }

    // Create result.
    SceneObject objnew;
    objnew.type = "BoolCommon";
    objnew.name = "BoolCommon";
    objnew.shape = shapestack.back();

    // Keep result id stable across undo/redo.
    if (m_id != 0)
    {
        objnew.id = m_id;
    }

    m_id = m_doc->addObject(objnew);
    m_resultObj = objnew;
    m_resultObj.id = m_id;
    m_hasResult = true;

    m_viewport->displayDocumentObject(m_id);

    return true;
}

void BoolBrepCommenCommand::undo()
{
    
}

void BoolBrepCommenCommand::redo()
{
   
}
