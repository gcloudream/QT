#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QFileSystemModel>
#include <QTreeView>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "myqopenglwidget.h"
#include "pcdreader.h"
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui {

class MainWindow; }
class QAction;
class QStackedWidget;
QT_END_NAMESPACE

class LineplotWidget;

struct PCDHeader {
    std::vector<QString> fields;
    std::vector<int> sizes;
    std::vector<char> types;
    std::vector<int> counts;
    int width = 0;
    int height = 0;
    int points = 0;
    QString data_type = "ascii";
    qint64 header_size = 0;
};

class MainWindow : public QMainWindow
{


    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openProject();
    void newProject();  // 新建项目槽函数
    void onCustomContextMenuRequested(const QPoint &pos);
    void createNewFile();
    void createNewFolder();
    void deleteSelectedItem();
    void renameSelectedItem();

    void PointCloud();
    void PointCloud2();

    void onImportModelTriggered();
    void onImportModelTriggered2();

    void loadTextureOp();        //加载纹理
    void deleteTextureOp();      //删除纹理
    void showColorNone();        //显示颜色
    void showColorRed();
    void showColorGreen();
    void showWireframe();        //三种显示方式
    void showFlat();
    void showFlatlines();
    void shadingGouraud();       //三种shading方式
    void shadingPhong();
    void shadingFlat();
    void rotationOp();           //两种变换方式
    void translationOp();
    void subdivisionOn();        //是否做细分
    void subdivisionOff();

    void generateFloorPlanWithTexture();
    void executeBashScript();

    void ClearAllPointClouds();

    void analyzePCDFile(const QString& filename);

    void showLineView();
    void showOriginalView();




private:
    Ui::MainWindow *ui;
    QFileSystemModel* m_dirModel;
    MyQOpenglWidget* m_pOpenglWidget;

    LineplotWidget *m_plotWidget;
    QStackedWidget *m_stackedWidget;
    QWidget *m_originalWidget;
    QWidget *m_lineViewWidget;
    QList<QAction*> m_originalMenuActions;

    // 新增私有方法
    void setupFileSystemModel(const QString &rootPath = "D:/BaiduNetdiskDownload/demoC3/demoC2",
                              const QStringList &filters = {"*.*"});

    void setupActions();
    QMenu *m_contextMenu;
    QAction *m_actionNewFile;
    QAction *m_actionNewFolder;
    QAction *m_actionDelete;
    QAction *m_actionRename;
    QAction *m_actionPointCloud;
    QAction *m_actionMesh;
    QAction *m_actionVec;

    std::vector<QVector3D> ReadVec3PointCloudASC(QString path);
    std::vector<QVector3D> ReadVec3PointCloudPLY(QString path);
    std::vector<QVector3D> ReadVec3PointCloudPCD(const QString& filename);
    std::vector<QVector3D> ReadVec3PointCloudTXT(const QString& filename);
    void unitMillimeter2UnitMeter(std::vector<QVector3D> &inOutCloud);
    std::vector<QVector3D> testData(int pointsNum);
    QVector3D randomVec3f();

    std::vector<QVector3D> m_currentCloud; // 新增当前点云存储

    // 已移除OpenglWindow，统一使用MyQOpenglWidget

    void setupStackedWidget();
    void createLineView();
    void updateMenusForLineView();
    void updateMenusForOriginalView();
    void setupLineViewMenus();


};
#endif // MAINWINDOW_H
