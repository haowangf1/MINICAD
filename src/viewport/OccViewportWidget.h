#pragma once

#include <QWidget>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

class OccViewportWidget final : public QWidget
{
  Q_OBJECT

public:
  explicit OccViewportWidget(QWidget* parent = nullptr);
  ~OccViewportWidget() override;

public slots:
//槽函数，由对应信号触发
  void fitAll();
  void addBox();
  void addSphere();

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

private:
  void initOcc();

private:
  Handle(V3d_Viewer) m_viewer;
  Handle(V3d_View) m_view;
  Handle(AIS_InteractiveContext) m_context;

  QPoint m_lastPos;
  enum class DragMode
  {
    None,
    Rotate,
    Pan
  } m_dragMode = DragMode::None;
};

