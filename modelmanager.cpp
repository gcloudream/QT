#include "modelmanager.h"
#include <gl/freeglut.h>
#include <IL/ilu.h>  // 新增此行

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <codecvt>
#include <locale>
#include <QDir>

#include <iostream>
#include <fstream>
#include <assimp/Importer.hpp>

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)<(y)?(y):(x))

const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 0.0f };
const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat green[] = { 0.0f, 1.0f, 1.0f, 1.0f };

void normalizeVector(aiVector3D& v) {
    GLfloat d = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x /= d;
    v.y /= d;
    v.z /= d;
}

// 修改后的ModelManager构造函数
ModelManager::ModelManager() {
    textureState = TextureOff;
    displayMode = Flat;
    displayColor = None;
    shadingMode = Gouraud;
    transformMode = Rotation;
    subdivisionDepth = 0;

    // 添加OpenGL状态初始化
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glFrontFace(GL_CCW);  // 设置正面为逆时针
}

ModelManager::~ModelManager() {
    textureIdMap.clear();

    if (textureIds != NULL) {
        delete[] textureIds;
        textureIds = NULL;
    }

    aiReleaseImport(scene);    //清除new的空间，防止内存泄露
}

string getBasePath(const string& path) {
    size_t pos = path.find_last_of("\\/");
    return (string::npos == pos) ? "" : path.substr(0, pos + 1);
}

bool ModelManager::importModel(const string& pFilepath) {
    // ifstream modelFilePath(pFile.c_str());
    // if (modelFilePath.fail()) {
    //     cout << "Error::could not read model path file." << endl;
    //     return false;
    // }

    // int modelCount = 4;
    // while (getline(modelFilePath, modelPath)) {
    //     if (modelPath.empty()) {
    //         cout << "Error::model path empty." << endl;
    //         return false;
    //     }
    //     else if (modelPath[0] == '#') {
    //         cout << "Next Line" << endl;
    //     }
    //     else {
    //         cout << "modelPath " << modelPath << endl;
    //         break;
    //     }

    //     modelCount--;
    //     if (modelCount <= 0)
    //         return false;
    // }
    // modelFilePath.close();
    // modelPath=;
    // qDebug() <<modelPath<<"123";

    scene = aiImportFile(pFilepath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);


    if (!scene)
        return false;
    else {
        cout << "Import successfully!" << endl;
        getBoundingBox();
        scene_center.x = (scene_min.x + scene_max.x) / 2.0f;    //设置模型的中心
        scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
        scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
    }
    return true;
}

void normalizePath(string& path) {
    // 统一替换正斜杠为反斜杠
    std::replace(path.begin(), path.end(), '/', '\\');

    // 使用Qt的路径规范化功能
    // QDir dir(QString::fromStdString(path));
    // path = dir.absolutePath().toStdString();
}

// 辅助函数：将UTF-8字符串转换为宽字符串
std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

bool ModelManager::loadTextures() {
    ILboolean success;

    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
        string err_msg = "Wrong DevIL version. Old devil.dll in system32/SysWow64?";
        cout << err_msg << endl;
        return false;
    }

    ilInit();

    //对于每一种材质 Material：
    for (unsigned int m = 0; m < scene->mNumMaterials; m++) {
        int texIndex = 0;
        aiReturn texFound = AI_SUCCESS;

        aiString path;

        //对当前材质，获取所有texture的图片id
        while (texFound == AI_SUCCESS) {
            texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
            textureIdMap[path.data] = NULL;    //把纹理图片路径加到map的key值，value（纹理指针）为空
            texIndex++;
        }
    }

    int numTextures = textureIdMap.size();    //texture数量

    ILuint* imageIds = NULL;
    imageIds = new ILuint[numTextures];

    ilGenImages(numTextures, imageIds);

    if (textureIds != NULL) {
        delete[] textureIds;
    }
    textureIds = new GLuint[numTextures];
    glGenTextures(numTextures, textureIds);    //根据纹理参数返回n个纹理名称（不一定是连续的整数集合）

    // get iterator
    map<string, GLuint*>::iterator itr = textureIdMap.begin();

    string basepath = getBasePath(modelPath);

    //对于每个texture
    for (int i = 0; i < numTextures; i++) {
        //save IL image ID
        string filename = (*itr).first;  // get filename
        cout << "filename " << filename << endl;
        (*itr).second = &textureIds[i];	  //把每个纹理Id放进map的value
        itr++;								  // next texture

        ilBindImage(imageIds[i]);    //每个图像id绑定一张图
        string fileloc = basepath + filename;
        normalizePath(fileloc); // 新增路径规范化
        cout << "fileloc: " << fileloc << endl;
        std::wstring wfileloc = utf8_to_wstring(fileloc); // 转换为宽字符串

        success = ilLoadImage(wfileloc.c_str()); // 使用宽字符路径
        // cout<<success+"111"<<endl;
        if (true) { // If no error occurred:
            success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
            if (!success) {
                // Error occurred
                return false;
            }
            // Binding of texture name
            glBindTexture(GL_TEXTURE_2D, textureIds[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // We will use linear interpolation for minifying filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            // Texture specification
            glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
                         ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
                         ilGetData());
            // we also want to be able to deal with odd texture dimensions
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        }
    }
    // if (!success) {
    //     ILenum error = ilGetError();
    //     std::cerr << "DevIL Error: " << error << " - " << iluErrorString(error) << std::endl;
    //     return false;
    // }
    // Because we have already copied image data into texture data  we can release memory used by image.
    ilDeleteImages(numTextures, imageIds);

    // Cleanup
    delete[] imageIds;
    imageIds = NULL;

    return true;
}

void ModelManager::getBoundingBox() {
    aiMatrix4x4 trafo;
    aiIdentityMatrix4(&trafo);

    scene_min.x = scene_min.y = scene_min.z =  1e10f;    //初始化为最大
    scene_max.x = scene_max.y = scene_max.z = -1e10f;    //初始化为最小
    getBoundingBoxRecursive(scene->mRootNode, &trafo);
}
void ModelManager::getBoundingBoxRecursive(const struct aiNode* nd, aiMatrix4x4* trafo) {
    aiMatrix4x4 prev;
    prev = *trafo;
    aiMultiplyMatrix4(trafo, &nd->mTransformation);

    unsigned int n, t;
    //对当前节点，遍历该节点的所有mMeshes(contains index to a mesh in scene.mMeshes[])
    for (n = 0; n < nd->mNumMeshes; ++n) {
        const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

        //对当前的mesh，遍历所有顶点：找到所有顶点里x/y/z的最大/最小值
        for (t = 0; t < mesh->mNumVertices; ++t) {
            aiVector3D tmp = mesh->mVertices[t];
            aiTransformVecByMatrix4(&tmp, trafo);    //转化为同一个坐标系下

            scene_min.x = MIN(scene_min.x, tmp.x);
            scene_min.y = MIN(scene_min.y, tmp.y);
            scene_min.z = MIN(scene_min.z, tmp.z);

            scene_max.x = MAX(scene_max.x, tmp.x);
            scene_max.y = MAX(scene_max.y, tmp.y);
            scene_max.z = MAX(scene_max.z, tmp.z);
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        getBoundingBoxRecursive(nd->mChildren[n], trafo);
    }
    *trafo = prev;
}

// 修改后的renderTheModel方法
void ModelManager::renderTheModel() {
    // 检查场景是否已加载
    if (!scene || !scene->mRootNode) {
        return; // 如果没有加载模型，直接返回
    }
    
    // 设置基本的OpenGL状态
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // 临时禁用背面剔除以显示所有面

    // 如果需要调试，可以使用线框模式
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    recursiveRender(scene, scene->mRootNode);
}
// 修改后的recursiveRender方法
void ModelManager::recursiveRender(const struct aiScene *sc, const struct aiNode* nd) {
    // 添加安全检查
    if (!sc || !nd) return;
    
    aiMatrix4x4 mTrans = nd->mTransformation;

    // 更新每个节点的变换方式
    mTrans.Transpose();
    glPushMatrix();
    glMultMatrixf((float*)&mTrans);

    // 对当前节点，遍历该节点的所有mMeshes
    for (unsigned int m = 0; m < nd->mNumMeshes; m++) {
        const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];

        // 调试信息
        // cout << "Rendering mesh " << m << " with " << mesh->mNumFaces << " faces" << endl;

        // 添加texture和材质
        applyMaterial(sc->mMaterials[mesh->mMaterialIndex]);

        // 检查法向量
        if (mesh->mNormals == NULL) {
            glDisable(GL_LIGHTING);
        } else {
            glEnable(GL_LIGHTING);
        }

        // 设置颜色材质
        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        // 对当前的mesh，遍历所有面face
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const struct aiFace* face = &(mesh->mFaces[f]);

            // 调试信息
            // cout << "Face " << f << " has " << face->mNumIndices << " vertices" << endl;

            GLenum face_mode;
            switch (face->mNumIndices) {
            case 1:
                face_mode = GL_POINTS;
                break;
            case 2:
                face_mode = GL_LINES;
                break;
            case 3:
                face_mode = GL_TRIANGLES;
                break;
            default:
                face_mode = GL_POLYGON;
                break;
            }

            processFace(mesh, face_mode, face);
        }

        glDisable(GL_COLOR_MATERIAL);
    }

    // 递归绘制其他子节点
    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        recursiveRender(sc, nd->mChildren[n]);
    }

    glPopMatrix();
}

// 修改后的processFace方法
void ModelManager::processFace(const struct aiMesh* mesh, GLenum face_mode, const struct aiFace* face) {
    // 调试信息
    // cout << "Processing face with mode: " << face_mode << ", indices: " << face->mNumIndices << endl;

    if (face_mode == GL_POLYGON) {    // 四边形或多边形
        if (displayMode == Flat || displayMode == Flatlines) {
            renderPolygonFaceFlat(mesh, face_mode, face);
        }
        if (displayMode == Wireframe || displayMode == Flatlines) {
            renderPolygonFaceWireframe(mesh, face);
        }
    }
    else if (face_mode == GL_TRIANGLES) {    // 三角形
        // 确保有足够的顶点
        if (face->mNumIndices != 3) {
            cout << "Warning: Triangle face doesn't have 3 vertices!" << endl;
            return;
        }

        aiVector3D vPos[3];
        aiVector3D vNor[3];
        aiVector3D vTexPos[3];

        // 获取三角形的三个顶点数据
        for (int i = 0; i < 3; i++) {
            int index = face->mIndices[i];

            // 检查索引是否有效
            if (index >= mesh->mNumVertices) {
                cout << "Error: Invalid vertex index " << index << endl;
                return;
            }

            vPos[i] = mesh->mVertices[index];

            // 获取法向量
            if (mesh->mNormals != NULL) {
                vNor[i] = mesh->mNormals[index];
            } else {
                // 如果没有法向量，计算面法向量
                if (i == 0) {
                    aiVector3D edge1 = vPos[1] - vPos[0];
                    aiVector3D edge2 = vPos[2] - vPos[0];
                    vNor[0] = edge1 ^ edge2;  // 叉积
                    normalizeVector(vNor[0]);
                    vNor[1] = vNor[2] = vNor[0];
                }
            }

            // 获取纹理坐标
            if (mesh->HasTextureCoords(0)) {
                vTexPos[i] = mesh->mTextureCoords[0][index];
            } else {
                vTexPos[i] = aiVector3D(0.0f, 0.0f, 0.0f);
            }
        }

        // 进行细分或直接渲染
        subdivision(vPos[0], vPos[1], vPos[2],
                    vNor[0], vNor[1], vNor[2],
                    vTexPos[0], vTexPos[1], vTexPos[2], subdivisionDepth);
    }
    else if (face_mode == GL_LINES) {
        // 处理线段
        renderLineFace(mesh, face);
    }
    else if (face_mode == GL_POINTS) {
        // 处理点
        renderPointFace(mesh, face);
    }
}

void ModelManager::subdivision(aiVector3D vPos1, aiVector3D vPos2, aiVector3D vPos3,
                               aiVector3D vNor1, aiVector3D vNor2, aiVector3D vNor3,
                               aiVector3D vTexPos1, aiVector3D vTexPos2, aiVector3D vTexPos3,
                               int depth) {
    if (depth == 0) {
        if (displayMode == Flat || displayMode == Flatlines) {
            if (shadingMode == Phong) {
                renderFaceFlatPhong(vPos1, vPos2, vPos3, vNor1, vNor2, vNor3, vTexPos1, vTexPos2, vTexPos3);
            }
            else {    //如果是Flat shading或Gouraud shading，只需要渲染一个面
                renderTriangleFaceFlat(vPos1, vPos2, vPos3, vNor1, vNor2, vNor3, vTexPos1, vTexPos2, vTexPos3);
            }
        }

        if (displayMode == Wireframe || displayMode == Flatlines) {
            renderTriangleFaceWireframe(vPos1, vPos2, vPos3);
        }

        return;
    }

    aiVector3D vPosMi[3];
    aiVector3D vNorMi[3];
    aiVector3D vTexPosMi[3];

    //取中间点
    vPosMi[0] = (vPos1 + vPos2) / 2.f;
    vPosMi[1] = (vPos2 + vPos3) / 2.f;
    vPosMi[2] = (vPos3 + vPos1) / 2.f;
    vNorMi[0] = (vNor1 + vNor2) / 2.f;
    vNorMi[1] = (vNor2 + vNor3) / 2.f;
    vNorMi[2] = (vNor3 + vNor1) / 2.f;
    normalizeVector(vNorMi[0]);
    normalizeVector(vNorMi[1]);
    normalizeVector(vNorMi[2]);
    vTexPosMi[0] = (vTexPos1 + vTexPos2) / 2.f;
    vTexPosMi[1] = (vTexPos2 + vTexPos3) / 2.f;
    vTexPosMi[2] = (vTexPos3 + vTexPos1) / 2.f;

    //顺序很重要
    subdivision(vPos1, vPosMi[0], vPosMi[2],
                vNor1, vNorMi[0], vNorMi[2],
                vTexPos1, vTexPosMi[0], vTexPosMi[2], depth - 1);
    subdivision(vPosMi[0], vPos2, vPosMi[1],
                vNorMi[0], vNor2, vNorMi[1],
                vTexPosMi[0], vTexPos2, vTexPosMi[1], depth - 1);
    subdivision(vPos3, vPosMi[2], vPosMi[1],
                vNor3, vNorMi[2], vNorMi[1],
                vTexPos3, vTexPosMi[2], vTexPosMi[1], depth - 1);
    subdivision(vPosMi[0], vPosMi[1], vPosMi[2],
                vNorMi[0], vNorMi[1], vNorMi[2],
                vTexPosMi[0], vTexPosMi[1], vTexPosMi[2], depth - 1);
}

void ModelManager::renderFaceFlatPhong(aiVector3D vPos1, aiVector3D vPos2, aiVector3D vPos3,
                                       aiVector3D vNor1, aiVector3D vNor2, aiVector3D vNor3,
                                       aiVector3D vTexPos1, aiVector3D vTexPos2, aiVector3D vTexPos3) {
    aiVector3D vPosMi[3];
    aiVector3D vNorMi[3];
    aiVector3D vTexPosMi[3];

    //取中间点
    vPosMi[0] = (vPos1 + vPos2) / 2.f;
    vPosMi[1] = (vPos2 + vPos3) / 2.f;
    vPosMi[2] = (vPos3 + vPos1) / 2.f;
    vNorMi[0] = (vNor1 + vNor2) / 2.f;
    vNorMi[1] = (vNor2 + vNor3) / 2.f;
    vNorMi[2] = (vNor3 + vNor1) / 2.f;
    normalizeVector(vNorMi[0]);
    normalizeVector(vNorMi[1]);
    normalizeVector(vNorMi[2]);
    vTexPosMi[0] = (vTexPos1 + vTexPos2) / 2.f;
    vTexPosMi[1] = (vTexPos2 + vTexPos3) / 2.f;
    vTexPosMi[2] = (vTexPos3 + vTexPos1) / 2.f;

    switch(displayColor) {
    case None: glColor4fv(white); break;
    case Red: glColor4fv(red); break;
    case Green: glColor4fv(green); break;
    }

    renderSubFaceFlatPhong(vPos1, vPosMi[0], vPosMi[2],
                           vNor1, vNorMi[0], vNorMi[2],
                           vTexPos1, vTexPosMi[0], vTexPosMi[2]);
    renderSubFaceFlatPhong(vPosMi[0], vPos2, vPosMi[1],
                           vNorMi[0], vNor2, vNorMi[1],
                           vTexPosMi[0], vTexPos2, vTexPosMi[1]);
    renderSubFaceFlatPhong(vPos3, vPosMi[2], vPosMi[1],
                           vNor3, vNorMi[2], vNorMi[1],
                           vTexPos3, vTexPosMi[2], vTexPosMi[1]);
    renderSubFaceFlatPhong(vPosMi[0], vPosMi[1], vPosMi[2],
                           vNorMi[0], vNorMi[1], vNorMi[2],
                           vTexPosMi[0], vTexPosMi[1], vTexPosMi[2]);
}

void ModelManager::renderSubFaceFlatPhong(aiVector3D vPos1, aiVector3D vPos2, aiVector3D vPos3,
                                          aiVector3D vNor1, aiVector3D vNor2, aiVector3D vNor3,
                                          aiVector3D vTexPos1, aiVector3D vTexPos2, aiVector3D vTexPos3) {
    glBegin(GL_TRIANGLES);
    glTexCoord2f(vTexPos1.x, 1 - vTexPos1.y);
    glNormal3f(vNor1.x, vNor1.y, vNor1.z);         //各个点自身的法向量
    glVertex3f(vPos1.x, vPos1.y, vPos1.z);

    glTexCoord2f(vTexPos2.x, 1 - vTexPos2.y);
    glNormal3f(vNor2.x, vNor2.y, vNor2.z);         //各个点自身的法向量
    glVertex3f(vPos2.x, vPos2.y, vPos2.z);

    glTexCoord2f(vTexPos3.x, 1 - vTexPos3.y);
    glNormal3f(vNor3.x, vNor3.y, vNor3.z);         //各个点自身的法向量
    glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    glEnd();
}

// 修改后的renderTriangleFaceFlat方法
void ModelManager::renderTriangleFaceFlat(aiVector3D vPos1, aiVector3D vPos2, aiVector3D vPos3,
                                          aiVector3D vNor1, aiVector3D vNor2, aiVector3D vNor3,
                                          aiVector3D vTexPos1, aiVector3D vTexPos2, aiVector3D vTexPos3) {
    // 设置颜色
    switch(displayColor) {
    case None: glColor4fv(white); break;
    case Red: glColor4fv(red); break;
    case Green: glColor4fv(green); break;
    }

    glBegin(GL_TRIANGLES);

    // 根据着色模式设置法向量
    if (shadingMode == FlatS) {
        // 平面着色：为整个三角形使用一个法向量
        glNormal3f(vNor1.x, vNor1.y, vNor1.z);

        glTexCoord2f(vTexPos1.x, 1 - vTexPos1.y);
        glVertex3f(vPos1.x, vPos1.y, vPos1.z);

        glTexCoord2f(vTexPos2.x, 1 - vTexPos2.y);
        glVertex3f(vPos2.x, vPos2.y, vPos2.z);

        glTexCoord2f(vTexPos3.x, 1 - vTexPos3.y);
        glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    }
    else if (shadingMode == Gouraud || shadingMode == Phong) {
        // Gouraud着色：每个顶点使用自己的法向量
        glNormal3f(vNor1.x, vNor1.y, vNor1.z);
        glTexCoord2f(vTexPos1.x, 1 - vTexPos1.y);
        glVertex3f(vPos1.x, vPos1.y, vPos1.z);

        glNormal3f(vNor2.x, vNor2.y, vNor2.z);
        glTexCoord2f(vTexPos2.x, 1 - vTexPos2.y);
        glVertex3f(vPos2.x, vPos2.y, vPos2.z);

        glNormal3f(vNor3.x, vNor3.y, vNor3.z);
        glTexCoord2f(vTexPos3.x, 1 - vTexPos3.y);
        glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    }

    glEnd();
}

void ModelManager::renderTriangleFaceWireframe(aiVector3D vPos1, aiVector3D vPos2, aiVector3D vPos3) {
    glColor4fv(black);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    glVertex3f(vPos1.x, vPos1.y, vPos1.z);
    glVertex3f(vPos2.x, vPos2.y, vPos2.z);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    glVertex3f(vPos2.x, vPos2.y, vPos2.z);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(vPos1.x, vPos1.y, vPos1.z);
    glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    glEnd();
}

// 修改后的renderPolygonFaceFlat方法
void ModelManager::renderPolygonFaceFlat(const struct aiMesh* mesh, GLenum face_mode, const struct aiFace* face) {
    // 确保有足够的顶点
    if (face->mNumIndices < 3) {
        cout << "Warning: Polygon face has less than 3 vertices!" << endl;
        return;
    }

    // 设置颜色
    switch(displayColor) {
    case None: glColor4fv(white); break;
    case Red: glColor4fv(red); break;
    case Green: glColor4fv(green); break;
    }

    int firstIndex = face->mIndices[0];

    glBegin(face_mode);

    for (int i = 0; i < face->mNumIndices; i++) {
        int index = face->mIndices[i];

        // 检查索引是否有效
        if (index >= mesh->mNumVertices) {
            cout << "Error: Invalid vertex index " << index << endl;
            continue;
        }

        // 设置纹理坐标
        if (mesh->HasTextureCoords(0)) {
            glTexCoord2f(mesh->mTextureCoords[0][index].x,
                         1 - mesh->mTextureCoords[0][index].y);
        }

        // 设置法向量
        if (mesh->mNormals != NULL) {
            if (shadingMode == FlatS) {
                glNormal3fv(&mesh->mNormals[firstIndex].x);  // 使用第一个点的法向量
            } else if (shadingMode == Gouraud || shadingMode == Phong) {
                glNormal3fv(&mesh->mNormals[index].x);       // 使用各个点的法向量
            }
        }

        // 设置顶点
        glVertex3fv(&(mesh->mVertices[index].x));
    }

    glEnd();
}

void ModelManager::renderPolygonFaceWireframe(const struct aiMesh* mesh, const struct aiFace* face) {
    //遍历所有顶点，每两个顶点之间连线
    for (int i = 0; i < face->mNumIndices; i++) {
        int indexA = face->mIndices[i];
        int indexB = face->mIndices[i == face->mNumIndices - 1 ? 0 : i + 1];

        glColor4fv(black);
        glLineWidth(1.0f);
        glBegin(GL_LINES);

        glVertex3fv(&(mesh->mVertices[indexA].x));
        glVertex3fv(&(mesh->mVertices[indexB].x));

        glEnd();
    }
}

void color4_to_float4(const aiColor4D *c, float f[4]) {
    f[0] = c->r;
    f[1] = c->g;
    f[2] = c->b;
    f[3] = c->a;
}

void set_float4(float f[4], float a, float b, float c, float d) {
    f[0] = a;
    f[1] = b;
    f[2] = c;
    f[3] = d;
}

// 修改后的applyMaterial方法
void ModelManager::applyMaterial(const aiMaterial *mtl) {
    float c[4];
    int ret1, ret2;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    aiColor4D emission;
    float shininess, strength;
    int two_sided;
    unsigned int max;

    int texIndex = 0;
    aiString texPath;

    // 处理纹理
    if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath)) {
        if (textureState == TextureOn) {
            // 绑定纹理
            unsigned int texId = *(textureIdMap[texPath.data]);
            glBindTexture(GL_TEXTURE_2D, texId);
            glEnable(GL_TEXTURE_2D);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // 设置材质属性
    set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        color4_to_float4(&diffuse, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
        color4_to_float4(&specular, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
        color4_to_float4(&ambient, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
        color4_to_float4(&emission, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    // 设置光泽度
    max = 1;
    ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    if (ret1 == AI_SUCCESS) {
        max = 1;
        ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
        if (ret2 == AI_SUCCESS)
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
        else
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    } else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
        set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
    }

    // 设置填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // 背面剔除设置 - 修改为更保守的设置
    max = 1;
    if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided) {
        glDisable(GL_CULL_FACE);
    } else {
        // 暂时禁用背面剔除以确保所有面都能显示
        glDisable(GL_CULL_FACE);
        // 如果确定模型正确，可以启用：glEnable(GL_CULL_FACE);
    }
}


// 新增：处理线段面的方法
void ModelManager::renderLineFace(const struct aiMesh* mesh, const struct aiFace* face) {
    if (face->mNumIndices != 2) return;

    glColor4fv(black);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    for (int i = 0; i < 2; i++) {
        int index = face->mIndices[i];
        glVertex3fv(&(mesh->mVertices[index].x));
    }
    glEnd();
}

// 新增：处理点面的方法
void ModelManager::renderPointFace(const struct aiMesh* mesh, const struct aiFace* face) {
    if (face->mNumIndices != 1) return;

    glColor4fv(white);
    glPointSize(5.0f);

    glBegin(GL_POINTS);
    int index = face->mIndices[0];
    glVertex3fv(&(mesh->mVertices[index].x));
    glEnd();
}

// 新增：使用着色器的渲染方法
void ModelManager::renderWithShader(GLuint posAttr, GLuint norAttr) {
    if (!scene || !scene->mRootNode) {
        return;
    }
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    recursiveRenderWithShader(scene, scene->mRootNode, posAttr, norAttr);
}

// 新增：着色器递归渲染
void ModelManager::recursiveRenderWithShader(const struct aiScene *sc, const struct aiNode* nd, GLuint posAttr, GLuint norAttr) {
    if (!sc || !nd) return;
    
    aiMatrix4x4 mTrans = nd->mTransformation;
    mTrans.Transpose();
    glPushMatrix();
    glMultMatrixf((float*)&mTrans);

    // 对当前节点，遍历该节点的所有mMeshes
    for (unsigned int m = 0; m < nd->mNumMeshes; m++) {
        const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];

        // 对当前的mesh，遍历所有面face
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const struct aiFace* face = &(mesh->mFaces[f]);
            processFaceWithShader(mesh, face, posAttr, norAttr);
        }
    }

    // 递归绘制其他子节点
    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        recursiveRenderWithShader(sc, nd->mChildren[n], posAttr, norAttr);
    }

    glPopMatrix();
}

// 新增：着色器面处理
void ModelManager::processFaceWithShader(const struct aiMesh* mesh, const struct aiFace* face, GLuint posAttr, GLuint norAttr) {
    if (face->mNumIndices != 3) return; // 只处理三角形
    
    // 准备顶点数据
    GLfloat vertices[9];  // 3个顶点 * 3个坐标
    GLfloat normals[9];   // 3个法向量 * 3个坐标
    
    for (int i = 0; i < 3; i++) {
        int index = face->mIndices[i];
        
        // 检查索引是否有效
        if (index >= mesh->mNumVertices) {
            return;
        }
        
        // 填充顶点数据
        vertices[i*3 + 0] = mesh->mVertices[index].x;
        vertices[i*3 + 1] = mesh->mVertices[index].y;
        vertices[i*3 + 2] = mesh->mVertices[index].z;
        
        // 填充法向量数据
        if (mesh->mNormals != NULL) {
            normals[i*3 + 0] = mesh->mNormals[index].x;
            normals[i*3 + 1] = mesh->mNormals[index].y;
            normals[i*3 + 2] = mesh->mNormals[index].z;
        } else {
            // 如果没有法向量，计算面法向量
            if (i == 0) {
                aiVector3D v0 = mesh->mVertices[face->mIndices[0]];
                aiVector3D v1 = mesh->mVertices[face->mIndices[1]];
                aiVector3D v2 = mesh->mVertices[face->mIndices[2]];
                
                aiVector3D edge1 = v1 - v0;
                aiVector3D edge2 = v2 - v0;
                aiVector3D normal = edge1 ^ edge2;
                normalizeVector(normal);
                
                for (int j = 0; j < 3; j++) {
                    normals[j*3 + 0] = normal.x;
                    normals[j*3 + 1] = normal.y;
                    normals[j*3 + 2] = normal.z;
                }
            }
        }
    }
    
    // 绑定顶点属性
    glEnableVertexAttribArray(posAttr);
    glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    
    glEnableVertexAttribArray(norAttr);
    glVertexAttribPointer(norAttr, 3, GL_FLOAT, GL_FALSE, 0, normals);
    
    // 绘制三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // 禁用顶点属性
    glDisableVertexAttribArray(posAttr);
    glDisableVertexAttribArray(norAttr);
}
