#pragma once

#include "ICommand.h"

#include <QString>

#include "model/SceneObject.h"

class Document;
class OccViewportWidget;

class CreatePrimitiveCommand final : public ICommand
{
public:
  enum class PrimitiveType
  {
    Box,
    Sphere,
  };

  struct BoxParams
  {
    double dx = 100.0;
    double dy = 80.0;
    double dz = 60.0;
  };

  struct SphereParams
  {
    double r = 20.0;
  };

  CreatePrimitiveCommand(Document* doc, OccViewportWidget* viewport, BoxParams params);
  CreatePrimitiveCommand(Document* doc, OccViewportWidget* viewport, SphereParams params);

  QString title() const override;
  QString error() const override { return m_error; }

  bool execute() override;
  void undo() override;
  void redo() override;

private:
  bool buildObjectOnce();

private:
  Document* m_doc = nullptr;               // non-owning
  OccViewportWidget* m_viewport = nullptr; // non-owning

  PrimitiveType m_type = PrimitiveType::Box;
  BoxParams m_box;
  SphereParams m_sphere;

  unsigned long long m_id = 0;
  SceneObject m_obj; // persisted for redo (includes id)
  QString m_error;

  bool m_built = false;
};

