#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lineplotwidget.h"
#include <QDir>
#include <QDesktopServices>
#include <QtCore/qrandom.h>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QKeySequence>
#include <QProgressDialog>
#include <QFileInfo>
#include <QStackedWidget>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dirModel(nullptr)
    , m_plotWidget(nullptr)
    , m_stackedWidget(nullptr)
    , m_originalWidget(nullptr)
    , m_lineViewWidget(nullptr)
    , m_wallExtractionManager(nullptr)
{
    ui->setupUi(this);

    // 设置响应式窗口属性
    setMinimumSize(1024, 768);  // 最小支持分辨率
    resize(1400, 900);          // 默认窗口大小
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 设置堆叠部件
    setupStackedWidget();

    // 创建OpenGL窗口部件并设置父对象和响应式大小
    m_pOpenglWidget = new MyQOpenglWidget(ui->openGLWidget);
    m_pOpenglWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 使用布局管理器而不是固定大小
    QVBoxLayout* openglLayout = new QVBoxLayout(ui->openGLWidget);
    openglLayout->setContentsMargins(0, 0, 0, 0);
    openglLayout->addWidget(m_pOpenglWidget);

    // setupFileSystemModel();  // 调用封装方法
    setupActions();

    // 初始化墙面提取模块
    initializeWallExtractionModule();

    // 创建并添加阶段一演示Widget
    m_stage1DemoWidget = new Stage1DemoWidget(this);
    m_stage1DemoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tabWidget->addTab(m_stage1DemoWidget, "阶段一演示");

    // 确保tabWidget支持响应式布局
    ui->tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 重新配置中央部件的响应式布局
    setupResponsiveLayout();

    // 设置3D显示标签页的响应式布局
    setup3DDisplayResponsiveLayout();

    // 延迟再次调用，确保UI完全初始化
    QTimer::singleShot(100, this, [this]() {
        qDebug() << "Delayed responsive layout setup";
        setupResponsiveLayout();
        setup3DDisplayResponsiveLayout();
    });

    // 监听tabWidget的大小变化
    if (ui->tabWidget) {
        ui->tabWidget->installEventFilter(this);
    }

    connect(ui->actionOpen,&QAction::triggered,this,&MainWindow::openProject);
    connect(ui->actionNew,&QAction::triggered,this,&MainWindow::newProject);

    // 添加调试用的布局重建快捷键
    QAction* rebuildLayoutAction = new QAction("重建布局", this);
    rebuildLayoutAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(rebuildLayoutAction, &QAction::triggered, this, &MainWindow::forceLayoutRebuild);
    addAction(rebuildLayoutAction);

    // 添加3D显示布局重建快捷键
    QAction* rebuild3DLayoutAction = new QAction("重建3D布局", this);
    rebuild3DLayoutAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    connect(rebuild3DLayoutAction, &QAction::triggered, this, &MainWindow::setup3DDisplayResponsiveLayout);
    addAction(rebuild3DLayoutAction);
    // 修改双击连接，添加空指针检查
    connect(ui->treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        if (m_dirModel && !m_dirModel->isDir(index)) {  // 添加空指针检查
            QString filePath = m_dirModel->filePath(index);
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    });
    connect(ui->actionOpen_PLY,&QAction::triggered,this,&MainWindow::PointCloud);

    connect(ui->actionClearPointCloud,&QAction::triggered,this,&MainWindow::ClearAllPointClouds);

    connect(ui->actionLineView, &QAction::triggered, this, &MainWindow::showLineView);

    connect(ui->pushButtonX, &QPushButton::clicked, m_pOpenglWidget, &MyQOpenglWidget::setXView);
    connect(ui->pushButtonY, &QPushButton::clicked, m_pOpenglWidget, &MyQOpenglWidget::setYView);
    connect(ui->pushButtonZ, &QPushButton::clicked, m_pOpenglWidget, &MyQOpenglWidget::setZView);

    // 连接UI中的显示模式切换按钮
    connect(ui->btnPointCloud, &QPushButton::clicked, [this]() {
        m_pOpenglWidget->setViewMode(ViewMode::PointCloudOnly);
    });
    connect(ui->btnMesh, &QPushButton::clicked, [this]() {
        m_pOpenglWidget->setViewMode(ViewMode::MeshOnly);
    });
    connect(ui->btnHybrid, &QPushButton::clicked, [this]() {
        m_pOpenglWidget->setViewMode(ViewMode::Hybrid);
    });


    // 移除独立的OpenglWindow，统一使用MyQOpenglWidget

    //绑定脚本
    connect(ui->actionVectorization_2,&QAction::triggered,this,&MainWindow::executeBashScript);
    connect(ui->actiongenerate_floorplan_with_texture,&QAction::triggered,this,&MainWindow::generateFloorPlanWithTexture);

    // connect(ui->pushButton,&QPushButton::clicked,this, &MainWindow::onImportModelTriggered);

    connect(ui->actionTextureOn, SIGNAL(triggered()), this, SLOT(loadTextureOp()));
    connect(ui->actionTextureOff, SIGNAL(triggered()), this, SLOT(deleteTextureOp()));
    connect(ui->actionColorNone, &QAction::triggered, this, &MainWindow::showColorNone);
    connect(ui->actionColorRed, &QAction::triggered, this, &MainWindow::showColorRed);
    connect(ui->actionColorGreen, &QAction::triggered, this, &MainWindow::showColorGreen);
    connect(ui->actionWireframe, &QAction::triggered, this, &MainWindow::showWireframe);
    connect(ui->actionFlat, &QAction::triggered, this, &MainWindow::showFlat);
    connect(ui->actionFlatlines, &QAction::triggered, this, &MainWindow::showFlatlines);
    connect(ui->actionGouraud_Shading, &QAction::triggered, this, &MainWindow::shadingGouraud);
    connect(ui->actionPhong_Shading, &QAction::triggered, this, &MainWindow::shadingPhong);
    connect(ui->actionFlat_Shading, &QAction::triggered, this, &MainWindow::shadingFlat);
    connect(ui->actionRotate, &QAction::triggered, this, &MainWindow::rotationOp);
    connect(ui->actionTranslate, &QAction::triggered, this, &MainWindow::translationOp);
    connect(ui->actionSubdivisionOn, &QAction::triggered, this, &MainWindow::subdivisionOn);
    connect(ui->actionSubdivisionOff, &QAction::triggered, this, &MainWindow::subdivisionOff);
}

MainWindow::~MainWindow()
{
    // 清理墙面提取模块
    if (m_wallExtractionManager) {
        m_wallExtractionManager->deactivateModule();
        m_wallExtractionManager.reset();
    }

    if (m_dirModel) {
        delete m_dirModel;
        m_dirModel = nullptr;
    }
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    qDebug() << "=== MainWindow::resizeEvent ===";
    qDebug() << "Old size:" << event->oldSize();
    qDebug() << "New size:" << event->size();

    // 检查中央部件布局状态
    if (ui->centralwidget && ui->centralwidget->layout()) {
        qDebug() << "Central widget has layout manager";
        ui->centralwidget->layout()->invalidate();
        ui->centralwidget->layout()->activate();
    } else {
        qDebug() << "WARNING: Central widget has no layout manager!";
    }

    // 检查左侧面板状态
    if (ui->verticalLayoutWidget) {
        qDebug() << "Left panel geometry:" << ui->verticalLayoutWidget->geometry();
        qDebug() << "Left panel visible:" << ui->verticalLayoutWidget->isVisible();
    }

    // 确保tabWidget正确调整大小
    if (ui->tabWidget) {
        qDebug() << "TabWidget geometry:" << ui->tabWidget->geometry();
        qDebug() << "TabWidget visible:" << ui->tabWidget->isVisible();
        ui->tabWidget->updateGeometry();
    }

    // 确保OpenGL widget正确调整大小
    if (m_pOpenglWidget) {
        qDebug() << "Updating OpenGL widget size:" << m_pOpenglWidget->size();
        m_pOpenglWidget->update();
    }

    // 通知墙面提取模块窗口大小变化
    if (m_stage1DemoWidget) {
        qDebug() << "Updating Stage1DemoWidget size:" << m_stage1DemoWidget->size();
        // 延迟调用，确保tabWidget已经调整完成
        QTimer::singleShot(10, this, [this]() {
            if (m_stage1DemoWidget) {
                qDebug() << "Force updating Stage1DemoWidget layout";
                m_stage1DemoWidget->forceLayoutUpdate();
            }
        });
    }

    qDebug() << "=== MainWindow::resizeEvent completed ===";
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    qDebug() << "=== MainWindow::showEvent called ===";
    qDebug() << "Window size:" << size();

    // 确保响应式布局在窗口显示时正确应用
    QTimer::singleShot(50, this, [this]() {
        qDebug() << "Final responsive layout setup on show";
        forceLayoutRebuild();
        setup3DDisplayResponsiveLayout();
    });
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->tabWidget && event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        qDebug() << "TabWidget resized to:" << resizeEvent->size();

        // 通知Stage1DemoWidget更新布局
        if (m_stage1DemoWidget) {
            QTimer::singleShot(0, this, [this]() {
                m_stage1DemoWidget->forceLayoutUpdate();
            });
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupResponsiveLayout()
{
    qDebug() << "=== Setting up responsive layout for MainWindow ===";

    // 获取中央部件
    QWidget* centralWidget = ui->centralwidget;
    if (!centralWidget) {
        qDebug() << "ERROR: Central widget not found!";
        return;
    }

    qDebug() << "Central widget size:" << centralWidget->size();

    // 获取控件并检查状态
    QWidget* leftPanel = ui->verticalLayoutWidget;
    QTabWidget* tabWidget = ui->tabWidget;

    if (!leftPanel) {
        qDebug() << "ERROR: Left panel (verticalLayoutWidget) not found!";
        return;
    }
    if (!tabWidget) {
        qDebug() << "ERROR: TabWidget not found!";
        return;
    }

    qDebug() << "Before layout - Left panel geometry:" << leftPanel->geometry();
    qDebug() << "Before layout - TabWidget geometry:" << tabWidget->geometry();

    // 强制清除固定geometry设置
    leftPanel->setGeometry(QRect()); // 清除固定geometry
    tabWidget->setGeometry(QRect()); // 清除固定geometry

    // 确保控件的父对象正确
    leftPanel->setParent(centralWidget);
    tabWidget->setParent(centralWidget);

    // 删除现有布局
    if (centralWidget->layout()) {
        qDebug() << "Deleting existing layout";
        delete centralWidget->layout();
    }

    // 创建新的响应式布局
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(4);

    // 配置左侧文件树面板
    leftPanel->setMinimumWidth(200);
    leftPanel->setMaximumWidth(350);
    leftPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPanel->show(); // 确保可见
    mainLayout->addWidget(leftPanel, 0); // 不拉伸
    qDebug() << "Left panel configured - size policy set";

    // 配置右侧tabWidget
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabWidget->show(); // 确保可见
    mainLayout->addWidget(tabWidget, 1); // 拉伸占据剩余空间
    qDebug() << "TabWidget configured for responsive layout";

    // 应用新布局
    centralWidget->setLayout(mainLayout);

    // 强制激活布局
    mainLayout->invalidate();
    mainLayout->activate();
    centralWidget->updateGeometry();

    qDebug() << "After layout - Left panel geometry:" << leftPanel->geometry();
    qDebug() << "After layout - TabWidget geometry:" << tabWidget->geometry();
    qDebug() << "=== Responsive layout setup completed ===";
}

void MainWindow::forceLayoutRebuild()
{
    qDebug() << "=== Force layout rebuild ===";

    // 获取中央部件
    QWidget* centralWidget = ui->centralwidget;
    if (!centralWidget) {
        qDebug() << "ERROR: Central widget not found!";
        return;
    }

    // 获取控件
    QWidget* leftPanel = ui->verticalLayoutWidget;
    QTabWidget* tabWidget = ui->tabWidget;

    if (!leftPanel || !tabWidget) {
        qDebug() << "ERROR: Required widgets not found!";
        return;
    }

    // 完全重建布局
    // 1. 移除所有子控件
    if (centralWidget->layout()) {
        QLayout* oldLayout = centralWidget->layout();
        while (QLayoutItem* item = oldLayout->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->setParent(nullptr);
            }
            delete item;
        }
        delete oldLayout;
    }

    // 2. 重新设置父对象
    leftPanel->setParent(centralWidget);
    tabWidget->setParent(centralWidget);

    // 3. 创建新布局
    QHBoxLayout* newLayout = new QHBoxLayout(centralWidget);
    newLayout->setContentsMargins(2, 2, 2, 2);
    newLayout->setSpacing(4);

    // 4. 重新配置控件
    leftPanel->setMinimumWidth(200);
    leftPanel->setMaximumWidth(350);
    leftPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPanel->show();

    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabWidget->show();

    // 5. 添加到布局
    newLayout->addWidget(leftPanel, 0);
    newLayout->addWidget(tabWidget, 1);

    // 6. 强制更新
    centralWidget->setLayout(newLayout);
    newLayout->activate();
    centralWidget->updateGeometry();

    qDebug() << "Layout rebuild completed";
    qDebug() << "Left panel geometry:" << leftPanel->geometry();
    qDebug() << "TabWidget geometry:" << tabWidget->geometry();
}

void MainWindow::setup3DDisplayResponsiveLayout()
{
    qDebug() << "=== Setting up 3D Display responsive layout ===";

    // 获取3D显示标签页
    QWidget* tab3D = ui->tab_unified;
    if (!tab3D) {
        qDebug() << "ERROR: 3D display tab not found!";
        return;
    }

    // 获取控件
    QWidget* displayControlWidget = ui->displayControlWidget;
    QOpenGLWidget* openGLWidget = ui->openGLWidget;

    if (!displayControlWidget || !openGLWidget) {
        qDebug() << "ERROR: 3D display widgets not found!";
        qDebug() << "  displayControlWidget:" << (displayControlWidget ? "OK" : "NULL");
        qDebug() << "  openGLWidget:" << (openGLWidget ? "OK" : "NULL");
        return;
    }

    qDebug() << "Before layout - Control widget geometry:" << displayControlWidget->geometry();
    qDebug() << "Before layout - OpenGL widget geometry:" << openGLWidget->geometry();

    // 清除固定geometry设置
    displayControlWidget->setGeometry(QRect());
    openGLWidget->setGeometry(QRect());

    // 确保控件的父对象正确
    displayControlWidget->setParent(tab3D);
    openGLWidget->setParent(tab3D);

    // 删除现有布局
    if (tab3D->layout()) {
        qDebug() << "Deleting existing 3D tab layout";
        delete tab3D->layout();
    }

    // 创建主布局（垂直布局）
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);

    // 设置控制区域的响应式布局
    setupDisplayControlLayout(displayControlWidget);

    // 配置控制区域
    displayControlWidget->setMinimumHeight(40);
    displayControlWidget->setMaximumHeight(50);
    displayControlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(displayControlWidget, 0); // 不拉伸

    // 配置OpenGL区域
    openGLWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    openGLWidget->setMinimumSize(400, 300);
    mainLayout->addWidget(openGLWidget, 1); // 拉伸占据剩余空间

    // 应用新布局
    tab3D->setLayout(mainLayout);

    // 强制激活布局
    mainLayout->invalidate();
    mainLayout->activate();
    tab3D->updateGeometry();

    qDebug() << "After layout - Control widget geometry:" << displayControlWidget->geometry();
    qDebug() << "After layout - OpenGL widget geometry:" << openGLWidget->geometry();
    qDebug() << "=== 3D Display responsive layout setup completed ===";
}

void MainWindow::setupDisplayControlLayout(QWidget* controlWidget)
{
    qDebug() << "=== Setting up display control layout ===";

    if (!controlWidget) {
        qDebug() << "ERROR: Control widget is null!";
        return;
    }

    // 获取所有控制按钮
    QPushButton* btnPointCloud = ui->btnPointCloud;
    QPushButton* btnMesh = ui->btnMesh;
    QPushButton* btnHybrid = ui->btnHybrid;
    QPushButton* pushButtonX = ui->pushButtonX;
    QPushButton* pushButtonY = ui->pushButtonY;
    QPushButton* pushButtonZ = ui->pushButtonZ;

    // 检查按钮是否存在
    if (!btnPointCloud || !btnMesh || !btnHybrid || !pushButtonX || !pushButtonY || !pushButtonZ) {
        qDebug() << "ERROR: Some control buttons not found!";
        return;
    }

    // 清除所有按钮的固定geometry
    btnPointCloud->setGeometry(QRect());
    btnMesh->setGeometry(QRect());
    btnHybrid->setGeometry(QRect());
    pushButtonX->setGeometry(QRect());
    pushButtonY->setGeometry(QRect());
    pushButtonZ->setGeometry(QRect());

    // 设置按钮的父对象
    btnPointCloud->setParent(controlWidget);
    btnMesh->setParent(controlWidget);
    btnHybrid->setParent(controlWidget);
    pushButtonX->setParent(controlWidget);
    pushButtonY->setParent(controlWidget);
    pushButtonZ->setParent(controlWidget);

    // 删除现有布局
    if (controlWidget->layout()) {
        delete controlWidget->layout();
    }

    // 创建水平布局
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(8);

    // 左侧显示模式按钮组
    controlLayout->addWidget(btnPointCloud);
    controlLayout->addWidget(btnMesh);
    controlLayout->addWidget(btnHybrid);

    // 中间弹性空间
    controlLayout->addStretch(1);

    // 右侧视图控制按钮组
    controlLayout->addWidget(pushButtonX);
    controlLayout->addWidget(pushButtonY);
    controlLayout->addWidget(pushButtonZ);

    // 设置按钮样式和大小策略
    QList<QPushButton*> allButtons = {btnPointCloud, btnMesh, btnHybrid, pushButtonX, pushButtonY, pushButtonZ};
    for (QPushButton* btn : allButtons) {
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        btn->setMinimumHeight(28);
        btn->setMaximumHeight(32);

        // 设置现代化按钮样式
        btn->setStyleSheet(
            "QPushButton {"
            "   padding: 4px 12px;"
            "   background-color: #f8f9fa;"
            "   border: 1px solid #dee2e6;"
            "   border-radius: 4px;"
            "   color: #495057;"
            "   font-weight: 500;"
            "}"
            "QPushButton:hover {"
            "   background-color: #e9ecef;"
            "   border-color: #adb5bd;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #dee2e6;"
            "   border-color: #6c757d;"
            "}"
        );
    }

    // 应用布局
    controlWidget->setLayout(controlLayout);

    qDebug() << "Display control layout setup completed";
}

// 暂时注释掉这些高级功能，先解决主要的显示问题
void MainWindow::loadTextureOp() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::deleteTextureOp() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::showColorNone() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::showColorRed() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::showColorGreen() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::showWireframe() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::showFlat() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::showFlatlines() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::shadingGouraud() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::shadingPhong() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::shadingFlat() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::rotationOp() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::translationOp() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::subdivisionOn() { /* TODO: 需要在MyQOpenglWidget中实现 */ }
void MainWindow::subdivisionOff() { /* TODO: 需要在MyQOpenglWidget中实现 */ }



void MainWindow::generateFloorPlanWithTexture() {
    // 使用配置管理获取路径
    Config& config = Config::instance();
    QString workingDir = config.getTextureWorkingDirectory();
    QString pythonPath = config.getPythonPath();
    QString script = "generate_floorplan_with_texture.py";

    // 验证路径是否有效
    if (pythonPath.isEmpty()) {
        QMessageBox::warning(this, "配置错误", 
                           "未找到Python解释器。请在配置中设置正确的Python路径。");
        return;
    }
    
    if (workingDir.isEmpty() || !QDir(workingDir).exists()) {
        QMessageBox::warning(this, "配置错误", 
                           QString("工作目录不存在：%1\n请检查floorplan_code_v1/texture目录路径。").arg(workingDir));
        return;
    }

    QProcess process;
    process.setWorkingDirectory(workingDir);
    process.start(pythonPath, QStringList() << script);

    if (process.waitForFinished(30000)) {
        if (process.exitCode() == 0) {
            qDebug() << "执行成功:" << process.readAllStandardOutput();
            statusBar()->showMessage("平面图生成完成", 3000);
        } else {
            qDebug() << "执行失败:" << process.readAllStandardError();
            QMessageBox::warning(this, "执行错误", 
                               QString("Python脚本执行失败:\n%1").arg(QString(process.readAllStandardError())));
        }
    } else {
        QMessageBox::warning(this, "超时错误", "Python脚本执行超时 (30秒)");
    }
}

//初始化
void MainWindow::setupStackedWidget()
{
    // 保存原始的中央部件
    m_originalWidget = centralWidget();

    // 创建堆叠部件
    m_stackedWidget = new QStackedWidget(this);

    // 将原始部件添加到堆叠部件中
    m_stackedWidget->addWidget(m_originalWidget);

    // 设置堆叠部件为中央部件
    setCentralWidget(m_stackedWidget);
}

void MainWindow::showLineView()
{
    // 如果线条视图还没有创建，创建它
    if (!m_lineViewWidget) {
        createLineView();
    }

    // 切换到线条视图
    m_stackedWidget->setCurrentWidget(m_lineViewWidget);

    // 设置窗口标题
    setWindowTitle("线条绘制查看器");

    // 更新菜单
    updateMenusForLineView();

    statusBar()->showMessage("已切换到线条绘制视图");
}

//返回界面
void MainWindow::showOriginalView()
{
    // 切换回原始视图
    m_stackedWidget->setCurrentWidget(m_originalWidget);

    // 恢复窗口标题
    setWindowTitle("主窗口");

    // 恢复菜单
    updateMenusForOriginalView();

    statusBar()->showMessage("已返回主界面");
}

//创建新界面
void MainWindow::createLineView()
{
    // 创建线条视图部件
    m_lineViewWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_lineViewWidget);

    // 创建工具栏或返回按钮
    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);

    QPushButton *backButton = new QPushButton("← 返回主界面", this);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showOriginalView);
    toolbarLayout->addWidget(backButton);
    toolbarLayout->addStretch();

    layout->addWidget(toolbar);

    // 创建并添加绘图部件
    m_plotWidget = new LineplotWidget(this);
    layout->addWidget(m_plotWidget);

    // 将线条视图添加到堆叠部件
    m_stackedWidget->addWidget(m_lineViewWidget);
}

//进入新界面前保留原始菜单
void MainWindow::updateMenusForLineView()
{
    // 保存原始菜单
    if (m_originalMenuActions.isEmpty()) {
        foreach(QAction *action, menuBar()->actions()) {
            m_originalMenuActions.append(action);
        }
    }

    // 清除菜单栏
    menuBar()->clear();

    // 添加线条视图的菜单
    setupLineViewMenus();
}


// 回退ui
void MainWindow::updateMenusForOriginalView()
{
    // 清除当前菜单
    menuBar()->clear();

    // 恢复原始菜单
    foreach(QAction *action, m_originalMenuActions) {
        menuBar()->addAction(action);
    }
}


//显示线条框
void MainWindow::setupLineViewMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");

    QAction *openAction = fileMenu->addAction("打开文件(&O)");
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("打开一个线条数据文件");
    connect(openAction, &QAction::triggered, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        "打开线条数据文件",
                                                        "",
                                                        "文本文件 (*.txt);;所有文件 (*)");
        if (!fileName.isEmpty()) {
            m_plotWidget->setFilePath(fileName);
            m_plotWidget->plotLinesFromFile();
            statusBar()->showMessage(QString("已加载文件: %1").arg(QFileInfo(fileName).fileName()));
        }
    });

    fileMenu->addSeparator();

    QAction *backAction = fileMenu->addAction("返回主界面(&B)");
    backAction->setShortcut(QKeySequence("Ctrl+B"));
    backAction->setStatusTip("返回到主界面");
    connect(backAction, &QAction::triggered, this, &MainWindow::showOriginalView);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");

    QAction *refreshAction = viewMenu->addAction("刷新(&R)");
    refreshAction->setShortcut(QKeySequence::Refresh);
    refreshAction->setStatusTip("刷新当前绘图");
    connect(refreshAction, &QAction::triggered, [this]() {
        if (m_plotWidget) {
            m_plotWidget->plotLinesFromFile();
            statusBar()->showMessage("绘图已刷新");
        }
    });

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");

    QAction *aboutAction = helpMenu->addAction("关于(&A)");
    aboutAction->setStatusTip("显示关于此应用程序的信息");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于",
                           "线条绘制查看器 v1.0\n\n"
                           "一个用于可视化文本文件中线条数据的Qt应用程序。\n\n"
                           "文件格式：每行应包含四个用空格分隔的数字：\n"
                           "x1 y1 x2 y2\n\n"
                           "支持注释行（以 # 或 // 开头）和空行。");
    });

    QAction *aboutQtAction = helpMenu->addAction("关于Qt(&Q)");
    aboutQtAction->setStatusTip("显示关于Qt的信息");
    connect(aboutQtAction, &QAction::triggered, &QApplication::aboutQt);
}




void MainWindow::executeBashScript() {
    // 使用配置管理获取路径
    Config& config = Config::instance();
    QString pythonPath = config.getPythonPath();
    QString workingDir = config.getFloorplanWorkingDirectory();
    QString scriptPath = "./bash_run.py";

    // 验证配置
    if (pythonPath.isEmpty()) {
        QMessageBox::warning(this, "配置错误", 
                           "未找到Python解释器。请在配置中设置正确的Python路径。");
        return;
    }
    
    if (workingDir.isEmpty() || !QDir(workingDir).exists()) {
        QMessageBox::warning(this, "配置错误", 
                           QString("工作目录不存在：%1\n请检查floorplan_code_v1/py_script目录路径。").arg(workingDir));
        return;
    }

    // 准备脚本执行
    QProcess *process = new QProcess(this);
    process->setWorkingDirectory(workingDir);

    // 准备参数列表
    QStringList arguments;
    arguments << scriptPath;

    // 连接信号槽来处理进程完成事件
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus status) {
                Q_UNUSED(status)
                if (exitCode == 0) {
                    qDebug() << "Python脚本执行完成，退出代码:" << exitCode;
                    qDebug() << "输出:" << process->readAllStandardOutput();
                    statusBar()->showMessage("矢量化处理完成", 3000);
                } else {
                    qDebug() << "Python脚本执行失败，退出代码:" << exitCode;
                    qDebug() << "错误信息:" << process->readAllStandardError();
                    QMessageBox::warning(this, "执行错误", 
                                       QString("矢量化处理失败:\n%1").arg(QString(process->readAllStandardError())));
                }
                process->deleteLater();  // 自动清理进程对象
            });

    // 启动Python脚本
    qDebug() << "执行Python解释器:" << pythonPath;
    qDebug() << "工作目录:" << workingDir;
    qDebug() << "脚本路径:" << scriptPath;

    process->start(pythonPath, arguments);

    // 处理进程启动
    if (!process->waitForStarted(5000)) {
        qDebug() << "错误: 无法启动Python进程";
        QMessageBox::critical(this, "启动错误", 
                             QString("无法启动Python进程:\n%1").arg(process->errorString()));
        process->deleteLater();
        return;
    }

    qDebug() << "Python脚本已启动，正在后台运行...";
    statusBar()->showMessage("正在执行矢量化处理...", 0);
}

// mainwindow.cpp
void MainWindow::onImportModelTriggered()
{
    // 弹出文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择Mesh模型文件",
        QDir::homePath(),
        "模型文件 (*.dae *.obj *.fbx *.3ds *.ply *.stl);;所有文件 (*.*)"
        );

    if (filePath.isEmpty()) {
        qDebug() << "用户取消选择";
        return;
    }

    // 显示加载进度
    QProgressDialog progress("正在加载模型...", "取消", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    // 传递路径到 OpenGL 窗口
    if (m_pOpenglWidget->loadMeshModel(filePath)) {
        progress.close();
        qDebug() << "模型加载成功：" << filePath;
        QMessageBox::information(this, "成功", QString("模型加载成功！\n文件: %1").arg(QFileInfo(filePath).fileName()));
    } else {
        progress.close();
        QMessageBox::critical(this, "错误", QString("无法加载模型文件！\n请检查文件格式和路径：\n%1").arg(filePath));
    }
}
void MainWindow::onImportModelTriggered2()
{
    // 1. 获取选中的文件路径
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (!currentIndex.isValid() || !m_dirModel) {  // 添加模型检查
        QMessageBox::warning(this, "未选择文件", "请先在目录树中选择一个Mesh模型文件");
        return;
    }

    // 2. 验证文件类型
    QString filePath = m_dirModel->filePath(currentIndex);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir()) {
        QMessageBox::warning(this, "选择错误", "请选择文件而不是文件夹");
        return;
    }

    QString suffix = fileInfo.suffix().toLower();
    QStringList supportedFormats = {"dae", "obj", "fbx", "3ds", "ply", "stl"};
    if (!supportedFormats.contains(suffix)) {
        QMessageBox::warning(this, "格式错误",
                             QString("支持的模型格式: %1\n当前文件类型: .%2")
                             .arg(supportedFormats.join(", "))
                             .arg(fileInfo.suffix()));
        return;
    }

    // 显示加载进度
    QProgressDialog progress("正在加载模型...", "取消", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    if (m_pOpenglWidget && m_pOpenglWidget->loadMeshModel(filePath)) {
        progress.close();
        qDebug() << "模型加载成功：" << filePath;
        QMessageBox::information(this, "成功", QString("模型加载成功！\n文件: %1").arg(fileInfo.fileName()));
    } else {
        progress.close();
        QMessageBox::critical(this, "错误", QString("无法加载模型文件！\n请检查文件格式和路径：\n%1").arg(filePath));
    }
}


// 封装的核心方法
void MainWindow::setupFileSystemModel(const QString &rootPath, const QStringList &filters)
{
    // 如果已有模型则清理
    if (m_dirModel) {
        // 先断开可能的连接
        ui->treeView->setModel(nullptr);
        delete m_dirModel;
        m_dirModel = nullptr;  // 重要：设置为nullptr
    }

    // 1. 创建并配置模型
    m_dirModel = new QFileSystemModel(this);
    m_dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    m_dirModel->setNameFilters(filters);
    m_dirModel->setNameFilterDisables(false);
    m_dirModel->setReadOnly(false);

    // 2. 设置根路径并获取索引
    QModelIndex rootIndex = m_dirModel->setRootPath(rootPath);

    // 3. 配置视图
    ui->treeView->setModel(m_dirModel);
    ui->treeView->setRootIndex(rootIndex);
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);

    // 4. 隐藏指定列
    const QVector<int> columnsToHide = {1, 2, 3};
    for (int col : columnsToHide) {
        ui->treeView->setColumnHidden(col, true);
    }
}

void MainWindow::openProject()
{
    // 弹窗选择文件夹
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("选择项目文件夹"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!folderPath.isEmpty()) {
        setupFileSystemModel(folderPath,{"*.*"}); // 调用模型设置函数
    }
}

void MainWindow::newProject()
{
    // 步骤1：选择父目录
    QString parentDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择项目父目录"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly
        );

    if (parentDir.isEmpty()) return;

    // 步骤2：输入项目名称
    bool ok;
    QString projectName = QInputDialog::getText(
        this,
        tr("新建项目"),
        tr("项目名称:"),
        QLineEdit::Normal,
        "",
        &ok
        );

    // // 创建目录名称校验：
    // if (projectName.contains(QRegExp("[\\\\/:*?\"<>|]"))) {
    //     QMessageBox::warning(this, tr("错误"), tr("包含非法字符"));
    //     return;
    // }

    if (!ok || projectName.trimmed().isEmpty()) return;

    // 步骤3：构建完整路径
    QDir targetDir(parentDir);
    QString projectPath = targetDir.filePath(projectName);

    // 检查目录是否存在
    if (QDir(projectPath).exists()) {
        QMessageBox::warning(
            this,
            tr("错误"),
            tr("目录已存在：\n%1").arg(projectPath)
            );
        return;
    }

    // 步骤4：创建目录
    if (!targetDir.mkdir(projectName)) {
        QMessageBox::critical(
            this,
            tr("错误"),
            tr("无法创建目录：\n%1").arg(projectPath)
            );
        return;
    }

    // 在创建目录后添加模板文件
    QFile projFile(projectPath + "/project.config");
    if (projFile.open(QIODevice::WriteOnly)) {
        // 增强版配置文件生成
        QString configContent = QString("[Project]\n"
                                        "name=%1\n"
                                        "created=%2\n"
                                        "version=1.0.0\n"
                                        "author=%3\n")
                                    .arg(projectName)
                                    .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                                    .arg(qgetenv("USERNAME"));  // 获取系统用户名

        projFile.write(configContent.toUtf8());
    }

    // 步骤5：初始化项目（可选）
    // 可以在此处创建默认配置文件等

    // 加载新项目
    setupFileSystemModel(projectPath,{"*.*"});

}

// 实现上下文菜单
void MainWindow::setupActions() {

    // 创建菜单和动作
    m_contextMenu = new QMenu(this);

    m_actionNewFile = new QAction("新建文件", this);
    m_actionNewFolder = new QAction("新建文件夹", this);
    m_actionDelete = new QAction("删除", this);
    m_actionRename = new QAction("重命名", this);
    m_actionPointCloud = new QAction("点云显示",this);
    m_actionMesh = new QAction("Mesh显示",this);
    m_actionVec = new QAction("矢量化",this);

    m_contextMenu->addAction(m_actionNewFile);
    m_contextMenu->addAction(m_actionNewFolder);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionDelete);
    m_contextMenu->addAction(m_actionRename);
    m_contextMenu->addAction(m_actionMesh);
    m_contextMenu->addAction(m_actionPointCloud);
    m_contextMenu->addAction(m_actionVec);

    // 连接信号槽
    connect(m_actionNewFile, &QAction::triggered, this, &MainWindow::createNewFile);
    connect(m_actionNewFolder, &QAction::triggered, this, &MainWindow::createNewFolder);
    connect(m_actionDelete, &QAction::triggered, this, &MainWindow::deleteSelectedItem);
    connect(m_actionRename, &QAction::triggered, this, &MainWindow::renameSelectedItem);
    connect(m_actionVec, &QAction::triggered,this,&MainWindow::executeBashScript);//矢量化
    // connect(m_actionPointCloud, &QAction::triggered, this, &MainWindow::on_pushButton_openfile_clicked);
    // 修改为显式指定信号参数（避免重载歧义）：
    connect(m_actionPointCloud, &QAction::triggered, this, [this]() {
        MainWindow::PointCloud2();
    });
    connect(m_actionMesh, &QAction::triggered, this, [this]() {
        MainWindow::onImportModelTriggered2();
    });

    // 启用右键菜单
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onCustomContextMenuRequested);
}

void MainWindow::onCustomContextMenuRequested(const QPoint &pos) {
    // 无论是否有效都显示菜单
    m_contextMenu->exec(ui->treeView->viewport()->mapToGlobal(pos));
}
// 新建文件/文件夹
void MainWindow::createNewFile() {
    QModelIndex currentIndex = ui->treeView->currentIndex();
    QString parentPath;

    // 确定父目录路径
    if (currentIndex.isValid() && m_dirModel->isDir(currentIndex)) {
        parentPath = m_dirModel->filePath(currentIndex);
    } else {
        parentPath = m_dirModel->rootPath();
    }

    // 生成唯一文件名
    QString baseName = "新建文件.txt";
    QString filePath;
    int counter = 0;
    do {
        filePath = QDir(parentPath).filePath(
            counter > 0 ? QString("新建文件(%1).txt").arg(counter) : baseName
            );
        counter++;
    } while (QFile::exists(filePath));

    // 创建文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        QModelIndex newIndex = m_dirModel->index(filePath);
        ui->treeView->scrollTo(newIndex);
    } else {
        QMessageBox::warning(this, "错误", QString("无法创建文件：\n%1").arg(file.errorString()));
    }
}

void MainWindow::createNewFolder() {
    QModelIndex currentIndex = ui->treeView->currentIndex();
    QString parentPath;

    // 校验是否可以在该位置创建
    if (currentIndex.isValid()) {
        if (!m_dirModel->isDir(currentIndex)) {
            QMessageBox::warning(this, "错误", "不能在文件内创建文件夹！");
            return;
        }
        parentPath = m_dirModel->filePath(currentIndex);
    } else {
        parentPath = m_dirModel->rootPath();
    }

    // 生成唯一文件夹名
    QString baseName = "新建文件夹";
    QString folderPath;
    int counter = 0;
    do {
        folderPath = QDir(parentPath).filePath(
            counter > 0 ? QString("新建文件夹(%1)").arg(counter) : baseName
            );
        counter++;
    } while (QDir(folderPath).exists());

    // 创建文件夹
    if (!m_dirModel->mkdir(m_dirModel->index(parentPath), baseName).isValid()) {
        QMessageBox::warning(this, "错误", "无法创建文件夹！");
    }
}
// 删除操作
void MainWindow::deleteSelectedItem() {
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (!currentIndex.isValid()) return;

    QString path = m_dirModel->filePath(currentIndex);
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除", "确定要删除 '" + path + "' 吗？",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (m_dirModel->isDir(currentIndex)) {
            if (!QDir(path).removeRecursively()) {
                QMessageBox::warning(this, "错误", "删除文件夹失败！");
            }
        } else {
            if (!QFile::remove(path)) {
                QMessageBox::warning(this, "错误", "删除文件失败！");
            }
        }
    }
}
// 重命名操作
void MainWindow::renameSelectedItem() {
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (currentIndex.isValid()) {
        ui->treeView->edit(currentIndex);  // 触发编辑模式
    }
}

/* 打开点云文件按钮点击事件处理 */
void MainWindow::PointCloud()
{
    // 弹出文件选择对话框，支持PLY、TXT和PCD三种格式
    QString qfile = QFileDialog::getOpenFileName(
        this,
        "打开点云文件",
        "",
        "点云文件 (*.ply *.txt *.pcd);;PLY文件 (*.ply);;TXT文件 (*.txt);;PCD文件 (*.pcd);;所有文件 (*)"
        );
    if(qfile.isEmpty()) return;

    // 根据文件扩展名确定读取方式
    QFileInfo fileInfo(qfile);
    QString extension = fileInfo.suffix().toLower();
    std::vector<QVector3D> cloud;

    // // 如果是PCD文件，先进行分析（调试用）
    // if(extension == "pcd") {
    //     analyzePCDFile(qfile);  // 这行可以在调试完成后删除
    // }

    if(extension == "ply") {
        // 读取PLY格式点云
        cloud = ReadVec3PointCloudPLY(qfile);
    }
    else if(extension == "txt") {
        // 读取TXT格式点云
        cloud = ReadVec3PointCloudTXT(qfile);
    }
    else if(extension == "pcd") {
        // 读取PCD格式点云
        cloud = ReadVec3PointCloudPCD(qfile);
    }
    else {
        qDebug() << "不支持的文件格式：" << extension;
        QMessageBox::warning(this, "错误", "不支持的文件格式！\n仅支持PLY、TXT和PCD格式的点云文件。");
        return;
    }

    if(cloud.empty()) {
        qDebug() << "点云数据为空或读取失败";
        QMessageBox::warning(this, "错误", "点云数据为空或读取失败！");
        return;
    }

    // 显示点云并测量耗时
    QTime startTime = QTime::currentTime();
    m_currentCloud = cloud; // 保存当前点云数据
    m_pOpenglWidget->showPointCloud(cloud);
    qDebug() << "点云加载耗时：" << startTime.msecsTo(QTime::currentTime()) << "ms";
    qDebug() << "成功加载点云，共" << cloud.size() << "个点";
}

// 使用PCDReader类读取PCD文件
std::vector<QVector3D> MainWindow::ReadVec3PointCloudPCD(const QString &filename)
{
    return PCDReader::ReadVec3PointCloudPCD(filename);
}

// PCD文件分析函数（调试用）
void MainWindow::analyzePCDFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开PCD文件进行分析：" << filename;
        return;
    }

    QTextStream in(&file);
    QString line;
    int lineNumber = 0;
    QString dataType = "未知";
    int expectedPoints = 0;

    qDebug() << "=== PCD文件分析开始 ===";
    qDebug() << "文件路径：" << filename;
    qDebug() << "文件大小：" << file.size() << "字节";

    // 分析文件头
    while (!in.atEnd() && lineNumber < 50) {
        line = in.readLine();
        lineNumber++;
        qDebug() << QString("第%1行: %2").arg(lineNumber, 2).arg(line);

        // 提取关键信息
        if (line.trimmed().startsWith("POINTS")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                expectedPoints = parts[1].toInt();
                qDebug() << "*** 期望点数：" << expectedPoints;
            }
        }
        else if (line.trimmed().startsWith("DATA")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                dataType = parts[1];
                qDebug() << "*** 数据格式：" << dataType;
            }
            break;
        }
    }

    // 分析数据部分的前几行
    if (!in.atEnd()) {
        qDebug() << "--- 数据部分预览 ---";
        qDebug() << "当前文件指针位置：" << in.pos();

        int dataLineCount = 0;
        QStringList sampleLines;

        while (!in.atEnd() && dataLineCount < 10) {
            line = in.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith("#")) {
                dataLineCount++;
                sampleLines << line;
                qDebug() << QString("数据第%1行: %2").arg(dataLineCount).arg(line);

                // 分析第一行数据的格式
                if (dataLineCount == 1) {
                    QStringList coords = line.split(' ', Qt::SkipEmptyParts);
                    qDebug() << "*** 第一行数据字段数：" << coords.size();
                    for (int i = 0; i < coords.size() && i < 6; i++) {
                        bool ok;
                        float val = coords[i].toFloat(&ok);
                        qDebug() << QString("    字段%1: %2 (转换结果: %3, 值: %4)")
                                        .arg(i+1).arg(coords[i]).arg(ok ? "成功" : "失败").arg(val);
                    }
                }
            }
        }

        // 检查是否真的是二进制数据
        if (dataType.toLower() == "binary" && !sampleLines.isEmpty()) {
            qDebug() << "*** 注意：文件声明为binary但可以读取为文本，可能是ASCII格式";
        }
    }

    file.close();
    qDebug() << "=== PCD文件分析结束 ===";
    qDebug() << "总结：期望点数=" << expectedPoints << ", 数据类型=" << dataType;
}




// 在MainWindow类中添加清空所有点云的函数

void MainWindow::ClearAllPointClouds()
{
    // 清空当前点云数据
    m_currentCloud.clear();

    // 清空OpenGL显示
    m_pOpenglWidget->clearPointCloud();

    // 更新状态栏
    statusBar()->showMessage("已清空所有点云", 2000);

    qDebug() << "[点云清空] 已清空所有点云数据";
}

// 追加显示
void MainWindow::PointCloud2()
{
    // 1. 获取选中的文件路径
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (!currentIndex.isValid() || !m_dirModel) {  // 添加模型检查
        QMessageBox::warning(this, "未选择文件", "请先在目录树中选择一个点云文件");
        return;
    }

    // 2. 验证文件类型
    QString filePath = m_dirModel->filePath(currentIndex);
    QFileInfo fileInfo(filePath);
    if (fileInfo.isDir()) {
        QMessageBox::warning(this, "选择错误", "请选择文件而不是文件夹");
        return;
    }

    QString extension = fileInfo.suffix().toLower();
    if (extension != "ply" && extension != "txt") {
        QMessageBox::warning(this, "格式错误",
                             QString("仅支持PLY和TXT文件，当前文件类型: .%1").arg(fileInfo.suffix()));
        return;
    }

    // 3. 根据文件格式读取点云数据
    std::vector<QVector3D> cloud;
    if (extension == "ply") {
        cloud = ReadVec3PointCloudPLY(filePath);
    }
    else if (extension == "txt") {
        cloud = ReadVec3PointCloudTXT(filePath);
    }

    if (cloud.empty()) {
        QMessageBox::critical(this, "数据错误",
                              QString("无法读取点云数据：\n%1\n可能原因：\n1. 文件格式不正确\n2. 文件已损坏\n3. 文件为空").arg(filePath));
        return;
    }

    // 4. 显示点云（带性能监控）
    QElapsedTimer timer;
    timer.start();

    // 检查是否有现有点云数据
    bool hasExistingCloud = !m_currentCloud.empty();

    if (hasExistingCloud) {
        // 追加显示新点云
        size_t originalSize = m_currentCloud.size();
        m_currentCloud.insert(m_currentCloud.end(), cloud.begin(), cloud.end());
        m_pOpenglWidget->appendPointCloud(cloud);

        qDebug() << "[点云追加]"
                 << "\n  文件路径:" << filePath
                 << "\n  文件类型:" << extension.toUpper()
                 << "\n  新增点数:" << cloud.size()
                 << "\n  原有点数:" << originalSize
                 << "\n  总点数:" << m_currentCloud.size()
                 << "\n  耗时:" << timer.elapsed() << "ms";

        // 状态栏显示追加信息
        statusBar()->showMessage(QString("追加点云: +%1个点，总计%2个点 (%3ms)")
                                     .arg(cloud.size())
                                     .arg(m_currentCloud.size())
                                     .arg(timer.elapsed()), 3000);
    }
    else {
        // 首次显示点云
        m_currentCloud = cloud;
        m_pOpenglWidget->showPointCloud(cloud);

        qDebug() << "[点云加载]"
                 << "\n  文件路径:" << filePath
                 << "\n  文件类型:" << extension.toUpper()
                 << "\n  点数:" << cloud.size()
                 << "\n  耗时:" << timer.elapsed() << "ms";

        // 状态栏显示加载信息
        statusBar()->showMessage(QString("成功加载点云: %1个点 (%2ms)")
                                     .arg(cloud.size())
                                     .arg(timer.elapsed()), 3000);
    }
}

/* 读取TXT格式点云文件的函数实现 */
std::vector<QVector3D> MainWindow::ReadVec3PointCloudTXT(const QString& filename)
{
    std::vector<QVector3D> cloud;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开TXT文件：" << filename;
        return cloud;
    }

    QTextStream in(&file);
    QString line;
    int lineNumber = 0;

    while(!in.atEnd()) {
        line = in.readLine().trimmed();
        lineNumber++;

        // 跳过空行和注释行
        if(line.isEmpty() || line.startsWith("#") || line.startsWith("//")) {
            continue;
        }

        // 分割字符串，支持空格、制表符分隔
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        if(parts.size() >= 3) {
            bool ok1, ok2, ok3;
            float x = parts[0].toFloat(&ok1);
            float y = parts[1].toFloat(&ok2);
            float z = parts[2].toFloat(&ok3);

            if(ok1 && ok2 && ok3) {
                cloud.push_back(QVector3D(x, y, z));
            }
            else {
                qDebug() << "TXT文件第" << lineNumber << "行数据格式错误：" << line;
            }
        }
        else {
            qDebug() << "TXT文件第" << lineNumber << "行数据不足3列：" << line;
        }
    }

    file.close();
    qDebug() << "从TXT文件读取了" << cloud.size() << "个点";
    return cloud;
}


//读取ply格式点云
std::vector<QVector3D> MainWindow::ReadVec3PointCloudPLY(QString path)
{
    std::vector<QVector3D> cloud;

    // 增强文件校验
    if (!QFile::exists(path)) {
        qDebug() << "[Error] File not exists:" << path;
        return cloud;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[Error] Cannot open file:" << file.errorString();
        return cloud;
    }

    QTextStream in(&file);
    QString line;
    int vertexCount = 0;
    int readCount = 0;

    // 读取文件头
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 检查PLY魔数
        if (line == "ply") {
            continue;
        } else if (line.startsWith("format")) {
            if (!line.contains("ascii")) {
                qDebug() << "[Error] Only support ASCII PLY format";
                return cloud;
            }
        } else if (line.startsWith("element vertex")) {
            QStringList parts = line.split(QRegularExpression("\\s+"));
            if (parts.size() >= 3) {
                vertexCount = parts[2].toInt();
            }
        } else if (line.startsWith("end_header")) {
            break;
        }
    }

    // 校验顶点数量
    if (vertexCount <= 0) {
        qDebug() << "[Error] Invalid vertex count:" << vertexCount;
        return cloud;
    }

    // 读取顶点数据
    cloud.reserve(vertexCount);
    while (!in.atEnd() && readCount < vertexCount) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() >= 3) {
            bool okX, okY, okZ;
            float x = parts[0].toFloat(&okX);
            float y = parts[1].toFloat(&okY);
            float z = parts[2].toFloat(&okZ);

            if (okX && okY && okZ) {
                cloud.emplace_back(x, y, z);
                readCount++;
            }
        }
    }

    // 校验实际读取数量
    if (readCount != vertexCount) {
        qDebug() << "[Warning] Expect" << vertexCount << "points, actual read" << readCount;
    }

    return cloud;
}





/* 从ASC文件读取三维点云数据 */
std::vector<QVector3D> MainWindow::ReadVec3PointCloudASC(QString path)
{
    std::vector<QVector3D> cloud;
    QFile file(path);

    // 文件打开失败处理
    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开ASC文件：" << path;
        return cloud;
    }

    // 读取文件内容并分割为行
    QTextStream in(&file);
    QString ramData = in.readAll();
    QStringList list = ramData.split("\n");

    // 预分配内存（行数-1是考虑最后可能有空行）
    cloud.resize(list.count()-1);

    // 逐行解析数据
    for (int i = 0; i < list.count() - 1; i++) {
        QStringList listline = list.at(i).split(" ");

        // 确保至少有XYZ三个坐标值
        if(listline.size() >= 3) {
            cloud[i].setX(listline.at(0).toFloat());
            cloud[i].setY(listline.at(1).toFloat());
            cloud[i].setZ(listline.at(2).toFloat());
        }
    }
    return cloud;
}





/* 将点云单位从毫米转换为米 */
void MainWindow::unitMillimeter2UnitMeter(std::vector<QVector3D> &inOutCloud)
{
    for(auto& point : inOutCloud) {
        point /= 1000.0f; // 每个坐标分量除以1000
    }
}

/* 生成测试点云数据 */
std::vector<QVector3D> MainWindow::testData(int pointsNum)
{
    std::vector<QVector3D> cloud;
    cloud.reserve(pointsNum); // 预分配内存

    // 生成指定数量的随机点
    for (int i = 0; i < pointsNum; i++) {
        cloud.push_back(randomVec3f());
    }
    return cloud;
}

/* 生成随机三维向量 */
QVector3D MainWindow::randomVec3f()
{
    // 生成范围在[-25, 25]的X坐标
    float x = (QRandomGenerator::global()->bounded(100)) / 2.0f
              - (QRandomGenerator::global()->bounded(100)) / 2.0f;

    // 生成范围在[-25, 25]的Y坐标
    float y = (QRandomGenerator::global()->bounded(100)) / 2.0f
              - (QRandomGenerator::global()->bounded(100)) / 2.0f;

    // 生成范围在[-5, 5]的Z坐标
    float z = (QRandomGenerator::global()->bounded(100)) / 10.0f
              - (QRandomGenerator::global()->bounded(100)) / 10.0f;

    return QVector3D(x, y, z);
}

// ==================== 墙面提取模块相关方法 ====================

void MainWindow::initializeWallExtractionModule()
{
    try {
        // 创建墙面提取管理器
        m_wallExtractionManager = std::make_unique<WallExtraction::WallExtractionManager>(this);

        // 连接信号槽
        connect(m_wallExtractionManager.get(),
                &WallExtraction::WallExtractionManager::interactionModeChanged,
                this, &MainWindow::onWallExtractionModeChanged);

        connect(m_wallExtractionManager.get(),
                &WallExtraction::WallExtractionManager::statusMessageChanged,
                this, &MainWindow::onWallExtractionStatusChanged);

        connect(m_wallExtractionManager.get(),
                &WallExtraction::WallExtractionManager::errorOccurred,
                this, &MainWindow::onWallExtractionError);

        // 初始化管理器
        if (m_wallExtractionManager->initialize()) {
            qDebug() << "Wall extraction module initialized successfully";
            statusBar()->showMessage("Wall extraction module ready", 2000);
        } else {
            qCritical() << "Failed to initialize wall extraction module";
            QMessageBox::warning(this, "Warning",
                               "Failed to initialize wall extraction module");
        }

    } catch (const std::exception& e) {
        qCritical() << "Exception during wall extraction module initialization:" << e.what();
        QMessageBox::critical(this, "Error",
                            QString("Failed to initialize wall extraction module: %1").arg(e.what()));
    }
}

void MainWindow::activateWallExtraction()
{
    if (!m_wallExtractionManager) {
        qWarning() << "Wall extraction manager not initialized";
        return;
    }

    if (m_wallExtractionManager->activateModule()) {
        qDebug() << "Wall extraction module activated";
        // 这里可以添加UI状态更新
    } else {
        qWarning() << "Failed to activate wall extraction module";
        QMessageBox::warning(this, "Warning", "Failed to activate wall extraction module");
    }
}

void MainWindow::deactivateWallExtraction()
{
    if (!m_wallExtractionManager) {
        return;
    }

    if (m_wallExtractionManager->deactivateModule()) {
        qDebug() << "Wall extraction module deactivated";
        // 这里可以添加UI状态更新
    } else {
        qWarning() << "Failed to deactivate wall extraction module";
    }
}

void MainWindow::onWallExtractionModeChanged(WallExtraction::InteractionMode mode)
{
    QString modeString;
    switch (mode) {
        case WallExtraction::InteractionMode::PointCloudView:
            modeString = "Point Cloud View";
            break;
        case WallExtraction::InteractionMode::LineDrawing:
            modeString = "Line Drawing";
            break;
        case WallExtraction::InteractionMode::WallExtraction:
            modeString = "Wall Extraction";
            break;
        case WallExtraction::InteractionMode::WireframeView:
            modeString = "Wireframe View";
            break;
    }

    qDebug() << "Wall extraction mode changed to:" << modeString;
    statusBar()->showMessage(QString("Mode: %1").arg(modeString), 3000);
}

void MainWindow::onWallExtractionStatusChanged(const QString& message)
{
    qDebug() << "Wall extraction status:" << message;
    statusBar()->showMessage(message, 5000);
}

void MainWindow::onWallExtractionError(const QString& error)
{
    qCritical() << "Wall extraction error:" << error;
    QMessageBox::warning(this, "Wall Extraction Error", error);
}
