#include "line_drawing_tool.h"
#include <QDebug>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QCursor>
#include <algorithm>
#include <functional>
#include <cmath>

namespace WallExtraction {

// LineSegment 方法实现
QJsonObject LineSegment::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["startPoint"] = QJsonArray{startPoint.x(), startPoint.y(), startPoint.z()};
    obj["endPoint"] = QJsonArray{endPoint.x(), endPoint.y(), endPoint.z()};
    obj["polylineId"] = polylineId;
    obj["description"] = description;
    obj["createdTime"] = createdTime.toString(Qt::ISODate);
    obj["color"] = color.name();
    return obj;
}

LineSegment LineSegment::fromJson(const QJsonObject& json)
{
    LineSegment segment;
    segment.id = json["id"].toInt();

    QJsonArray startArray = json["startPoint"].toArray();
    segment.startPoint = QVector3D(startArray[0].toDouble(),
                                  startArray[1].toDouble(),
                                  startArray[2].toDouble());

    QJsonArray endArray = json["endPoint"].toArray();
    segment.endPoint = QVector3D(endArray[0].toDouble(),
                                endArray[1].toDouble(),
                                endArray[2].toDouble());

    segment.polylineId = json["polylineId"].toInt();
    segment.description = json["description"].toString();
    segment.createdTime = QDateTime::fromString(json["createdTime"].toString(), Qt::ISODate);
    segment.color = QColor(json["color"].toString());
    segment.isSelected = false;

    return segment;
}

// Polyline 方法实现
float Polyline::totalLength(const std::vector<LineSegment>& segments) const
{
    float total = 0.0f;
    for (int segmentId : segmentIds) {
        auto it = std::find_if(segments.begin(), segments.end(),
                              [segmentId](const LineSegment& seg) { return seg.id == segmentId; });
        if (it != segments.end()) {
            total += it->length();
        }
    }
    return total;
}

QJsonObject Polyline::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;

    QJsonArray segmentArray;
    for (int segmentId : segmentIds) {
        segmentArray.append(segmentId);
    }
    obj["segmentIds"] = segmentArray;

    obj["description"] = description;
    obj["createdTime"] = createdTime.toString(Qt::ISODate);
    obj["isClosed"] = isClosed;
    obj["color"] = color.name();
    return obj;
}

Polyline Polyline::fromJson(const QJsonObject& json)
{
    Polyline polyline;
    polyline.id = json["id"].toInt();

    QJsonArray segmentArray = json["segmentIds"].toArray();
    for (const QJsonValue& value : segmentArray) {
        polyline.segmentIds.push_back(value.toInt());
    }

    polyline.description = json["description"].toString();
    polyline.createdTime = QDateTime::fromString(json["createdTime"].toString(), Qt::ISODate);
    polyline.isClosed = json["isClosed"].toBool();
    polyline.color = QColor(json["color"].toString());

    return polyline;
}

// LineDrawingTool 构造函数和基本方法
LineDrawingTool::LineDrawingTool(QWidget* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_parentWidget(parent)
    , m_currentDrawingMode(DrawingMode::None)
    , m_currentEditMode(EditMode::None)
    , m_nextSegmentId(1)
    , m_nextPolylineId(1)
    , m_isDrawing(false)
    , m_isEditing(false)
    , m_currentPolylineId(-1)
    , m_editingSegmentId(-1)
    , m_editingStartPoint(false)
    , m_selectionTolerance(5.0f)
    , m_endpointTolerance(8.0f)
    , m_defaultLineColor(Qt::blue)
    , m_selectedLineColor(Qt::red)
    , m_previewLineColor(Qt::gray)
{
    qDebug() << "LineDrawingTool created";
}

LineDrawingTool::~LineDrawingTool()
{
    qDebug() << "LineDrawingTool destroyed";
}

bool LineDrawingTool::initialize()
{
    if (m_initialized) {
        return true;
    }

    try {
        initializeDataStructures();
        setupEventHandling();

        m_initialized = true;
        qDebug() << "LineDrawingTool initialized successfully";

        return true;
    } catch (const std::exception& e) {
        qDebug() << "LineDrawingTool initialization failed:" << e.what();
        emit errorOccurred(QString("初始化失败: %1").arg(e.what()));
        return false;
    }
}

bool LineDrawingTool::isInitialized() const
{
    return m_initialized;
}

void LineDrawingTool::reset()
{
    clearAll();
    m_currentDrawingMode = DrawingMode::None;
    m_currentEditMode = EditMode::None;
    m_isDrawing = false;
    m_isEditing = false;
    m_currentPolylineId = -1;
    m_currentPolylineSegments.clear();

    qDebug() << "LineDrawingTool reset";
}

void LineDrawingTool::initializeDataStructures()
{
    m_lineSegments.clear();
    m_polylines.clear();
    m_selectedSegmentIds.clear();
    m_highlightedSegments.clear();
    m_currentPolylineSegments.clear();

    m_nextSegmentId = 1;
    m_nextPolylineId = 1;
}

void LineDrawingTool::setupEventHandling()
{
    // 设置内部更新定时器（如果需要）
    // 这里可以添加定时器来处理实时更新
}

// 绘制模式管理
DrawingMode LineDrawingTool::getDrawingMode() const
{
    return m_currentDrawingMode;
}

void LineDrawingTool::setDrawingMode(DrawingMode mode)
{
    if (m_currentDrawingMode != mode) {
        // 取消当前操作
        if (m_isDrawing) {
            m_isDrawing = false;
            emit operationCancelled("绘制操作已取消");
        }

        m_currentDrawingMode = mode;

        // 重置相关状态
        if (mode != DrawingMode::Edit) {
            m_currentEditMode = EditMode::None;
        }

        emit drawingModeChanged(mode);
        qDebug() << "Drawing mode changed to:" << static_cast<int>(mode);
    }
}

EditMode LineDrawingTool::getEditMode() const
{
    return m_currentEditMode;
}

void LineDrawingTool::setEditMode(EditMode mode)
{
    if (m_currentEditMode != mode) {
        // 取消当前编辑操作
        if (m_isEditing) {
            m_isEditing = false;
            emit operationCancelled("编辑操作已取消");
        }

        m_currentEditMode = mode;
        emit editModeChanged(mode);
        qDebug() << "Edit mode changed to:" << static_cast<int>(mode);
    }
}

// 数据访问
const std::vector<LineSegment>& LineDrawingTool::getLineSegments() const
{
    return m_lineSegments;
}

const std::vector<Polyline>& LineDrawingTool::getPolylines() const
{
    return m_polylines;
}

std::vector<LineSegmentInfo> LineDrawingTool::getSegmentInfoList() const
{
    std::vector<LineSegmentInfo> infoList;
    infoList.reserve(m_lineSegments.size());

    for (const auto& segment : m_lineSegments) {
        LineSegmentInfo info;
        info.id = segment.id;
        info.startPoint = segment.startPoint;
        info.endPoint = segment.endPoint;
        info.length = segment.length();
        info.polylineId = segment.polylineId;
        info.description = segment.description;
        info.createdTime = segment.createdTime;
        info.isSelected = segment.isSelected;
        infoList.push_back(info);
    }

    return infoList;
}

std::vector<LineSegmentInfo> LineDrawingTool::getSortedSegmentInfoList(SortCriteria criteria, bool ascending) const
{
    std::vector<LineSegmentInfo> infoList = getSegmentInfoList();

    // 排序函数
    std::function<bool(const LineSegmentInfo&, const LineSegmentInfo&)> compareFn;

    switch (criteria) {
        case SortCriteria::ById:
            compareFn = [](const LineSegmentInfo& a, const LineSegmentInfo& b) {
                return a.id < b.id;
            };
            break;
        case SortCriteria::ByLength:
            compareFn = [](const LineSegmentInfo& a, const LineSegmentInfo& b) {
                return a.length < b.length;
            };
            break;
        case SortCriteria::ByCreatedTime:
            compareFn = [](const LineSegmentInfo& a, const LineSegmentInfo& b) {
                return a.createdTime < b.createdTime;
            };
            break;
        case SortCriteria::BySelection:
            compareFn = [](const LineSegmentInfo& a, const LineSegmentInfo& b) {
                return a.isSelected < b.isSelected;
            };
            break;
    }

    if (ascending) {
        std::sort(infoList.begin(), infoList.end(), compareFn);
    } else {
        std::sort(infoList.begin(), infoList.end(), [&compareFn](const LineSegmentInfo& a, const LineSegmentInfo& b) {
            return !compareFn(a, b);
        });
    }

    return infoList;
}

LineSegmentInfo LineDrawingTool::getSegmentInfo(int segmentId) const
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    if (it != m_lineSegments.end()) {
        LineSegmentInfo info;
        info.id = it->id;
        info.startPoint = it->startPoint;
        info.endPoint = it->endPoint;
        info.length = it->length();
        info.polylineId = it->polylineId;
        info.description = it->description;
        info.createdTime = it->createdTime;
        info.isSelected = it->isSelected;
        return info;
    }

    // 返回无效的信息
    LineSegmentInfo invalidInfo;
    invalidInfo.id = -1;
    return invalidInfo;
}

// 选择管理
const std::unordered_set<int>& LineDrawingTool::getSelectedSegmentIds() const
{
    return m_selectedSegmentIds;
}

void LineDrawingTool::selectSegment(int segmentId, bool multiSelect)
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    if (it == m_lineSegments.end()) {
        qDebug() << "Segment not found:" << segmentId;
        return;
    }

    // 如果不是多选，先清除其他选择
    if (!multiSelect) {
        clearSelection();
    }

    // 选择指定线段
    if (m_selectedSegmentIds.find(segmentId) == m_selectedSegmentIds.end()) {
        m_selectedSegmentIds.insert(segmentId);
        it->isSelected = true;
        emit lineSegmentSelected(segmentId);
        qDebug() << "Segment selected:" << segmentId;
    }
}

void LineDrawingTool::deselectSegment(int segmentId)
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    if (it != m_lineSegments.end() && m_selectedSegmentIds.erase(segmentId) > 0) {
        it->isSelected = false;
        emit lineSegmentDeselected(segmentId);
        qDebug() << "Segment deselected:" << segmentId;
    }
}

void LineDrawingTool::clearSelection()
{
    for (int segmentId : m_selectedSegmentIds) {
        auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                              [segmentId](LineSegment& seg) { return seg.id == segmentId; });
        if (it != m_lineSegments.end()) {
            it->isSelected = false;
            emit lineSegmentDeselected(segmentId);
        }
    }

    m_selectedSegmentIds.clear();
    qDebug() << "Selection cleared";
}

void LineDrawingTool::selectAll()
{
    for (auto& segment : m_lineSegments) {
        if (!segment.isSelected) {
            segment.isSelected = true;
            m_selectedSegmentIds.insert(segment.id);
            emit lineSegmentSelected(segment.id);
        }
    }

    qDebug() << "All segments selected:" << m_selectedSegmentIds.size();
}

std::vector<int> LineDrawingTool::getSelectedSegmentIdsVector() const
{
    std::vector<int> result;
    result.reserve(m_selectedSegmentIds.size());
    for (int id : m_selectedSegmentIds) {
        result.push_back(id);
    }
    return result;
}

void LineDrawingTool::deleteSelectedSegments()
{
    if (m_selectedSegmentIds.empty()) {
        qDebug() << "No segments selected for deletion";
        return;
    }

    std::vector<int> idsToDelete(m_selectedSegmentIds.begin(), m_selectedSegmentIds.end());

    for (int segmentId : idsToDelete) {
        removeLineSegment(segmentId);
    }

    m_selectedSegmentIds.clear();
    emit operationCompleted(QString("已删除 %1 个线段").arg(idsToDelete.size()));
    qDebug() << "Deleted" << idsToDelete.size() << "selected segments";
}

// 线段操作
int LineDrawingTool::addLineSegment(const QVector3D& startPoint, const QVector3D& endPoint,
                                   int polylineId, const QString& description)
{
    LineSegment segment;
    segment.id = m_nextSegmentId++;
    segment.startPoint = startPoint;
    segment.endPoint = endPoint;
    segment.polylineId = polylineId;
    segment.description = description;
    segment.createdTime = QDateTime::currentDateTime();
    segment.isSelected = false;
    segment.color = m_defaultLineColor;

    m_lineSegments.push_back(segment);

    emit lineSegmentAdded(segment);
    qDebug() << "Line segment added:" << segment.id << "length:" << segment.length();

    return segment.id;
}

bool LineDrawingTool::removeLineSegment(int segmentId)
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    if (it == m_lineSegments.end()) {
        qDebug() << "Segment not found for removal:" << segmentId;
        return false;
    }

    // 从选择集合中移除
    m_selectedSegmentIds.erase(segmentId);

    // 从多段线中移除（如果属于多段线）
    if (it->polylineId != -1) {
        auto polylineIt = std::find_if(m_polylines.begin(), m_polylines.end(),
                                      [polylineId = it->polylineId](const Polyline& poly) {
                                          return poly.id == polylineId;
                                      });
        if (polylineIt != m_polylines.end()) {
            auto& segmentIds = polylineIt->segmentIds;
            segmentIds.erase(std::remove(segmentIds.begin(), segmentIds.end(), segmentId),
                           segmentIds.end());
        }
    }

    m_lineSegments.erase(it);

    emit lineSegmentRemoved(segmentId);
    qDebug() << "Line segment removed:" << segmentId;

    return true;
}

bool LineDrawingTool::updateLineSegment(int segmentId, const LineSegment& newSegment)
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    if (it == m_lineSegments.end()) {
        qDebug() << "Segment not found for update:" << segmentId;
        return false;
    }

    // 保持ID和选择状态
    LineSegment updatedSegment = newSegment;
    updatedSegment.id = segmentId;
    updatedSegment.isSelected = it->isSelected;

    *it = updatedSegment;

    emit lineSegmentUpdated(segmentId, updatedSegment);
    qDebug() << "Line segment updated:" << segmentId;

    return true;
}

bool LineDrawingTool::updateSegmentDescription(int segmentId, const QString& description)
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    if (it == m_lineSegments.end()) {
        qDebug() << "Segment not found for description update:" << segmentId;
        return false;
    }

    it->description = description;

    emit lineSegmentUpdated(segmentId, *it);
    qDebug() << "Segment description updated:" << segmentId << "to:" << description;

    return true;
}

LineSegment* LineDrawingTool::getLineSegment(int segmentId)
{
    auto it = std::find_if(m_lineSegments.begin(), m_lineSegments.end(),
                          [segmentId](const LineSegment& seg) { return seg.id == segmentId; });

    return (it != m_lineSegments.end()) ? &(*it) : nullptr;
}

// 多段线操作
int LineDrawingTool::createPolyline(const std::vector<int>& segmentIds, const QString& description)
{
    if (segmentIds.empty()) {
        qDebug() << "Cannot create polyline with empty segment list";
        return -1;
    }

    // 验证所有线段都存在
    for (int segmentId : segmentIds) {
        if (!getLineSegment(segmentId)) {
            qDebug() << "Segment not found for polyline creation:" << segmentId;
            return -1;
        }
    }

    Polyline polyline;
    polyline.id = m_nextPolylineId++;
    polyline.segmentIds = segmentIds;
    polyline.description = description;
    polyline.createdTime = QDateTime::currentDateTime();
    polyline.isClosed = false;
    polyline.color = Qt::darkBlue;

    // 更新线段的多段线ID
    for (int segmentId : segmentIds) {
        LineSegment* segment = getLineSegment(segmentId);
        if (segment) {
            segment->polylineId = polyline.id;
        }
    }

    m_polylines.push_back(polyline);

    emit polylineAdded(polyline);
    qDebug() << "Polyline created:" << polyline.id << "with" << segmentIds.size() << "segments";

    return polyline.id;
}

bool LineDrawingTool::removePolyline(int polylineId)
{
    auto it = std::find_if(m_polylines.begin(), m_polylines.end(),
                          [polylineId](const Polyline& poly) { return poly.id == polylineId; });

    if (it == m_polylines.end()) {
        qDebug() << "Polyline not found for removal:" << polylineId;
        return false;
    }

    // 更新相关线段的多段线ID
    for (int segmentId : it->segmentIds) {
        LineSegment* segment = getLineSegment(segmentId);
        if (segment) {
            segment->polylineId = -1;
        }
    }

    m_polylines.erase(it);

    emit polylineRemoved(polylineId);
    qDebug() << "Polyline removed:" << polylineId;

    return true;
}

bool LineDrawingTool::updatePolyline(int polylineId, const Polyline& newPolyline)
{
    auto it = std::find_if(m_polylines.begin(), m_polylines.end(),
                          [polylineId](const Polyline& poly) { return poly.id == polylineId; });

    if (it == m_polylines.end()) {
        qDebug() << "Polyline not found for update:" << polylineId;
        return false;
    }

    // 保持ID
    Polyline updatedPolyline = newPolyline;
    updatedPolyline.id = polylineId;

    *it = updatedPolyline;

    emit polylineUpdated(polylineId, updatedPolyline);
    qDebug() << "Polyline updated:" << polylineId;

    return true;
}

Polyline* LineDrawingTool::getPolyline(int polylineId)
{
    auto it = std::find_if(m_polylines.begin(), m_polylines.end(),
                          [polylineId](const Polyline& poly) { return poly.id == polylineId; });

    return (it != m_polylines.end()) ? &(*it) : nullptr;
}

// 工具方法
void LineDrawingTool::clearAll()
{
    m_lineSegments.clear();
    m_polylines.clear();
    m_selectedSegmentIds.clear();
    m_highlightedSegments.clear();
    m_currentPolylineSegments.clear();

    m_nextSegmentId = 1;
    m_nextPolylineId = 1;

    qDebug() << "All data cleared";
}

void LineDrawingTool::clearAllSegments()
{
    m_lineSegments.clear();
    m_selectedSegmentIds.clear();
    m_highlightedSegments.clear();
    m_currentPolylineSegments.clear();

    m_nextSegmentId = 1;

    emit operationCompleted("所有线段已清除");
    qDebug() << "All segments cleared";
}

void LineDrawingTool::clearAllPolylines()
{
    m_polylines.clear();
    m_nextPolylineId = 1;

    emit operationCompleted("所有多段线已清除");
    qDebug() << "All polylines cleared";
}

int LineDrawingTool::getNextSegmentId() const
{
    return m_nextSegmentId;
}

int LineDrawingTool::getNextPolylineId() const
{
    return m_nextPolylineId;
}

// 统计信息
int LineDrawingTool::getTotalSegmentCount() const
{
    return static_cast<int>(m_lineSegments.size());
}

int LineDrawingTool::getSelectedSegmentCount() const
{
    return static_cast<int>(m_selectedSegmentIds.size());
}

int LineDrawingTool::getTotalPolylineCount() const
{
    return static_cast<int>(m_polylines.size());
}

float LineDrawingTool::getTotalLength() const
{
    float totalLength = 0.0f;
    for (const auto& segment : m_lineSegments) {
        totalLength += segment.length();
    }
    return totalLength;
}

// 数据持久化
bool LineDrawingTool::saveToFile(const QString& filename) const
{
    try {
        QJsonDocument document = exportToJson();

        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot open file for writing:" << filename;
            return false;
        }

        file.write(document.toJson());
        file.close();

        qDebug() << "Data saved to file:" << filename;
        return true;

    } catch (const std::exception& e) {
        qDebug() << "Error saving to file:" << e.what();
        return false;
    }
}

bool LineDrawingTool::loadFromFile(const QString& filename)
{
    try {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open file for reading:" << filename;
            return false;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qDebug() << "JSON parse error:" << error.errorString();
            return false;
        }

        bool success = importFromJson(document);
        if (success) {
            qDebug() << "Data loaded from file:" << filename;
        }

        return success;

    } catch (const std::exception& e) {
        qDebug() << "Error loading from file:" << e.what();
        return false;
    }
}

QJsonDocument LineDrawingTool::exportToJson() const
{
    QJsonObject root;

    // 版本信息
    root["version"] = "1.0";
    root["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 线段数据
    QJsonArray segmentsArray;
    for (const auto& segment : m_lineSegments) {
        segmentsArray.append(segment.toJson());
    }
    root["lineSegments"] = segmentsArray;

    // 多段线数据
    QJsonArray polylinesArray;
    for (const auto& polyline : m_polylines) {
        polylinesArray.append(polyline.toJson());
    }
    root["polylines"] = polylinesArray;

    // 统计信息
    QJsonObject stats;
    stats["totalSegments"] = getTotalSegmentCount();
    stats["totalPolylines"] = getTotalPolylineCount();
    stats["totalLength"] = getTotalLength();
    root["statistics"] = stats;

    return QJsonDocument(root);
}

bool LineDrawingTool::importFromJson(const QJsonDocument& document)
{
    try {
        QJsonObject root = document.object();

        // 检查版本
        QString version = root["version"].toString();
        if (version != "1.0") {
            qDebug() << "Unsupported version:" << version;
            // 可以在这里添加版本兼容性处理
        }

        // 清除现有数据
        clearAll();

        // 导入线段数据
        QJsonArray segmentsArray = root["lineSegments"].toArray();
        for (const QJsonValue& value : segmentsArray) {
            LineSegment segment = LineSegment::fromJson(value.toObject());
            m_lineSegments.push_back(segment);

            // 更新ID计数器
            if (segment.id >= m_nextSegmentId) {
                m_nextSegmentId = segment.id + 1;
            }
        }

        // 导入多段线数据
        QJsonArray polylinesArray = root["polylines"].toArray();
        for (const QJsonValue& value : polylinesArray) {
            Polyline polyline = Polyline::fromJson(value.toObject());
            m_polylines.push_back(polyline);

            // 更新ID计数器
            if (polyline.id >= m_nextPolylineId) {
                m_nextPolylineId = polyline.id + 1;
            }
        }

        // 验证数据完整性
        validateDataIntegrity();

        qDebug() << "Imported" << m_lineSegments.size() << "segments and"
                 << m_polylines.size() << "polylines";

        return true;

    } catch (const std::exception& e) {
        qDebug() << "Error importing JSON:" << e.what();
        return false;
    }
}

void LineDrawingTool::validateDataIntegrity()
{
    // 验证多段线中的线段ID是否都存在
    for (auto& polyline : m_polylines) {
        auto& segmentIds = polyline.segmentIds;
        segmentIds.erase(
            std::remove_if(segmentIds.begin(), segmentIds.end(),
                          [this](int segmentId) {
                              return !getLineSegment(segmentId);
                          }),
            segmentIds.end()
        );
    }

    // 移除空的多段线
    m_polylines.erase(
        std::remove_if(m_polylines.begin(), m_polylines.end(),
                      [](const Polyline& polyline) {
                          return polyline.segmentIds.empty();
                      }),
        m_polylines.end()
    );

    qDebug() << "Data integrity validated";
}

// 编辑操作
bool LineDrawingTool::splitSegment(int segmentId, const QVector3D& splitPoint)
{
    LineSegment* segment = getLineSegment(segmentId);
    if (!segment) {
        qDebug() << "Segment not found for splitting:" << segmentId;
        return false;
    }

    // 创建两个新线段
    QVector3D originalStart = segment->startPoint;
    QVector3D originalEnd = segment->endPoint;

    // 第一段：原起点到分割点
    segment->endPoint = splitPoint;

    // 第二段：分割点到原终点
    int newSegmentId = addLineSegment(splitPoint, originalEnd, segment->polylineId,
                                     segment->description + " (分割)");

    // 如果属于多段线，更新多段线的线段列表
    if (segment->polylineId != -1) {
        Polyline* polyline = getPolyline(segment->polylineId);
        if (polyline) {
            auto& segmentIds = polyline->segmentIds;
            auto it = std::find(segmentIds.begin(), segmentIds.end(), segmentId);
            if (it != segmentIds.end()) {
                // 在原线段后插入新线段
                segmentIds.insert(it + 1, newSegmentId);
            }
        }
    }

    emit lineSegmentUpdated(segmentId, *segment);
    qDebug() << "Segment split:" << segmentId << "-> new segment:" << newSegmentId;

    return true;
}

bool LineDrawingTool::mergeSegments(int segmentId1, int segmentId2)
{
    LineSegment* segment1 = getLineSegment(segmentId1);
    LineSegment* segment2 = getLineSegment(segmentId2);

    if (!segment1 || !segment2) {
        qDebug() << "One or both segments not found for merging:" << segmentId1 << segmentId2;
        return false;
    }

    // 检查线段是否可以合并（端点相连）
    QVector3D newStart, newEnd;
    bool canMerge = false;

    if (segment1->endPoint == segment2->startPoint) {
        // segment1 -> segment2
        newStart = segment1->startPoint;
        newEnd = segment2->endPoint;
        canMerge = true;
    } else if (segment1->startPoint == segment2->endPoint) {
        // segment2 -> segment1
        newStart = segment2->startPoint;
        newEnd = segment1->endPoint;
        canMerge = true;
    } else if (segment1->endPoint == segment2->endPoint) {
        // segment1 -> reverse(segment2)
        newStart = segment1->startPoint;
        newEnd = segment2->startPoint;
        canMerge = true;
    } else if (segment1->startPoint == segment2->startPoint) {
        // reverse(segment1) -> segment2
        newStart = segment1->endPoint;
        newEnd = segment2->endPoint;
        canMerge = true;
    }

    if (!canMerge) {
        qDebug() << "Segments cannot be merged - not connected:" << segmentId1 << segmentId2;
        return false;
    }

    // 更新第一个线段
    segment1->startPoint = newStart;
    segment1->endPoint = newEnd;
    segment1->description += " (合并)";

    // 删除第二个线段
    removeLineSegment(segmentId2);

    emit lineSegmentUpdated(segmentId1, *segment1);
    qDebug() << "Segments merged:" << segmentId1 << "and" << segmentId2;

    return true;
}

bool LineDrawingTool::moveEndpoint(int segmentId, bool isStartPoint, const QVector3D& newPosition)
{
    LineSegment* segment = getLineSegment(segmentId);
    if (!segment) {
        qDebug() << "Segment not found for endpoint move:" << segmentId;
        return false;
    }

    if (isStartPoint) {
        segment->startPoint = newPosition;
    } else {
        segment->endPoint = newPosition;
    }

    emit lineSegmentUpdated(segmentId, *segment);
    qDebug() << "Endpoint moved for segment:" << segmentId;

    return true;
}

// 事件处理（基础框架，具体实现需要与渲染系统集成）
bool LineDrawingTool::handleMousePressEvent(QMouseEvent* event)
{
    if (!m_initialized || !event) {
        return false;
    }

    switch (m_currentDrawingMode) {
        case DrawingMode::SingleLine:
            return handleSingleLineDrawing(event);
        case DrawingMode::Polyline:
            return handlePolylineDrawing(event);
        case DrawingMode::Selection:
            return handleSelection(event);
        case DrawingMode::Edit:
            return handleEditing(event);
        default:
            return false;
    }
}

bool LineDrawingTool::handleMouseMoveEvent(QMouseEvent* event)
{
    if (!m_initialized || !event) {
        return false;
    }

    QVector2D screenPos(event->position().x(), event->position().y());
    QVector3D worldPos = screenToWorld(screenPos);

    // 处理实时预览
    if (m_isDrawing) {
        switch (m_currentDrawingMode) {
            case DrawingMode::SingleLine:
            case DrawingMode::Polyline:
                // 更新预览终点
                m_currentEndPoint = worldPos;
                updateVisualFeedback();
                return true;

            default:
                break;
        }
    }

    // 处理编辑模式的拖拽
    if (m_isEditing && m_currentDrawingMode == DrawingMode::Edit) {
        if (m_currentEditMode == EditMode::MoveEndpoint) {
            // 实时更新端点位置
            if (moveEndpoint(m_editingSegmentId, m_editingStartPoint, worldPos)) {
                updateVisualFeedback();
                return true;
            }
        }
    }

    // 处理鼠标悬停高亮
    if (!m_isDrawing && !m_isEditing) {
        int hoveredSegmentId = findSegmentAtPoint(screenPos, m_selectionTolerance);

        // 更新高亮状态
        for (const auto& segment : m_lineSegments) {
            bool shouldHighlight = (segment.id == hoveredSegmentId);
            highlightSegment(segment.id, shouldHighlight);
        }

        if (hoveredSegmentId != -1) {
            updateVisualFeedback();
            return true;
        }
    }

    return false;
}

bool LineDrawingTool::handleMouseReleaseEvent(QMouseEvent* event)
{
    if (!m_initialized || !event) {
        return false;
    }

    // 处理拖拽结束等操作
    // 具体实现需要与渲染系统集成
    return false;
}

bool LineDrawingTool::handleKeyPressEvent(QKeyEvent* event)
{
    if (!m_initialized || !event) {
        return false;
    }

    switch (event->key()) {
        case Qt::Key_Delete:
            // 删除选中的线段
            for (int segmentId : m_selectedSegmentIds) {
                removeLineSegment(segmentId);
            }
            return true;

        case Qt::Key_Escape:
            // 取消当前操作
            if (m_isDrawing) {
                m_isDrawing = false;
                emit operationCancelled("绘制操作已取消");
            }
            if (m_isEditing) {
                m_isEditing = false;
                emit operationCancelled("编辑操作已取消");
            }
            return true;

        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+A 全选
                selectAll();
                return true;
            }
            break;

        default:
            break;
    }

    return false;
}

// 事件处理辅助方法的具体实现
bool LineDrawingTool::handleSingleLineDrawing(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QVector2D screenPos(event->position().x(), event->position().y());
        QVector3D worldPos = screenToWorld(screenPos);

        if (!m_isDrawing) {
            // 开始绘制新线段
            m_isDrawing = true;
            m_currentStartPoint = worldPos;
            emit operationStarted("开始绘制单线段");
            qDebug() << "Single line drawing started at:" << worldPos;
        } else {
            // 完成线段绘制
            m_currentEndPoint = worldPos;

            // 检查线段长度是否有效
            float length = m_currentStartPoint.distanceToPoint(m_currentEndPoint);
            if (length < 0.001f) {
                qDebug() << "Line segment too short, ignoring";
                m_isDrawing = false;
                return true;
            }

            // 创建线段
            int segmentId = addLineSegment(m_currentStartPoint, m_currentEndPoint, -1, "单线段");

            m_isDrawing = false;
            emit operationCompleted("单线段绘制完成");
            qDebug() << "Single line segment created:" << segmentId << "length:" << length;
        }

        updateVisualFeedback();
        return true;
    } else if (event->button() == Qt::RightButton && m_isDrawing) {
        // 取消当前绘制
        m_isDrawing = false;
        emit operationCancelled("单线段绘制已取消");
        updateVisualFeedback();
        return true;
    }

    return false;
}

bool LineDrawingTool::handlePolylineDrawing(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QVector2D screenPos(event->position().x(), event->position().y());
        QVector3D worldPos = screenToWorld(screenPos);

        if (!m_isDrawing) {
            // 开始绘制多段线
            m_isDrawing = true;
            m_currentStartPoint = worldPos;
            m_currentPolylineSegments.clear();
            emit operationStarted("开始绘制多段线");
            qDebug() << "Polyline drawing started at:" << worldPos;
        } else {
            // 添加新的线段到多段线
            m_currentEndPoint = worldPos;

            // 检查线段长度是否有效
            float length = m_currentStartPoint.distanceToPoint(m_currentEndPoint);
            if (length >= 0.001f) {
                // 创建线段（暂时不分配多段线ID）
                int segmentId = addLineSegment(m_currentStartPoint, m_currentEndPoint, -1, "多段线段");
                m_currentPolylineSegments.push_back(segmentId);

                // 下一段的起点是当前段的终点
                m_currentStartPoint = m_currentEndPoint;

                qDebug() << "Polyline segment added:" << segmentId << "length:" << length;
            }
        }

        updateVisualFeedback();
        return true;
    } else if (event->button() == Qt::RightButton) {
        if (m_isDrawing && !m_currentPolylineSegments.empty()) {
            // 完成多段线绘制
            int polylineId = createPolyline(m_currentPolylineSegments, "多段线");

            m_isDrawing = false;
            m_currentPolylineSegments.clear();
            emit operationCompleted("多段线绘制完成");
            qDebug() << "Polyline created:" << polylineId;
        } else if (m_isDrawing) {
            // 取消多段线绘制
            m_isDrawing = false;
            m_currentPolylineSegments.clear();
            emit operationCancelled("多段线绘制已取消");
        }

        updateVisualFeedback();
        return true;
    }

    return false;
}

bool LineDrawingTool::handleSelection(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QVector2D screenPos(event->position().x(), event->position().y());

        // 查找点击位置附近的线段
        int segmentId = findSegmentAtPoint(screenPos, m_selectionTolerance);

        if (segmentId != -1) {
            bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;

            // 检查线段是否已被选中
            bool isSelected = m_selectedSegmentIds.find(segmentId) != m_selectedSegmentIds.end();

            if (isSelected && multiSelect) {
                // 取消选择
                deselectSegment(segmentId);
                qDebug() << "Segment deselected:" << segmentId;
            } else {
                // 选择线段
                selectSegment(segmentId, multiSelect);
                qDebug() << "Segment selected:" << segmentId;
            }
        } else if (!(event->modifiers() & Qt::ControlModifier)) {
            // 点击空白区域且不是多选模式，清除所有选择
            clearSelection();
            qDebug() << "Selection cleared";
        }

        updateVisualFeedback();
        return true;
    }

    return false;
}

bool LineDrawingTool::handleEditing(QMouseEvent* event)
{
    switch (m_currentEditMode) {
        case EditMode::MoveEndpoint:
            return handleMoveEndpoint(event);
        case EditMode::SplitSegment:
            return handleSplitSegment(event);
        case EditMode::MergeSegments:
            return handleMergeSegments(event);
        default:
            return false;
    }
}

bool LineDrawingTool::handleMoveEndpoint(QMouseEvent* event)
{
    QVector2D screenPos(event->position().x(), event->position().y());

    if (event->button() == Qt::LeftButton) {
        if (!m_isEditing) {
            // 查找端点附近的线段
            bool isStartPoint;
            int segmentId = -1;

            // 遍历所有线段，查找最近的端点
            for (const auto& segment : m_lineSegments) {
                if (isPointNearEndpoint(screenPos, segment.id, isStartPoint, m_endpointTolerance)) {
                    segmentId = segment.id;
                    break;
                }
            }

            if (segmentId != -1) {
                // 开始端点移动
                m_isEditing = true;
                m_editingSegmentId = segmentId;
                m_editingStartPoint = isStartPoint;

                LineSegment* segment = getLineSegment(segmentId);
                if (segment) {
                    m_originalPosition = isStartPoint ? segment->startPoint : segment->endPoint;
                }

                emit operationStarted("开始移动端点");
                qDebug() << "Started moving endpoint of segment:" << segmentId;
                return true;
            }
        } else {
            // 完成端点移动
            QVector3D newPosition = screenToWorld(screenPos);

            if (moveEndpoint(m_editingSegmentId, m_editingStartPoint, newPosition)) {
                m_isEditing = false;
                emit operationCompleted("端点移动完成");
                qDebug() << "Endpoint moved for segment:" << m_editingSegmentId;
            }

            updateVisualFeedback();
            return true;
        }
    } else if (event->button() == Qt::RightButton && m_isEditing) {
        // 取消端点移动
        if (moveEndpoint(m_editingSegmentId, m_editingStartPoint, m_originalPosition)) {
            m_isEditing = false;
            emit operationCancelled("端点移动已取消");
            qDebug() << "Endpoint move cancelled for segment:" << m_editingSegmentId;
        }

        updateVisualFeedback();
        return true;
    }

    return false;
}

bool LineDrawingTool::handleSplitSegment(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QVector2D screenPos(event->position().x(), event->position().y());

        // 查找点击位置附近的线段
        int segmentId = findSegmentAtPoint(screenPos, m_selectionTolerance);

        if (segmentId != -1) {
            QVector3D splitPoint = screenToWorld(screenPos);

            if (splitSegment(segmentId, splitPoint)) {
                emit operationCompleted("线段分割完成");
                qDebug() << "Segment split:" << segmentId;
                updateVisualFeedback();
                return true;
            } else {
                emit warningOccurred("线段分割失败");
            }
        }
    }

    return false;
}

bool LineDrawingTool::handleMergeSegments(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QVector2D screenPos(event->position().x(), event->position().y());

        // 查找点击位置附近的线段
        int segmentId = findSegmentAtPoint(screenPos, m_selectionTolerance);

        if (segmentId != -1) {
            // 查找与当前线段相连的其他线段
            LineSegment* segment = getLineSegment(segmentId);
            if (!segment) return false;

            // 简化实现：与选中的第一个线段合并
            if (!m_selectedSegmentIds.empty()) {
                int otherSegmentId = *m_selectedSegmentIds.begin();
                if (otherSegmentId != segmentId) {
                    if (mergeSegments(segmentId, otherSegmentId)) {
                        emit operationCompleted("线段合并完成");
                        qDebug() << "Segments merged:" << segmentId << "and" << otherSegmentId;
                        updateVisualFeedback();
                        return true;
                    } else {
                        emit warningOccurred("线段合并失败：线段不相连");
                    }
                }
            } else {
                // 选择线段用于后续合并
                selectSegment(segmentId, false);
                emit warningOccurred("请选择另一个线段进行合并");
            }
        }
    }

    return false;
}

// 几何计算辅助方法的具体实现
int LineDrawingTool::findSegmentAtPoint(const QVector2D& screenPoint, float tolerance) const
{
    float minDistance = tolerance;
    int closestSegmentId = -1;

    for (const auto& segment : m_lineSegments) {
        // 将线段端点转换到屏幕坐标
        QVector2D startScreen = worldToScreen(segment.startPoint);
        QVector2D endScreen = worldToScreen(segment.endPoint);

        // 计算点到线段的距离
        float distance = distancePointToLineSegment(screenPoint, startScreen, endScreen);

        if (distance < minDistance) {
            minDistance = distance;
            closestSegmentId = segment.id;
        }
    }

    return closestSegmentId;
}

// 辅助函数：计算点到线段的距离
float LineDrawingTool::distancePointToLineSegment(const QVector2D& point,
                                                  const QVector2D& lineStart,
                                                  const QVector2D& lineEnd) const
{
    QVector2D line = lineEnd - lineStart;
    float lineLength = line.length();

    if (lineLength < 0.001f) {
        // 线段退化为点
        return (point - lineStart).length();
    }

    // 计算点在线段上的投影参数
    QVector2D pointToStart = point - lineStart;
    float t = QVector2D::dotProduct(pointToStart, line) / (lineLength * lineLength);

    if (t < 0.0f) {
        // 投影在线段起点之前
        return (point - lineStart).length();
    } else if (t > 1.0f) {
        // 投影在线段终点之后
        return (point - lineEnd).length();
    } else {
        // 投影在线段上
        QVector2D projection = lineStart + t * line;
        return (point - projection).length();
    }
}

bool LineDrawingTool::isPointNearEndpoint(const QVector2D& screenPoint, int segmentId,
                                         bool& isStartPoint, float tolerance) const
{
    const LineSegment* segment = nullptr;
    for (const auto& seg : m_lineSegments) {
        if (seg.id == segmentId) {
            segment = &seg;
            break;
        }
    }

    if (!segment) {
        return false;
    }

    // 将端点转换到屏幕坐标
    QVector2D startScreen = worldToScreen(segment->startPoint);
    QVector2D endScreen = worldToScreen(segment->endPoint);

    // 计算到两个端点的距离
    float distanceToStart = (screenPoint - startScreen).length();
    float distanceToEnd = (screenPoint - endScreen).length();

    if (distanceToStart <= tolerance && distanceToStart <= distanceToEnd) {
        isStartPoint = true;
        return true;
    } else if (distanceToEnd <= tolerance) {
        isStartPoint = false;
        return true;
    }

    return false;
}

QVector3D LineDrawingTool::screenToWorld(const QVector2D& screenPoint) const
{
    // 简化的屏幕到世界坐标转换
    // 在实际应用中，这需要与3D渲染系统的投影矩阵集成

    if (!m_parentWidget) {
        return QVector3D(screenPoint.x(), screenPoint.y(), 0.0f);
    }

    // 获取窗口尺寸
    float width = m_parentWidget->width();
    float height = m_parentWidget->height();

    if (width <= 0 || height <= 0) {
        return QVector3D(screenPoint.x(), screenPoint.y(), 0.0f);
    }

    // 将屏幕坐标转换为标准化设备坐标 (-1 到 1)
    float x = (2.0f * screenPoint.x() / width) - 1.0f;
    float y = 1.0f - (2.0f * screenPoint.y() / height);

    // 简化的世界坐标转换（假设俯视图，Z=0）
    // 在实际应用中需要考虑相机位置、投影矩阵等
    float worldX = x * 50.0f; // 假设视野范围为100单位
    float worldY = y * 50.0f;
    float worldZ = 0.0f;      // 俯视图，所有点在Z=0平面

    return QVector3D(worldX, worldY, worldZ);
}

QVector2D LineDrawingTool::worldToScreen(const QVector3D& worldPoint) const
{
    // 简化的世界到屏幕坐标转换
    // 在实际应用中，这需要与3D渲染系统的投影矩阵集成

    if (!m_parentWidget) {
        return QVector2D(worldPoint.x(), worldPoint.y());
    }

    // 获取窗口尺寸
    float width = m_parentWidget->width();
    float height = m_parentWidget->height();

    if (width <= 0 || height <= 0) {
        return QVector2D(worldPoint.x(), worldPoint.y());
    }

    // 简化的屏幕坐标转换（假设俯视图）
    float x = (worldPoint.x() / 50.0f + 1.0f) * width * 0.5f;
    float y = (1.0f - worldPoint.y() / 50.0f) * height * 0.5f;

    return QVector2D(x, y);
}

void LineDrawingTool::updateSegmentIndices()
{
    // 重新整理线段索引
    // 如果需要的话可以在这里实现
}

void LineDrawingTool::updatePolylineIndices()
{
    // 重新整理多段线索引
    // 如果需要的话可以在这里实现
}

void LineDrawingTool::updateVisualFeedback()
{
    // 更新视觉反馈
    // 具体实现需要与渲染系统集成
}

void LineDrawingTool::highlightSegment(int segmentId, bool highlight)
{
    // 高亮显示线段
    // 具体实现需要与渲染系统集成
    Q_UNUSED(segmentId)
    Q_UNUSED(highlight)
}

void LineDrawingTool::onInternalUpdate()
{
    // 内部更新处理
    updateVisualFeedback();
}

} // namespace WallExtraction
