#include <QtTest/QtTest>
#include <QObject>
#include <memory>
#include "wall_extraction_manager.h"

class WallExtractionManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基础功能测试
    void testManagerCreation();
    void testManagerInitialization();
    void testModuleIntegration();
    void testSignalSlotConnections();
    
    // 状态管理测试
    void testInteractionModeSwitch();
    void testModuleActivation();
    void testModuleDeactivation();
    
    // 错误处理测试
    void testInvalidStateHandling();
    void testExceptionHandling();

private:
    std::unique_ptr<WallExtraction::WallExtractionManager> m_manager;
    QWidget* m_testParent;
};

void WallExtractionManagerTest::initTestCase()
{
    // 测试套件初始化
    qDebug() << "Starting WallExtractionManager test suite";
}

void WallExtractionManagerTest::cleanupTestCase()
{
    // 测试套件清理
    qDebug() << "Finished WallExtractionManager test suite";
}

void WallExtractionManagerTest::init()
{
    // 每个测试前的初始化
    m_testParent = new QWidget();
    m_manager = std::make_unique<WallExtraction::WallExtractionManager>(m_testParent);
}

void WallExtractionManagerTest::cleanup()
{
    // 每个测试后的清理
    m_manager.reset();
    delete m_testParent;
    m_testParent = nullptr;
}

void WallExtractionManagerTest::testManagerCreation()
{
    // 测试管理器创建
    QVERIFY(m_manager != nullptr);
    QVERIFY(m_manager->parent() == m_testParent);
    
    // 测试初始状态
    QCOMPARE(m_manager->isInitialized(), false);
    QCOMPARE(m_manager->getCurrentMode(), WallExtraction::InteractionMode::PointCloudView);
}

void WallExtractionManagerTest::testManagerInitialization()
{
    // 测试管理器初始化
    bool result = m_manager->initialize();
    QVERIFY(result);
    QVERIFY(m_manager->isInitialized());
    
    // 验证子组件是否正确创建
    QVERIFY(m_manager->getLineDrawingTool() != nullptr);
    QVERIFY(m_manager->getWallFittingAlgorithm() != nullptr);
    QVERIFY(m_manager->getWireframeGenerator() != nullptr);
}

void WallExtractionManagerTest::testModuleIntegration()
{
    // 测试与MainWindow的集成
    m_manager->initialize();
    
    // 验证信号连接
    QSignalSpy modeSwitchSpy(m_manager.get(), 
                            &WallExtraction::WallExtractionManager::interactionModeChanged);
    
    m_manager->setInteractionMode(WallExtraction::InteractionMode::LineDrawing);
    QCOMPARE(modeSwitchSpy.count(), 1);
    
    auto arguments = modeSwitchSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<WallExtraction::InteractionMode>(), 
             WallExtraction::InteractionMode::LineDrawing);
}

void WallExtractionManagerTest::testSignalSlotConnections()
{
    // 测试信号槽连接
    m_manager->initialize();
    
    QSignalSpy statusSpy(m_manager.get(), 
                        &WallExtraction::WallExtractionManager::statusMessageChanged);
    
    m_manager->setStatusMessage("Test status message");
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(statusSpy.first().first().toString(), QString("Test status message"));
}

void WallExtractionManagerTest::testInteractionModeSwitch()
{
    // 测试交互模式切换
    m_manager->initialize();
    
    // 测试所有模式切换
    QList<WallExtraction::InteractionMode> modes = {
        WallExtraction::InteractionMode::LineDrawing,
        WallExtraction::InteractionMode::WallExtraction,
        WallExtraction::InteractionMode::WireframeView,
        WallExtraction::InteractionMode::PointCloudView
    };
    
    for (auto mode : modes) {
        m_manager->setInteractionMode(mode);
        QCOMPARE(m_manager->getCurrentMode(), mode);
    }
}

void WallExtractionManagerTest::testModuleActivation()
{
    // 测试模块激活
    m_manager->initialize();
    
    bool result = m_manager->activateModule();
    QVERIFY(result);
    QVERIFY(m_manager->isActive());
}

void WallExtractionManagerTest::testModuleDeactivation()
{
    // 测试模块停用
    m_manager->initialize();
    m_manager->activateModule();
    
    bool result = m_manager->deactivateModule();
    QVERIFY(result);
    QVERIFY(!m_manager->isActive());
}

void WallExtractionManagerTest::testInvalidStateHandling()
{
    // 测试无效状态处理
    
    // 未初始化时的操作应该失败
    QVERIFY(!m_manager->activateModule());
    QVERIFY(!m_manager->setInteractionMode(WallExtraction::InteractionMode::LineDrawing));
}

void WallExtractionManagerTest::testExceptionHandling()
{
    // 测试异常处理
    m_manager->initialize();
    
    // 测试异常情况下的行为
    try {
        m_manager->processInvalidOperation();
        QFAIL("Expected WallExtractionException was not thrown");
    } catch (const WallExtraction::WallExtractionException& e) {
        QVERIFY(QString(e.what()).contains("Invalid operation"));
    }
}

QTEST_MAIN(WallExtractionManagerTest)
#include "wall_extraction_manager_test.moc"
