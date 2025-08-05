#include "wireframe_generator.h"
#include <QDebug>

namespace WallExtraction {

WireframeGenerator::WireframeGenerator(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    qDebug() << "WireframeGenerator created (placeholder)";
}

WireframeGenerator::~WireframeGenerator()
{
    qDebug() << "WireframeGenerator destroyed";
}

bool WireframeGenerator::initialize()
{
    m_initialized = true;
    qDebug() << "WireframeGenerator initialized (placeholder)";
    return true;
}

bool WireframeGenerator::isInitialized() const
{
    return m_initialized;
}

} // namespace WallExtraction
