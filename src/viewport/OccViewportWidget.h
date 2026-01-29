#pragma once

#include <QWidget>
#include <QString>

#include <unordered_map>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

class QPaintEngine;
class Document;
struct SceneObject;

class OccViewportWidget final : public QWidget
{
  Q_OBJECT

public:
  explicit OccViewportWidget(QWidget* parent = nullptr);
  ~OccViewportWidget() override;

  void setDocument(Document* doc);

  // Parse STEP file into a SceneObject (no display). Returns false on failure.
  bool importStepToObject(const QString& filePath, SceneObject& outObj, QString* outError = nullptr) const;

  // Display/remove objects by Document id (maintains id <-> AIS mapping).
  bool displayDocumentObject(unsigned long long id);
  bool removeDocumentObject(unsigned long long id);

public slots:
  void fitAll();
  void addBox();
  void addSphere();
  void setWireframe();
  void setShaded();

signals:
  // Emitted after selection changes (click select). Empty strings mean "no selection".
  void selectionInfoChanged(const QString& name, const QString& type);

  // Request actions on current Document selection (handled by MainWindow/CommandManager).
  void requestHideSelected();
  void requestDeleteSelected();

protected:
  QPaintEngine* paintEngine() const override;
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void showEvent(QShowEvent* event) override;

  void contextMenuEvent(QContextMenuEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

private:
  void initOcc();
  void applyDisplayMode(int aisDisplayMode);
  void emitSelectionInfo();

private:
  Handle(V3d_Viewer) m_viewer;
  Handle(V3d_View) m_view;
  Handle(AIS_InteractiveContext) m_context;
  Document* m_doc = nullptr; // non-owning

  int m_boxCounter = 0;
  int m_sphereCounter = 0;
  mutable int m_stepCounter = 0;
  std::unordered_map<const void*, QString> m_objectNames;
  std::unordered_map<unsigned long long, Handle(AIS_InteractiveObject)> m_idToAis;
  std::unordered_map<const void*, unsigned long long> m_aisToId;

  QPoint m_lastPos;
  QPoint m_pressPos;
  enum class DragMode
  {
    None,
    Rotate,
    Pan
  } m_dragMode = DragMode::None;
};
