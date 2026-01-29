#include "OccViewportWidget.h"

#include "model/Document.h"
#include "model/SceneObject.h"

#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QWheelEvent>
#include <QPaintEngine>
#include <QShowEvent>
#include <QApplication>
#include <QFileInfo>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_NeutralWindow.hxx>

#include <AIS_Trihedron.hxx>
#include <AIS_Shape.hxx>
#include <AIS_DisplayMode.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Geom_Axis2Placement.hxx>
#include <gp_Ax2.hxx>
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>

namespace
{
static QString shapeTypeToString(const TopAbs_ShapeEnum t)
{
  switch (t)
  {
    case TopAbs_COMPOUND: return "COMPOUND";
    case TopAbs_COMPSOLID: return "COMPSOLID";
    case TopAbs_SOLID: return "SOLID";
    case TopAbs_SHELL: return "SHELL";
    case TopAbs_FACE: return "FACE";
    case TopAbs_WIRE: return "WIRE";
    case TopAbs_EDGE: return "EDGE";
    case TopAbs_VERTEX: return "VERTEX";
    case TopAbs_SHAPE: return "SHAPE";
  }
  return "UNKNOWN";
}
} // namespace

OccViewportWidget::OccViewportWidget(QWidget* parent)
  : QWidget(parent)
{
  // 1. WA_NativeWindow: 确保该 Widget 拥有独立的 HWND
  setAttribute(Qt::WA_NativeWindow);
  // 2. WA_PaintOnScreen: 禁用 Qt 的后台缓冲，允许 OCCT 直接绘制
  setAttribute(Qt::WA_PaintOnScreen);
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  
  setMouseTracking(true);

  // 初始化 OCCT
  initOcc();
}

OccViewportWidget::~OccViewportWidget() = default;

void OccViewportWidget::setDocument(Document* doc)
{
  m_doc = doc;
}

QPaintEngine* OccViewportWidget::paintEngine() const
{
  return nullptr;
}

void OccViewportWidget::fitAll()
{
  if (m_view.IsNull())
  {
    return;
  }
  m_view->FitAll();
  m_view->Redraw();
}

void OccViewportWidget::setWireframe()
{
  applyDisplayMode(AIS_WireFrame);
}

void OccViewportWidget::setShaded()
{
  applyDisplayMode(AIS_Shaded);
}

void OccViewportWidget::addSphere()
{
  if (m_context.IsNull() || m_view.IsNull())
  {
    return;
  }
  
  double r = 20.;
  TopoDS_Shape shape = BRepPrimAPI_MakeSphere(r);
  // Ensure triangulation exists for shaded display.
  BRepMesh_IncrementalMesh(shape, 0.5);
  Handle(AIS_Shape) ais = new AIS_Shape(shape);
  m_objectNames[ais.get()] = QString("Sphere%1").arg(++m_sphereCounter);
  m_context->Display(ais, AIS_Shaded, 0, Standard_True);

  m_view->FitAll();
  m_view->Redraw();
}

void OccViewportWidget::addBox()
{
  if (m_context.IsNull() || m_view.IsNull())
  {
    return;
  }

  const double dx = 100.0;
  const double dy = 80.0;
  const double dz = 60.0;

  TopoDS_Shape shape = BRepPrimAPI_MakeBox(dx, dy, dz).Shape();
  // Ensure triangulation exists for shaded display.
  BRepMesh_IncrementalMesh(shape, 0.5);
  Handle(AIS_Shape) ais = new AIS_Shape(shape);
  m_objectNames[ais.get()] = QString("Box%1").arg(++m_boxCounter);
  m_context->Display(ais, AIS_Shaded, 0, Standard_True);

  m_view->FitAll();
  m_view->Redraw();
}

bool OccViewportWidget::importStepToObject(const QString& filePath, SceneObject& outObj, QString* outError) const
{
  STEPControl_Reader reader;
  const IFSelect_ReturnStatus status = reader.ReadFile(filePath.toLocal8Bit().constData());
  if (status != IFSelect_RetDone)
  {
    if (outError) { *outError = "STEP reader: ReadFile failed"; }
    return false;
  }

  // Transfer all roots into OCCT shapes.
  if (reader.TransferRoots() <= 0)
  {
    if (outError) { *outError = "STEP reader: TransferRoots returned 0"; }
    return false;
  }

  TopoDS_Shape shape = reader.OneShape();
  if (shape.IsNull())
  {
    if (outError) { *outError = "STEP reader: OneShape is null"; }
    return false;
  }

  outObj = SceneObject{};
  outObj.id = 0;
  outObj.type = "STEP";
  const QString base = QFileInfo(filePath).completeBaseName();
  outObj.name = base.isEmpty() ? QString("STEP%1").arg(++m_stepCounter)
                               : base;
  outObj.shape = shape;
  outObj.trsf = gp_Trsf(); // identity
  return true;
}

bool OccViewportWidget::displayDocumentObject(unsigned long long id)
{
  if (m_doc == nullptr || m_context.IsNull() || m_view.IsNull())
  {
    return false;
  }

  const auto objOpt = m_doc->getObject(id);
  if (!objOpt.has_value())
  {
    return false;
  }
  const SceneObject& obj = *objOpt;
  if (obj.shape.IsNull())
  {
    return false;
  }

  // Remove existing display if any.
  (void)removeDocumentObject(id);

  TopoDS_Shape shapeToDisplay = obj.shape;
  // Apply local transform if any.
  shapeToDisplay = shapeToDisplay.Located(TopLoc_Location(obj.trsf));

  // Ensure triangulation exists for shaded display (mesh each face).
  for (TopExp_Explorer exp(shapeToDisplay, TopAbs_FACE); exp.More(); exp.Next())
  {
    const TopoDS_Face& face = TopoDS::Face(exp.Current());
    BRepMesh_IncrementalMesh(face, 0.5);
  }

  Handle(AIS_Shape) ais = new AIS_Shape(shapeToDisplay);
  m_idToAis[id] = ais;
  m_aisToId[ais.get()] = id;
  // Keep name around for property panel fallback.
  m_objectNames[ais.get()] = obj.name;

  m_context->Display(ais, AIS_Shaded, 0, Standard_True);
  m_view->FitAll();
  m_view->Redraw();
  return true;
}

bool OccViewportWidget::removeDocumentObject(unsigned long long id)
{
  if (m_context.IsNull())
  {
    return false;
  }
  auto it = m_idToAis.find(id);
  if (it == m_idToAis.end())
  {
    return false;
  }
  const Handle(AIS_InteractiveObject) obj = it->second;
  if (!obj.IsNull())
  {
    m_context->Remove(obj, Standard_True);
    m_objectNames.erase(obj.get());
    m_aisToId.erase(obj.get());
  }
  m_idToAis.erase(it);
  return true;
}

void OccViewportWidget::emitSelectionInfo()
{
  if (m_context.IsNull())
  {
    emit selectionInfoChanged("", "");
    return;
  }

  // OCCT API differs between versions; use iterator-based selection access
  // (InitSelected/MoreSelected/SelectedInteractive) for compatibility.
  m_context->InitSelected();
  if (!m_context->MoreSelected())
  {
    emit selectionInfoChanged("", "");
    return;
  }

  Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
  if (obj.IsNull())
  {
    emit selectionInfoChanged("", "");
    return;
  }

  const void* key = obj.get();
  QString name = "Unnamed";
  QString type;

  // If object is tracked by Document id, prefer Document metadata.
  if (m_doc != nullptr)
  {
    if (auto itId = m_aisToId.find(key); itId != m_aisToId.end())
    {
      const auto docObj = m_doc->getObject(itId->second);
      if (docObj.has_value())
      {
        name = docObj->name;
        type = docObj->type;
      }
    }
  }

  // Fallback to local naming (used for primitives created before commandization).
  if (type.isEmpty())
  {
    if (auto it = m_objectNames.find(key); it != m_objectNames.end())
    {
      name = it->second;
    }
    type = QString::fromLatin1(obj->DynamicType()->Name());
  }

  if (Handle(AIS_Shape) sh = Handle(AIS_Shape)::DownCast(obj); !sh.IsNull())
  {
    type += QString(" (%1)").arg(shapeTypeToString(sh->Shape().ShapeType()));
  }

  emit selectionInfoChanged(name, type);
}

void OccViewportWidget::applyDisplayMode(int aisDisplayMode)
{
  if (m_context.IsNull() || m_view.IsNull())
  {
    return;
  }

  // Update default for future objects.
  m_context->SetDisplayMode(aisDisplayMode, Standard_False);

  // Apply to all currently displayed objects.
  AIS_ListOfInteractive displayed;
  m_context->DisplayedObjects(displayed);
  for (AIS_ListIteratorOfListOfInteractive it(displayed); it.More(); it.Next())
  {
    const Handle(AIS_InteractiveObject)& obj = it.Value();
    if (!obj.IsNull())
    {
      m_context->SetDisplayMode(obj, aisDisplayMode, Standard_False);
    }
  }

  // Force immediate redraw.
  m_context->UpdateCurrentViewer();
  m_view->Redraw();
}

void OccViewportWidget::paintEvent(QPaintEvent* /*event*/)
{
  if (!m_view.IsNull())
  {
    m_view->Redraw();
  }
}

void OccViewportWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  if (!m_view.IsNull())
  {
    const int valWidth = width() * devicePixelRatio();
    const int valHeight = height() * devicePixelRatio();
    
    // 使用 Handle::DownCast 将 Aspect_Window 安全转换为 Aspect_NeutralWindow
    Handle(Aspect_NeutralWindow) neutralWindow = Handle(Aspect_NeutralWindow)::DownCast(m_view->Window());
    if (!neutralWindow.IsNull())
    {
      neutralWindow->SetSize(valWidth, valHeight);
    }
    m_view->MustBeResized();
  }
}

void OccViewportWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  if (!m_view.IsNull())
  {
    const int valWidth = width() * devicePixelRatio();
    const int valHeight = height() * devicePixelRatio();
    
    Handle(Aspect_NeutralWindow) neutralWindow = Handle(Aspect_NeutralWindow)::DownCast(m_view->Window());
    if (!neutralWindow.IsNull())
    {
      neutralWindow->SetSize(valWidth, valHeight);
    }
    m_view->MustBeResized();
    m_view->Redraw();
  }
}

void OccViewportWidget::mousePressEvent(QMouseEvent* event)
{
  m_lastPos = event->pos();
  m_pressPos = m_lastPos;

  if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier))
  {
    m_dragMode = DragMode::Rotate;
    if (!m_view.IsNull())
    {
      const int dpr = devicePixelRatio();
      m_view->StartRotation(m_lastPos.x() * dpr, m_lastPos.y() * dpr);
    }
  }
  else if (event->button() == Qt::MiddleButton)
  {
    m_dragMode = DragMode::Pan;
  }
  else
  {
    m_dragMode = DragMode::None;
  }

  QWidget::mousePressEvent(event);
}

void OccViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
  const QPoint p = event->pos();

  if (m_view.IsNull())
  {
    m_lastPos = p;
    QWidget::mouseMoveEvent(event);
    return;
  }

  if (m_dragMode == DragMode::Rotate && (event->buttons() & Qt::LeftButton))
  {
    const int dpr = devicePixelRatio();
    m_view->Rotation(p.x() * dpr, p.y() * dpr);
    m_view->Redraw();
  }
  else if (m_dragMode == DragMode::Pan && (event->buttons() & Qt::MiddleButton))
  {
    const QPoint delta = p - m_lastPos;
    const int dpr = devicePixelRatio();
    m_view->Pan(delta.x() * dpr, -delta.y() * dpr);
    m_view->Redraw();
  }
  else
  {
    // Hover pre-highlight (dynamic highlight) on mouse move when not dragging.
    if (!m_context.IsNull())
    {
      const int dpr = devicePixelRatio();
      m_context->MoveTo(p.x() * dpr, p.y() * dpr, m_view, Standard_True);
      m_view->Redraw();
    }
  }

  m_lastPos = p;
  QWidget::mouseMoveEvent(event);
}

void OccViewportWidget::contextMenuEvent(QContextMenuEvent* event)
{
  if (event == nullptr)
  {
    return;
  }

  if (m_doc == nullptr)
  {
    return;
  }

  QMenu menu(this);
  QAction* showAllAction = menu.addAction(tr("Show All"));
  menu.addSeparator();
  QAction* hideAction = menu.addAction(tr("Hide"));
  QAction* delAction = menu.addAction(tr("Delete"));

  const auto& sel = m_doc->selection();
  const bool hasSel = !sel.empty();
  hideAction->setEnabled(hasSel);
  delAction->setEnabled(hasSel);

  QAction* chosen = menu.exec(event->globalPos());
  if (chosen == showAllAction)
  {
    emit requestShowAll();
  }
  else if (chosen == hideAction)
  {
    emit requestHideSelected();
  }
  else if (chosen == delAction)
  {
    emit requestDeleteSelected();
  }
}

void OccViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
  // Left click select (without Alt) when movement is small.
  if (event->button() == Qt::LeftButton && !(event->modifiers() & Qt::AltModifier))
  {
    const QPoint releasePos = event->pos();
    constexpr int kClickThresholdPx = 3; // in Qt logical pixels
    if ((releasePos - m_pressPos).manhattanLength() <= kClickThresholdPx
        && !m_context.IsNull() && !m_view.IsNull())
    {
      const int dpr = devicePixelRatio();
      // Ensure detection at click point is up-to-date, then select.
      m_context->MoveTo(releasePos.x() * dpr, releasePos.y() * dpr, m_view, Standard_True);
      const bool multi =
          (event->modifiers() & Qt::ShiftModifier) || (event->modifiers() & Qt::ControlModifier);
      if (multi)
      {
        // Add/toggle selection without clearing existing selection.
        m_context->ShiftSelect(Standard_True);
      }
      else
      {
        // Single select (clears previous selection).
        m_context->Select(Standard_True);
      }
      m_context->UpdateCurrentViewer();
      m_view->Redraw();

      // Sync selection back into Document.
      if (m_doc != nullptr)
      {
        std::vector<unsigned long long> ids;
        m_context->InitSelected();
        while (m_context->MoreSelected())
        {
          const Handle(AIS_InteractiveObject) selObj = m_context->SelectedInteractive();
          if (!selObj.IsNull())
          {
            const void* key = selObj.get();
            if (auto it = m_aisToId.find(key); it != m_aisToId.end())
            {
              ids.push_back(it->second);
            }
          }
          m_context->NextSelected();
        }

        // Clicking empty space with no modifier should clear selection.
        // With Shift/Ctrl, OCCT usually keeps selection unchanged, so ids will reflect that.
        m_doc->setSelection(ids);
      }

      emitSelectionInfo();
    }
  }

  m_dragMode = DragMode::None;
  QWidget::mouseReleaseEvent(event);
}

void OccViewportWidget::wheelEvent(QWheelEvent* event)
{
  if (m_view.IsNull())
  {
    QWidget::wheelEvent(event);
    return;
  }

  const int delta = event->angleDelta().y();
  if (delta == 0)
  {
    QWidget::wheelEvent(event);
    return;
  }

  const QPoint p = event->position().toPoint();
  const int step = (delta > 0) ? -60 : 60;
  m_view->Zoom(p.x(), p.y(), p.x() + step, p.y() + step);
  m_view->Redraw();

  QWidget::wheelEvent(event);
}

void OccViewportWidget::initOcc()
{
  Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
  Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(displayConnection);

  m_viewer = new V3d_Viewer(graphicDriver);
  m_viewer->SetDefaultLights();
  m_viewer->SetLightOn();

  m_context = new AIS_InteractiveContext(m_viewer);
  // Default to shaded mode so primitives show triangles (not wireframe).
  m_context->SetDisplayMode(AIS_Shaded, Standard_False);

  m_view = m_viewer->CreateView();
  m_view->SetBackgroundColor(Quantity_NOC_GRAY20);

  // 使用 Aspect_NeutralWindow 代替 WNT_Window
  Handle(Aspect_NeutralWindow) window = new Aspect_NeutralWindow();
  
  // 绑定原生的 HWND
  window->SetNativeHandle((Aspect_Drawable)winId());
  
  // 设置初始尺寸（处理 DPI）
  const int valWidth = width() * devicePixelRatio();
  const int valHeight = height() * devicePixelRatio();
  window->SetSize(valWidth, valHeight);

  m_view->SetWindow(window);
  if (!window->IsMapped())
  {
    window->Map();
  }
  
  m_view->MustBeResized();

  m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.08, V3d_ZBUFFER);

  Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(gp_Ax2());
  Handle(AIS_Trihedron) aisTrihedron = new AIS_Trihedron(placement);
  m_objectNames[aisTrihedron.get()] = "Trihedron";
  m_context->Display(aisTrihedron, Standard_False);

  m_view->FitAll();
  m_view->Redraw();
}
