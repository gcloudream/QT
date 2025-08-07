#include <iostream>
#include <vector>
#include <QVector3D>
#include <QDebug>
#include <QCoreApplication>
#include "PCDReader.h"
#include "MinBoundingBox.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "=== PCD文件修复测试 ===" << std::endl;
    
    // 测试PCD文件路径
    QString pcdFile = "C:/Users/18438/Desktop/Resources/109sub_11_30_empty.pcd";
    
    // 创建PCD读取器
    PCDReader reader;
    
    std::cout << "开始读取PCD文件..." << std::endl;
    std::vector<QVector3D> cloud = reader.ReadVec3PointCloudPCD(pcdFile);
    
    if (cloud.empty()) {
        std::cout << "❌ 读取失败或点云为空" << std::endl;
        return -1;
    }
    
    std::cout << "✅ 成功读取 " << cloud.size() << " 个点" << std::endl;
    
    // 计算坐标范围
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    
    int validCount = 0;
    for (const auto& p : cloud) {
        if (std::isfinite(p.x()) && std::isfinite(p.y()) && std::isfinite(p.z())) {
            minX = std::min(minX, p.x());
            maxX = std::max(maxX, p.x());
            minY = std::min(minY, p.y());
            maxY = std::max(maxY, p.y());
            minZ = std::min(minZ, p.z());
            maxZ = std::max(maxZ, p.z());
            validCount++;
        }
    }
    
    std::cout << "📊 坐标统计：" << std::endl;
    std::cout << "   有效点数: " << validCount << " / " << cloud.size() << std::endl;
    std::cout << "   X范围: [" << minX << ", " << maxX << "] (跨度: " << (maxX - minX) << ")" << std::endl;
    std::cout << "   Y范围: [" << minY << ", " << maxY << "] (跨度: " << (maxY - minY) << ")" << std::endl;
    std::cout << "   Z范围: [" << minZ << ", " << maxZ << "] (跨度: " << (maxZ - minZ) << ")" << std::endl;
    
    // 测试包围盒计算
    MinBoundingBox bbox;
    bool bboxValid = bbox.calculateMinBoundingBox(cloud);
    
    std::cout << "📦 包围盒测试：" << std::endl;
    std::cout << "   计算结果: " << (bboxValid ? "成功" : "失败") << std::endl;
    
    if (bboxValid) {
        QVector3D minPoint = bbox.getMinPoint();
        QVector3D maxPoint = bbox.getMaxPoint();
        QVector3D center = bbox.getCenterPoint();
        
        std::cout << "   最小点: (" << minPoint.x() << ", " << minPoint.y() << ", " << minPoint.z() << ")" << std::endl;
        std::cout << "   最大点: (" << maxPoint.x() << ", " << maxPoint.y() << ", " << maxPoint.z() << ")" << std::endl;
        std::cout << "   中心点: (" << center.x() << ", " << center.y() << ", " << center.z() << ")" << std::endl;
        std::cout << "   尺寸: " << bbox.width() << " × " << bbox.height() << " × " << bbox.depth() << std::endl;
    }
    
    // 输出前10个点的坐标
    std::cout << "🔍 前10个点的坐标：" << std::endl;
    for (int i = 0; i < std::min(10, (int)cloud.size()); ++i) {
        const auto& p = cloud[i];
        std::cout << "   点" << i << ": (" << p.x() << ", " << p.y() << ", " << p.z() << ")" << std::endl;
    }
    
    std::cout << "=== 测试完成 ===" << std::endl;
    
    return 0;
}
