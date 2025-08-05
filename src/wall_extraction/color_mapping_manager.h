#ifndef COLOR_MAPPING_MANAGER_H
#define COLOR_MAPPING_MANAGER_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector2D>
#include <vector>
#include <map>
#include "las_reader.h"

namespace WallExtraction {

// 前向声明
struct ColoredPoint;

// 颜色方案枚举
enum class ColorScheme {
    Height,         // 基于高度
    Intensity,      // 基于强度
    Classification, // 基于分类
    RGB,           // 原始RGB
    Custom         // 自定义
};

// 颜色方案定义
struct ColorSchemeDefinition {
    QString name;
    std::vector<std::pair<float, QColor>> colors; // 值-颜色对
    float minValue;
    float maxValue;
    bool isDiscrete;  // 是否为离散颜色（如分类）
    
    ColorSchemeDefinition() : minValue(0.0f), maxValue(1.0f), isDiscrete(false) {}
};

// 颜色映射结果
struct ColorMappingResult {
    QColor color;
    float normalizedValue;  // 归一化值 [0,1]
    bool isValid;
    
    ColorMappingResult() : normalizedValue(0.0f), isValid(false) {}
    ColorMappingResult(const QColor& c, float v) : color(c), normalizedValue(v), isValid(true) {}
};

/**
 * @brief 颜色映射管理器
 * 
 * 负责管理各种颜色映射方案，将点云属性值映射为颜色，
 * 支持高度、强度、分类等多种映射方式。
 */
class ColorMappingManager : public QObject
{
    Q_OBJECT

public:
    explicit ColorMappingManager(QObject* parent = nullptr);
    ~ColorMappingManager();

    /**
     * @brief 检查管理器是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const;

    /**
     * @brief 设置颜色方案
     * @param scheme 颜色方案
     */
    void setColorScheme(ColorScheme scheme);

    /**
     * @brief 设置颜色方案（按名称）
     * @param schemeName 方案名称
     */
    void setColorScheme(const QString& schemeName);

    /**
     * @brief 获取当前颜色方案
     * @return 当前方案
     */
    ColorScheme getCurrentColorScheme() const;

    /**
     * @brief 获取可用的颜色方案列表
     * @return 方案名称列表
     */
    QStringList getAvailableColorSchemes() const;

    /**
     * @brief 设置值范围
     * @param minValue 最小值
     * @param maxValue 最大值
     */
    void setValueRange(float minValue, float maxValue);

    /**
     * @brief 获取值范围
     * @return 值范围（最小值，最大值）
     */
    std::pair<float, float> getValueRange() const;

    /**
     * @brief 自动计算值范围
     * @param points 点云数据
     * @param attributeName 属性名称
     */
    void autoCalculateValueRange(const std::vector<PointWithAttributes>& points, 
                                const QString& attributeName = QString());

    /**
     * @brief 应用颜色映射
     * @param points 带属性的点云数据
     * @return 带颜色的点数据
     */
    std::vector<ColoredPoint> applyColorMapping(const std::vector<PointWithAttributes>& points);

    /**
     * @brief 应用颜色映射（仅坐标）
     * @param points 点云坐标数据
     * @return 带颜色的点数据
     */
    std::vector<ColoredPoint> applyColorMapping(const std::vector<QVector3D>& points);

    /**
     * @brief 获取指定值的颜色
     * @param value 值
     * @return 对应的颜色
     */
    QColor getColorForValue(float value) const;

    /**
     * @brief 获取颜色映射结果
     * @param value 值
     * @return 映射结果
     */
    ColorMappingResult getColorMapping(float value) const;

    /**
     * @brief 添加自定义颜色方案
     * @param scheme 颜色方案定义
     * @return 添加是否成功
     */
    bool addCustomColorScheme(const ColorSchemeDefinition& scheme);

    /**
     * @brief 移除自定义颜色方案
     * @param schemeName 方案名称
     * @return 移除是否成功
     */
    bool removeCustomColorScheme(const QString& schemeName);

    /**
     * @brief 获取颜色方案定义
     * @param schemeName 方案名称
     * @return 方案定义
     */
    ColorSchemeDefinition getColorSchemeDefinition(const QString& schemeName) const;

    /**
     * @brief 保存颜色方案到文件
     * @param filename 文件名
     * @return 保存是否成功
     */
    bool saveColorSchemes(const QString& filename) const;

    /**
     * @brief 从文件加载颜色方案
     * @param filename 文件名
     * @return 加载是否成功
     */
    bool loadColorSchemes(const QString& filename);

    /**
     * @brief 设置透明度
     * @param alpha 透明度 [0,1]
     */
    void setAlpha(float alpha);

    /**
     * @brief 获取透明度
     * @return 透明度值
     */
    float getAlpha() const;

    /**
     * @brief 启用/禁用颜色插值
     * @param enabled 是否启用
     */
    void setInterpolationEnabled(bool enabled);

    /**
     * @brief 检查颜色插值是否启用
     * @return 是否启用
     */
    bool isInterpolationEnabled() const;

    /**
     * @brief 生成颜色条图像
     * @param width 宽度
     * @param height 高度
     * @return 颜色条图像
     */
    QImage generateColorBar(int width, int height) const;

signals:
    /**
     * @brief 颜色方案改变信号
     * @param schemeName 新方案名称
     */
    void colorSchemeChanged(const QString& schemeName);

    /**
     * @brief 值范围改变信号
     * @param minValue 最小值
     * @param maxValue 最大值
     */
    void valueRangeChanged(float minValue, float maxValue);

    /**
     * @brief 状态消息信号
     * @param message 状态消息
     */
    void statusMessage(const QString& message);

    /**
     * @brief 错误发生信号
     * @param error 错误消息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 初始化内置颜色方案
     */
    void initializeBuiltinColorSchemes();

    /**
     * @brief 创建高度颜色方案
     * @return 颜色方案定义
     */
    ColorSchemeDefinition createHeightColorScheme() const;

    /**
     * @brief 创建强度颜色方案
     * @return 颜色方案定义
     */
    ColorSchemeDefinition createIntensityColorScheme() const;

    /**
     * @brief 创建分类颜色方案
     * @return 颜色方案定义
     */
    ColorSchemeDefinition createClassificationColorScheme() const;

    /**
     * @brief 线性插值颜色
     * @param color1 颜色1
     * @param color2 颜色2
     * @param t 插值参数 [0,1]
     * @return 插值后的颜色
     */
    QColor interpolateColor(const QColor& color1, const QColor& color2, float t) const;

    /**
     * @brief 在颜色方案中查找颜色
     * @param scheme 颜色方案
     * @param normalizedValue 归一化值 [0,1]
     * @return 对应的颜色
     */
    QColor findColorInScheme(const ColorSchemeDefinition& scheme, float normalizedValue) const;

    /**
     * @brief 归一化值
     * @param value 原始值
     * @return 归一化值 [0,1]
     */
    float normalizeValue(float value) const;

    /**
     * @brief 从点云属性提取值
     * @param point 点数据
     * @return 提取的值
     */
    float extractValueFromPoint(const PointWithAttributes& point) const;

    /**
     * @brief 验证颜色方案
     * @param scheme 颜色方案
     * @return 是否有效
     */
    bool validateColorScheme(const ColorSchemeDefinition& scheme) const;

private:
    bool m_initialized;
    ColorScheme m_currentScheme;
    QString m_currentSchemeName;
    float m_minValue;
    float m_maxValue;
    float m_alpha;
    bool m_interpolationEnabled;
    
    // 颜色方案存储
    std::map<QString, ColorSchemeDefinition> m_colorSchemes;
    
    // 缓存
    mutable std::map<float, QColor> m_colorCache;
    mutable bool m_cacheValid;
};

} // namespace WallExtraction

// 注册元类型
Q_DECLARE_METATYPE(WallExtraction::ColorScheme)
Q_DECLARE_METATYPE(WallExtraction::ColorSchemeDefinition)
Q_DECLARE_METATYPE(WallExtraction::ColorMappingResult)

#endif // COLOR_MAPPING_MANAGER_H
