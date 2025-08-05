#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QResizeEvent>

class ResponsiveTestWidget : public QWidget
{
    Q_OBJECT

public:
    ResponsiveTestWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
        setMinimumSize(800, 600);
        resize(1200, 800);
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        qDebug() << "ResponsiveTestWidget resized to:" << event->size();
        
        // 测试动态调整
        if (m_leftPanel && m_rightPanel) {
            int width = event->size().width();
            int leftWidth = qMax(200, width / 4);
            leftWidth = qMin(400, leftWidth);
            
            m_leftPanel->setMaximumWidth(leftWidth);
            qDebug() << "Left panel width adjusted to:" << leftWidth;
        }
    }

private:
    void setupUI()
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // 顶部工具栏
        QWidget* toolbar = new QWidget();
        toolbar->setMinimumHeight(50);
        toolbar->setMaximumHeight(80);
        toolbar->setStyleSheet("background-color: #f0f0f0; border-bottom: 2px solid #ccc;");
        
        QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
        toolbarLayout->addWidget(new QLabel("响应式UI测试"));
        toolbarLayout->addStretch();
        
        QPushButton* testBtn = new QPushButton("测试按钮");
        testBtn->setStyleSheet(
            "QPushButton {"
            "   padding: 0.5em 1em;"
            "   background-color: #007bff;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 0.3em;"
            "}"
        );
        toolbarLayout->addWidget(testBtn);
        
        mainLayout->addWidget(toolbar);
        
        // 主内容区域
        QHBoxLayout* contentLayout = new QHBoxLayout();
        
        // 左侧面板
        m_leftPanel = new QWidget();
        m_leftPanel->setMinimumWidth(200);
        m_leftPanel->setMaximumWidth(400);
        m_leftPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_leftPanel->setStyleSheet("background-color: white; border-right: 2px solid #ccc;");
        
        QVBoxLayout* leftLayout = new QVBoxLayout(m_leftPanel);
        leftLayout->addWidget(new QLabel("控制面板"));
        leftLayout->addWidget(new QPushButton("按钮1"));
        leftLayout->addWidget(new QPushButton("按钮2"));
        leftLayout->addStretch();
        
        contentLayout->addWidget(m_leftPanel, 0);
        
        // 右侧显示区域
        m_rightPanel = new QWidget();
        m_rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_rightPanel->setStyleSheet("background-color: #f8f9fa; border: 1px solid #ccc;");
        
        QVBoxLayout* rightLayout = new QVBoxLayout(m_rightPanel);
        QLabel* displayLabel = new QLabel("显示区域\n这里会根据窗口大小自动调整");
        displayLabel->setAlignment(Qt::AlignCenter);
        displayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        rightLayout->addWidget(displayLabel);
        
        contentLayout->addWidget(m_rightPanel, 1);
        
        mainLayout->addLayout(contentLayout, 1);
        
        // 底部状态栏
        QWidget* statusBar = new QWidget();
        statusBar->setMinimumHeight(30);
        statusBar->setMaximumHeight(40);
        statusBar->setStyleSheet("background-color: #f0f0f0; border-top: 1px solid #ccc;");
        
        QHBoxLayout* statusLayout = new QHBoxLayout(statusBar);
        statusLayout->addWidget(new QLabel("状态：就绪"));
        statusLayout->addStretch();
        
        mainLayout->addWidget(statusBar);
    }

private:
    QWidget* m_leftPanel;
    QWidget* m_rightPanel;
};

#include "test_responsive_ui.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ResponsiveTestWidget window;
    window.setWindowTitle("响应式UI测试");
    window.show();
    
    return app.exec();
}
