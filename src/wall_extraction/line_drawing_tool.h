#ifndef LINE_DRAWING_TOOL_H
#define LINE_DRAWING_TOOL_H

#include <QObject>
#include <QWidget>

namespace WallExtraction {

/**
 * @brief 线段绘制工具类（占位符实现）
 * 
 * 这是一个占位符实现，用于T1.1任务的基础架构搭建。
 * 完整实现将在T2.1任务中完成。
 */
class LineDrawingTool : public QObject
{
    Q_OBJECT

public:
    explicit LineDrawingTool(QWidget* parent = nullptr);
    ~LineDrawingTool();

    // 占位符方法
    bool initialize();
    bool isInitialized() const;

signals:
    void errorOccurred(const QString& error);

private:
    bool m_initialized;
    QWidget* m_parentWidget;
};

} // namespace WallExtraction

#endif // LINE_DRAWING_TOOL_H
