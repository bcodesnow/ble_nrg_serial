#include "catchcontroller.h"

CatchController::CatchController(QObject *parent) : QObject(parent)
{

}

bool CatchController::sdEnabled() const
{
    return m_sdEnabled;
}
