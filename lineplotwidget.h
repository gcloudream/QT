#ifndef LINEPLOTWIDGET_H
#define LINEPLOTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>
#include <QDebug>
#include <QPointF>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QFontMetrics>

// 线条数据结构
struct LineData {
    QPointF start;
    QPointF end;

    LineData() : start(0, 0), end(0, 0) {}
    LineData(double x1, double y1, double x2, double y2)
        : start(x1, y1), end(x2, y2) {}
};

class LineplotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LineplotWidget(QWidget *parent = nullptr);
    ~LineplotWidget();

    void setFilePath(const QString &path);
    void plotLinesFromFile();

public slots:
    void openFile();
    void refreshPlot();
    void clearLines();


protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void calculateBounds();
    void transformPoint(const QPointF &point, QPointF &transformed);
    double calculateTickInterval(double range);
    void drawAxes(QPainter &painter);
    double calculateLineLength(const LineData &line);
    void drawLineLengthLabels(QPainter &painter);

    // 新增：面积计算相关
    struct Polygon {
        QList<QPointF> vertices;
        double area;
        bool isClosed;
    };

    QList<Polygon> m_polygons;
    double m_totalArea;

    // 新增：面积计算方法
    void extractPolygons();
    QList<QPointF> connectLines(const QList<LineData> &lines);
    double calculatePolygonArea(const QList<QPointF> &vertices);
    double calculateTotalArea();
    bool isPointEqual(const QPointF &p1, const QPointF &p2, double tolerance = 0.001);
    void findConnectedPath(const QList<LineData> &lines, QList<QPointF> &path, QList<bool> &used);
    void drawPolygons(QPainter &painter);

private:
    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_openButton;
    QPushButton *m_refreshButton;
    QLabel *m_filePathLabel;

    // 数据
    QString m_filePath;
    QList<LineData> m_lines;

    // 绘图参数
    double m_minX, m_maxX, m_minY, m_maxY;
    int m_margin;
    QString m_plotTitle;

    // 绘图样式
    QPen m_linePen;
    QPen m_axisPen;
    QPen m_gridPen;
    QBrush m_backgroundBrush;
};

#endif // LINEPLOTWIDGET_H
