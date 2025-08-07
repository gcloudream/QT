#ifndef UI_INTEGRATION_HELPER_H
#define UI_INTEGRATION_HELPER_H

#include <QObject>
#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <memory>

namespace WallExtraction {

// 前向声明
class WallExtractionManager;
class WallFittingProgressDialog;
class WallFittingResultDialog;
struct WallFittingResult;

/**
 * @brief UI集成助手类
 * 
 * 负责将墙面提取功能集成到主界面中，包括：
 * - 菜单和工具栏集成
 * - 进度显示管理
 * - 结果显示管理
 * - 状态栏更新
 * - 快捷键支持
 */
class UIIntegrationHelper : public QObject
{
    Q_OBJECT

public:
    explicit UIIntegrationHelper(QWidget* mainWindow, QObject* parent = nullptr);
    ~UIIntegrationHelper();

    // 初始化和设置
    bool initialize();
    void setWallExtractionManager(WallExtractionManager* manager);
    
    // UI组件集成
    void integrateMenus(QMenu* parentMenu);
    void integrateToolBar(QToolBar* toolBar);
    void integrateStatusBar(QStatusBar* statusBar);
    
    // 进度管理
    void showProgressDialog();
    void hideProgressDialog();
    bool isProgressDialogVisible() const;
    
    // 结果显示
    void showResultDialog(const WallFittingResult& result);
    void hideResultDialog();
    bool isResultDialogVisible() const;
    
    // 状态管理
    void updateStatus(const QString& message);
    void updateProgress(int percentage, const QString& status);
    void clearStatus();
    
    // 快捷键支持
    void setupShortcuts();
    void enableShortcuts(bool enabled);

public slots:
    // 墙面拟合操作
    void startLineBasedWallFitting();
    void startAutoWallFitting();
    void cancelWallFitting();
    void clearAllData();
    
    // 数据操作
    void exportWallData();
    void importWallData();
    
    // 显示操作
    void showWallFittingHelp();
    void showAboutDialog();

signals:
    // 操作信号
    void lineBasedWallFittingRequested();
    void autoWallFittingRequested();
    void wallFittingCancelled();
    void dataCleared();
    
    // 数据信号
    void wallDataExported(const QString& filename);
    void wallDataImported(const QString& filename);
    
    // UI状态信号
    void progressDialogShown();
    void progressDialogHidden();
    void resultDialogShown();
    void resultDialogHidden();

private slots:
    // 墙面提取管理器信号处理
    void onWallFittingStarted();
    void onWallFittingCompleted(const WallFittingResult& result);
    void onWallFittingFailed(const QString& error);
    void onWallFittingProgress(int percentage, const QString& status);
    
    // 进度对话框信号处理
    void onProgressDialogCancelled();
    
    // 结果对话框信号处理
    void onResultExported(const QString& filename);
    void onWallSelected(int wallId);
    void onPlaneSelected(int planeId);
    
    // 状态栏更新
    void updateStatusBarProgress();

private:
    void setupActions();
    void setupConnections();
    void createProgressIndicator();
    void updateActionStates();
    
    // 样式设置
    void applyStyles();
    QString getActionStyle() const;
    QString getProgressStyle() const;

private:
    // 主窗口引用
    QWidget* m_mainWindow;
    
    // 核心组件
    WallExtractionManager* m_wallExtractionManager;
    
    // UI对话框
    std::unique_ptr<WallFittingProgressDialog> m_progressDialog;
    std::unique_ptr<WallFittingResultDialog> m_resultDialog;
    
    // UI组件
    QMenu* m_wallExtractionMenu;
    QToolBar* m_wallExtractionToolBar;
    QStatusBar* m_statusBar;
    
    // 状态栏组件
    QLabel* m_statusLabel;
    QProgressBar* m_statusProgressBar;
    QLabel* m_pointCountLabel;
    QLabel* m_wallCountLabel;
    
    // 动作
    QAction* m_lineBasedFittingAction;
    QAction* m_autoFittingAction;
    QAction* m_cancelFittingAction;
    QAction* m_clearDataAction;
    QAction* m_exportDataAction;
    QAction* m_importDataAction;
    QAction* m_showHelpAction;
    QAction* m_aboutAction;
    
    // 状态管理
    bool m_initialized;
    bool m_processingActive;
    
    // 进度更新定时器
    QTimer* m_statusUpdateTimer;
    
    // 常量
    static const int STATUS_UPDATE_INTERVAL = 500; // 毫秒
    static const int PROGRESS_BAR_WIDTH = 200;
};

} // namespace WallExtraction

#endif // UI_INTEGRATION_HELPER_H
