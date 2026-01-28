#pragma once

#include "ICommand.h"

#include <QString>

#include "model/SceneObject.h"
#include<vector>

class Document;
class OccViewportWidget;

// Boolean Common (intersection) for selected shapes (Undo/Redo-able).
class BoolBrepCommenCommand final : public ICommand
{
public:
  BoolBrepCommenCommand(Document* doc, OccViewportWidget* viewport);

  QString title() const override;
  QString error() const override { return m_error; }
  bool execute() override;
  void undo() override;
  void redo() override;

private:
  Document* m_doc = nullptr;               // non-owning
  OccViewportWidget* m_viewport = nullptr; // non-owning

  // Saved state for undo/redo.
  unsigned long long m_id = 0;            // result object id (0 means "no result")
  std::vector<SceneObject> m_objs;        // removed objects (with original ids)
  SceneObject m_resultObj;                // created result object (with m_id)
  bool m_hasResult = false;

  QString m_error;
};

