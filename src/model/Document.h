#pragma once

#include <QObject>

#include <optional>
#include <unordered_map>
#include <vector>

#include "SceneObject.h"

// Minimal document/scene model for Milestone C.
// Owns scene objects and selection state.
class Document final : public QObject
{
  Q_OBJECT

public:
  explicit Document(QObject* parent = nullptr);

  unsigned long long addObject(SceneObject obj);
  bool removeObject(unsigned long long id);

  std::optional<SceneObject> getObject(unsigned long long id) const;
  std::vector<unsigned long long> allObjectIds() const;

  void setSelection(const std::vector<unsigned long long>& ids);
  const std::vector<unsigned long long>& selection() const;

signals:
  void objectsChanged();
  void selectionChanged();

private:
  unsigned long long m_nextId = 1;
  std::unordered_map<unsigned long long, SceneObject> m_objects;
  std::vector<unsigned long long> m_selection;
};

