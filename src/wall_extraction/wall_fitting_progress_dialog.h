#ifndef WALL_FITTING_PROGRESS_DIALOG_H
#define WALL_FITTING_PROGRESS_DIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QElapsedTimer>
#include <QTextEdit>

namespace WallExtraction {

/**
 * @brief 墙面拟合进度对话框
 * 
 * 显示墙面拟合算法的执行进度，包括：
 * - 进度条显示
 * - 状态文本更新
 * - 取消操作支持
 * - 详细日志显示
 * - 时间估算
 */
class WallFittingProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WallFittingProgressDialog(QWidget* parent = nullptr);
    ~WallFittingProgressDialog();

    // 进度控制
    void setProgress(int percentage, const QString& status);
    void setMaximum(int maximum);
    void setValue(int value);
    
    // 状态管理
    void startProgress(const QString& title = "墙面拟合进行中");
    void completeProgress(const QString& message = "墙面拟合完成");
    void failProgress(const QString& error = "墙面拟合失败");
    void resetProgress();
    
    // 日志管理
    void addLogMessage(const QString& message);
    void clearLog();
    void setLogVisible(bool visible);
    
    // 时间估算
    void updateTimeEstimate();
    QString getElapsedTimeString() const;
    QString getEstimatedTimeString() const;
    
    // 取消支持
    bool isCancelled() const;
    void setCancelEnabled(bool enabled);

signals:
    void cancelled();
    void progressUpdated(int percentage);

public slots:
    void onProgressChanged(int percentage, const QString& status);
    void onProcessingStarted();
    void onProcessingCompleted();
    void onProcessingFailed(const QString& error);

private slots:
    void onCancelClicked();
    void onDetailsToggled();
    void updateDisplay();

private:
    void setupUI();
    void setupLayout();
    void setupConnections();
    void updateProgressText();
    void updateTimeDisplay();
    
    // 样式设置
    void applyStyles();
    QString getProgressBarStyle() const;
    QString getButtonStyle(const QString& color) const;

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    
    QLabel* m_titleLabel;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    QLabel* m_percentageLabel;
    QLabel* m_timeLabel;
    
    QPushButton* m_cancelButton;
    QPushButton* m_detailsButton;
    QPushButton* m_closeButton;
    
    QTextEdit* m_logTextEdit;
    
    // 状态变量
    bool m_cancelled;
    bool m_completed;
    bool m_detailsVisible;
    
    int m_currentProgress;
    int m_maxProgress;
    QString m_currentStatus;
    
    // 时间管理
    QElapsedTimer m_elapsedTimer;
    QTimer* m_updateTimer;
    
    // 时间估算
    qint64 m_startTime;
    qint64 m_lastUpdateTime;
    QList<QPair<qint64, int>> m_progressHistory;  // 时间戳和进度对
    
    // 样式常量
    static const int DIALOG_MIN_WIDTH = 400;
    static const int DIALOG_MIN_HEIGHT = 200;
    static const int DIALOG_MAX_HEIGHT = 600;
    static const int UPDATE_INTERVAL = 100;  // 毫秒
};

} // namespace WallExtraction

#endif // WALL_FITTING_PROGRESS_DIALOG_H
