#include "wall_fitting_progress_dialog.h"
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QDebug>

namespace WallExtraction {

WallFittingProgressDialog::WallFittingProgressDialog(QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_percentageLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_cancelButton(nullptr)
    , m_detailsButton(nullptr)
    , m_closeButton(nullptr)
    , m_logTextEdit(nullptr)
    , m_cancelled(false)
    , m_completed(false)
    , m_detailsVisible(false)
    , m_currentProgress(0)
    , m_maxProgress(100)
    , m_currentStatus("准备中...")
    , m_updateTimer(nullptr)
    , m_startTime(0)
    , m_lastUpdateTime(0)
{
    setupUI();
    setupLayout();
    setupConnections();
    applyStyles();
    
    // 设置对话框属性
    setWindowTitle("墙面拟合进度");
    setModal(true);
    setMinimumSize(DIALOG_MIN_WIDTH, DIALOG_MIN_HEIGHT);
    
    // 居中显示
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
    
    qDebug() << "WallFittingProgressDialog created";
}

WallFittingProgressDialog::~WallFittingProgressDialog()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
    }
    qDebug() << "WallFittingProgressDialog destroyed";
}

void WallFittingProgressDialog::setupUI()
{
    // 创建主要组件
    m_titleLabel = new QLabel("墙面拟合进行中", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);
    
    m_statusLabel = new QLabel(m_currentStatus, this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, m_maxProgress);
    m_progressBar->setValue(m_currentProgress);
    m_progressBar->setTextVisible(false);
    
    m_percentageLabel = new QLabel("0%", this);
    m_percentageLabel->setAlignment(Qt::AlignCenter);
    
    m_timeLabel = new QLabel("预计时间: --", this);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    
    // 按钮
    m_cancelButton = new QPushButton("取消", this);
    m_detailsButton = new QPushButton("详细信息", this);
    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setVisible(false);
    
    // 日志文本框
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setVisible(false);
    m_logTextEdit->setMaximumHeight(200);
    m_logTextEdit->setReadOnly(true);
    
    // 更新定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(UPDATE_INTERVAL);
}

void WallFittingProgressDialog::setupLayout()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 添加组件到主布局
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_percentageLabel);
    m_mainLayout->addWidget(m_timeLabel);
    
    // 按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addWidget(m_detailsButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_logTextEdit);
    
    setLayout(m_mainLayout);
}

void WallFittingProgressDialog::setupConnections()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &WallFittingProgressDialog::onCancelClicked);
    connect(m_detailsButton, &QPushButton::clicked, this, &WallFittingProgressDialog::onDetailsToggled);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_updateTimer, &QTimer::timeout, this, &WallFittingProgressDialog::updateDisplay);
}

void WallFittingProgressDialog::setProgress(int percentage, const QString& status)
{
    m_currentProgress = qBound(0, percentage, m_maxProgress);
    m_currentStatus = status;
    
    // 记录进度历史用于时间估算
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    m_progressHistory.append(qMakePair(currentTime, m_currentProgress));
    
    // 保持历史记录在合理范围内
    if (m_progressHistory.size() > 10) {
        m_progressHistory.removeFirst();
    }
    
    updateProgressText();
    updateTimeEstimate();
    
    emit progressUpdated(percentage);
}

void WallFittingProgressDialog::startProgress(const QString& title)
{
    m_titleLabel->setText(title);
    m_cancelled = false;
    m_completed = false;
    m_currentProgress = 0;
    m_progressHistory.clear();
    
    m_elapsedTimer.start();
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_lastUpdateTime = m_startTime;
    
    m_cancelButton->setVisible(true);
    m_closeButton->setVisible(false);
    
    m_updateTimer->start();
    
    addLogMessage(QString("开始: %1").arg(title));
    qDebug() << "Progress started:" << title;
}

void WallFittingProgressDialog::completeProgress(const QString& message)
{
    m_completed = true;
    m_currentProgress = m_maxProgress;
    m_currentStatus = message;
    
    m_updateTimer->stop();
    
    m_cancelButton->setVisible(false);
    m_closeButton->setVisible(true);
    
    updateProgressText();
    addLogMessage(QString("完成: %1").arg(message));
    
    qDebug() << "Progress completed:" << message;
}

void WallFittingProgressDialog::failProgress(const QString& error)
{
    m_completed = true;
    m_currentStatus = QString("错误: %1").arg(error);
    
    m_updateTimer->stop();
    
    m_cancelButton->setVisible(false);
    m_closeButton->setVisible(true);
    
    updateProgressText();
    addLogMessage(QString("失败: %1").arg(error));
    
    qDebug() << "Progress failed:" << error;
}

void WallFittingProgressDialog::addLogMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    
    m_logTextEdit->append(logEntry);
    m_logTextEdit->ensureCursorVisible();
}

void WallFittingProgressDialog::updateProgressText()
{
    m_progressBar->setValue(m_currentProgress);
    m_statusLabel->setText(m_currentStatus);
    m_percentageLabel->setText(QString("%1%").arg(m_currentProgress));
}

void WallFittingProgressDialog::updateTimeEstimate()
{
    if (m_progressHistory.size() < 2 || m_currentProgress <= 0) {
        m_timeLabel->setText("预计时间: --");
        return;
    }
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = currentTime - m_startTime;
    
    // 计算平均速度
    int progressDelta = m_currentProgress;
    qint64 timeDelta = elapsed;
    
    if (timeDelta > 0 && progressDelta > 0) {
        qint64 estimatedTotal = (timeDelta * m_maxProgress) / progressDelta;
        qint64 remaining = estimatedTotal - elapsed;
        
        QString elapsedStr = getElapsedTimeString();
        QString remainingStr = QString::number(qMax(0LL, remaining / 1000)) + "秒";
        
        m_timeLabel->setText(QString("已用时: %1, 预计剩余: %2").arg(elapsedStr, remainingStr));
    }
}

QString WallFittingProgressDialog::getElapsedTimeString() const
{
    qint64 elapsed = m_elapsedTimer.elapsed();
    int seconds = elapsed / 1000;
    int minutes = seconds / 60;
    seconds %= 60;
    
    if (minutes > 0) {
        return QString("%1分%2秒").arg(minutes).arg(seconds);
    } else {
        return QString("%1秒").arg(seconds);
    }
}

void WallFittingProgressDialog::onCancelClicked()
{
    m_cancelled = true;
    emit cancelled();
    addLogMessage("用户取消操作");
    qDebug() << "Progress cancelled by user";
}

void WallFittingProgressDialog::onDetailsToggled()
{
    m_detailsVisible = !m_detailsVisible;
    m_logTextEdit->setVisible(m_detailsVisible);
    
    if (m_detailsVisible) {
        m_detailsButton->setText("隐藏详细信息");
        setMaximumHeight(DIALOG_MAX_HEIGHT);
        resize(width(), DIALOG_MAX_HEIGHT);
    } else {
        m_detailsButton->setText("详细信息");
        setMaximumHeight(DIALOG_MIN_HEIGHT + 100);
        resize(width(), DIALOG_MIN_HEIGHT + 100);
    }
}

void WallFittingProgressDialog::updateDisplay()
{
    updateTimeEstimate();
}

bool WallFittingProgressDialog::isCancelled() const
{
    return m_cancelled;
}

void WallFittingProgressDialog::applyStyles()
{
    // 设置现代化样式
    setStyleSheet(QString(
        "QDialog {"
        "    background-color: #f5f5f5;"
        "    border-radius: 8px;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "    font-size: 12px;"
        "}"
        "QLabel#titleLabel {"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "}"
        "%1"
        "%2"
        "%3"
    ).arg(getProgressBarStyle())
     .arg(getButtonStyle("#3498db"))
     .arg("QTextEdit { background-color: white; border: 1px solid #ddd; border-radius: 4px; }"));
}

QString WallFittingProgressDialog::getProgressBarStyle() const
{
    return QString(
        "QProgressBar {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    text-align: center;"
        "    background-color: #ecf0f1;"
        "    height: 20px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #3498db;"
        "    border-radius: 3px;"
        "}"
    );
}

QString WallFittingProgressDialog::getButtonStyle(const QString& color) const
{
    return QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "}"
    ).arg(color)
     .arg(color + "dd")  // 稍微透明的悬停效果
     .arg(color + "aa"); // 更透明的按下效果
}

// 槽函数实现
void WallFittingProgressDialog::onProgressChanged(int percentage, const QString& status)
{
    setProgress(percentage, status);
}

void WallFittingProgressDialog::onProcessingStarted()
{
    startProgress("墙面拟合算法执行中");
}

void WallFittingProgressDialog::onProcessingCompleted()
{
    completeProgress("墙面拟合成功完成");
}

void WallFittingProgressDialog::onProcessingFailed(const QString& error)
{
    failProgress(error);
}

} // namespace WallExtraction
