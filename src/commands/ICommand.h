#pragma once

#include <QString>

// Command interface for Undo/Redo.
// Concrete commands should be self-contained and reversible.
class ICommand
{
public:
  virtual ~ICommand() = default;

  // Optional: used by UI (Edit->Undo "Create Box", etc.)
  virtual QString title() const { return {}; }

  // Optional: used by UI to show failure reason when execute() returns false.
  virtual QString error() const { return {}; }

  // Execute command for the first time. Return false if command failed and should not be pushed to history.
  virtual bool execute() = 0;

  // Undo effects of execute()/redo().
  virtual void undo() = 0;

  // Re-apply effects after undo().
  // Default behavior can be overridden; by default we call execute() again.
  virtual void redo() { (void)execute(); }
};

