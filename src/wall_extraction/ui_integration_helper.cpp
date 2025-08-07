#include "ui_integration_helper.h"
#include "wall_extraction_manager.h"
#include "wall_fitting_progress_dialog.h"
#include "wall_fitting_result_dialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeySequence>
#include <QShortcut>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDebug>

namespace WallExtraction {

UIIntegrationHelper::UIIntegrationHelper(QWidget* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_wallExtractionManager(nullptr)
    , m_progressDialog(nullptr)
    , m_resultDialog(nullptr)
    , m_wallExtractionMenu(nullptr)
    , m_wallExtractionToolBar(nullptr)
    , m_statusBar(nullptr)
    , m_statusLabel(nullptr)
    , m_statusProgressBar(nullptr)
    , m_pointCountLabel(nullptr)
    , m_wallCountLabel(nullptr)
    , m_lineBasedFittingAction(nullptr)
    , m_autoFittingAction(nullptr)
    , m_cancelFittingAction(nullptr)
    , m_clearDataAction(nullptr)
    , m_exportDataAction(nullptr)
    , m_importDataAction(nullptr)
    , m_showHelpAction(nullptr)
    , m_aboutAction(nullptr)
    , m_initialized(false)
    , m_processingActive(false)
    , m_statusUpdateTimer(nullptr)
{
    qDebug() << "UIIntegrationHelper created";
}

UIIntegrationHelper::~UIIntegrationHelper()
{
    if (m_statusUpdateTimer) {
        m_statusUpdateTimer->stop();
        delete m_statusUpdateTimer;
    }
    qDebug() << "UIIntegrationHelper destroyed";
}

bool UIIntegrationHelper::initialize()
{
    if (m_initialized) {
        qWarning() << "UIIntegrationHelper already initialized";
        return true;
    }
    
    try {
        setupActions();
        createProgressIndicator();
        setupShortcuts();
        applyStyles();
        
        m_initialized = true;
        qDebug() << "UIIntegrationHelper initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Failed to initialize UIIntegrationHelper:" << e.what();
        return false;
    }
}

void UIIntegrationHelper::setWallExtractionManager(WallExtractionManager* manager)
{
    if (m_wallExtractionManager) {
        // 断开之前的连接
        disconnect(m_wallExtractionManager, nullptr, this, nullptr);
    }
    
    m_wallExtractionManager = manager;
    
    if (m_wallExtractionManager) {
        setupConnections();
        qDebug() << "WallExtractionManager set and connected";
    }
}

void UIIntegrationHelper::setupActions()
{
    // 基于线段的墙面拟合
    m_lineBasedFittingAction = new QAction("基于线段的墙面拟合", this);
    m_lineBasedFittingAction->setIcon(QIcon(":/icons/line_fitting.png"));
    m_lineBasedFittingAction->setShortcut(QKeySequence("Ctrl+L"));
    m_lineBasedFittingAction->setStatusTip("基于用户绘制线段进行墙面拟合");
    m_lineBasedFittingAction->setToolTip("基于线段的墙面拟合 (Ctrl+L)");
    
    // 自动墙面拟合
    m_autoFittingAction = new QAction("自动墙面拟合", this);
    m_autoFittingAction->setIcon(QIcon(":/icons/auto_fitting.png"));
    m_autoFittingAction->setShortcut(QKeySequence("Ctrl+A"));
    m_autoFittingAction->setStatusTip("基于点云自动进行墙面拟合");
    m_autoFittingAction->setToolTip("自动墙面拟合 (Ctrl+A)");
    
    // 取消拟合
    m_cancelFittingAction = new QAction("取消拟合", this);
    m_cancelFittingAction->setIcon(QIcon(":/icons/cancel.png"));
    m_cancelFittingAction->setShortcut(QKeySequence("Escape"));
    m_cancelFittingAction->setStatusTip("取消当前的墙面拟合操作");
    m_cancelFittingAction->setToolTip("取消拟合 (Esc)");
    m_cancelFittingAction->setEnabled(false);
    
    // 清除数据
    m_clearDataAction = new QAction("清除所有数据", this);
    m_clearDataAction->setIcon(QIcon(":/icons/clear.png"));
    m_clearDataAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    m_clearDataAction->setStatusTip("清除所有线段和墙面数据");
    m_clearDataAction->setToolTip("清除所有数据 (Ctrl+Shift+C)");
    
    // 导出数据
    m_exportDataAction = new QAction("导出墙面数据", this);
    m_exportDataAction->setIcon(QIcon(":/icons/export.png"));
    m_exportDataAction->setShortcut(QKeySequence("Ctrl+E"));
    m_exportDataAction->setStatusTip("导出墙面拟合结果");
    m_exportDataAction->setToolTip("导出墙面数据 (Ctrl+E)");
    
    // 导入数据
    m_importDataAction = new QAction("导入墙面数据", this);
    m_importDataAction->setIcon(QIcon(":/icons/import.png"));
    m_importDataAction->setShortcut(QKeySequence("Ctrl+I"));
    m_importDataAction->setStatusTip("导入墙面数据");
    m_importDataAction->setToolTip("导入墙面数据 (Ctrl+I)");
    
    // 帮助
    m_showHelpAction = new QAction("墙面拟合帮助", this);
    m_showHelpAction->setIcon(QIcon(":/icons/help.png"));
    m_showHelpAction->setShortcut(QKeySequence("F1"));
    m_showHelpAction->setStatusTip("显示墙面拟合功能帮助");
    m_showHelpAction->setToolTip("帮助 (F1)");
    
    // 关于
    m_aboutAction = new QAction("关于墙面拟合", this);
    m_aboutAction->setIcon(QIcon(":/icons/about.png"));
    m_aboutAction->setStatusTip("关于墙面拟合功能");
    m_aboutAction->setToolTip("关于");
    
    // 连接动作信号
    connect(m_lineBasedFittingAction, &QAction::triggered, this, &UIIntegrationHelper::startLineBasedWallFitting);
    connect(m_autoFittingAction, &QAction::triggered, this, &UIIntegrationHelper::startAutoWallFitting);
    connect(m_cancelFittingAction, &QAction::triggered, this, &UIIntegrationHelper::cancelWallFitting);
    connect(m_clearDataAction, &QAction::triggered, this, &UIIntegrationHelper::clearAllData);
    connect(m_exportDataAction, &QAction::triggered, this, &UIIntegrationHelper::exportWallData);
    connect(m_importDataAction, &QAction::triggered, this, &UIIntegrationHelper::importWallData);
    connect(m_showHelpAction, &QAction::triggered, this, &UIIntegrationHelper::showWallFittingHelp);
    connect(m_aboutAction, &QAction::triggered, this, &UIIntegrationHelper::showAboutDialog);
}

void UIIntegrationHelper::integrateMenus(QMenu* parentMenu)
{
    if (!parentMenu || !m_initialized) {
        qWarning() << "Cannot integrate menus: invalid parent menu or not initialized";
        return;
    }
    
    // 创建墙面提取子菜单
    m_wallExtractionMenu = parentMenu->addMenu("墙面提取");
    m_wallExtractionMenu->setIcon(QIcon(":/icons/wall_extraction.png"));
    
    // 添加动作到菜单
    m_wallExtractionMenu->addAction(m_lineBasedFittingAction);
    m_wallExtractionMenu->addAction(m_autoFittingAction);
    m_wallExtractionMenu->addSeparator();
    m_wallExtractionMenu->addAction(m_cancelFittingAction);
    m_wallExtractionMenu->addSeparator();
    m_wallExtractionMenu->addAction(m_clearDataAction);
    m_wallExtractionMenu->addSeparator();
    m_wallExtractionMenu->addAction(m_exportDataAction);
    m_wallExtractionMenu->addAction(m_importDataAction);
    m_wallExtractionMenu->addSeparator();
    m_wallExtractionMenu->addAction(m_showHelpAction);
    m_wallExtractionMenu->addAction(m_aboutAction);
    
    qDebug() << "Menus integrated successfully";
}

void UIIntegrationHelper::integrateToolBar(QToolBar* toolBar)
{
    if (!toolBar || !m_initialized) {
        qWarning() << "Cannot integrate toolbar: invalid toolbar or not initialized";
        return;
    }
    
    m_wallExtractionToolBar = toolBar;
    
    // 添加动作到工具栏
    toolBar->addAction(m_lineBasedFittingAction);
    toolBar->addAction(m_autoFittingAction);
    toolBar->addAction(m_cancelFittingAction);
    toolBar->addSeparator();
    toolBar->addAction(m_clearDataAction);
    toolBar->addSeparator();
    toolBar->addAction(m_exportDataAction);
    toolBar->addAction(m_importDataAction);
    
    qDebug() << "Toolbar integrated successfully";
}

void UIIntegrationHelper::integrateStatusBar(QStatusBar* statusBar)
{
    if (!statusBar || !m_initialized) {
        qWarning() << "Cannot integrate status bar: invalid status bar or not initialized";
        return;
    }
    
    m_statusBar = statusBar;
    
    // 添加状态栏组件
    m_statusLabel = new QLabel("就绪", statusBar);
    m_statusProgressBar = new QProgressBar(statusBar);
    m_statusProgressBar->setVisible(false);
    m_statusProgressBar->setMaximumWidth(PROGRESS_BAR_WIDTH);
    
    m_pointCountLabel = new QLabel("点数: 0", statusBar);
    m_wallCountLabel = new QLabel("墙面: 0", statusBar);
    
    statusBar->addWidget(m_statusLabel, 1);
    statusBar->addPermanentWidget(m_statusProgressBar);
    statusBar->addPermanentWidget(m_pointCountLabel);
    statusBar->addPermanentWidget(m_wallCountLabel);
    
    qDebug() << "Status bar integrated successfully";
}

void UIIntegrationHelper::createProgressIndicator()
{
    // 创建状态更新定时器
    m_statusUpdateTimer = new QTimer(this);
    m_statusUpdateTimer->setInterval(STATUS_UPDATE_INTERVAL);
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &UIIntegrationHelper::updateStatusBarProgress);
}

void UIIntegrationHelper::setupShortcuts()
{
    // 快捷键已在setupActions中设置
    qDebug() << "Shortcuts set up";
}

void UIIntegrationHelper::setupConnections()
{
    if (!m_wallExtractionManager) {
        qWarning() << "Cannot setup connections: no WallExtractionManager";
        return;
    }
    
    // 连接墙面提取管理器的信号
    connect(m_wallExtractionManager, &WallExtractionManager::wallFittingStarted,
            this, &UIIntegrationHelper::onWallFittingStarted);
    
    connect(m_wallExtractionManager, &WallExtractionManager::wallFittingCompleted,
            this, &UIIntegrationHelper::onWallFittingCompleted);
    
    connect(m_wallExtractionManager, &WallExtractionManager::wallFittingFailed,
            this, &UIIntegrationHelper::onWallFittingFailed);
    
    connect(m_wallExtractionManager, &WallExtractionManager::wallFittingProgress,
            this, &UIIntegrationHelper::onWallFittingProgress);
    
    qDebug() << "Connections established";
}

void UIIntegrationHelper::showProgressDialog()
{
    if (!m_progressDialog) {
        m_progressDialog = std::make_unique<WallFittingProgressDialog>(m_mainWindow);
        connect(m_progressDialog.get(), &WallFittingProgressDialog::cancelled,
                this, &UIIntegrationHelper::onProgressDialogCancelled);
    }
    
    m_progressDialog->show();
    m_progressDialog->raise();
    m_progressDialog->activateWindow();
    
    emit progressDialogShown();
    qDebug() << "Progress dialog shown";
}

void UIIntegrationHelper::hideProgressDialog()
{
    if (m_progressDialog && m_progressDialog->isVisible()) {
        m_progressDialog->hide();
        emit progressDialogHidden();
        qDebug() << "Progress dialog hidden";
    }
}

void UIIntegrationHelper::showResultDialog(const WallFittingResult& result)
{
    if (!m_resultDialog) {
        m_resultDialog = std::make_unique<WallFittingResultDialog>(result, m_mainWindow);
        connect(m_resultDialog.get(), &WallFittingResultDialog::resultExported,
                this, &UIIntegrationHelper::onResultExported);
        connect(m_resultDialog.get(), &WallFittingResultDialog::wallSelected,
                this, &UIIntegrationHelper::onWallSelected);
        connect(m_resultDialog.get(), &WallFittingResultDialog::planeSelected,
                this, &UIIntegrationHelper::onPlaneSelected);
    } else {
        m_resultDialog->setResult(result);
    }
    
    m_resultDialog->show();
    m_resultDialog->raise();
    m_resultDialog->activateWindow();
    
    emit resultDialogShown();
    qDebug() << "Result dialog shown with" << result.walls.size() << "walls";
}

void UIIntegrationHelper::updateStatus(const QString& message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
    qDebug() << "Status updated:" << message;
}

void UIIntegrationHelper::updateProgress(int percentage, const QString& status)
{
    if (m_statusProgressBar) {
        m_statusProgressBar->setValue(percentage);
        m_statusProgressBar->setVisible(percentage > 0 && percentage < 100);
    }
    
    if (m_progressDialog) {
        m_progressDialog->setProgress(percentage, status);
    }
    
    updateStatus(status);
}

void UIIntegrationHelper::startLineBasedWallFitting()
{
    if (!m_wallExtractionManager) {
        QMessageBox::warning(m_mainWindow, "错误", "墙面提取管理器未初始化");
        return;
    }
    
    emit lineBasedWallFittingRequested();
    qDebug() << "Line-based wall fitting requested";
}

void UIIntegrationHelper::startAutoWallFitting()
{
    if (!m_wallExtractionManager) {
        QMessageBox::warning(m_mainWindow, "错误", "墙面提取管理器未初始化");
        return;
    }
    
    emit autoWallFittingRequested();
    qDebug() << "Auto wall fitting requested";
}

void UIIntegrationHelper::onWallFittingStarted()
{
    m_processingActive = true;
    updateActionStates();
    showProgressDialog();
    
    if (m_progressDialog) {
        m_progressDialog->startProgress("墙面拟合进行中");
    }
    
    m_statusUpdateTimer->start();
    updateStatus("墙面拟合开始...");
}

void UIIntegrationHelper::onWallFittingCompleted(const WallFittingResult& result)
{
    m_processingActive = false;
    updateActionStates();
    
    m_statusUpdateTimer->stop();
    
    if (m_progressDialog) {
        m_progressDialog->completeProgress("墙面拟合完成");
    }
    
    // 显示结果对话框
    showResultDialog(result);
    
    // 更新状态栏
    updateStatus(QString("墙面拟合完成：提取到 %1 个墙面").arg(result.walls.size()));
    
    if (m_wallCountLabel) {
        m_wallCountLabel->setText(QString("墙面: %1").arg(result.walls.size()));
    }
    
    // 隐藏进度条
    if (m_statusProgressBar) {
        m_statusProgressBar->setVisible(false);
    }
}

void UIIntegrationHelper::onWallFittingFailed(const QString& error)
{
    m_processingActive = false;
    updateActionStates();
    
    m_statusUpdateTimer->stop();
    
    if (m_progressDialog) {
        m_progressDialog->failProgress(error);
    }
    
    updateStatus(QString("墙面拟合失败: %1").arg(error));
    
    // 隐藏进度条
    if (m_statusProgressBar) {
        m_statusProgressBar->setVisible(false);
    }
    
    QMessageBox::critical(m_mainWindow, "墙面拟合失败", error);
}

void UIIntegrationHelper::onWallFittingProgress(int percentage, const QString& status)
{
    updateProgress(percentage, status);
}

void UIIntegrationHelper::updateActionStates()
{
    bool canStart = !m_processingActive;
    bool canCancel = m_processingActive;
    
    if (m_lineBasedFittingAction) m_lineBasedFittingAction->setEnabled(canStart);
    if (m_autoFittingAction) m_autoFittingAction->setEnabled(canStart);
    if (m_cancelFittingAction) m_cancelFittingAction->setEnabled(canCancel);
    if (m_clearDataAction) m_clearDataAction->setEnabled(canStart);
    if (m_exportDataAction) m_exportDataAction->setEnabled(canStart);
    if (m_importDataAction) m_importDataAction->setEnabled(canStart);
}

void UIIntegrationHelper::applyStyles()
{
    // 样式应用将在后续实现
}

QString UIIntegrationHelper::getActionStyle() const
{
    return QString();
}

QString UIIntegrationHelper::getProgressStyle() const
{
    return QString();
}

bool UIIntegrationHelper::isProgressDialogVisible() const
{
    return m_progressDialog && m_progressDialog->isVisible();
}

bool UIIntegrationHelper::isResultDialogVisible() const
{
    return m_resultDialog && m_resultDialog->isVisible();
}

void UIIntegrationHelper::clearStatus()
{
    updateStatus("就绪");
}

void UIIntegrationHelper::enableShortcuts(bool enabled)
{
    Q_UNUSED(enabled)
    // 快捷键启用/禁用逻辑
}

void UIIntegrationHelper::cancelWallFitting()
{
    emit wallFittingCancelled();
    qDebug() << "Wall fitting cancellation requested";
}

void UIIntegrationHelper::clearAllData()
{
    if (QMessageBox::question(m_mainWindow, "确认", "确定要清除所有数据吗？") == QMessageBox::Yes) {
        emit dataCleared();
        updateStatus("数据已清除");
        qDebug() << "Data clearing requested";
    }
}

void UIIntegrationHelper::exportWallData()
{
    QString fileName = QFileDialog::getSaveFileName(
        m_mainWindow,
        "导出墙面数据",
        QString("wall_data_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JSON Files (*.json);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        emit wallDataExported(fileName);
        qDebug() << "Wall data export requested:" << fileName;
    }
}

void UIIntegrationHelper::importWallData()
{
    QString fileName = QFileDialog::getOpenFileName(
        m_mainWindow,
        "导入墙面数据",
        "",
        "JSON Files (*.json);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        emit wallDataImported(fileName);
        qDebug() << "Wall data import requested:" << fileName;
    }
}

void UIIntegrationHelper::showWallFittingHelp()
{
    QMessageBox::information(m_mainWindow, "墙面拟合帮助", 
        "墙面拟合功能帮助:\n\n"
        "1. 基于线段的墙面拟合 (Ctrl+L):\n"
        "   - 首先使用线段绘制工具绘制墙面轮廓\n"
        "   - 然后执行基于线段的墙面拟合\n\n"
        "2. 自动墙面拟合 (Ctrl+A):\n"
        "   - 直接基于点云数据自动检测墙面\n"
        "   - 无需手动绘制线段\n\n"
        "3. 其他功能:\n"
        "   - Esc: 取消当前操作\n"
        "   - Ctrl+Shift+C: 清除所有数据\n"
        "   - Ctrl+E: 导出结果\n"
        "   - Ctrl+I: 导入数据");
}

void UIIntegrationHelper::showAboutDialog()
{
    QMessageBox::about(m_mainWindow, "关于墙面拟合",
        "墙面拟合功能 v1.0\n\n"
        "基于RANSAC算法的墙面检测和拟合工具\n"
        "支持基于用户线段和自动检测两种模式\n\n"
        "开发团队: AI Assistant");
}

void UIIntegrationHelper::onProgressDialogCancelled()
{
    cancelWallFitting();
}

void UIIntegrationHelper::onResultExported(const QString& filename)
{
    updateStatus(QString("结果已导出到: %1").arg(filename));
}

void UIIntegrationHelper::onWallSelected(int wallId)
{
    qDebug() << "Wall selected:" << wallId;
    // 这里可以添加墙面高亮显示逻辑
}

void UIIntegrationHelper::onPlaneSelected(int planeId)
{
    qDebug() << "Plane selected:" << planeId;
    // 这里可以添加平面高亮显示逻辑
}

void UIIntegrationHelper::updateStatusBarProgress()
{
    // 定期更新状态栏进度显示
    if (m_processingActive && m_statusProgressBar && m_statusProgressBar->isVisible()) {
        // 可以在这里添加更详细的进度更新逻辑
    }
}

void UIIntegrationHelper::hideResultDialog()
{
    if (m_resultDialog && m_resultDialog->isVisible()) {
        m_resultDialog->hide();
        emit resultDialogHidden();
        qDebug() << "Result dialog hidden";
    }
}

} // namespace WallExtraction
