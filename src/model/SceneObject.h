#pragma once

#include <QString>

#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>

// Minimal scene object representation for Milestone C.
// This is the "source of truth" (not AIS handles).
struct SceneObject
{
  unsigned long long id = 0;
  QString name;
  QString type; // "Box" / "Cylinder" / "Sphere" / "STEP" / ...

  TopoDS_Shape shape;
  gp_Trsf trsf;
};

