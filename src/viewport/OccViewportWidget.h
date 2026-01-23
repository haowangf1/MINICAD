#pragma once

#include <QWidget>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

class QPaintEngine;

class OccViewportWidget final : public QWidget
{
  Q_OBJECT

public:
  explicit OccViewportWidget(QWidget* parent = nullptr);
  ~OccViewportWidget() override;

public slots:
  void fitAll();
  void addBox();
  void addSphere();
  void setWireframe();
  void setShaded();

protected:
  QPaintEngine* paintEngine() const override;
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void showEvent(QShowEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

private:
  void initOcc();
  void applyDisplayMode(int aisDisplayMode);

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
