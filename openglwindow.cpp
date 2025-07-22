#include "openglwindow.h"
#include <gl/freeglut.h>
#include <filesystem> // 必须包含此头文件

#define SCALE_FACTOR 0.1f
#define ROTATE_FACTOR 2.0f
#define TRANSLATE_FACTOR 15.0f
#define LOOKING_DISTANCE_FACTOR 4.0f

GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 300.0f, 0.0f, 1.0f };    //光照位置
GLfloat LightPosition2[] = { 0.0f, -300.0f, 0.0f, 1.0f };


float int2float(int intensity) {
    return (float)intensity / 255.0f;
}

OpenglWindow::OpenglWindow(QWidget *parent) :QOpenGLWidget(parent)  {
    size = 1.f;
    rotateAngleHoriFi = rotateAngleVertFi = 0;
    rotateAngleHori = rotateAngleVert = 0;
    translateHoriFi = translateVertFi = 0;
    translateHori = translateVert = 0;

    mouseOriPosX = -1;
    mouseOriPosY = -1;

    modelManager = new ModelManager;
}

OpenglWindow::~OpenglWindow() {
    delete modelManager;
}

// 修改 loadModel 函数，添加调试信息
bool OpenglWindow::loadModel(const QString& configPath)
{
    std::string utf8Path = configPath.toStdString();
    std::filesystem::path adjustedPath(utf8Path);
    adjustedPath.make_preferred();
    std::string pathStr = adjustedPath.string();

    qDebug() << pathStr;
    if (!modelManager->importModel(pathStr)) {
        qDebug() << "导入模型失败：" << pathStr;
        return false;
    }

    // 添加调试信息
    printSceneInfo();

    update();
    return true;
}

// 修改 initializeGL 函数 - 重点解决三角形显示问题
void OpenglWindow::initializeGL() {
    if (!modelManager->importModel("C:\\Users\\18438\\Desktop\\Resources\\Duck\\duck_triangulate.dae"))
        cout << "Import model error!" << endl;

    printSceneInfo();

    qDebug() <<"初始化";
    glEnable(GL_TEXTURE_2D);

    glClearColor(int2float(40), int2float(10), int2float(40), 1.f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // ========== 关键修改：处理面的渲染问题 ==========

    // 1. 完全禁用背面剔除
    glDisable(GL_CULL_FACE);

    // 2. 设置多边形模式 - 确保正反面都被绘制
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // 3. 启用双面光照
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // 4. 处理法向量问题
    glEnable(GL_NORMALIZE);  // 自动归一化法向量
    glEnable(GL_RESCALE_NORMAL);  // 缩放时重新计算法向量

    // 5. 设置前向面的定义（逆时针为正面）
    glFrontFace(GL_CCW);  // 尝试改为 GL_CW 如果还是有问题

    // 6. 启用多边形偏移，避免Z-fighting
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    //设置光照
    glEnable(GL_LIGHTING);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);

    glEnable(GL_LIGHT2);
    glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, LightPosition2);
}

// 修改 paintGL 函数 - 添加调试渲染模式
void OpenglWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // ========== 确保渲染设置正确 ==========
    glDisable(GL_CULL_FACE);  // 再次确认禁用背面剔除
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // 确保填充模式

    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 简化相机设置，使用固定角度观察
    float maxDimension = std::max({
        modelManager->scene_max.x - modelManager->scene_min.x,
        modelManager->scene_max.y - modelManager->scene_min.y,
        modelManager->scene_max.z - modelManager->scene_min.z
    });

    float cameraDistance = maxDimension * 3.0f;

    // 相机位置：从斜上方观察，确保能看到模型
    gluLookAt(cameraDistance, cameraDistance * 0.5f, cameraDistance,
              modelManager->scene_center.x,
              modelManager->scene_center.y,
              modelManager->scene_center.z,
              0, 1.0f, 0);

    //放缩
    glScalef(size, size, size);

    //平移
    glTranslatef(translateHoriFi + translateHori, translateVertFi + translateVert, 0);

    //旋转
    glRotatef(rotateAngleHoriFi + rotateAngleHori, 0.f, 1.f, 0.f);
    glRotatef(rotateAngleVertFi + rotateAngleVert, 1.f, 0.f, 0.f);

    //平移：把中心固定为模型中心
    glTranslatef(-(modelManager->scene_center.x),
                 -(modelManager->scene_center.y),
                 -(modelManager->scene_center.z));

    // ========== 调试：尝试手动绘制一些测试几何体 ==========
    // 可以临时启用这段代码来测试OpenGL设置是否正确
    /*
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();
    glEnable(GL_LIGHTING);
    */

    modelManager->renderTheModel();

    update();
}
// 新增调试函数：绘制坐标轴
void OpenglWindow::drawDebugAxes() {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);

    float axisLength = 50.0f;

    glBegin(GL_LINES);
    // X轴 - 红色
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(axisLength, 0.0f, 0.0f);

    // Y轴 - 绿色
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, axisLength, 0.0f);

    // Z轴 - 蓝色
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, axisLength);
    glEnd();

    glEnable(GL_LIGHTING);
    glLineWidth(1.0f);
}

// 新增调试函数：绘制包围盒
void OpenglWindow::drawBoundingBox() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f); // 黄色
    glLineWidth(1.0f);

    float minX = modelManager->scene_min.x;
    float minY = modelManager->scene_min.y;
    float minZ = modelManager->scene_min.z;
    float maxX = modelManager->scene_max.x;
    float maxY = modelManager->scene_max.y;
    float maxZ = modelManager->scene_max.z;

    glBegin(GL_LINE_LOOP);
    // 底面
    glVertex3f(minX, minY, minZ);
    glVertex3f(maxX, minY, minZ);
    glVertex3f(maxX, maxY, minZ);
    glVertex3f(minX, maxY, minZ);
    glEnd();

    glBegin(GL_LINE_LOOP);
    // 顶面
    glVertex3f(minX, minY, maxZ);
    glVertex3f(maxX, minY, maxZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(minX, maxY, maxZ);
    glEnd();

    glBegin(GL_LINES);
    // 竖直连接线
    glVertex3f(minX, minY, minZ); glVertex3f(minX, minY, maxZ);
    glVertex3f(maxX, minY, minZ); glVertex3f(maxX, minY, maxZ);
    glVertex3f(maxX, maxY, minZ); glVertex3f(maxX, maxY, maxZ);
    glVertex3f(minX, maxY, minZ); glVertex3f(minX, maxY, maxZ);
    glEnd();

    glEnable(GL_LIGHTING);
}

// 新增调试函数：强制渲染所有面
void OpenglWindow::forceRenderAllFaces() {
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    qDebug() << "强制渲染所有面";
    update();
}

// 添加调试函数：强制wireframe模式查看所有边
void OpenglWindow::debugWireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    update();
}

// 新增调试函数：切换深度测试
void OpenglWindow::toggleDepthTest() {
    static bool depthTestEnabled = true;
    depthTestEnabled = !depthTestEnabled;

    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
        qDebug() << "深度测试：开启";
    } else {
        glDisable(GL_DEPTH_TEST);
        qDebug() << "深度测试：关闭";
    }
    update();
}

// 新增调试函数：切换渲染模式
void OpenglWindow::debugRenderMode() {
    static int mode = 0;
    mode = (mode + 1) % 3;

    switch(mode) {
    case 0:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        qDebug() << "渲染模式：填充+光照";
        break;
    case 1:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);
        qDebug() << "渲染模式：线框";
        break;
    case 2:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_LIGHTING);
        glColor3f(0.8f, 0.8f, 0.8f);
        qDebug() << "渲染模式：填充无光照";
        break;
    }
    update();
}

// 添加调试函数：强制填充模式
void OpenglWindow::debugFilled() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
    update();
}

// 添加调试函数：测试不同的面定义方向
void OpenglWindow::toggleFrontFace() {
    static bool ccw = true;
    ccw = !ccw;
    glFrontFace(ccw ? GL_CCW : GL_CW);
    qDebug() << "Front face set to:" << (ccw ? "CCW" : "CW");
    update();
}

// 其他函数保持不变...
void OpenglWindow::printSceneInfo() {
    if (modelManager) {
        qDebug() << "Scene Min:" << modelManager->scene_min.x << modelManager->scene_min.y << modelManager->scene_min.z;
        qDebug() << "Scene Max:" << modelManager->scene_max.x << modelManager->scene_max.y << modelManager->scene_max.z;
        qDebug() << "Scene Center:" << modelManager->scene_center.x << modelManager->scene_center.y << modelManager->scene_center.z;

        float sceneDepth = modelManager->scene_max.z - modelManager->scene_min.z;
        float sceneWidth = modelManager->scene_max.x - modelManager->scene_min.x;
        float sceneHeight = modelManager->scene_max.y - modelManager->scene_min.y;

        qDebug() << "Scene Dimensions - Width:" << sceneWidth << "Height:" << sceneHeight << "Depth:" << sceneDepth;

        if (sceneDepth < sceneWidth * 0.2f || sceneDepth < sceneHeight * 0.2f) {
            qDebug() << "Warning: This appears to be a thin wall-like model.";
        }
    }
}

// 添加简单的相机重置功能
void OpenglWindow::resetView() {
    size = 1.0f;
    rotateAngleHoriFi = rotateAngleVertFi = 0;
    rotateAngleHori = rotateAngleVert = 0;
    translateHoriFi = translateVertFi = 0;
    translateHori = translateVert = 0;
    update();
}
// 修改 resizeGL 函数 - 确保正确的投影设置
void OpenglWindow::resizeGL(int width, int height) {
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 调整近远平面，避免裁剪问题
    float aspect = (float)width / (float)height;
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    gluPerspective(fov, aspect, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    qDebug() << "窗口尺寸:" << width << "x" << height << "宽高比:" << aspect;
}

void OpenglWindow::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() == Qt::LeftButton) {
        mouseOriPosX = event->pos().x();
        mouseOriPosY = event->pos().y();
    }
}

void OpenglWindow::mouseMoveEvent(QMouseEvent *event) {
    int mouseNowPosX = event->pos().x();
    int mouseNowPosY = event->pos().y();
    int dx = mouseNowPosX - mouseOriPosX;    //相对距离
    int dy = mouseNowPosY - mouseOriPosY;

    if (modelManager->transformMode == Rotation) {
        rotateAngleHori = (GLfloat)dx / ROTATE_FACTOR;
        rotateAngleVert = (GLfloat)dy / ROTATE_FACTOR;
    }
    else {
        translateHori = (GLfloat)dx / TRANSLATE_FACTOR;
        translateVert = (GLfloat)-dy / TRANSLATE_FACTOR;
    }
}

void OpenglWindow::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    if (modelManager->transformMode == Rotation) {
        rotateAngleHoriFi += rotateAngleHori;
        rotateAngleHori = 0;
        rotateAngleVertFi += rotateAngleVert;
        rotateAngleVert = 0;
    }
    else {
        translateHoriFi += translateHori;
        translateHori = 0;
        translateVertFi += translateVert;
        translateVert = 0;
    }
}

// void OpenglWindow::wheelEvent(QWheelEvent * event) {
//     if (event->delta() > 0) {
//         if (size > SCALE_FACTOR)
//             size -= SCALE_FACTOR;
//     }
//     else {
//         if (size < SCALE_FACTOR * 30.0f)
//             size += SCALE_FACTOR;
//     }
// }

void OpenglWindow::wheelEvent(QWheelEvent* event) {
    // 获取垂直滚动增量（支持触控板和高精度鼠标）
    const QPoint delta = event->angleDelta();

    if (!delta.isNull()) {
        if (delta.y() > 0) {  // 向上滚动
            if (size > SCALE_FACTOR)
                size -= SCALE_FACTOR;
        }
        else if (delta.y() < 0) {  // 向下滚动
            if (size < SCALE_FACTOR * 30.0f)
                size += SCALE_FACTOR;
        }
    }
    event->accept(); // 标记事件已处理
}

void OpenglWindow::loadTextureOp() {
    if (modelManager->textureState == TextureOff) {
        if (!modelManager->loadTextures())
            cout << "Load Texture Failure..." << endl;
    }
    modelManager->textureState = TextureOn;
}

void OpenglWindow::deleteTextureOp() {
    modelManager->textureState = TextureOff;
}

void OpenglWindow::showWireframe() {
    modelManager->displayMode = Wireframe;
}

void OpenglWindow::showFlat() {
    modelManager->displayMode = Flat;
}

void OpenglWindow::showFlatlines() {
    modelManager->displayMode = Flatlines;
}

void OpenglWindow::showColorNone() {
    modelManager->displayColor = None;
    update();
    cout << modelManager->displayColor << endl;
}

void OpenglWindow::showColorRed() {
    modelManager->displayColor = Red;
    update();
    cout << modelManager->displayColor << endl;
}

void OpenglWindow::showColorGreen() {
    modelManager->displayColor = Green;
    update();
    cout << modelManager->displayColor << endl;
}

void OpenglWindow::shadingGouraud() {
    modelManager->shadingMode = Gouraud;
}

void OpenglWindow::shadingPhong() {
    modelManager->shadingMode = Phong;
}

void OpenglWindow::shadingFlat() {
    modelManager->shadingMode = FlatS;
}

void OpenglWindow::rotationOp() {
    modelManager->transformMode = Rotation;
}

void OpenglWindow::translationOp() {
    modelManager->transformMode = Translation;
}

void OpenglWindow::subdivisionOn() {
    modelManager->subdivisionDepth = 1;
}

void OpenglWindow::subdivisionOff() {
    modelManager->subdivisionDepth = 0;
}
