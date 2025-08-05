#include "wall_fitting_algorithm.h"
#include <QDebug>

namespace WallExtraction {

WallFittingAlgorithm::WallFittingAlgorithm(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    qDebug() << "WallFittingAlgorithm created (placeholder)";
}

WallFittingAlgorithm::~WallFittingAlgorithm()
{
    qDebug() << "WallFittingAlgorithm destroyed";
}

bool WallFittingAlgorithm::initialize()
{
    m_initialized = true;
    qDebug() << "WallFittingAlgorithm initialized (placeholder)";
    return true;
}

bool WallFittingAlgorithm::isInitialized() const
{
    return m_initialized;
}

} // namespace WallExtraction
