#ifndef MYQOPENGLWIDGET_H
#define MYQOPENGLWIDGET_H

// Use Qt's OpenGL support instead of GLEW
#include <gl/freeglut.h>

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtOpenGL/QOpenGLVertexArrayObject>
#include <QPainter>
#include <QOpenGLContext>
#include <QtOpenGL/QOpenGLPaintDevice>
#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtOpenGL/QOpenGLShader>
#include <QTimer>
#include <QVector3D>
#include <QQuaternion>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTime>
#include <QScopedPointer>
#include "MinBoundingBox.h"
#include "modelmanager.h"

using namespace std;

// 显示模式枚举
enum class ViewMode {
    PointCloudOnly,
    MeshOnly,
    Hybrid
};

QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
class QOpenGLShaderProgram;
class QOpenGLShader;
QT_END_NAMESPACE

class MyQOpenglWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyQOpenglWidget(QWidget *parent = 0);
    ~MyQOpenglWidget();
    void showPointCloud(const std::vector<QVector3D>& cloud);
    virtual void resizeGL(int w, int h);
    void setBackgroundColor(QVector3D color);
    void clearPointCloud();
    void appendPointCloud(const std::vector<QVector3D> &cloud);
    
    // 新增mesh功能
    bool loadMeshModel(const QString& modelPath);
    void clearMeshModel();
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const;
    
    // 渲染控制
    void setMeshVisible(bool visible);
    void setPointCloudVisible(bool visible);
    
    // 设置是否显示坐标轴
    void setShowAxis(bool show);
    // 获取当前坐标轴显示状态
    bool getShowAxis() const;



protected:
    virtual void initializeGL(void);
    virtual void paintGL(void);

    struct VertexInfo
    {
        float pos[3] = {0.0,0.0,0.0};
        float normal[3] = {0.0,0.0,1.0};
        float color[4] = {1.0,1.0,1.0,1.0};
    };
private slots:
    void onTimerOut(void);
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void mouseMoveEvent(QMouseEvent * e);
    virtual void mouseReleaseEvent(QMouseEvent * e);
    virtual void wheelEvent(QWheelEvent * e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void leaveEvent(QEvent *);
    virtual void enterEvent(QEvent *);


public slots:  // 添加这个部分
    // 视图切换函数 - 移动到public slots
    void setXView();        // X轴视图
    void setYView();        // Y轴视图
    void setZView();        // Z轴视图
    void setIsometricView(); // 等轴视图

private:
    QTimer *m_Timer;
    QScopedPointer<QOpenGLContext> m_context;
    QScopedPointer<QOpenGLShaderProgram> m_Program;
    QScopedPointer<QOpenGLShader> m_VertexShader;
    QScopedPointer<QOpenGLShader> m_FragmentShader;
    
    // Mesh渲染着色器
    QScopedPointer<QOpenGLShaderProgram> m_MeshProgram;
    QScopedPointer<QOpenGLShader> m_MeshVertexShader;
    QScopedPointer<QOpenGLShader> m_MeshFragmentShader;

    // shader spara
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_norAttr;
    GLuint m_matrixUniform;
    GLuint m_VBO;
    GLuint m_EBO;
    
    // Mesh着色器参数
    GLuint m_meshPosAttr;
    GLuint m_meshNorAttr;
    GLuint m_meshMatrixUniform;
    GLuint m_meshViewMatrixUniform;
    GLuint m_meshLightPosUniform;
    GLuint m_meshLightColorUniform;
    GLuint m_meshAmbientUniform;
    QScopedPointer<QOpenGLVertexArrayObject> m_VAO;
    QVector4D m_backgroundColor;

    //store points
    QVector<VertexInfo> m_PointsVertex;
    MinBoundingBox m_box;

    // mesh渲染相关
    ViewMode m_viewMode;
    bool m_meshVisible;
    bool m_pointCloudVisible;
    ModelManager* m_modelManager;

    QVector3D m_lineMove;
    QQuaternion m_rotate;
    QVector3D m_rotationAxis;
    float m_scale;
    bool m_bShowAxis;
    GLuint m_Axisindices[6];

    QVector2D m_lastPoint;

    GLuint createGPUProgram(QString nVertexShaderFile, QString nFragmentShaderFile);
    void GetShaderUniformPara();
    bool InitShader();
    bool InitMeshShader();
    void GetMeshShaderUniformPara();

    void LineMove(QVector2D posOrgin, QVector2D posEnd);
    void Rotate(QVector2D posOrgin, QVector2D posEnd);
    void modelZoomInOrOut(bool ZoomInOrOut);
    QVector3D pixelPosToViewPos(const QVector2D& p);
    
    // 渲染函数声明
    void renderPointCloud();
    void renderAxis();
    void calRotation(QVector2D posOrgin, QVector2D posEnd);
    void initPointCloud(const std::vector<QVector3D>& cloud);
    void gray2Pseudocolor(const QVector3D pos, float color[4]);
    void changePointCloud();
    void setMatrixUniform();
    void ResetView();
    void initCloud();
    void addAxisData();

    void appendPointCloudData(const std::vector<QVector3D> &cloud);
    
    // mesh渲染函数
    void renderMesh();
    void renderTestTriangle(); // 测试三角形渲染
    void initMeshGL();


    //debug
    void debugMsg(QString msg,QTime start);

};

#endif // MYQOPENGLWIDGET_H
