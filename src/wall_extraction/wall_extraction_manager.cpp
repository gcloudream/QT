#include "wall_extraction_manager.h"
#include "line_drawing_tool.h"
#include "wall_fitting_algorithm.h"
#include "wireframe_generator.h"
#include <QDebug>
#include <QMetaType>
#include <QVector3D>

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
    , m_isProcessing(false)
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

bool WallExtractionManager::performLineBasedWallFitting(const std::vector<QVector3D>& pointCloud)
{
    if (!m_initialized) {
        qWarning() << "Cannot perform wall fitting on uninitialized manager";
        return false;
    }

    if (m_isProcessing) {
        qWarning() << "Wall fitting already in progress";
        return false;
    }

    if (pointCloud.empty()) {
        qWarning() << "Cannot perform wall fitting with empty point cloud";
        return false;
    }

    try {
        m_isProcessing = true;
        m_currentPointCloud = pointCloud;

        emit wallFittingStarted();
        setStatusMessage("开始基于线段的墙面拟合");

        // 获取用户绘制的线段
        const auto& userLines = m_lineDrawingTool->getLineSegments();
        if (userLines.empty()) {
            m_isProcessing = false;
            QString error = "没有可用的用户绘制线段";
            emit wallFittingFailed(error);
            return false;
        }

        qDebug() << "Starting line-based wall fitting with" << userLines.size()
                 << "user lines and" << pointCloud.size() << "points";

        // 执行基于线段的墙面拟合
        m_lastWallFittingResult = m_wallFittingAlgorithm->fitWallsFromLines(pointCloud, userLines);

        m_isProcessing = false;

        if (m_lastWallFittingResult.success) {
            emit wallFittingCompleted(m_lastWallFittingResult);
            setStatusMessage(QString("墙面拟合完成：提取到 %1 个墙面")
                           .arg(m_lastWallFittingResult.walls.size()));
            qDebug() << "Line-based wall fitting completed successfully";
            return true;
        } else {
            emit wallFittingFailed(m_lastWallFittingResult.errorMessage);
            setStatusMessage("墙面拟合失败");
            return false;
        }

    } catch (const std::exception& e) {
        m_isProcessing = false;
        QString error = QString("墙面拟合过程中发生异常: %1").arg(e.what());
        qCritical() << error;
        emit wallFittingFailed(error);
        return false;
    }
}

bool WallExtractionManager::performAutoWallFitting(const std::vector<QVector3D>& pointCloud)
{
    if (!m_initialized) {
        qWarning() << "Cannot perform wall fitting on uninitialized manager";
        return false;
    }

    if (m_isProcessing) {
        qWarning() << "Wall fitting already in progress";
        return false;
    }

    if (pointCloud.empty()) {
        qWarning() << "Cannot perform wall fitting with empty point cloud";
        return false;
    }

    try {
        m_isProcessing = true;
        m_currentPointCloud = pointCloud;

        emit wallFittingStarted();
        setStatusMessage("开始自动墙面拟合");

        qDebug() << "Starting automatic wall fitting with" << pointCloud.size() << "points";

        // 执行自动墙面拟合
        m_lastWallFittingResult = m_wallFittingAlgorithm->fitWallsFromPointCloud(pointCloud);

        m_isProcessing = false;

        if (m_lastWallFittingResult.success) {
            emit wallFittingCompleted(m_lastWallFittingResult);
            setStatusMessage(QString("自动墙面拟合完成：提取到 %1 个墙面")
                           .arg(m_lastWallFittingResult.walls.size()));
            qDebug() << "Automatic wall fitting completed successfully";
            return true;
        } else {
            emit wallFittingFailed(m_lastWallFittingResult.errorMessage);
            setStatusMessage("自动墙面拟合失败");
            return false;
        }

    } catch (const std::exception& e) {
        m_isProcessing = false;
        QString error = QString("自动墙面拟合过程中发生异常: %1").arg(e.what());
        qCritical() << error;
        emit wallFittingFailed(error);
        return false;
    }
}

WallFittingResult WallExtractionManager::getLastWallFittingResult() const
{
    return m_lastWallFittingResult;
}

void WallExtractionManager::clearAllData()
{
    if (m_isProcessing) {
        qWarning() << "Cannot clear data while processing";
        return;
    }

    try {
        // 清除线段绘制工具的数据
        if (m_lineDrawingTool) {
            m_lineDrawingTool->clearAll();
        }

        // 清除墙面拟合算法的数据
        if (m_wallFittingAlgorithm) {
            m_wallFittingAlgorithm->reset();
        }

        // 清除缓存数据
        m_currentPointCloud.clear();
        m_lastWallFittingResult = WallFittingResult();

        setStatusMessage("所有数据已清除");
        qDebug() << "All data cleared";

    } catch (const std::exception& e) {
        qCritical() << "Exception during data clearing:" << e.what();
        emit errorOccurred(QString("清除数据时发生错误: %1").arg(e.what()));
    }
}

bool WallExtractionManager::exportWallData(const QString& filename) const
{
    if (!m_initialized) {
        qWarning() << "Cannot export data from uninitialized manager";
        return false;
    }

    try {
        // 导出线段数据
        if (m_lineDrawingTool && !m_lineDrawingTool->saveToFile(filename + "_lines.json")) {
            qWarning() << "Failed to export line data";
            return false;
        }

        // 导出墙面数据（如果有的话）
        if (m_lastWallFittingResult.success && !m_lastWallFittingResult.walls.empty()) {
            // 这里可以添加墙面数据的导出逻辑
            qDebug() << "Wall data export not yet implemented";
        }

        qDebug() << "Data exported to" << filename;
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during data export:" << e.what();
        return false;
    }
}

bool WallExtractionManager::importWallData(const QString& filename)
{
    if (!m_initialized) {
        qWarning() << "Cannot import data to uninitialized manager";
        return false;
    }

    if (m_isProcessing) {
        qWarning() << "Cannot import data while processing";
        return false;
    }

    try {
        // 导入线段数据
        if (m_lineDrawingTool && !m_lineDrawingTool->loadFromFile(filename + "_lines.json")) {
            qWarning() << "Failed to import line data";
            return false;
        }

        setStatusMessage("数据导入完成");
        qDebug() << "Data imported from" << filename;
        return true;

    } catch (const std::exception& e) {
        qCritical() << "Exception during data import:" << e.what();
        emit errorOccurred(QString("导入数据时发生错误: %1").arg(e.what()));
        return false;
    }
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
    // 连接线段绘制工具的信号
    if (m_lineDrawingTool) {
        connect(m_lineDrawingTool.get(), &LineDrawingTool::errorOccurred,
                this, &WallExtractionManager::handleComponentError);

        connect(m_lineDrawingTool.get(), &LineDrawingTool::operationCompleted,
                this, [this](const QString& operation) {
                    setStatusMessage(QString("线段操作完成: %1").arg(operation));
                });
    }

    // 连接墙面拟合算法的信号
    if (m_wallFittingAlgorithm) {
        connect(m_wallFittingAlgorithm.get(), &WallFittingAlgorithm::errorOccurred,
                this, &WallExtractionManager::handleComponentError);

        connect(m_wallFittingAlgorithm.get(), &WallFittingAlgorithm::progressChanged,
                this, &WallExtractionManager::wallFittingProgress);

        connect(m_wallFittingAlgorithm.get(), &WallFittingAlgorithm::processingStarted,
                this, &WallExtractionManager::wallFittingStarted);

        connect(m_wallFittingAlgorithm.get(), &WallFittingAlgorithm::processingCompleted,
                this, [this](const WallFittingResult& result) {
                    m_lastWallFittingResult = result;
                    emit wallFittingCompleted(result);
                });

        connect(m_wallFittingAlgorithm.get(), &WallFittingAlgorithm::processingFailed,
                this, &WallExtractionManager::wallFittingFailed);
    }

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
