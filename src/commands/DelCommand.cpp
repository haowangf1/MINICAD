#include "DelCommand.h"

DelCommand::DelCommand(Document * doc, OccViewportWidget * viewport)
:m_doc(doc),m_viewport(viewport)
{

}

QString DelCommand::title() const
{
    return "Del";
}

bool DelCommand::execute()
{
 
  return false;
}
