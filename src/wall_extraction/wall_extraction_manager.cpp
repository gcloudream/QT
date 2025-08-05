#include "wall_extraction_manager.h"
#include "line_drawing_tool.h"
#include "wall_fitting_algorithm.h"
#include "wireframe_generator.h"
#include <QDebug>
#include <QMetaType>

namespace WallExtraction {

// WallExtractionException 实现
WallExtractionException::WallExtractionException(const QString& message)
    : m_message(message)
    , m_detailedMessage(QString("WallExtractionException: %1").arg(message))
{
}

const char* WallExtractionException::what() const noexcept
{
    return m_message.toLocal8Bit().constData();
}

QString WallExtractionException::getDetailedMessage() const
{
    return m_detailedMessage;
}

// WallExtractionManager 实现
WallExtractionManager::WallExtractionManager(QWidget* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_active(false)
    , m_currentMode(InteractionMode::PointCloudView)
    , m_parentWidget(parent)
{
    // 注册元类型以支持信号槽
    qRegisterMetaType<InteractionMode>("InteractionMode");
    
    qDebug() << "WallExtractionManager created";
}

WallExtractionManager::~WallExtractionManager()
{
    cleanup();
    qDebug() << "WallExtractionManager destroyed";
}

bool WallExtractionManager::initialize()
{
    if (m_initialized) {
        qWarning() << "WallExtractionManager already initialized";
        return true;
    }

    try {
        // 初始化子组件
        if (!initializeComponents()) {
            qCritical() << "Failed to initialize components";
            return false;
        }

        // 设置信号槽连接
        setupConnections();

        m_initialized = true;
        setStatusMessage("Wall extraction module initialized successfully");
        
        qDebug() << "WallExtractionManager initialized successfully";
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during initialization:" << e.what();
        cleanup();
        return false;
    }
}

bool WallExtractionManager::isInitialized() const
{
    return m_initialized;
}

bool WallExtractionManager::activateModule()
{
    if (!m_initialized) {
        qWarning() << "Cannot activate uninitialized module";
        return false;
    }

    if (m_active) {
        qDebug() << "Module already active";
        return true;
    }

    try {
        m_active = true;
        emit moduleActivationChanged(true);
        setStatusMessage("Wall extraction module activated");
        
        qDebug() << "Wall extraction module activated";
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during module activation:" << e.what();
        m_active = false;
        return false;
    }
}

bool WallExtractionManager::deactivateModule()
{
    if (!m_active) {
        qDebug() << "Module already inactive";
        return true;
    }

    try {
        // 重置到默认模式
        setInteractionMode(InteractionMode::PointCloudView);
        
        m_active = false;
        emit moduleActivationChanged(false);
        setStatusMessage("Wall extraction module deactivated");
        
        qDebug() << "Wall extraction module deactivated";
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during module deactivation:" << e.what();
        return false;
    }
}

bool WallExtractionManager::isActive() const
{
    return m_active;
}

bool WallExtractionManager::setInteractionMode(InteractionMode mode)
{
    if (!m_initialized) {
        qWarning() << "Cannot set interaction mode on uninitialized manager";
        return false;
    }

    if (m_currentMode == mode) {
        qDebug() << "Interaction mode already set to" << static_cast<int>(mode);
        return true;
    }

    try {
        InteractionMode oldMode = m_currentMode;
        m_currentMode = mode;
        
        // 根据模式更新状态消息
        QString modeString;
        switch (mode) {
            case InteractionMode::PointCloudView:
                modeString = "Point Cloud View";
                break;
            case InteractionMode::LineDrawing:
                modeString = "Line Drawing";
                break;
            case InteractionMode::WallExtraction:
                modeString = "Wall Extraction";
                break;
            case InteractionMode::WireframeView:
                modeString = "Wireframe View";
                break;
        }
        
        setStatusMessage(QString("Switched to %1 mode").arg(modeString));
        emit interactionModeChanged(mode);
        
        qDebug() << "Interaction mode changed from" << static_cast<int>(oldMode) 
                 << "to" << static_cast<int>(mode);
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during mode switch:" << e.what();
        return false;
    }
}

InteractionMode WallExtractionManager::getCurrentMode() const
{
    return m_currentMode;
}

void WallExtractionManager::setStatusMessage(const QString& message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged(message);
        qDebug() << "Status message:" << message;
    }
}

LineDrawingTool* WallExtractionManager::getLineDrawingTool() const
{
    return m_lineDrawingTool.get();
}

WallFittingAlgorithm* WallExtractionManager::getWallFittingAlgorithm() const
{
    return m_wallFittingAlgorithm.get();
}

WireframeGenerator* WallExtractionManager::getWireframeGenerator() const
{
    return m_wireframeGenerator.get();
}

void WallExtractionManager::processInvalidOperation()
{
    throw WallExtractionException("Invalid operation requested");
}

void WallExtractionManager::handleComponentError(const QString& error)
{
    QString fullError = QString("Component error: %1").arg(error);
    qCritical() << fullError;
    emit errorOccurred(fullError);
}

bool WallExtractionManager::initializeComponents()
{
    try {
        // 创建线段绘制工具
        m_lineDrawingTool = std::make_unique<LineDrawingTool>(m_parentWidget);
        if (!m_lineDrawingTool) {
            qCritical() << "Failed to create LineDrawingTool";
            return false;
        }

        // 创建墙面拟合算法
        m_wallFittingAlgorithm = std::make_unique<WallFittingAlgorithm>();
        if (!m_wallFittingAlgorithm) {
            qCritical() << "Failed to create WallFittingAlgorithm";
            return false;
        }

        // 创建线框图生成器
        m_wireframeGenerator = std::make_unique<WireframeGenerator>();
        if (!m_wireframeGenerator) {
            qCritical() << "Failed to create WireframeGenerator";
            return false;
        }

        qDebug() << "All components initialized successfully";
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during component initialization:" << e.what();
        return false;
    }
}

void WallExtractionManager::setupConnections()
{
    // 这里将来会连接子组件的信号槽
    // 目前先建立基础框架
    
    qDebug() << "Signal-slot connections established";
}

void WallExtractionManager::cleanup()
{
    if (m_active) {
        deactivateModule();
    }

    // 智能指针会自动清理资源
    m_wireframeGenerator.reset();
    m_wallFittingAlgorithm.reset();
    m_lineDrawingTool.reset();

    m_initialized = false;
    qDebug() << "WallExtractionManager cleaned up";
}

bool WallExtractionManager::validateState() const
{
    return m_initialized && m_parentWidget != nullptr;
}

} // namespace WallExtraction
