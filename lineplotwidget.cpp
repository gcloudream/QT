#include "LineplotWidget.h"
#include <QApplication>
#include <QMainWindow>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


void LineplotWidget::extractPolygons()
{
    m_polygons.clear();
    m_totalArea = 0.0;

    if (m_lines.isEmpty()) {
        return;
    }

    // 创建线段副本，用于标记已使用的线段
    QList<LineData> availableLines = m_lines;
    QList<bool> lineUsed(m_lines.size(), false);

    // 查找所有连通的多边形
    for (int i = 0; i < availableLines.size(); ++i) {
        if (lineUsed[i]) continue;

        QList<QPointF> polygonVertices;
        findConnectedPath(availableLines, polygonVertices, lineUsed);

        if (polygonVertices.size() >= 3) {
            Polygon polygon;
            polygon.vertices = polygonVertices;

            // 检查是否封闭
            polygon.isClosed = isPointEqual(polygonVertices.first(), polygonVertices.last());

            // 如果不封闭，尝试封闭它
            if (!polygon.isClosed && polygonVertices.size() > 2) {
                polygon.vertices.append(polygonVertices.first());
                polygon.isClosed = true;
            }

            // 计算面积
            if (polygon.isClosed && polygon.vertices.size() >= 4) {
                polygon.area = calculatePolygonArea(polygon.vertices);
                if (polygon.area > 0.01) { // 过滤面积太小的多边形
                    m_polygons.append(polygon);
                    m_totalArea += polygon.area;
                }
            }
        }
    }

    qDebug() << QString("提取到 %1 个多边形，总面积: %2 平方米")
                    .arg(m_polygons.size()).arg(m_totalArea, 0, 'f', 3);
}



void LineplotWidget::drawPolygons(QPainter &painter)
{
    if (m_polygons.isEmpty()) return;

    // 设置半透明填充
    QColor fillColors[] = {
        QColor(255, 0, 0, 30),    // 红色
        QColor(0, 255, 0, 30),    // 绿色
        QColor(0, 0, 255, 30),    // 蓝色
        QColor(255, 255, 0, 30),  // 黄色
        QColor(255, 0, 255, 30),  // 品红
        QColor(0, 255, 255, 30)   // 青色
    };

    // 计算绘图区域
    int uiHeight = 80;
    QRect plotRect(m_margin, m_margin + uiHeight,
                   width() - 2*m_margin, height() - 2*m_margin - uiHeight);

    for (int i = 0; i < m_polygons.size(); ++i) {
        const Polygon &polygon = m_polygons[i];

        if (polygon.isClosed && polygon.vertices.size() >= 3) {
            // 转换顶点坐标
            QPolygonF screenPolygon;
            bool hasValidPoints = false;

            for (const QPointF &vertex : polygon.vertices) {
                QPointF screenPoint;
                transformPoint(vertex, screenPoint);
                screenPolygon.append(screenPoint);

                // 检查是否有点在绘图区域内
                if (plotRect.contains(screenPoint.toPoint())) {
                    hasValidPoints = true;
                }
            }

            // 只有当多边形至少有一部分在绘图区域内时才绘制
            if (hasValidPoints || screenPolygon.boundingRect().intersects(plotRect)) {
                // 填充多边形
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(fillColors[i % 6]));
                painter.drawPolygon(screenPolygon);

                // 绘制边框
                painter.setPen(QPen(fillColors[i % 6].darker(150), 2));
                painter.setBrush(Qt::NoBrush);
                painter.drawPolygon(screenPolygon);
            }
        }
    }
}

void LineplotWidget::findConnectedPath(const QList<LineData> &lines, QList<QPointF> &path, QList<bool> &used)
{
    path.clear();

    // 找到第一个未使用的线段作为起点
    int startIndex = -1;
    for (int i = 0; i < lines.size(); ++i) {
        if (!used[i]) {
            startIndex = i;
            break;
        }
    }

    if (startIndex == -1) return;

    // 开始构建路径
    used[startIndex] = true;
    path.append(lines[startIndex].start);
    path.append(lines[startIndex].end);

    QPointF currentPoint = lines[startIndex].end;
    bool foundConnection = true;

    // 继续查找连接的线段
    while (foundConnection && path.size() < lines.size() * 2) {
        foundConnection = false;

        for (int i = 0; i < lines.size(); ++i) {
            if (used[i]) continue;

            const LineData &line = lines[i];

            // 检查当前点是否连接到这条线段
            if (isPointEqual(currentPoint, line.start)) {
                // 连接到线段的起点
                used[i] = true;
                currentPoint = line.end;
                path.append(currentPoint);
                foundConnection = true;
                break;
            }
            else if (isPointEqual(currentPoint, line.end)) {
                // 连接到线段的终点
                used[i] = true;
                currentPoint = line.start;
                path.append(currentPoint);
                foundConnection = true;
                break;
            }
        }

        // 检查是否形成了封闭路径
        if (foundConnection && path.size() > 3) {
            if (isPointEqual(currentPoint, path.first())) {
                break; // 找到封闭路径
            }
        }
    }
}


/**
 * @brief LineplotWidget::calculatePolygonArea
 * @param vertices
 * @return
 * 从散乱线段中识别封闭区域
 *路径连接：智能连接相邻线段形成连续路径
 *面积计算：使用鞋带公式计算真实几何面积
 *结果汇总：处理多个独立区域并统计总面积
 */
double LineplotWidget::calculatePolygonArea(const QList<QPointF> &vertices)
{
    if (vertices.size() < 3) {
        return 0.0;
    }

    double area = 0.0;
    int n = vertices.size();

    // 使用鞋带公式计算多边形面积
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += vertices[i].x() * vertices[j].y();
        area -= vertices[j].x() * vertices[i].y();
    }

    return std::abs(area) / 2.0;
}

double LineplotWidget::calculateTotalArea()
{
    return m_totalArea;
}

bool LineplotWidget::isPointEqual(const QPointF &p1, const QPointF &p2, double tolerance)
{
    if (tolerance <= 0) tolerance = 1e-6; // Default tolerance

    double dx = std::abs(p1.x() - p2.x());
    double dy = std::abs(p1.y() - p2.y());

    return (dx < tolerance) && (dy < tolerance);
}

LineplotWidget::LineplotWidget(QWidget *parent)
    : QWidget(parent)
    , m_totalArea(0.0)  // 新增：初始化总面积
    , m_minX(0), m_maxX(0), m_minY(0), m_maxY(0)
    , m_margin(80)
    , m_plotTitle("平面图线条显示 (单位: 米)")
{
    setupUI();

    // 设置绘图样式
    m_linePen.setColor(Qt::blue);
    m_linePen.setWidth(2);
    m_backgroundBrush.setColor(Qt::white);
    m_backgroundBrush.setStyle(Qt::SolidPattern);

    // 坐标轴样式
    m_axisPen.setColor(Qt::black);
    m_axisPen.setWidth(1);
    m_gridPen.setColor(Qt::lightGray);
    m_gridPen.setWidth(1);
    m_gridPen.setStyle(Qt::DotLine);

    setMinimumSize(800, 600);
    setWindowTitle("线条绘制查看器");
}

LineplotWidget::~LineplotWidget()
{
    // Qt的父子关系会自动处理清理工作
}

void LineplotWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // 按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_openButton = new QPushButton("打开文件", this);
    m_refreshButton = new QPushButton("刷新绘图", this);

    connect(m_openButton, &QPushButton::clicked, this, &LineplotWidget::openFile);
    connect(m_refreshButton, &QPushButton::clicked, this, &LineplotWidget::refreshPlot);

    m_buttonLayout->addWidget(m_openButton);
    m_buttonLayout->addWidget(m_refreshButton);
    m_buttonLayout->addStretch();

    // 文件路径标签
    m_filePathLabel = new QLabel("未选择文件", this);
    m_filePathLabel->setStyleSheet("QLabel { color: gray; font-size: 10pt; }");

    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_filePathLabel);
    // 注意：不需要添加this到布局中，因为绘图会在paintEvent中处理
    m_mainLayout->addStretch(1); // 添加伸缩空间给绘图区域
}

void LineplotWidget::setFilePath(const QString &path)
{
    m_filePath = path;
    QFileInfo fileInfo(path);
    m_filePathLabel->setText(QString("文件: %1").arg(fileInfo.fileName()));
}

void LineplotWidget::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开线条数据文件",
                                                    m_filePath,
                                                    "文本文件 (*.txt);;所有文件 (*)");

    if (!fileName.isEmpty()) {
        setFilePath(fileName);
        plotLinesFromFile();
    }
}

void LineplotWidget::refreshPlot()
{
    plotLinesFromFile();
}

void LineplotWidget::plotLinesFromFile()
{
    clearLines();

    if (m_filePath.isEmpty()) {
        qDebug() << "文件路径为空";
        return;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", QString("无法打开文件 %1").arg(m_filePath));
        qDebug() << "文件" << m_filePath << "打开失败";
        return;
    }

    int lineNumber = 0;
    int validLines = 0;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        // 跳过空行和注释行
        if (line.isEmpty() || line.startsWith("#") || line.startsWith("//")) {
            continue;
        }

        // 按空格分割并转换为浮点数
        QStringList values = line.split(' ', Qt::SkipEmptyParts);

        if (values.size() >= 4) {
            bool ok1, ok2, ok3, ok4;
            double x1 = values[0].toDouble(&ok1);
            double x2 = values[1].toDouble(&ok2);
            double y1 = values[2].toDouble(&ok3);
            double y2 = values[3].toDouble(&ok4);

            if (ok1 && ok2 && ok3 && ok4) {
                double dx = x2 - x1;
                double dy = y2 - y1;
                double length = std::sqrt(dx * dx + dy * dy);

                if (length > 0.01) {
                    m_lines.append(LineData(x1, y1, x2, y2));
                    validLines++;
                } else {
                    qDebug() << QString("第%1行线段过短已跳过 (长度: %2m): %3").arg(lineNumber).arg(length, 0, 'f', 3).arg(line);
                }
            } else {
                qDebug() << QString("第%1行数据格式错误: %2").arg(lineNumber).arg(line);
            }
        } else {
            qDebug() << QString("第%1行数据不足4个值: %2").arg(lineNumber).arg(line);
        }
    }

    file.close();

    qDebug() << QString("读取完成：总行数%1，有效线条%2").arg(lineNumber).arg(validLines);

    if (!m_lines.isEmpty()) {
        calculateBounds();
        extractPolygons(); // 新增：提取多边形并计算面积
        update();
    } else {
        QMessageBox::information(this, "信息", "文件中没有找到有效的线条数据");
    }
}

void LineplotWidget::clearLines()
{
    m_lines.clear();
    m_polygons.clear();
    m_totalArea = 0.0;
    update();
}

void LineplotWidget::calculateBounds()
{
    if (m_lines.isEmpty()) {
        // Set default bounds for empty case
        m_minX = m_maxX = 0.0;
        m_minY = m_maxY = 0.0;
        return;
    }

    // Initialize bounds with first point
    m_minX = m_maxX = m_lines[0].start.x();
    m_minY = m_maxY = m_lines[0].start.y();

    // Find min/max values
    for (const LineData &line : m_lines) {
        m_minX = std::min({m_minX, line.start.x(), line.end.x()});
        m_maxX = std::max({m_maxX, line.start.x(), line.end.x()});
        m_minY = std::min({m_minY, line.start.y(), line.end.y()});
        m_maxY = std::max({m_maxY, line.start.y(), line.end.y()});
    }

    // Add margin
    double rangeX = m_maxX - m_minX;
    double rangeY = m_maxY - m_minY;
    double padding = 0.1; // 10% margin

    // CRITICAL FIX: Ensure minimum range to prevent zero ranges
    if (rangeX < 0.001) {
        rangeX = 1.0;
        double center = (m_minX + m_maxX) / 2.0;
        m_minX = center - 0.5;
        m_maxX = center + 0.5;
    }
    if (rangeY < 0.001) {
        rangeY = 1.0;
        double center = (m_minY + m_maxY) / 2.0;
        m_minY = center - 0.5;
        m_maxY = center + 0.5;
    }

    m_minX -= rangeX * padding;
    m_maxX += rangeX * padding;
    m_minY -= rangeY * padding;
    m_maxY += rangeY * padding;
}

void LineplotWidget::transformPoint(const QPointF &point, QPointF &transformed)
{
    // Consider UI control space
    int uiHeight = 80;

    // Calculate actual drawing area size
    double plotWidth = width() - 2 * m_margin;
    double plotHeight = height() - 2 * m_margin - uiHeight;

    // Calculate data range
    double dataWidth = m_maxX - m_minX;
    double dataHeight = m_maxY - m_minY;

    // CRITICAL FIX: Add proper bounds checking
    if (dataWidth <= 0 || dataHeight <= 0 || plotWidth <= 0 || plotHeight <= 0) {
        transformed = QPointF(m_margin, m_margin + uiHeight);
        return; // ADDED: Prevent division by zero
    }

    // Calculate scale ratio
    double scaleX = plotWidth / dataWidth;
    double scaleY = plotHeight / dataHeight;
    double scale = std::min(scaleX, scaleY) * 0.9; // Leave some margin

    // Calculate centering offset
    double scaledDataWidth = dataWidth * scale;
    double scaledDataHeight = dataHeight * scale;
    double centerOffsetX = (plotWidth - scaledDataWidth) / 2.0;
    double centerOffsetY = (plotHeight - scaledDataHeight) / 2.0;

    // Convert from data coordinates to screen coordinates
    double x = (point.x() - m_minX) * scale + m_margin + centerOffsetX;
    double y = height() - ((point.y() - m_minY) * scale + m_margin + centerOffsetY + uiHeight);

    transformed.setX(x);
    transformed.setY(y);
}

double LineplotWidget::calculateLineLength(const LineData &line)
{
    double dx = line.end.x() - line.start.x();
    double dy = line.end.y() - line.start.y();
    return std::sqrt(dx * dx + dy * dy);
}

void LineplotWidget::drawLineLengthLabels(QPainter &painter)
{
    if (m_lines.isEmpty()) return;

    painter.setPen(QPen(Qt::darkRed, 1));
    QFont labelFont = painter.font();
    labelFont.setPointSize(8);
    labelFont.setBold(true);
    painter.setFont(labelFont);

    // 用于避免标注重叠的已使用区域列表
    QList<QRect> usedRects;

    // 存储已显示的长度和位置，避免重复标注
    struct LabelInfo {
        double length;
        QPointF position;
        QString text;
    };
    QList<LabelInfo> existingLabels;

    for (const LineData &line : m_lines) {
        QPointF start, end;
        transformPoint(line.start, start);
        transformPoint(line.end, end);

        // 计算线段长度
        double length = calculateLineLength(line);

        // 跳过小于0.1米(10厘米)的线段标注
        if (length < 0.1) {
            qDebug() << QString("跳过短线段标注，长度: %1m").arg(length, 0, 'f', 3);
            continue;
        }

        // 计算线段中点
        QPointF midPoint = (start + end) / 2.0;

        // 检查是否已有相似长度的标注在附近
        bool skipLabel = false;
        for (const LabelInfo &existing : existingLabels) {
            double lengthDiff = std::abs(existing.length - length);
            double distance = std::sqrt(std::pow(existing.position.x() - midPoint.x(), 2) +
                                        std::pow(existing.position.y() - midPoint.y(), 2));

            // 如果长度差小于2cm且距离小于50像素，跳过这个标注
            if (lengthDiff < 0.02 && distance < 50) {
                skipLabel = true;
                break;
            }
        }

        if (skipLabel) continue;

        // 计算线段角度
        double angle = std::atan2(end.y() - start.y(), end.x() - start.x()) * 180.0 / M_PI;

        // 如果角度太陡，让文本保持水平
        bool keepHorizontal = (std::abs(angle) > 60 && std::abs(angle) < 120) ||
                              (std::abs(angle) > 240 && std::abs(angle) < 300);

        // 格式化长度文本
        QString lengthText = QString("%1m").arg(length, 0, 'f', 2);

        // 计算文本偏移，避免与线段重叠
        QPointF offset(0, -20); // 默认向上偏移

        // 根据线段方向调整偏移
        if (!keepHorizontal) {
            double perpAngle = angle + 90.0;
            double radians = perpAngle * M_PI / 180.0;
            offset = QPointF(20 * std::cos(radians), 20 * std::sin(radians));
        }

        QPointF textPos = midPoint + offset;

        painter.save();

        // 创建半透明背景
        QFontMetrics fm(labelFont);
        QRect textRect = fm.boundingRect(lengthText);
        textRect.moveCenter(textPos.toPoint());
        textRect.adjust(-3, -2, 3, 2);

        // 检查是否与已有标注重叠，如果重叠则调整位置
        bool overlap = false;
        for (const QRect &usedRect : usedRects) {
            QRect expandedUsed = usedRect.adjusted(-10, -10, 10, 10);
            if (textRect.intersects(expandedUsed)) {
                overlap = true;
                break;
            }
        }

        // 如果重叠，尝试不同的偏移位置
        if (overlap) {
            QPointF alternativeOffsets[] = {
                QPointF(0, 25),    // 下方
                QPointF(-35, 0),   // 左侧
                QPointF(35, 0),    // 右侧
                QPointF(-25, -25), // 左上
                QPointF(25, -25),  // 右上
                QPointF(-25, 25),  // 左下
                QPointF(25, 25),   // 右下
                QPointF(0, -35),   // 更远的上方
                QPointF(-45, 0),   // 更远的左侧
                QPointF(45, 0)     // 更远的右侧
            };

            bool foundPosition = false;
            for (const QPointF &altOffset : alternativeOffsets) {
                QPointF altTextPos = midPoint + altOffset;
                QRect altTextRect = fm.boundingRect(lengthText);
                altTextRect.moveCenter(altTextPos.toPoint());
                altTextRect.adjust(-3, -2, 3, 2);

                bool altOverlap = false;
                for (const QRect &usedRect : usedRects) {
                    QRect expandedUsed = usedRect.adjusted(-10, -10, 10, 10);
                    if (altTextRect.intersects(expandedUsed)) {
                        altOverlap = true;
                        break;
                    }
                }

                if (!altOverlap) {
                    textPos = altTextPos;
                    textRect = altTextRect;
                    foundPosition = true;
                    break;
                }
            }

            // 如果所有位置都重叠，跳过这个标注
            if (!foundPosition) {
                painter.restore();
                continue;
            }
        }

        // 记录使用的区域和标注信息
        usedRects.append(textRect);
        LabelInfo labelInfo;
        labelInfo.length = length;
        labelInfo.position = midPoint;
        labelInfo.text = lengthText;
        existingLabels.append(labelInfo);

        painter.fillRect(textRect, QBrush(QColor(255, 255, 255, 190)));
        painter.setPen(QPen(Qt::lightGray, 1));
        painter.drawRect(textRect);

        // 绘制文本
        painter.setPen(QPen(Qt::darkRed, 1));

        if (!keepHorizontal && std::abs(angle) < 45) {
            // 对于接近水平的线段，可以旋转文本
            painter.translate(textPos);
            painter.rotate(angle);
            painter.drawText(QPoint(-textRect.width()/2, 5), lengthText);
        } else {
            // 其他情况保持文本水平
            painter.drawText(textRect, Qt::AlignCenter, lengthText);
        }

        painter.restore();
    }
}

double LineplotWidget::calculateTickInterval(double range)
{
    // 计算合适的刻度间隔
    if (range <= 0) return 1.0;

    double roughInterval = range / 8.0; // 目标约8个刻度
    double magnitude = std::pow(10.0, std::floor(std::log10(roughInterval)));
    double normalized = roughInterval / magnitude;

    if (normalized <= 1.0) return magnitude;
    else if (normalized <= 2.0) return 2.0 * magnitude;
    else if (normalized <= 5.0) return 5.0 * magnitude;
    else return 10.0 * magnitude;
}

void LineplotWidget::drawAxes(QPainter &painter)
{
    if (m_lines.isEmpty()) return;

    painter.setPen(m_axisPen);

    // 计算绘图区域（考虑顶部UI元素）
    int uiHeight = 80;
    int plotLeft = m_margin;
    int plotRight = width() - m_margin;
    int plotTop = m_margin + uiHeight;
    int plotBottom = height() - m_margin;

    // 绘制坐标轴边框
    painter.drawRect(plotLeft, plotTop, plotRight - plotLeft, plotBottom - plotTop);

    // 计算刻度间隔
    double xTickInterval = calculateTickInterval(m_maxX - m_minX);
    double yTickInterval = calculateTickInterval(m_maxY - m_minY);

    // 绘制X轴刻度和标签
    double xStart = std::ceil(m_minX / xTickInterval) * xTickInterval;
    for (double x = xStart; x <= m_maxX; x += xTickInterval) {
        QPointF dataPoint(x, m_minY);
        QPointF screenPoint;
        transformPoint(dataPoint, screenPoint);
        double screenX = screenPoint.x();

        if (screenX >= plotLeft && screenX <= plotRight) {
            // 绘制刻度线
            painter.setPen(m_axisPen);
            painter.drawLine(screenX, plotBottom, screenX, plotBottom + 5);

            // 绘制网格线
            painter.setPen(m_gridPen);
            painter.drawLine(screenX, plotTop, screenX, plotBottom);

            // 绘制标签
            painter.setPen(m_axisPen);
            QString label = QString::number(x, 'f', 1) + "m";
            QRect textRect(screenX - 30, plotBottom + 8, 60, 20);
            painter.drawText(textRect, Qt::AlignCenter, label);
        }
    }

    // 绘制Y轴刻度和标签
    double yStart = std::ceil(m_minY / yTickInterval) * yTickInterval;
    for (double y = yStart; y <= m_maxY; y += yTickInterval) {
        QPointF dataPoint(m_minX, y);
        QPointF screenPoint;
        transformPoint(dataPoint, screenPoint);
        double screenY = screenPoint.y();

        if (screenY >= plotTop && screenY <= plotBottom) {
            // 绘制刻度线
            painter.setPen(m_axisPen);
            painter.drawLine(plotLeft - 5, screenY, plotLeft, screenY);

            // 绘制网格线
            painter.setPen(m_gridPen);
            painter.drawLine(plotLeft, screenY, plotRight, screenY);

            // 绘制标签
            painter.setPen(m_axisPen);
            QString label = QString::number(y, 'f', 1) + "m";
            QRect textRect(5, screenY - 10, m_margin - 10, 20);
            painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, label);
        }
    }

    // 绘制坐标轴标题
    painter.setPen(m_axisPen);
    painter.save();
    painter.translate(15, (plotTop + plotBottom) / 2);
    painter.rotate(-90);
    painter.drawText(0, 0, "Y坐标 (米)");
    painter.restore();

    painter.drawText(QRect(0, height() - 25, width(), 20), Qt::AlignCenter, "X坐标 (米)");
}

void LineplotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 清除背景
    painter.fillRect(rect(), m_backgroundBrush);

    // 绘制标题
    painter.setPen(QPen(Qt::black, 1));
    QFont titleFont = painter.font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 50, width(), 30), Qt::AlignCenter, m_plotTitle);

    if (m_lines.isEmpty()) {
        painter.setPen(QPen(Qt::gray, 1));
        QFont normalFont = painter.font();
        normalFont.setBold(false);
        normalFont.setPointSize(10);
        painter.setFont(normalFont);
        painter.drawText(rect(), Qt::AlignCenter,
                         "无数据显示\n\n请打开包含线条数据的文件\n文件格式：每行包含四个数字 x1 y1 x2 y2");
        return;
    }

    // 设置普通字体
    QFont normalFont = painter.font();
    normalFont.setBold(false);
    normalFont.setPointSize(9);
    painter.setFont(normalFont);

    // 绘制坐标轴和网格
    drawAxes(painter);

    // 绘制线条
    painter.setPen(m_linePen);
    for (const LineData &line : m_lines) {
        QPointF start, end;
        transformPoint(line.start, start);
        transformPoint(line.end, end);

        // 确保线条在绘图区域内或穿过绘图区域
        int uiHeight = 80;
        QRect plotRect(m_margin, m_margin + uiHeight,
                       width() - 2*m_margin, height() - 2*m_margin - uiHeight);

        // 简化的裁剪检查 - 如果任一端点在区域内或线条可能穿过区域就绘制
        if (plotRect.contains(start.toPoint()) || plotRect.contains(end.toPoint()) ||
            (start.x() < plotRect.right() && end.x() > plotRect.left() &&
             start.y() < plotRect.bottom() && end.y() > plotRect.top())) {
            painter.drawLine(start, end);
        }
    }

    // 绘制长度标注
    drawLineLengthLabels(painter);

    // 绘制统计信息（包含面积信息）
    painter.setPen(QPen(Qt::darkBlue, 1));
    QFont infoFont = painter.font();
    infoFont.setPointSize(8);
    painter.setFont(infoFont);

    // 计算多边形信息
    QString polygonInfo = "";
    if (!m_polygons.isEmpty()) {
        polygonInfo = QString("多边形数量: %1\n精确总面积: %2 m²\n")
                          .arg(m_polygons.size())
                          .arg(m_totalArea, 0, 'f', 3);

        // 显示各个多边形的面积
        for (int i = 0; i < m_polygons.size() && i < 3; ++i) {
            polygonInfo += QString("多边形%1: %2 m²\n")
                               .arg(i + 1)
                               .arg(m_polygons[i].area, 0, 'f', 3);
        }
        if (m_polygons.size() > 3) {
            polygonInfo += "...\n";
        }
    }

    QString info = QString("线条数量: %1\n"
                           "X范围: %2m ~ %3m\n"
                           "Y范围: %4m ~ %5m\n"
                           "边界尺寸: %6m × %7m\n"
                           "%8")
                       .arg(m_lines.size())
                       .arg(m_minX, 0, 'f', 2)
                       .arg(m_maxX, 0, 'f', 2)
                       .arg(m_minY, 0, 'f', 2)
                       .arg(m_maxY, 0, 'f', 2)
                       .arg(m_maxX - m_minX, 0, 'f', 2)
                       .arg(m_maxY - m_minY, 0, 'f', 2)
                       .arg(polygonInfo);

    QRect infoRect(width() - 220, 90, 210, 160);
    painter.fillRect(infoRect, QBrush(QColor(255, 255, 255, 200))); // 半透明背景
    painter.setPen(QPen(Qt::lightGray, 1));
    painter.drawRect(infoRect);
    painter.setPen(QPen(Qt::darkBlue, 1));
    painter.drawText(infoRect.adjusted(5, 5, -5, -5), Qt::AlignLeft | Qt::AlignTop, info);
}
void LineplotWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_lines.isEmpty()) {
        calculateBounds();
    }
}
