#include "line_drawing_tool.h"
#include <QDebug>

namespace WallExtraction {

LineDrawingTool::LineDrawingTool(QWidget* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_parentWidget(parent)
{
    qDebug() << "LineDrawingTool created (placeholder)";
}

LineDrawingTool::~LineDrawingTool()
{
    qDebug() << "LineDrawingTool destroyed";
}

bool LineDrawingTool::initialize()
{
    m_initialized = true;
    qDebug() << "LineDrawingTool initialized (placeholder)";
    return true;
}

bool LineDrawingTool::isInitialized() const
{
    return m_initialized;
}

} // namespace WallExtraction
