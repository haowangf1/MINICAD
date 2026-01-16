#include "OccViewportWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>

#include <AIS_Trihedron.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Geom_Axis2Placement.hxx>
#include <gp_Ax2.hxx>

OccViewportWidget::OccViewportWidget(QWidget* parent)
  : QWidget(parent)
{
  // OCC uses its own OpenGL rendering; avoid Qt painting over it.
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setMouseTracking(true);

  initOcc();
}

OccViewportWidget::~OccViewportWidget() = default;

void OccViewportWidget::fitAll()
{
  if (m_view.IsNull())
  {
    return;
  }
  m_view->FitAll();
  m_view->Redraw();
}

void OccViewportWidget::addSphere()
{
  if (m_context.IsNull() || m_view.IsNull())
  {
    return;
  }
  
  double r = 20.;
  TopoDS_Shape shape = BRepPrimAPI_MakeSphere(r);
  Handle(AIS_Shape) ais = new AIS_Shape(shape);
  m_context->Display(ais, Standard_True);

  m_view->FitAll();
  m_view->Redraw();

}

void OccViewportWidget::addBox()
{
  if (m_context.IsNull() || m_view.IsNull())
  {
    return;
  }

  // Default size in "model units" (you can treat as mm).
  const double dx = 100.0;
  const double dy = 80.0;
  const double dz = 60.0;

  TopoDS_Shape shape = BRepPrimAPI_MakeBox(dx, dy, dz).Shape();
  Handle(AIS_Shape) ais = new AIS_Shape(shape);
  m_context->Display(ais, Standard_True);

  m_view->FitAll();
  m_view->Redraw();
}

void OccViewportWidget::paintEvent(QPaintEvent* event)
{
  QWidget::paintEvent(event);
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
    m_view->MustBeResized();
  }
}

void OccViewportWidget::mousePressEvent(QMouseEvent* event)
{
  m_lastPos = event->pos();

  if (event->button() == Qt::LeftButton)
  {
    m_dragMode = DragMode::Rotate;
    if (!m_view.IsNull())
    {
      m_view->StartRotation(m_lastPos.x(), m_lastPos.y());
    }
  }
  else if (event->button() == Qt::MiddleButton)
  {
    m_dragMode = DragMode::Pan;
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
    m_view->Rotation(p.x(), p.y());
    m_view->Redraw();
  }
  else if (m_dragMode == DragMode::Pan && (event->buttons() & Qt::MiddleButton))
  {
    const QPoint delta = p - m_lastPos;
    m_view->Pan(delta.x(), -delta.y());
    m_view->Redraw();
  }

  m_lastPos = p;
  QWidget::mouseMoveEvent(event);
}

void OccViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
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

  // Use rectangle zoom around cursor. It's simple and widely compatible across OCCT versions.
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

  m_view = m_viewer->CreateView();
  m_view->SetBackgroundColor(Quantity_NOC_GRAY20);

  // Bind OCC view to this Qt widget native window (HWND on Windows).
  const Aspect_Handle hwnd = reinterpret_cast<Aspect_Handle>(winId());
  Handle(WNT_Window) window = new WNT_Window(hwnd);
  m_view->SetWindow(window);
  if (!window->IsMapped())
  {
    window->Map();
  }

  // Show a small trihedron (axis widget) so you know the view is alive.
  m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.08, V3d_ZBUFFER);

  // Also add a trihedron object into the scene (optional but nice for confidence).
  Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(gp_Ax2());
  Handle(AIS_Trihedron) aisTrihedron = new AIS_Trihedron(placement);
  m_context->Display(aisTrihedron, Standard_False);

  m_view->FitAll();
  m_view->Redraw();
}

