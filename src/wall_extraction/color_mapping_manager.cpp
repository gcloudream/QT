#include "color_mapping_manager.h"
#include "top_down_view_renderer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QImage>
#include <QtMath>
#include <algorithm>

namespace WallExtraction {

ColorMappingManager::ColorMappingManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_currentScheme(ColorScheme::Height)
    , m_currentSchemeName("Height")
    , m_minValue(0.0f)
    , m_maxValue(100.0f)
    , m_alpha(1.0f)
    , m_interpolationEnabled(true)
    , m_cacheValid(false)
{
    // 注册元类型
    qRegisterMetaType<ColorScheme>("ColorScheme");
    qRegisterMetaType<ColorSchemeDefinition>("ColorSchemeDefinition");
    qRegisterMetaType<ColorMappingResult>("ColorMappingResult");
    
    // 初始化内置颜色方案
    initializeBuiltinColorSchemes();
    
    m_initialized = true;
    qDebug() << "ColorMappingManager created with" << m_colorSchemes.size() << "color schemes";
}

ColorMappingManager::~ColorMappingManager()
{
    qDebug() << "ColorMappingManager destroyed";
}

bool ColorMappingManager::isInitialized() const
{
    return m_initialized;
}

void ColorMappingManager::setColorScheme(ColorScheme scheme)
{
    QString schemeName;
    switch (scheme) {
        case ColorScheme::Height:
            schemeName = "Height";
            break;
        case ColorScheme::Intensity:
            schemeName = "Intensity";
            break;
        case ColorScheme::Classification:
            schemeName = "Classification";
            break;
        case ColorScheme::RGB:
            schemeName = "RGB";
            break;
        case ColorScheme::Custom:
            schemeName = "Custom";
            break;
    }
    
    setColorScheme(schemeName);
}

void ColorMappingManager::setColorScheme(const QString& schemeName)
{
    if (m_colorSchemes.find(schemeName) == m_colorSchemes.end()) {
        emit errorOccurred(QString("Color scheme '%1' not found").arg(schemeName));
        return;
    }
    
    if (m_currentSchemeName != schemeName) {
        m_currentSchemeName = schemeName;
        
        // 更新当前方案枚举
        if (schemeName == "Height") {
            m_currentScheme = ColorScheme::Height;
        } else if (schemeName == "Intensity") {
            m_currentScheme = ColorScheme::Intensity;
        } else if (schemeName == "Classification") {
            m_currentScheme = ColorScheme::Classification;
        } else if (schemeName == "RGB") {
            m_currentScheme = ColorScheme::RGB;
        } else {
            m_currentScheme = ColorScheme::Custom;
        }
        
        // 清除缓存
        m_colorCache.clear();
        m_cacheValid = false;
        
        emit colorSchemeChanged(schemeName);
        emit statusMessage(QString("Color scheme changed to '%1'").arg(schemeName));
    }
}

ColorScheme ColorMappingManager::getCurrentColorScheme() const
{
    return m_currentScheme;
}

QStringList ColorMappingManager::getAvailableColorSchemes() const
{
    QStringList schemes;
    for (const auto& pair : m_colorSchemes) {
        schemes << pair.first;
    }
    return schemes;
}

void ColorMappingManager::setValueRange(float minValue, float maxValue)
{
    if (minValue >= maxValue) {
        emit errorOccurred("Invalid value range: min must be less than max");
        return;
    }
    
    if (m_minValue != minValue || m_maxValue != maxValue) {
        m_minValue = minValue;
        m_maxValue = maxValue;
        
        // 清除缓存
        m_colorCache.clear();
        m_cacheValid = false;
        
        emit valueRangeChanged(minValue, maxValue);
        emit statusMessage(QString("Value range set to [%1, %2]").arg(minValue).arg(maxValue));
    }
}

std::pair<float, float> ColorMappingManager::getValueRange() const
{
    return {m_minValue, m_maxValue};
}

void ColorMappingManager::autoCalculateValueRange(const std::vector<PointWithAttributes>& points, 
                                                 const QString& attributeName)
{
    if (points.empty()) {
        return;
    }
    
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();
    
    QString attrName = attributeName.isEmpty() ? 
        (m_currentScheme == ColorScheme::Height ? "z" : 
         m_currentScheme == ColorScheme::Intensity ? "intensity" : "classification") : 
        attributeName;
    
    for (const auto& point : points) {
        float value = 0.0f;
        
        if (attrName == "z" || attrName == "height") {
            value = point.position.z();
        } else if (point.attributes.contains(attrName)) {
            value = point.attributes[attrName].toFloat();
        } else {
            continue;
        }
        
        minVal = qMin(minVal, value);
        maxVal = qMax(maxVal, value);
    }
    
    if (minVal < maxVal) {
        // 添加一点边距
        float range = maxVal - minVal;
        float margin = range * 0.05f;
        setValueRange(minVal - margin, maxVal + margin);
    }
}

std::vector<ColoredPoint> ColorMappingManager::applyColorMapping(const std::vector<PointWithAttributes>& points)
{
    std::vector<ColoredPoint> coloredPoints;
    coloredPoints.reserve(points.size());
    
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& point = points[i];
        float value = extractValueFromPoint(point);
        QColor color = getColorForValue(value);
        
        // 应用透明度
        if (m_alpha < 1.0f) {
            color.setAlphaF(m_alpha);
        }
        
        ColoredPoint coloredPoint;
        coloredPoint.color = color;
        coloredPoint.originalIndex = i;
        coloredPoints.push_back(coloredPoint);
    }
    
    return coloredPoints;
}

std::vector<ColoredPoint> ColorMappingManager::applyColorMapping(const std::vector<QVector3D>& points)
{
    std::vector<ColoredPoint> coloredPoints;
    coloredPoints.reserve(points.size());
    
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& point = points[i];
        float value = 0.0f;
        
        // 对于仅坐标的点，只能使用高度映射
        if (m_currentScheme == ColorScheme::Height) {
            value = point.z();
        } else {
            // 其他方案使用默认颜色
            value = m_minValue;
        }
        
        QColor color = getColorForValue(value);
        
        // 应用透明度
        if (m_alpha < 1.0f) {
            color.setAlphaF(m_alpha);
        }
        
        ColoredPoint coloredPoint;
        coloredPoint.color = color;
        coloredPoint.originalIndex = i;
        coloredPoints.push_back(coloredPoint);
    }
    
    return coloredPoints;
}

QColor ColorMappingManager::getColorForValue(float value) const
{
    // 检查缓存
    if (m_cacheValid && m_colorCache.find(value) != m_colorCache.end()) {
        return m_colorCache[value];
    }
    
    auto it = m_colorSchemes.find(m_currentSchemeName);
    if (it == m_colorSchemes.end()) {
        return QColor(Qt::gray);
    }
    
    const ColorSchemeDefinition& scheme = it->second;
    float normalizedValue = normalizeValue(value);
    QColor color = findColorInScheme(scheme, normalizedValue);
    
    // 缓存结果
    m_colorCache[value] = color;
    
    return color;
}

ColorMappingResult ColorMappingManager::getColorMapping(float value) const
{
    QColor color = getColorForValue(value);
    float normalizedValue = normalizeValue(value);
    return ColorMappingResult(color, normalizedValue);
}

bool ColorMappingManager::addCustomColorScheme(const ColorSchemeDefinition& scheme)
{
    if (!validateColorScheme(scheme)) {
        emit errorOccurred(QString("Invalid color scheme: %1").arg(scheme.name));
        return false;
    }
    
    m_colorSchemes[scheme.name] = scheme;
    
    emit statusMessage(QString("Custom color scheme '%1' added").arg(scheme.name));
    return true;
}

bool ColorMappingManager::removeCustomColorScheme(const QString& schemeName)
{
    // 不允许删除内置方案
    QStringList builtinSchemes = {"Height", "Intensity", "Classification", "RGB"};
    if (builtinSchemes.contains(schemeName)) {
        emit errorOccurred("Cannot remove builtin color scheme");
        return false;
    }
    
    auto it = m_colorSchemes.find(schemeName);
    if (it == m_colorSchemes.end()) {
        return false;
    }
    
    m_colorSchemes.erase(it);
    
    // 如果删除的是当前方案，切换到默认方案
    if (m_currentSchemeName == schemeName) {
        setColorScheme(ColorScheme::Height);
    }
    
    emit statusMessage(QString("Custom color scheme '%1' removed").arg(schemeName));
    return true;
}

ColorSchemeDefinition ColorMappingManager::getColorSchemeDefinition(const QString& schemeName) const
{
    auto it = m_colorSchemes.find(schemeName);
    if (it != m_colorSchemes.end()) {
        return it->second;
    }
    return ColorSchemeDefinition();
}

void ColorMappingManager::setAlpha(float alpha)
{
    alpha = qBound(0.0f, alpha, 1.0f);
    if (m_alpha != alpha) {
        m_alpha = alpha;
        
        // 清除缓存
        m_colorCache.clear();
        m_cacheValid = false;
        
        emit statusMessage(QString("Alpha set to %1").arg(alpha));
    }
}

float ColorMappingManager::getAlpha() const
{
    return m_alpha;
}

void ColorMappingManager::setInterpolationEnabled(bool enabled)
{
    if (m_interpolationEnabled != enabled) {
        m_interpolationEnabled = enabled;
        
        // 清除缓存
        m_colorCache.clear();
        m_cacheValid = false;
        
        emit statusMessage(QString("Color interpolation %1").arg(enabled ? "enabled" : "disabled"));
    }
}

bool ColorMappingManager::isInterpolationEnabled() const
{
    return m_interpolationEnabled;
}

QImage ColorMappingManager::generateColorBar(int width, int height) const
{
    QImage colorBar(width, height, QImage::Format_ARGB32);
    
    auto it = m_colorSchemes.find(m_currentSchemeName);
    if (it == m_colorSchemes.end()) {
        colorBar.fill(Qt::gray);
        return colorBar;
    }
    
    const ColorSchemeDefinition& scheme = it->second;
    
    for (int x = 0; x < width; ++x) {
        float normalizedValue = static_cast<float>(x) / (width - 1);
        QColor color = findColorInScheme(scheme, normalizedValue);
        
        for (int y = 0; y < height; ++y) {
            colorBar.setPixelColor(x, y, color);
        }
    }
    
    return colorBar;
}

// 私有方法实现
void ColorMappingManager::initializeBuiltinColorSchemes()
{
    m_colorSchemes["Height"] = createHeightColorScheme();
    m_colorSchemes["Intensity"] = createIntensityColorScheme();
    m_colorSchemes["Classification"] = createClassificationColorScheme();
    
    // RGB方案（使用原始RGB值）
    ColorSchemeDefinition rgbScheme;
    rgbScheme.name = "RGB";
    rgbScheme.colors = {{0.0f, QColor(Qt::white)}}; // 占位符
    rgbScheme.isDiscrete = false;
    m_colorSchemes["RGB"] = rgbScheme;
}

ColorSchemeDefinition ColorMappingManager::createHeightColorScheme() const
{
    ColorSchemeDefinition scheme;
    scheme.name = "Height";
    scheme.isDiscrete = false;
    
    // 蓝色到红色的高度映射
    scheme.colors = {
        {0.0f, QColor(0, 0, 255)},      // 蓝色（低）
        {0.25f, QColor(0, 255, 255)},   // 青色
        {0.5f, QColor(0, 255, 0)},      // 绿色
        {0.75f, QColor(255, 255, 0)},   // 黄色
        {1.0f, QColor(255, 0, 0)}       // 红色（高）
    };
    
    return scheme;
}

ColorSchemeDefinition ColorMappingManager::createIntensityColorScheme() const
{
    ColorSchemeDefinition scheme;
    scheme.name = "Intensity";
    scheme.isDiscrete = false;
    
    // 黑色到白色的强度映射
    scheme.colors = {
        {0.0f, QColor(0, 0, 0)},        // 黑色（低强度）
        {1.0f, QColor(255, 255, 255)}   // 白色（高强度）
    };
    
    return scheme;
}

ColorSchemeDefinition ColorMappingManager::createClassificationColorScheme() const
{
    ColorSchemeDefinition scheme;
    scheme.name = "Classification";
    scheme.isDiscrete = true;
    
    // 标准LAS分类颜色
    scheme.colors = {
        {0.0f, QColor(128, 128, 128)},  // 未分类 - 灰色
        {1.0f, QColor(139, 69, 19)},    // 地面 - 棕色
        {2.0f, QColor(0, 128, 0)},      // 低植被 - 绿色
        {3.0f, QColor(0, 255, 0)},      // 中植被 - 亮绿色
        {4.0f, QColor(34, 139, 34)},    // 高植被 - 深绿色
        {5.0f, QColor(255, 0, 0)},      // 建筑物 - 红色
        {6.0f, QColor(255, 165, 0)},    // 噪声 - 橙色
        {7.0f, QColor(0, 0, 255)},      // 关键点 - 蓝色
        {8.0f, QColor(255, 192, 203)},  // 水体 - 粉色
        {9.0f, QColor(128, 0, 128)}     // 重叠点 - 紫色
    };
    
    return scheme;
}

QColor ColorMappingManager::interpolateColor(const QColor& color1, const QColor& color2, float t) const
{
    t = qBound(0.0f, t, 1.0f);

    int r = static_cast<int>(color1.red() * (1.0f - t) + color2.red() * t);
    int g = static_cast<int>(color1.green() * (1.0f - t) + color2.green() * t);
    int b = static_cast<int>(color1.blue() * (1.0f - t) + color2.blue() * t);
    int a = static_cast<int>(color1.alpha() * (1.0f - t) + color2.alpha() * t);

    return QColor(r, g, b, a);
}

QColor ColorMappingManager::findColorInScheme(const ColorSchemeDefinition& scheme, float normalizedValue) const
{
    if (scheme.colors.empty()) {
        return QColor(Qt::gray);
    }

    normalizedValue = qBound(0.0f, normalizedValue, 1.0f);

    // 对于离散方案（如分类），找最近的值
    if (scheme.isDiscrete) {
        float minDistance = std::numeric_limits<float>::max();
        QColor closestColor = scheme.colors[0].second;

        for (const auto& colorPair : scheme.colors) {
            float distance = qAbs(colorPair.first - normalizedValue);
            if (distance < minDistance) {
                minDistance = distance;
                closestColor = colorPair.second;
            }
        }

        return closestColor;
    }

    // 对于连续方案，进行插值
    if (scheme.colors.size() == 1) {
        return scheme.colors[0].second;
    }

    // 找到包围normalizedValue的两个颜色点
    for (size_t i = 0; i < scheme.colors.size() - 1; ++i) {
        float value1 = scheme.colors[i].first;
        float value2 = scheme.colors[i + 1].first;

        if (normalizedValue >= value1 && normalizedValue <= value2) {
            if (!m_interpolationEnabled || value1 == value2) {
                return scheme.colors[i].second;
            }

            float t = (normalizedValue - value1) / (value2 - value1);
            return interpolateColor(scheme.colors[i].second, scheme.colors[i + 1].second, t);
        }
    }

    // 如果超出范围，返回边界颜色
    if (normalizedValue < scheme.colors[0].first) {
        return scheme.colors[0].second;
    } else {
        return scheme.colors.back().second;
    }
}

float ColorMappingManager::normalizeValue(float value) const
{
    if (m_maxValue == m_minValue) {
        return 0.0f;
    }

    return (value - m_minValue) / (m_maxValue - m_minValue);
}

float ColorMappingManager::extractValueFromPoint(const PointWithAttributes& point) const
{
    switch (m_currentScheme) {
        case ColorScheme::Height:
            return point.position.z();

        case ColorScheme::Intensity:
            if (point.attributes.contains("intensity")) {
                return point.attributes["intensity"].toFloat();
            }
            break;

        case ColorScheme::Classification:
            if (point.attributes.contains("classification")) {
                return point.attributes["classification"].toFloat();
            }
            break;

        case ColorScheme::RGB:
            // RGB方案使用原始RGB值，这里返回亮度
            if (point.attributes.contains("red") &&
                point.attributes.contains("green") &&
                point.attributes.contains("blue")) {
                float r = point.attributes["red"].toFloat() / 65535.0f;
                float g = point.attributes["green"].toFloat() / 65535.0f;
                float b = point.attributes["blue"].toFloat() / 65535.0f;
                return (r + g + b) / 3.0f; // 平均亮度
            }
            break;

        case ColorScheme::Custom:
            // 自定义方案，尝试从属性中提取值
            if (point.attributes.contains("value")) {
                return point.attributes["value"].toFloat();
            }
            break;
    }

    // 默认返回高度
    return point.position.z();
}

bool ColorMappingManager::validateColorScheme(const ColorSchemeDefinition& scheme) const
{
    if (scheme.name.isEmpty()) {
        return false;
    }

    if (scheme.colors.empty()) {
        return false;
    }

    // 检查颜色值是否按顺序排列
    for (size_t i = 1; i < scheme.colors.size(); ++i) {
        if (scheme.colors[i].first < scheme.colors[i-1].first) {
            return false;
        }
    }

    return true;
}

bool ColorMappingManager::saveColorSchemes(const QString& filename) const
{
    QJsonObject root;
    QJsonArray schemesArray;

    for (const auto& pair : m_colorSchemes) {
        const ColorSchemeDefinition& scheme = pair.second;

        QJsonObject schemeObj;
        schemeObj["name"] = scheme.name;
        schemeObj["isDiscrete"] = scheme.isDiscrete;
        schemeObj["minValue"] = scheme.minValue;
        schemeObj["maxValue"] = scheme.maxValue;

        QJsonArray colorsArray;
        for (const auto& colorPair : scheme.colors) {
            QJsonObject colorObj;
            colorObj["value"] = colorPair.first;
            colorObj["red"] = colorPair.second.red();
            colorObj["green"] = colorPair.second.green();
            colorObj["blue"] = colorPair.second.blue();
            colorObj["alpha"] = colorPair.second.alpha();
            colorsArray.append(colorObj);
        }
        schemeObj["colors"] = colorsArray;

        schemesArray.append(schemeObj);
    }

    root["colorSchemes"] = schemesArray;

    QJsonDocument doc(root);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        return true;
    }

    return false;
}

bool ColorMappingManager::loadColorSchemes(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    QJsonArray schemesArray = root["colorSchemes"].toArray();

    for (const auto& value : schemesArray) {
        QJsonObject schemeObj = value.toObject();

        ColorSchemeDefinition scheme;
        scheme.name = schemeObj["name"].toString();
        scheme.isDiscrete = schemeObj["isDiscrete"].toBool();
        scheme.minValue = schemeObj["minValue"].toDouble();
        scheme.maxValue = schemeObj["maxValue"].toDouble();

        QJsonArray colorsArray = schemeObj["colors"].toArray();
        for (const auto& colorValue : colorsArray) {
            QJsonObject colorObj = colorValue.toObject();
            float val = colorObj["value"].toDouble();
            QColor color(colorObj["red"].toInt(),
                        colorObj["green"].toInt(),
                        colorObj["blue"].toInt(),
                        colorObj["alpha"].toInt());
            scheme.colors.emplace_back(val, color);
        }

        if (validateColorScheme(scheme)) {
            m_colorSchemes[scheme.name] = scheme;
        }
    }

    return true;
}

} // namespace WallExtraction
