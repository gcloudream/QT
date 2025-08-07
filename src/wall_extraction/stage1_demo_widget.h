#ifndef STAGE1_DEMO_WIDGET_H
#define STAGE1_DEMO_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QFileDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <QResizeEvent>
#include <QShowEvent>
#include <QFrame>
#include <memory>

// 前向声明
namespace WallExtraction {
    class WallExtractionManager;
    class TopDownViewRenderer;
    class ColorMappingManager;
    class PointCloudLODManager;
    class PointCloudMemoryManager;
    class SpatialIndex;
    class LineDrawingToolbar;
    class LinePropertyPanel;
    enum class ColorScheme;
    enum class TopDownRenderMode;
    struct PointWithAttributes;
}

/**
 * @brief 阶段一功能演示Widget
 * 
 * 展示阶段一完成的所有功能：
 * - 多格式点云支持
 * - LOD系统
 * - 俯视图渲染
 * - 颜色映射
 * - 性能优化
 */
class Stage1DemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Stage1DemoWidget(QWidget *parent = nullptr);
    ~Stage1DemoWidget();

    // 公共方法
    void forceLayoutUpdate();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    // 文件操作
    void loadPointCloudFile();
    void generateTestData();
    void clearPointCloud();
    
    // LOD控制
    void onLODLevelChanged(int level);
    void onLODStrategyChanged(int strategy);
    void generateLODLevels();
    
    // 颜色映射控制
    void onColorSchemeChanged(int scheme);
    void onColorRangeChanged();
    void generateColorBar();
    
    // 俯视图渲染控制
    void onRenderModeChanged(int mode);
    void onPointSizeChanged(double size);
    void renderTopDownView();
    void saveRenderResult();
    void saveCurrentImage();

    // 线段绘制控制
    void onLineDrawingModeChanged();
    void onLineSegmentAdded();
    void onLineSegmentSelected(int segmentId);
    void onLineSegmentRemoved(int segmentId);
    void toggleLinePropertyPanel();

    // 实时更新
    void updateRenderView();

private:
    // UI组件创建
    void setupUI();
    void createFileControlGroup();

    // 新的界面创建方法
    void createTopToolbar();
    void createControlPanel();
    void createStatusBar();

    // 紧凑控件创建方法
    void createFileControlButtons();
    void createRenderButtons();
    void createCompactLODControl();
    void createCompactColorControl();
    void createCompactRenderControl();
    void createLineDrawingControls();

    // 响应式样式管理
    QString getResponsiveButtonStyle(const QString& baseColor, bool isPrimary = false);
    QString getResponsiveGroupBoxStyle();
    QString getResponsiveLabelStyle(int level = 2);
    void updateResponsiveStyles();

    // 信号连接
    void connectSignals();
    void createLODControlGroup();
    void createColorMappingGroup();
    void createRenderControlGroup();
    void createDisplayArea();

    // 数据处理
    void processLoadedPointCloud();
    void updatePointCloudInfo();
    void generateSampleData(int pointCount);

    // 渲染更新
    void updateTopDownView();
    void updateColorMapping();
    void updateLODDisplay();

    // 俯视图渲染优化方法
    QSize calculateOptimalRenderSize() const;
    QRectF calculatePointCloudBounds() const;
    void optimizeColorMappingForTopDown();

    // 点云数据分析方法
    void analyzePointCloudData();
    void validatePointCloudAttributes();
    void debugRenderingPipeline();

    // 大数据量处理方法
    std::vector<WallExtraction::PointWithAttributes> performIntelligentSampling(
        const std::vector<WallExtraction::PointWithAttributes>& points,
        size_t targetCount) const;

    // 测试数据生成方法
    void generateValidTestData(int pointCount);

private:
    // 主布局
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    QVBoxLayout* m_displayLayout;

    // 新的界面组件
    QWidget* m_topToolbar;
    QWidget* m_controlPanel;
    QWidget* m_statusBar;

    // 紧凑控件组件
    QWidget* m_compactLODWidget;
    QWidget* m_compactColorWidget;
    QWidget* m_compactRenderWidget;

    // 线段绘制UI组件
    WallExtraction::LineDrawingToolbar* m_lineDrawingToolbar;
    WallExtraction::LinePropertyPanel* m_linePropertyPanel;
    
    // 控制组
    QGroupBox* m_fileControlGroup;
    QGroupBox* m_lodControlGroup;
    QGroupBox* m_colorMappingGroup;
    QGroupBox* m_renderControlGroup;
    
    // 文件控制
    QPushButton* m_loadFileButton;
    QPushButton* m_generateDataButton;
    QPushButton* m_clearDataButton;
    QLabel* m_fileInfoLabel;
    
    // LOD控制
    QComboBox* m_lodStrategyCombo;
    QSlider* m_lodLevelSlider;
    QLabel* m_lodLevelLabel;
    QPushButton* m_generateLODButton;
    QLabel* m_lodInfoLabel;
    
    // 颜色映射控制
    QComboBox* m_colorSchemeCombo;
    QDoubleSpinBox* m_minValueSpin;
    QDoubleSpinBox* m_maxValueSpin;
    QPushButton* m_generateColorBarButton;
    QLabel* m_colorBarLabel;
    
    // 渲染控制
    QComboBox* m_renderModeCombo;
    QDoubleSpinBox* m_pointSizeSpin;
    QPushButton* m_renderButton;
    QPushButton* m_saveImageButton;
    
    // 显示区域
    QLabel* m_renderDisplayLabel;
    
    // 状态标签已删除
    
    // 核心组件
    std::unique_ptr<WallExtraction::WallExtractionManager> m_wallManager;
    std::unique_ptr<WallExtraction::TopDownViewRenderer> m_renderer;
    std::unique_ptr<WallExtraction::ColorMappingManager> m_colorMapper;
    std::unique_ptr<WallExtraction::PointCloudLODManager> m_lodManager;
    std::unique_ptr<WallExtraction::PointCloudMemoryManager> m_memoryManager;
    std::unique_ptr<WallExtraction::SpatialIndex> m_spatialIndex;
    
    // 数据存储
    std::vector<WallExtraction::PointWithAttributes> m_currentPointCloud;
    std::vector<QVector3D> m_currentSimpleCloud;
    QString m_currentFileName;
    
    // 定时器
    QTimer* m_updateTimer;
    QElapsedTimer* m_performanceTimer;
    
    // 统计数据
    struct Statistics {
        size_t pointCount = 0;
        size_t memoryUsage = 0;
        qint64 lastRenderTime = 0;
        double fps = 0.0;
        int lodLevels = 0;
        size_t spatialIndexNodes = 0;
    } m_stats;
};

#endif // STAGE1_DEMO_WIDGET_H
