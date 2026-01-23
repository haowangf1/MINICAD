#include "Document.h"

#include <algorithm>

Document::Document(QObject* parent)
  : QObject(parent)
{
}

unsigned long long Document::addObject(SceneObject obj)
{
  if (obj.id == 0)
  {
    obj.id = m_nextId++;
  }
  else if (obj.id >= m_nextId)
  {
    m_nextId = obj.id + 1;
  }

  const auto id = obj.id;
  m_objects[id] = std::move(obj);
  emit objectsChanged();
  return id;
}

bool Document::removeObject(unsigned long long id)
{
  const auto it = m_objects.find(id);
  if (it == m_objects.end())
  {
    return false;
  }
  m_objects.erase(it);

  // Remove from selection if present.
  const auto beforeSize = m_selection.size();
  m_selection.erase(
      std::remove_if(m_selection.begin(), m_selection.end(),
                     [&](unsigned long long x) { return x == id; }),
      m_selection.end());
  const bool selChanged = (m_selection.size() != beforeSize);

  emit objectsChanged();
  if (selChanged)
  {
    emit selectionChanged();
  }
  return true;
}

std::optional<SceneObject> Document::getObject(unsigned long long id) const
{
  const auto it = m_objects.find(id);
  if (it == m_objects.end())
  {
    return std::nullopt;
  }
  return it->second;
}

std::vector<unsigned long long> Document::allObjectIds() const
{
  std::vector<unsigned long long> ids;
  ids.reserve(m_objects.size());
  for (const auto& kv : m_objects)
  {
    ids.push_back(kv.first);
  }
  return ids;
}

void Document::setSelection(const std::vector<unsigned long long>& ids)
{
  m_selection = ids;
  emit selectionChanged();
}

const std::vector<unsigned long long>& Document::selection() const
{
  return m_selection;
}

