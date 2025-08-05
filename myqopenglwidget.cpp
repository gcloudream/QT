#include "myqopenglwidget.h"
#include <QDebug>
#include <QtMath>
//点云显示
static const char *vertexShaderSource =
    "attribute highp vec3 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "  col=colAttr;\n"
    "  gl_Position=matrix * vec4(posAttr,1.0f);\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";

// Mesh渲染着色器 - 改进版本
static const char *meshVertexShaderSource =
    "attribute highp vec3 position;\n"
    "attribute highp vec3 normal;\n"
    "uniform highp mat4 mvp_matrix;\n"
    "uniform highp mat4 view_matrix;\n"
    "uniform highp vec3 light_position;\n"
    "uniform highp vec3 light_color;\n"
    "uniform lowp float ambient_strength;\n"
    "varying lowp vec4 fragment_color;\n"
    "void main() {\n"
    "    gl_Position = mvp_matrix * vec4(position, 1.0);\n"
    "    \n"
    "    // 改进的光照计算\n"
    "    vec3 norm = normalize(normal);\n"
    "    vec3 light_dir = normalize(light_position - position);\n"
    "    \n"
    "    // 环境光\n"
    "    vec3 ambient = ambient_strength * light_color;\n"
    "    \n"
    "    // 漫反射\n"
    "    float diff = max(dot(norm, light_dir), 0.0);\n"
    "    vec3 diffuse = diff * light_color * 0.8;\n"
    "    \n"
    "    // 使用更好的材质颜色\n"
    "    vec3 object_color = vec3(0.9, 0.8, 0.6); // 金黄色\n"
    "    vec3 result = (ambient + diffuse) * object_color;\n"
    "    fragment_color = vec4(result, 1.0);\n"
    "}\n";

static const char *meshFragmentShaderSource =
    "varying lowp vec4 fragment_color;\n"
    "void main() {\n"
    "    gl_FragColor = fragment_color;\n"
    "}\n";

MyQOpenglWidget::MyQOpenglWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    ,m_Program(nullptr)
    ,m_MeshProgram(nullptr)
    //,m_VertexShader(nullptr)
    //,m_FragmentShader(nullptr)
    ,m_posAttr(0)
    ,m_colAttr(0)
    ,m_norAttr(0)
    ,m_matrixUniform(0)
    ,m_VBO(0)
    ,m_VAO(nullptr)
    ,m_viewMode(ViewMode::PointCloudOnly)
    ,m_meshVisible(true)
    ,m_pointCloudVisible(true)
    ,m_scale(1.0f)
    ,m_modelManager(nullptr)
    ,m_bShowAxis(false)
    ,m_shaderInitialized(false)
    ,m_meshShaderInitialized(false)
{
    m_Timer = new QTimer;
    // m_context.reset(new QOpenGLContext());
    m_Program.reset( new QOpenGLShaderProgram(this));
    m_VertexShader.reset(new QOpenGLShader(QOpenGLShader::Vertex));
    m_FragmentShader.reset( new QOpenGLShader(QOpenGLShader::Fragment));
    
    // 初始化mesh着色器
    m_MeshProgram.reset(new QOpenGLShaderProgram(this));
    m_MeshVertexShader.reset(new QOpenGLShader(QOpenGLShader::Vertex));
    m_MeshFragmentShader.reset(new QOpenGLShader(QOpenGLShader::Fragment));
    m_VAO.reset(new QOpenGLVertexArrayObject());

    m_PointsVertex = QVector<VertexInfo>();
    ResetView();
    for (GLuint i = 0; i < 6; ++i) {
        m_Axisindices[i] = i;
    }
    
    // 初始化ModelManager
    m_modelManager = new ModelManager();
    //this->grabKeyboard();
}

MyQOpenglWidget::~MyQOpenglWidget()
{
    if (m_modelManager) {
        delete m_modelManager;
        m_modelManager = nullptr;
    }
}

// 切换到X视图 (从X轴正方向看向YZ平面)
void MyQOpenglWidget::setXView()
{
    m_lineMove = QVector3D();
    m_rotate = QQuaternion();
    // 绕Y轴旋转90度，然后绕Z轴旋转180度
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,1,0), 90);
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 180);
    m_rotationAxis = QVector3D();
    m_scale = 1.0f;
    update(); // 触发重绘
}

// 切换到Y视图 (从Y轴正方向看向XZ平面)
void MyQOpenglWidget::setYView()
{
    m_lineMove = QVector3D();
    m_rotate = QQuaternion();
    // 绕X轴旋转-90度，然后绕Z轴旋转180度
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), -90);
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 180);
    m_rotationAxis = QVector3D();
    m_scale = 1.0f;
    update(); // 触发重绘
}

// 切换到Z视图 (从Z轴正方向看向XY平面)
void MyQOpenglWidget::setZView()
{
    m_lineMove = QVector3D();
    m_rotate = QQuaternion();
    // 只需要绕Z轴旋转180度（这是默认视图）
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 180);
    m_rotationAxis = QVector3D();
    m_scale = 1.0f;
    update(); // 触发重绘
}

// 切换到等轴视图 (3D等角视图)
void MyQOpenglWidget::setIsometricView()
{
    m_lineMove = QVector3D();
    m_rotate = QQuaternion();
    // 创建等轴视图：绕X轴旋转35.26度，然后绕Y轴旋转45度
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), 35.26f);
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,1,0), 45.0f);
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 180);
    m_rotationAxis = QVector3D();
    m_scale = 1.0f;
    update(); // 触发重绘
}

// 添加键盘快捷键支持（在现有的keyPressEvent中添加）
void MyQOpenglWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_R)
    {
        ResetView();
    }
    else if ((e->key()== Qt::Key_C) && (e->modifiers() == Qt::ControlModifier)){
        m_bShowAxis = !m_bShowAxis;
    }
    // 新增的视图快捷键
    else if (e->key() == Qt::Key_1) {
        setXView();
    }
    else if (e->key() == Qt::Key_2) {
        setYView();
    }
    else if (e->key() == Qt::Key_3) {
        setZView();
    }
    else if (e->key() == Qt::Key_4) {
        setIsometricView();
    }
    QWidget::keyPressEvent(e);
}

void MyQOpenglWidget::initializeGL()
{
    qDebug() << "=== MyQOpenglWidget::initializeGL() called ===";

    makeCurrent();

    // 初始化OpenGL函数
    initializeOpenGLFunctions();

    // 检查OpenGL版本
    qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
    qDebug() << "OpenGL Vendor:" << (const char*)glGetString(GL_VENDOR);
    qDebug() << "OpenGL Renderer:" << (const char*)glGetString(GL_RENDERER);

    bool binit = true;
    binit &= InitShader();
    binit &= InitMeshShader();

    if (!binit) {
        qDebug() << "ERROR: Shader initialization failed! Aborting OpenGL initialization.";
        return;
    }

    qDebug() << "SUCCESS: All shaders initialized successfully";
    glEnable(GL_DEPTH_TEST);

    m_VAO->create();
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    //init need import a point data
    initCloud();
    changePointCloud();

    // 初始化ModelManager的OpenGL
    if (m_modelManager) {
        m_modelManager->initializeGL();
    }

    QObject::connect(m_Timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
    m_Timer->start(30);
}

// 在MyQOpenglWidget类中添加以下函数

void MyQOpenglWidget::appendPointCloud(const std::vector<QVector3D> &cloud)
{
    if (cloud.empty()) return;

    QTime startTime = QTime::currentTime();
    appendPointCloudData(cloud);
    debugMsg("appendPointCloudData =", startTime);

    startTime = QTime::currentTime();
    changePointCloud();
    debugMsg("changePointCloud =", startTime);

    repaint();
}

// 修改2: 添加设置坐标轴显示状态的公共方法
void MyQOpenglWidget::setShowAxis(bool show)
{
    m_bShowAxis = show;
    update(); // 触发重绘
}

bool MyQOpenglWidget::getShowAxis() const
{
    return m_bShowAxis;
}

void MyQOpenglWidget::appendPointCloudData(const std::vector<QVector3D> &cloud)
{
    // 获取当前点云数据的大小（排除轴线数据）
    size_t currentCloudSize = m_PointsVertex.size() > 6 ? m_PointsVertex.size() - 6 : 0;

    // 扩展顶点数组
    size_t newTotalSize = currentCloudSize + cloud.size() + 6;
    m_PointsVertex.resize(newTotalSize);

    // 直接添加新点，不移动到原点
    for (size_t i = 0; i < cloud.size(); ++i) {
        const auto& p = cloud[i];

        size_t index = currentCloudSize + i + 6;
        m_PointsVertex[index].pos[0] = p.x();
        m_PointsVertex[index].pos[1] = p.y();
        m_PointsVertex[index].pos[2] = p.z();

        gray2Pseudocolor(p, m_PointsVertex[index].color);

        m_PointsVertex[index].normal[0] = 0.0f;
        m_PointsVertex[index].normal[1] = 1.0f;
        m_PointsVertex[index].normal[2] = 0.0f;
    }

    // 计算所有点的包围盒
    std::vector<QVector3D> allPoints;
    for (qsizetype i = 6; i < m_PointsVertex.size(); ++i) {
        allPoints.emplace_back(m_PointsVertex[i].pos[0],
                               m_PointsVertex[i].pos[1],
                               m_PointsVertex[i].pos[2]);
    }
    m_box.calculateMinBoundingBox(allPoints);

    // 更新轴线数据
    addAxisData();
}


// 添加清空点云的函数
void MyQOpenglWidget::clearPointCloud()
{
    m_PointsVertex.clear();
    initCloud(); // 重新初始化为默认状态
    changePointCloud();
    repaint();
}


void MyQOpenglWidget::showPointCloud(const std::vector<QVector3D> &cloud)
{
    QTime startTime = QTime::currentTime();
    initPointCloud(cloud);
    debugMsg("initPointCloud =",startTime);

    startTime = QTime::currentTime();
    changePointCloud();
    debugMsg("changePointCloud =",startTime);
    ResetView();
    repaint();
}

void MyQOpenglWidget::initPointCloud(const std::vector<QVector3D> &cloud)
{
    m_PointsVertex.clear();
    m_PointsVertex.resize(cloud.size() + 6);

    // 计算原始点云的包围盒
    m_box.calculateMinBoundingBox(cloud);
    QVector3D center = m_box.getCenterPoint();

    // 移动点云到原点并重新计算包围盒
    std::vector<QVector3D> movedCloud;
    movedCloud.reserve(cloud.size());
    for (const auto& p : cloud) {
        movedCloud.emplace_back(p.x() - center.x(), p.y() - center.y(), p.z() - center.z());
    }
    m_box.calculateMinBoundingBox(movedCloud); // 更新为移动后的包围盒

    addAxisData();

    for(size_t i = 0; i < movedCloud.size(); ++i)
    {
        //move cloud center to origin
        const auto& p = movedCloud[i];
        m_PointsVertex[i + 6].pos[0] = p.x();
        m_PointsVertex[i + 6].pos[1] = p.y();
        m_PointsVertex[i + 6].pos[2] = p.z();
        gray2Pseudocolor(QVector3D(p.x(), p.y(), p.z()), m_PointsVertex[i + 6].color);
        m_PointsVertex[i+6].normal[0] = 0.0f;
        m_PointsVertex[i+6].normal[1] = 1.0f;
        m_PointsVertex[i+6].normal[2] = 0.0f;
    }
}

void MyQOpenglWidget::addAxisData()
{
    // 计算轴线长度，使用包围盒的更大比例，让轴线更长
    float axisLength = std::max({m_box.width(), m_box.height(), m_box.depth()}) * 0.6f;
    // 如果没有点云数据，使用默认长度
    if (axisLength <= 0) {
        axisLength = 2.0f;
    }
    if (m_PointsVertex.size() >= 6) {
        // X轴 - 红色，降低饱和度和透明度让其看起来模糊
        m_PointsVertex[0] = VertexInfo();
        m_PointsVertex[0].pos[0] = 0.0f; m_PointsVertex[0].pos[1] = 0.0f; m_PointsVertex[0].pos[2] = 0.0f;
        m_PointsVertex[0].color[0] = 0.6f; m_PointsVertex[0].color[1] = 0.1f; m_PointsVertex[0].color[2] = 0.1f; m_PointsVertex[0].color[3] = 0.7f;
        m_PointsVertex[1] = VertexInfo();
        m_PointsVertex[1].pos[0] = axisLength; m_PointsVertex[1].pos[1] = 0.0f; m_PointsVertex[1].pos[2] = 0.0f;
        m_PointsVertex[1].color[0] = 0.6f; m_PointsVertex[1].color[1] = 0.1f; m_PointsVertex[1].color[2] = 0.1f; m_PointsVertex[1].color[3] = 0.7f;

        // Y轴 - 绿色，降低饱和度和透明度
        m_PointsVertex[2] = VertexInfo();
        m_PointsVertex[2].pos[0] = 0.0f; m_PointsVertex[2].pos[1] = 0.0f; m_PointsVertex[2].pos[2] = 0.0f;
        m_PointsVertex[2].color[0] = 0.1f; m_PointsVertex[2].color[1] = 0.6f; m_PointsVertex[2].color[2] = 0.1f; m_PointsVertex[2].color[3] = 0.7f;
        m_PointsVertex[3] = VertexInfo();
        m_PointsVertex[3].pos[0] = 0.0f; m_PointsVertex[3].pos[1] = axisLength; m_PointsVertex[3].pos[2] = 0.0f;
        m_PointsVertex[3].color[0] = 0.1f; m_PointsVertex[3].color[1] = 0.6f; m_PointsVertex[3].color[2] = 0.1f; m_PointsVertex[3].color[3] = 0.7f;

        // Z轴 - 蓝色，降低饱和度和透明度
        m_PointsVertex[4] = VertexInfo();
        m_PointsVertex[4].pos[0] = 0.0f; m_PointsVertex[4].pos[1] = 0.0f; m_PointsVertex[4].pos[2] = 0.0f;
        m_PointsVertex[4].color[0] = 0.1f; m_PointsVertex[4].color[1] = 0.1f; m_PointsVertex[4].color[2] = 0.6f; m_PointsVertex[4].color[3] = 0.7f;
        m_PointsVertex[5] = VertexInfo();
        m_PointsVertex[5].pos[0] = 0.0f; m_PointsVertex[5].pos[1] = 0.0f; m_PointsVertex[5].pos[2] = axisLength;
        m_PointsVertex[5].color[0] = 0.1f; m_PointsVertex[5].color[1] = 0.1f; m_PointsVertex[5].color[2] = 0.6f; m_PointsVertex[5].color[3] = 0.7f;
    }
}

// 替换原有的 gray2Pseudocolor 函数 颜色增强
void MyQOpenglWidget::gray2Pseudocolor(const QVector3D pos, float color[4])
{
    float fmin = m_box.getMinPoint().z();
    float fmax = m_box.getMaxPoint().z();

    // 归一化高度值到 [0, 1]
    float normalizedHeight = (pos.z() - fmin) / (fmax - fmin);
    normalizedHeight = qBound(0.0f, normalizedHeight, 1.0f);

    // 使用更鲜艳的颜色映射方案
    float r, g, b;

    if (normalizedHeight < 0.2f) {
        // 深蓝色 -> 蓝色
        float t = normalizedHeight / 0.2f;
        r = 0.0f;
        g = 0.0f;
        b = 0.5f + 0.5f * t;
    }
    else if (normalizedHeight < 0.4f) {
        // 蓝色 -> 青色
        float t = (normalizedHeight - 0.2f) / 0.2f;
        r = 0.0f;
        g = t;
        b = 1.0f;
    }
    else if (normalizedHeight < 0.6f) {
        // 青色 -> 绿色
        float t = (normalizedHeight - 0.4f) / 0.2f;
        r = 0.0f;
        g = 1.0f;
        b = 1.0f - t;
    }
    else if (normalizedHeight < 0.8f) {
        // 绿色 -> 黄色
        float t = (normalizedHeight - 0.6f) / 0.2f;
        r = t;
        g = 1.0f;
        b = 0.0f;
    }
    else {
        // 黄色 -> 红色
        float t = (normalizedHeight - 0.8f) / 0.2f;
        r = 1.0f;
        g = 1.0f - t * 0.5f; // 保持一些黄色成分
        b = 0.0f;
    }

    // 增强颜色饱和度和亮度
    float saturationBoost = 1.2f;
    r = qBound(0.0f, r * saturationBoost, 1.0f);
    g = qBound(0.0f, g * saturationBoost, 1.0f);
    b = qBound(0.0f, b * saturationBoost, 1.0f);

    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0f;
}
void MyQOpenglWidget::changePointCloud()
{
    if (m_PointsVertex.empty()) return;

    m_VAO->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexInfo) * m_PointsVertex.size(), m_PointsVertex.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Axisindices), m_Axisindices, GL_DYNAMIC_DRAW);

    // 配置顶点属性
    glEnableVertexAttribArray(m_posAttr);
    glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, pos));
    glEnableVertexAttribArray(m_colAttr);
    glVertexAttribPointer(m_colAttr, 4, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_VAO->release();
}

void MyQOpenglWidget::ResetView()
{
    m_lineMove = QVector3D();
    m_rotate = QQuaternion();
    m_rotate *= QQuaternion::fromAxisAndAngle(QVector3D(0,0,1),180);
    m_rotationAxis = QVector3D();
    m_scale = 1.0f;
    setBackgroundColor(QVector3D(0.2f,0.3f,0.3f));
}

void MyQOpenglWidget::setBackgroundColor(QVector3D color)
{
    m_backgroundColor= QVector4D(color,1.0f);
}

void MyQOpenglWidget::debugMsg(QString msg, QTime start)
{
    qDebug() << msg << start.msecsTo(QTime::currentTime()) << "ms";
}

void MyQOpenglWidget::resizeGL(int w, int h)
{
    qDebug() << "=== MyQOpenglWidget::resizeGL ===";
    qDebug() << "New size:" << w << "x" << h;
    qDebug() << "Widget geometry:" << geometry();
    qDebug() << "Parent size:" << (parentWidget() ? parentWidget()->size() : QSize(-1, -1));

    const qreal retinaScale = devicePixelRatio();
    qDebug() << "Retina scale:" << retinaScale;
    qDebug() << "Viewport size:" << (w * retinaScale) << "x" << (h * retinaScale);

    glViewport(0, 0, w * retinaScale, h * retinaScale);

    // 确保视图矩阵正确更新
    update();

    qDebug() << "=== OpenGL resize completed ===";
}


bool MyQOpenglWidget::InitShader()
{
    qDebug() << "=== Initializing Point Cloud Shader ===";

    // 检查是否已经初始化
    if (m_shaderInitialized) {
        qDebug() << "Point cloud shader already initialized";
        return true;
    }

    // 重新创建着色器程序以避免重复添加
    m_Program.reset(new QOpenGLShaderProgram(this));

    bool success = true;
    success &= m_Program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    success &= m_Program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    success &= m_Program->link();

    if (success) {
        qDebug() << "SUCCESS: Point cloud shader linked";
        GetShaderUniformPara();
        m_shaderInitialized = true;
    } else {
        qDebug() << "ERROR: Point cloud shader failed to link";
        qDebug() << "Link log:" << m_Program->log();
    }

    return success;
}

bool MyQOpenglWidget::InitMeshShader()
{
    qDebug() << "=== Initializing Mesh Shader ===";

    // 检查是否已经初始化
    if (m_meshShaderInitialized) {
        qDebug() << "Mesh shader already initialized";
        return true;
    }

    // 重新创建着色器程序以避免重复添加
    m_MeshProgram.reset(new QOpenGLShaderProgram(this));

    bool success = true;

    // 编译顶点着色器
    if (!m_MeshProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, meshVertexShaderSource)) {
        qDebug() << "ERROR: Failed to compile vertex shader!";
        qDebug() << "Vertex shader log:" << m_MeshProgram->log();
        success = false;
    } else {
        qDebug() << "SUCCESS: Vertex shader compiled";
    }

    // 编译片段着色器
    if (!m_MeshProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, meshFragmentShaderSource)) {
        qDebug() << "ERROR: Failed to compile fragment shader!";
        qDebug() << "Fragment shader log:" << m_MeshProgram->log();
        success = false;
    } else {
        qDebug() << "SUCCESS: Fragment shader compiled";
    }

    // 链接着色器程序
    if (!m_MeshProgram->link()) {
        qDebug() << "ERROR: Failed to link shader program!";
        qDebug() << "Link log:" << m_MeshProgram->log();
        success = false;
    } else {
        qDebug() << "SUCCESS: Shader program linked";
    }
    
    if (success) {
        GetMeshShaderUniformPara();

        // 验证属性位置
        qDebug() << "Shader attribute locations:";
        qDebug() << "  position:" << m_meshPosAttr;
        qDebug() << "  normal:" << m_meshNorAttr;
        qDebug() << "Shader uniform locations:";
        qDebug() << "  mvp_matrix:" << m_meshMatrixUniform;
        qDebug() << "  view_matrix:" << m_meshViewMatrixUniform;
        qDebug() << "  light_position:" << m_meshLightPosUniform;
        qDebug() << "  light_color:" << m_meshLightColorUniform;
        qDebug() << "  ambient_strength:" << m_meshAmbientUniform;

        // 检查是否有无效的位置
        if (m_meshPosAttr == -1 || m_meshNorAttr == -1) {
            qDebug() << "ERROR: Invalid attribute locations!";
            success = false;
        } else {
            m_meshShaderInitialized = true;
        }
    }

    qDebug() << "=== Mesh Shader Init Result:" << (success ? "SUCCESS" : "FAILED") << "===";
    return success;
}

void MyQOpenglWidget::GetShaderUniformPara()
{
    m_posAttr = m_Program->attributeLocation("posAttr");
    m_colAttr = m_Program->attributeLocation("colAttr");
    m_matrixUniform = m_Program->uniformLocation("matrix");
    //m_norAttr = m_Program->attributeLocation("norAttr");
}

void MyQOpenglWidget::GetMeshShaderUniformPara()
{
    m_meshPosAttr = m_MeshProgram->attributeLocation("position");
    m_meshNorAttr = m_MeshProgram->attributeLocation("normal");
    m_meshMatrixUniform = m_MeshProgram->uniformLocation("mvp_matrix");
    m_meshViewMatrixUniform = m_MeshProgram->uniformLocation("view_matrix");
    m_meshLightPosUniform = m_MeshProgram->uniformLocation("light_position");
    m_meshLightColorUniform = m_MeshProgram->uniformLocation("light_color");
    m_meshAmbientUniform = m_MeshProgram->uniformLocation("ambient_strength");
}

void MyQOpenglWidget::paintGL()
{
    makeCurrent();
    glClearColor(m_backgroundColor.x(), m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 根据显示模式选择渲染内容
    switch(m_viewMode) {
        case ViewMode::PointCloudOnly:
            if (m_pointCloudVisible) {
                renderPointCloud();
            }
            break;
            
        case ViewMode::MeshOnly:
            if (m_meshVisible) {
                renderMesh();
            }
            break;
            
        case ViewMode::Hybrid:
            if (m_pointCloudVisible) {
                renderPointCloud();
            }
            if (m_meshVisible) {
                renderMesh();
            }
            break;
    }
    
    // 渲染坐标轴
    if(m_bShowAxis) {
        renderAxis();
    }
}

void MyQOpenglWidget::setMatrixUniform()
{
    QMatrix4x4 matrix;
    QMatrix4x4 matrixPerspect;
    QMatrix4x4 matrixView;
    QMatrix4x4 matrixModel;

    // 使用移动后的包围盒参数
    QVector3D minPos = m_box.getMinPoint();
    QVector3D maxPos = m_box.getMaxPoint();
    float maxRange = qMax(qMax(m_box.width(), m_box.height()), m_box.depth());

    // 计算点云的最大半径（从原点到最远点的距离）
    float maxRadius = qMax(qMax(qAbs(minPos.x()), qAbs(maxPos.x())),
                           qMax(qMax(qAbs(minPos.y()), qAbs(maxPos.y())),
                                qMax(qAbs(minPos.z()), qAbs(maxPos.z()))));

    // 考虑旋转后的最大范围（对角线长度）
    float diagonalLength = QVector3D(maxPos - minPos).length();
    float safeRange = qMax(maxRadius, diagonalLength * 0.6f); // 0.6是安全系数

    // 扩大投影范围以确保旋转时不会被裁剪
    // float projectionRange = safeRange * m_scale * 1.2f; // 1.2是额外的安全边距
    float projectionRange = safeRange * 1.5f; // 移除 m_scale

    // 设置正交投影，使用对称的投影范围
    matrixPerspect.ortho(
        -projectionRange,
        projectionRange,
        -projectionRange,
        projectionRange,
        -maxRange * 10,  // 扩大近平面
        maxRange * 10    // 扩大远平面
        );

    // 调整视图矩阵，确保相机距离足够远
    float cameraDistance = qMax(maxRange * 3.0f, diagonalLength * 2.0f);
    matrixView.lookAt(QVector3D(0, 0, cameraDistance), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    matrixView.translate(m_lineMove);

    // 模型矩阵：先缩放再旋转
    matrixModel.scale(m_scale);
    matrixModel.rotate(m_rotate);

    matrix = matrixPerspect * matrixView * matrixModel;
    m_Program->setUniformValue(m_matrixUniform, matrix);
}

void MyQOpenglWidget::initCloud()
{

    m_PointsVertex.clear();
    VertexInfo point;
    point.pos[0] =0.0f ;
    point.pos[1] =0.0f ;
    point.pos[2] =0.0f ;
    point.color[0] = m_backgroundColor.x();
    point.color[1] = m_backgroundColor.y();
    point.color[2] = m_backgroundColor.z();
    point.color[3] = m_backgroundColor.w();
    point.normal[0] = 0.0f;
    point.normal[1] = 1.0f;
    point.normal[2] = 0.0f;
    m_PointsVertex.push_back(point);

    // 修改5: 初始化时也添加坐标轴数据
    m_PointsVertex.resize(7); // 1个点 + 6个轴点
    addAxisData();
}

void MyQOpenglWidget::onTimerOut()
{
    if(this->isVisible())
    {
        repaint();
    }
}

void MyQOpenglWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->buttons()&Qt::LeftButton || e->buttons()&Qt::MiddleButton)
    {
        setMouseTracking(true);
        m_lastPoint = QVector2D(e->position());
    }
}

void MyQOpenglWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons()&Qt::LeftButton)
    {
        Rotate(QVector2D(m_lastPoint), QVector2D(e->position()));
    }
    if (e->buttons()&Qt::MiddleButton)
    {
        LineMove(m_lastPoint, QVector2D(e->position()));
    }
    m_lastPoint = QVector2D(e->position());
}

void MyQOpenglWidget::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setMouseTracking(false);
}

// 【额外修复】确保滚轮事件正确处理
void MyQOpenglWidget::wheelEvent(QWheelEvent *e)
{
    // 添加调试输出
    qDebug() << "滚轮事件:" << e->angleDelta().y();

    if(e->angleDelta().y() > 0){
        qDebug() << "放大";
        modelZoomInOrOut(true);
    }else{
        qDebug() << "缩小";
        modelZoomInOrOut(false);
    }

    e->accept(); // 确保事件被处理
}


void MyQOpenglWidget::leaveEvent(QEvent *)
{
    //不释放键盘事件抓取，会让编辑框无法输入
    releaseKeyboard();
}

void MyQOpenglWidget::enterEvent(QEvent *)
{
    grabKeyboard();
}

GLuint MyQOpenglWidget::createGPUProgram(QString nVertexShaderFile, QString nFragmentShaderFile)
{
    bool isOK = m_VertexShader->compileSourceFile(nVertexShaderFile);
    if (!isOK)
    {
        qDebug() << "compile vertex shader fail" ;
        return 0;
    }
    if (!m_FragmentShader->compileSourceFile(nFragmentShaderFile))
    {
        qDebug() << "compile fragment shader fail";
        return 0;
    }
    m_Program->addShader(m_VertexShader.get());
    m_Program->addShader(m_FragmentShader.get());
    m_Program->link();
    GetShaderUniformPara();
    return m_Program->programId();
}

void MyQOpenglWidget::LineMove(QVector2D posOrgin, QVector2D posEnd)
{
    float ratio =0.003f*sqrt(m_box.width()*m_box.height()) ;
    float xoffset = posEnd.x() - posOrgin.x();
    float yoffset = posEnd.y() - posOrgin.y();

    m_lineMove.setX(m_lineMove.x()+xoffset*ratio);
    m_lineMove.setY(m_lineMove.y()-yoffset*ratio);
}

void MyQOpenglWidget::Rotate(QVector2D posOrgin, QVector2D posEnd)
{
    QVector2D diff = posEnd - posOrgin;
    qreal acc = diff.length() / 100.0;
    if (acc<0.01f)
    {
        return;
    }
    calRotation(posOrgin,posEnd);
}

// 修复 modelZoomInOrOut 函数，添加重绘调用
// 修改 modelZoomInOrOut 函数，允许更大的缩小倍数
void MyQOpenglWidget::modelZoomInOrOut(bool ZoomInOrOut)
{
    float oldScale = m_scale;

    if(ZoomInOrOut) // zoom in
    {
        m_scale *= 1.1f;
        if(m_scale > 10.0f)  // 保持最大缩放限制
            m_scale = 10.0f;
    }
    else // zoom out
    {
        m_scale *= 0.9f;
        if(m_scale < 0.001f)   // 大幅降低最小缩放限制，从0.1f改为0.001f
            m_scale = 0.001f;   // 这样可以缩小到原来的1/1000
    }

    // 如果缩放值发生变化，强制重绘
    if (oldScale != m_scale) {
        update(); // 触发重绘
    }

    // 调试输出缩放值
    qDebug() << "缩放值:" << m_scale;
}


void MyQOpenglWidget::calRotation(QVector2D posOrgin, QVector2D posEnd)
{
    QVector3D orginViewPos = pixelPosToViewPos(posOrgin);
    QVector3D endViewPos = pixelPosToViewPos(posEnd);
    float RotateAngle;
    RotateAngle = qRadiansToDegrees(std::acos(QVector3D::dotProduct(orginViewPos,endViewPos)));
    QVector3D axis;
    axis=QVector3D::crossProduct(orginViewPos, endViewPos);
    axis.normalize();
    m_rotate=  QQuaternion::fromAxisAndAngle(axis, RotateAngle)*m_rotate;
}

QVector3D MyQOpenglWidget::pixelPosToViewPos(const QVector2D &p)
{
    QVector3D viewPos(2.0 * float(p.x()) / width() - 1.0,
                      1.0 - 2.0 * float(p.y()) / height(),
                      0);
    float sqrZ = 1 - QVector3D::dotProduct(viewPos, viewPos);
    if(sqrZ>0)
    {
        viewPos.setZ(std::sqrt(sqrZ));
    }else{
        viewPos.normalize();
    }
    return viewPos;
}

// 新增的渲染函数实现
void MyQOpenglWidget::renderPointCloud()
{
    m_Program->bind();
    m_VAO->bind();
    setMatrixUniform();
    
    // 绘制点云
    glDrawArrays(GL_POINTS, 6,(GLsizei ) m_PointsVertex.size()-6);
    
    m_VAO->release();
    m_Program->release();
}

void MyQOpenglWidget::renderAxis()
{
    m_Program->bind();
    m_VAO->bind();
    setMatrixUniform();
    
    // 启用混合以支持透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5f); // 设置更细的轴线宽度
    glDrawElements(GL_LINES,6,GL_UNSIGNED_INT,0);
    glLineWidth(1.0f); // 恢复默认线宽
    glDisable(GL_BLEND); // 禁用混合
    
    m_VAO->release();
    m_Program->release();
}

void MyQOpenglWidget::renderMesh()
{
    qDebug() << "=== Starting Mesh Rendering ===";

    // 检查必要的组件
    if (!m_modelManager || !m_MeshProgram) {
        qDebug() << "ERROR: ModelManager or MeshProgram not available";
        qDebug() << "  ModelManager:" << (m_modelManager ? "OK" : "NULL");
        qDebug() << "  MeshProgram:" << (m_MeshProgram ? "OK" : "NULL");
        return;
    }

    // 检查着色器是否正确初始化
    if (!m_meshShaderInitialized) {
        qDebug() << "ERROR: Mesh shader not properly initialized";
        return;
    }

    // 检查着色器程序是否链接成功
    if (!m_MeshProgram->isLinked()) {
        qDebug() << "ERROR: Mesh shader program is not linked";
        return;
    }
    
    // 检查是否有可用的mesh模型
    if (!m_modelManager->hasScene()) {
        qDebug() << "ERROR: No mesh scene available for rendering";
        return;
    }
    
    qDebug() << "SUCCESS: ModelManager and scene are available";
    
    // 使用mesh着色器
    if (!m_MeshProgram->bind()) {
        qDebug() << "ERROR: Failed to bind mesh shader program";
        return;
    }
    qDebug() << "SUCCESS: Mesh shader program bound";
    
    // 计算变换矩阵
    QMatrix4x4 matrix;
    QMatrix4x4 matrixPerspect;
    QMatrix4x4 matrixView;
    QMatrix4x4 matrixModel;

    // 使用mesh模型的包围盒参数
    QVector3D sceneMin = m_modelManager->getSceneMin();
    QVector3D sceneMax = m_modelManager->getSceneMax();
    QVector3D sceneCenter = m_modelManager->getSceneCenter();
    
    qDebug() << "Scene bounds:";
    qDebug() << "  Min:" << sceneMin;
    qDebug() << "  Max:" << sceneMax;
    qDebug() << "  Center:" << sceneCenter;
    
    // 计算场景大小
    QVector3D sceneSize = sceneMax - sceneMin;
    float maxSize = qMax(qMax(sceneSize.x(), sceneSize.y()), sceneSize.z());
    qDebug() << "  Size:" << sceneSize << "MaxSize:" << maxSize;
    
    // 检查场景大小是否合理
    if (maxSize <= 0.0f || maxSize > 10000.0f) {
        qDebug() << "WARNING: Unusual scene size detected, using default values";
        maxSize = 10.0f;
        sceneCenter = QVector3D(0, 0, 0);
    }
    
    // 改进的投影矩阵设置 - 使用透视投影
    float fov = 45.0f;
    float aspect = (float)width() / (float)height();
    float nearPlane = maxSize * 0.01f;  // 更近的近平面
    float farPlane = maxSize * 20.0f;   // 更远的远平面
    
    matrixPerspect.perspective(fov, aspect, nearPlane, farPlane);
    qDebug() << "Projection setup - FOV:" << fov << "Aspect:" << aspect << "Near:" << nearPlane << "Far:" << farPlane;
    
    // 改进的视图矩阵 - 动态相机距离
    float cameraDistance = maxSize * 2.5f; // 调整相机距离  
    QVector3D cameraPos(0, 0, cameraDistance);
    QVector3D targetPos(0, 0, 0);
    QVector3D upVector(0, 1, 0);
    
    matrixView.lookAt(cameraPos, targetPos, upVector);
    matrixView.rotate(m_rotate);
    matrixView.translate(m_lineMove);
    
    qDebug() << "Camera setup - Distance:" << cameraDistance << "Position:" << cameraPos;

    // 模型矩阵 - 先将模型中心移到原点，然后应用用户变换
    matrixModel.translate(-sceneCenter);  // 将模型中心移到原点
    matrixModel.scale(m_scale);
    matrixModel.rotate(m_rotate);

    matrix = matrixPerspect * matrixView * matrixModel;
    
    // 优化的光照设置 - 多个光源
    QVector3D lightPos1 = sceneCenter + QVector3D(maxSize * 0.8f, maxSize * 0.8f, maxSize * 0.8f);
    QVector3D lightPos2 = sceneCenter + QVector3D(-maxSize * 0.5f, maxSize * 0.5f, maxSize * 0.8f);
    QVector3D finalLightPos = (lightPos1 + lightPos2) * 0.5f; // 取平均位置
    
    qDebug() << "Lighting setup:";
    qDebug() << "  Light1:" << lightPos1;
    qDebug() << "  Light2:" << lightPos2;
    qDebug() << "  Final light pos:" << finalLightPos;
    
    // 设置着色器参数
    m_MeshProgram->setUniformValue(m_meshMatrixUniform, matrix);
    m_MeshProgram->setUniformValue(m_meshViewMatrixUniform, matrixView);
    m_MeshProgram->setUniformValue(m_meshLightPosUniform, finalLightPos);
    m_MeshProgram->setUniformValue(m_meshLightColorUniform, QVector3D(1.0f, 0.95f, 0.8f)); // 暖白光
    m_MeshProgram->setUniformValue(m_meshAmbientUniform, 0.35f); // 适中的环境光
    
    qDebug() << "Shader uniforms set";
    
    // 优化的OpenGL状态设置
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    
    // 禁用面剔除以确保所有面都被渲染
    glDisable(GL_CULL_FACE);
    
    // 启用多重采样抗锯齿
    glEnable(GL_MULTISAMPLE);
    
    // 设置合适的线宽和点大小
    glLineWidth(1.0f);
    glPointSize(1.0f);
    
    qDebug() << "OpenGL states configured";
    
    // 清除OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "OpenGL error before rendering:" << QString::number(error, 16);
    }
    
    // 渲染mesh模型
    qDebug() << "Calling ModelManager renderWithShader...";
    m_modelManager->renderWithShader(m_meshPosAttr, m_meshNorAttr);
    
    // 检查渲染后的OpenGL错误
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "OpenGL error after rendering:" << QString::number(error, 16);
    }
    
    // 恢复状态
    glDisable(GL_MULTISAMPLE);
    m_MeshProgram->release();
    
    qDebug() << "=== Mesh Rendering Complete ===";
}

// 新增：测试三角形渲染
void MyQOpenglWidget::renderTestTriangle()
{
    qDebug() << "Rendering test triangle...";
    
    // 简单的测试三角形顶点数据
    static const GLfloat testVertices[] = {
        0.0f,  0.5f, 0.0f,  // 顶部
        -0.5f, -0.5f, 0.0f, // 左下
        0.5f, -0.5f, 0.0f   // 右下
    };
    
    static const GLfloat testNormals[] = {
        0.0f, 0.0f, 1.0f,   // 所有法向量都指向z正方向
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };
    
    // 启用顶点属性数组
    glEnableVertexAttribArray(m_meshPosAttr);
    glEnableVertexAttribArray(m_meshNorAttr);
    
    // 绑定顶点数据
    glVertexAttribPointer(m_meshPosAttr, 3, GL_FLOAT, GL_FALSE, 0, testVertices);
    glVertexAttribPointer(m_meshNorAttr, 3, GL_FLOAT, GL_FALSE, 0, testNormals);
    
    // 绘制测试三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // 禁用顶点属性数组
    glDisableVertexAttribArray(m_meshPosAttr);
    glDisableVertexAttribArray(m_meshNorAttr);
    
    qDebug() << "Test triangle rendered";
}

// 新增的mesh相关功能实现
bool MyQOpenglWidget::loadMeshModel(const QString& modelPath)
{
    if (!m_modelManager) {
        qDebug() << "ERROR: ModelManager not initialized";
        return false;
    }
    
    qDebug() << "Loading mesh model:" << modelPath;
    
    bool success = m_modelManager->importModel(modelPath.toStdString());
    if (success) {
        qDebug() << "Model loaded successfully, switching to MeshOnly view mode";
        // 自动切换到mesh显示模式
        setViewMode(ViewMode::MeshOnly);
        update(); // 触发重绘
    } else {
        qDebug() << "Failed to load model:" << modelPath;
    }
    return success;
}

void MyQOpenglWidget::clearMeshModel()
{
    // 由ModelManager处理模型清理
    if (m_modelManager) {
        // ModelManager应该提供清理函数，这里暂时用重新初始化
        delete m_modelManager;
        m_modelManager = new ModelManager();
        // 重新初始化OpenGL
        m_modelManager->initializeGL();
    }
    update();
}

void MyQOpenglWidget::setViewMode(ViewMode mode)
{
    m_viewMode = mode;
    update();
}

ViewMode MyQOpenglWidget::getViewMode() const
{
    return m_viewMode;
}

void MyQOpenglWidget::setMeshVisible(bool visible)
{
    m_meshVisible = visible;
    update();
}

void MyQOpenglWidget::setPointCloudVisible(bool visible)
{
    m_pointCloudVisible = visible;
    update();
}

void MyQOpenglWidget::initMeshGL()
{
    // 初始化mesh渲染所需的OpenGL设置
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}



