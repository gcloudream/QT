/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionAdd;
    QAction *actionOpen_PLY;
    QAction *actionVectorization_2;
    QAction *actionTextureOn;
    QAction *actionTextureOff;
    QAction *actionColorNone;
    QAction *actionColorRed;
    QAction *actionColorGreen;
    QAction *actionSubdiss;
    QAction *actionSubdis;
    QAction *actionSubdivisionOn;
    QAction *actionSubdivisionOff;
    QAction *actionRotate;
    QAction *actionTranslate;
    QAction *actionWireframe;
    QAction *actionFlat;
    QAction *actionFlatlines;
    QAction *actionGouraud_Shading;
    QAction *actionPhong_Shading;
    QAction *actionFlat_Shading;
    QAction *actionClearPointCloud;
    QAction *actionLineView;
    QAction *actiongenerate_floorplan_with_texture;
    QWidget *centralwidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QTreeView *treeView;
    QTabWidget *tabWidget;
    QWidget *tab_unified;
    QWidget *displayControlWidget;
    QPushButton *btnPointCloud;
    QPushButton *btnMesh;
    QPushButton *btnHybrid;
    QPushButton *pushButtonX;
    QPushButton *pushButtonY;
    QPushButton *pushButtonZ;
    QOpenGLWidget *openGLWidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuVive;
    QMenu *menuAnalyze;
    QMenu *menuTexture;
    QMenu *menuColor;
    QMenu *menuSubdivision;
    QMenu *menuTransform;
    QMenu *menuDisplay_Mode;
    QMenu *menuShading_Mode;
    QMenu *menuTools;
    QMenu *menuOther;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1177, 761);
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName("actionNew");
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName("actionOpen");
        actionAdd = new QAction(MainWindow);
        actionAdd->setObjectName("actionAdd");
        actionOpen_PLY = new QAction(MainWindow);
        actionOpen_PLY->setObjectName("actionOpen_PLY");
        actionVectorization_2 = new QAction(MainWindow);
        actionVectorization_2->setObjectName("actionVectorization_2");
        actionTextureOn = new QAction(MainWindow);
        actionTextureOn->setObjectName("actionTextureOn");
        actionTextureOff = new QAction(MainWindow);
        actionTextureOff->setObjectName("actionTextureOff");
        actionColorNone = new QAction(MainWindow);
        actionColorNone->setObjectName("actionColorNone");
        actionColorRed = new QAction(MainWindow);
        actionColorRed->setObjectName("actionColorRed");
        actionColorGreen = new QAction(MainWindow);
        actionColorGreen->setObjectName("actionColorGreen");
        actionSubdiss = new QAction(MainWindow);
        actionSubdiss->setObjectName("actionSubdiss");
        actionSubdis = new QAction(MainWindow);
        actionSubdis->setObjectName("actionSubdis");
        actionSubdivisionOn = new QAction(MainWindow);
        actionSubdivisionOn->setObjectName("actionSubdivisionOn");
        actionSubdivisionOff = new QAction(MainWindow);
        actionSubdivisionOff->setObjectName("actionSubdivisionOff");
        actionRotate = new QAction(MainWindow);
        actionRotate->setObjectName("actionRotate");
        actionTranslate = new QAction(MainWindow);
        actionTranslate->setObjectName("actionTranslate");
        actionWireframe = new QAction(MainWindow);
        actionWireframe->setObjectName("actionWireframe");
        actionFlat = new QAction(MainWindow);
        actionFlat->setObjectName("actionFlat");
        actionFlatlines = new QAction(MainWindow);
        actionFlatlines->setObjectName("actionFlatlines");
        actionGouraud_Shading = new QAction(MainWindow);
        actionGouraud_Shading->setObjectName("actionGouraud_Shading");
        actionPhong_Shading = new QAction(MainWindow);
        actionPhong_Shading->setObjectName("actionPhong_Shading");
        actionFlat_Shading = new QAction(MainWindow);
        actionFlat_Shading->setObjectName("actionFlat_Shading");
        actionClearPointCloud = new QAction(MainWindow);
        actionClearPointCloud->setObjectName("actionClearPointCloud");
        actionLineView = new QAction(MainWindow);
        actionLineView->setObjectName("actionLineView");
        actiongenerate_floorplan_with_texture = new QAction(MainWindow);
        actiongenerate_floorplan_with_texture->setObjectName("actiongenerate_floorplan_with_texture");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayoutWidget = new QWidget(centralwidget);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(-1, -1, 221, 731));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        treeView = new QTreeView(verticalLayoutWidget);
        treeView->setObjectName("treeView");

        verticalLayout->addWidget(treeView);

        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(220, 0, 941, 721));
        tab_unified = new QWidget();
        tab_unified->setObjectName("tab_unified");
        displayControlWidget = new QWidget(tab_unified);
        displayControlWidget->setObjectName("displayControlWidget");
        displayControlWidget->setGeometry(QRect(30, 0, 891, 35));
        btnPointCloud = new QPushButton(displayControlWidget);
        btnPointCloud->setObjectName("btnPointCloud");
        btnPointCloud->setGeometry(QRect(10, 5, 80, 25));
        btnMesh = new QPushButton(displayControlWidget);
        btnMesh->setObjectName("btnMesh");
        btnMesh->setGeometry(QRect(100, 5, 80, 25));
        btnHybrid = new QPushButton(displayControlWidget);
        btnHybrid->setObjectName("btnHybrid");
        btnHybrid->setGeometry(QRect(190, 5, 80, 25));
        pushButtonX = new QPushButton(displayControlWidget);
        pushButtonX->setObjectName("pushButtonX");
        pushButtonX->setGeometry(QRect(800, 5, 21, 25));
        pushButtonY = new QPushButton(displayControlWidget);
        pushButtonY->setObjectName("pushButtonY");
        pushButtonY->setGeometry(QRect(830, 5, 21, 25));
        pushButtonZ = new QPushButton(displayControlWidget);
        pushButtonZ->setObjectName("pushButtonZ");
        pushButtonZ->setGeometry(QRect(860, 5, 21, 25));
        openGLWidget = new QOpenGLWidget(tab_unified);
        openGLWidget->setObjectName("openGLWidget");
        openGLWidget->setGeometry(QRect(30, 40, 891, 641));
        tabWidget->addTab(tab_unified, QString());
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1177, 20));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName("menuEdit");
        menuVive = new QMenu(menubar);
        menuVive->setObjectName("menuVive");
        menuAnalyze = new QMenu(menubar);
        menuAnalyze->setObjectName("menuAnalyze");
        menuTexture = new QMenu(menuAnalyze);
        menuTexture->setObjectName("menuTexture");
        menuColor = new QMenu(menuAnalyze);
        menuColor->setObjectName("menuColor");
        menuSubdivision = new QMenu(menuAnalyze);
        menuSubdivision->setObjectName("menuSubdivision");
        menuTransform = new QMenu(menuAnalyze);
        menuTransform->setObjectName("menuTransform");
        menuDisplay_Mode = new QMenu(menuAnalyze);
        menuDisplay_Mode->setObjectName("menuDisplay_Mode");
        menuShading_Mode = new QMenu(menuAnalyze);
        menuShading_Mode->setObjectName("menuShading_Mode");
        menuTools = new QMenu(menubar);
        menuTools->setObjectName("menuTools");
        menuOther = new QMenu(menubar);
        menuOther->setObjectName("menuOther");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuVive->menuAction());
        menubar->addAction(menuAnalyze->menuAction());
        menubar->addAction(menuTools->menuAction());
        menubar->addAction(menuOther->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionAdd);
        menuFile->addAction(actionOpen_PLY);
        menuFile->addAction(actionVectorization_2);
        menuFile->addAction(actionClearPointCloud);
        menuFile->addAction(actionLineView);
        menuAnalyze->addAction(menuDisplay_Mode->menuAction());
        menuAnalyze->addAction(menuShading_Mode->menuAction());
        menuAnalyze->addAction(menuTexture->menuAction());
        menuAnalyze->addAction(menuColor->menuAction());
        menuAnalyze->addAction(menuTransform->menuAction());
        menuAnalyze->addAction(menuSubdivision->menuAction());
        menuTexture->addAction(actionTextureOn);
        menuTexture->addAction(actionTextureOff);
        menuColor->addAction(actionColorNone);
        menuColor->addAction(actionColorRed);
        menuColor->addAction(actionColorGreen);
        menuSubdivision->addAction(actionSubdivisionOn);
        menuSubdivision->addAction(actionSubdivisionOff);
        menuTransform->addAction(actionRotate);
        menuTransform->addAction(actionTranslate);
        menuDisplay_Mode->addAction(actionWireframe);
        menuDisplay_Mode->addAction(actionFlat);
        menuDisplay_Mode->addAction(actionFlatlines);
        menuShading_Mode->addAction(actionGouraud_Shading);
        menuShading_Mode->addAction(actionPhong_Shading);
        menuShading_Mode->addAction(actionFlat_Shading);
        menuTools->addAction(actiongenerate_floorplan_with_texture);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\345\242\231\344\275\223\346\217\220\345\217\226\346\274\224\347\244\272\347\263\273\347\273\237", nullptr));
        actionNew->setText(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272\351\241\271\347\233\256", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\351\241\271\347\233\256", nullptr));
        actionAdd->setText(QCoreApplication::translate("MainWindow", "\346\267\273\345\212\240\346\226\207\344\273\266", nullptr));
        actionOpen_PLY->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\347\202\271\344\272\221\346\226\207\344\273\266", nullptr));
        actionVectorization_2->setText(QCoreApplication::translate("MainWindow", "\347\237\242\351\207\217\345\214\226", nullptr));
        actionTextureOn->setText(QCoreApplication::translate("MainWindow", "TextureOn", nullptr));
        actionTextureOff->setText(QCoreApplication::translate("MainWindow", "TextureOff", nullptr));
        actionColorNone->setText(QCoreApplication::translate("MainWindow", "ColorNone", nullptr));
        actionColorRed->setText(QCoreApplication::translate("MainWindow", "ColorRed", nullptr));
        actionColorGreen->setText(QCoreApplication::translate("MainWindow", "ColorGreen", nullptr));
        actionSubdiss->setText(QCoreApplication::translate("MainWindow", "SubdivisionOn", nullptr));
        actionSubdis->setText(QCoreApplication::translate("MainWindow", "SubdivisionOff", nullptr));
        actionSubdivisionOn->setText(QCoreApplication::translate("MainWindow", "SubdivisionOn", nullptr));
        actionSubdivisionOff->setText(QCoreApplication::translate("MainWindow", "SubdivisionOff", nullptr));
        actionRotate->setText(QCoreApplication::translate("MainWindow", "Rotate", nullptr));
        actionTranslate->setText(QCoreApplication::translate("MainWindow", "Translate", nullptr));
        actionWireframe->setText(QCoreApplication::translate("MainWindow", "Wireframe", nullptr));
        actionFlat->setText(QCoreApplication::translate("MainWindow", "Flat", nullptr));
        actionFlatlines->setText(QCoreApplication::translate("MainWindow", "Flatlines", nullptr));
        actionGouraud_Shading->setText(QCoreApplication::translate("MainWindow", "Gouraud_Shading", nullptr));
        actionPhong_Shading->setText(QCoreApplication::translate("MainWindow", "Phong_Shading", nullptr));
        actionFlat_Shading->setText(QCoreApplication::translate("MainWindow", "Flat_Shading", nullptr));
        actionClearPointCloud->setText(QCoreApplication::translate("MainWindow", "ClearPointCloud", nullptr));
        actionLineView->setText(QCoreApplication::translate("MainWindow", "LineView", nullptr));
        actiongenerate_floorplan_with_texture->setText(QCoreApplication::translate("MainWindow", "generate_floorplan_with_texture", nullptr));
        btnPointCloud->setText(QCoreApplication::translate("MainWindow", "\347\202\271\344\272\221\346\250\241\345\274\217", nullptr));
        btnMesh->setText(QCoreApplication::translate("MainWindow", "Mesh\346\250\241\345\274\217", nullptr));
        btnHybrid->setText(QCoreApplication::translate("MainWindow", "\346\267\267\345\220\210\346\230\276\347\244\272", nullptr));
        pushButtonX->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        pushButtonY->setText(QCoreApplication::translate("MainWindow", "Y", nullptr));
        pushButtonZ->setText(QCoreApplication::translate("MainWindow", "Z", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_unified), QCoreApplication::translate("MainWindow", "3D\346\230\276\347\244\272", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "\347\274\226\350\276\221", nullptr));
        menuVive->setTitle(QCoreApplication::translate("MainWindow", "\350\247\206\345\233\276", nullptr));
        menuAnalyze->setTitle(QCoreApplication::translate("MainWindow", "\345\210\206\346\236\220", nullptr));
        menuTexture->setTitle(QCoreApplication::translate("MainWindow", "Texture", nullptr));
        menuColor->setTitle(QCoreApplication::translate("MainWindow", "Color", nullptr));
        menuSubdivision->setTitle(QCoreApplication::translate("MainWindow", "Subdivision", nullptr));
        menuTransform->setTitle(QCoreApplication::translate("MainWindow", "Transform", nullptr));
        menuDisplay_Mode->setTitle(QCoreApplication::translate("MainWindow", "Display Mode", nullptr));
        menuShading_Mode->setTitle(QCoreApplication::translate("MainWindow", "Shading Mode", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        menuOther->setTitle(QCoreApplication::translate("MainWindow", "Other", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
