---
inclusion: fileMatch
fileMatchPattern: "**/*wall*/**/*"
---

# 墙面提取模块开发标准

## 代码组织规范

### 命名空间使用
- 所有墙面提取相关代码必须放在 `WallExtraction` 命名空间中
- 避免污染全局命名空间和现有系统代码

### 文件命名约定
- 头文件使用小写字母和下划线：`wall_extraction_manager.h`
- 源文件对应：`wall_extraction_manager.cpp`
- 类名使用PascalCase：`WallExtractionManager`
- 方法名使用camelCase：`extractWallSegments()`

### 目录结构
```
src/wall_extraction/
├── core/                    # 核心算法
│   ├── wall_fitting.h/cpp
│   ├── ransac_algorithm.h/cpp
│   └── geometry_utils.h/cpp
├── ui/                      # 用户界面
│   ├── line_drawing_tool.h/cpp
│   ├── wall_extraction_toolbar.h/cpp
│   └── properties_panel.h/cpp
├── io/                      # 输入输出
│   ├── las_reader.h/cpp
│   ├── dxf_exporter.h/cpp
│   └── wireframe_generator.h/cpp
└── utils/                   # 工具类
    ├── point_cloud_processor.h/cpp
    └── spatial_index.h/cpp
```

## 编程规范

### 错误处理
- 使用自定义异常类 `WallExtractionException`
- 所有可能失败的操作必须有异常处理
- 提供详细的错误信息和上下文

```cpp
try {
    auto walls = extractWalls(pointCloud, userLines);
} catch (const WallExtractionException& e) {
    qDebug() << "Wall extraction failed:" << e.what();
    showErrorMessage(e.getDetailedMessage());
}
```

### 内存管理
- 优先使用智能指针 `std::unique_ptr`, `std::shared_ptr`
- 大型数据结构使用移动语义
- 及时释放不需要的点云数据

### 性能要求
- 所有算法必须支持大数据量处理（>100万点）
- 使用多线程处理耗时操作
- 实现进度回调机制

```cpp
class WallFittingAlgorithm {
public:
    void setProgressCallback(std::function<void(int)> callback);
    WallSegments extractWalls(const PointCloud& cloud);
};
```

## 测试要求

### 单元测试
- 每个算法类必须有对应的单元测试
- 测试覆盖率要求 > 80%
- 使用Qt Test框架

### 性能测试
- 提供性能基准测试
- 记录关键操作的执行时间
- 内存使用监控

### 数据测试
- 准备多种格式的测试数据
- 包含边界情况和异常数据
- 验证算法的鲁棒性

## 用户界面规范

### 交互设计
- 遵循现有系统的UI风格
- 提供清晰的操作反馈
- 支持撤销/重做操作

### 工具栏设计
- 图标使用统一的风格
- 按钮数量不超过8个
- 提供工具提示说明

### 状态管理
- 明确的操作模式切换
- 状态栏显示当前操作提示
- 进度条显示长时间操作

## 算法实现标准

### RANSAC算法
- 参数可配置（阈值、迭代次数等）
- 支持多种距离度量
- 提供置信度评估

### 几何处理
- 使用稳定的数值算法
- 处理退化情况（共线点等）
- 保持几何精度

### 优化策略
- 使用空间索引加速邻域搜索
- 实现多尺度处理
- 支持渐进式结果更新

## 配置管理

### 参数配置
```json
{
  "wallExtraction": {
    "ransac": {
      "threshold": 0.1,
      "maxIterations": 1000,
      "minInliers": 100
    },
    "geometry": {
      "minWallLength": 1.0,
      "maxWallGap": 0.5,
      "angleThreshold": 5.0
    },
    "visualization": {
      "lineColor": "#0066CC",
      "wallColor": "#FF6600",
      "lineWidth": 2.0
    }
  }
}
```

### 预设模板
- 提供不同建筑类型的预设参数
- 支持用户自定义参数保存
- 参数验证和范围检查

## 文档要求

### 代码文档
- 所有公共接口必须有详细注释
- 使用Doxygen格式
- 包含使用示例

### API文档
- 提供完整的API参考
- 包含参数说明和返回值
- 示例代码和最佳实践

### 用户文档
- 操作步骤说明
- 常见问题解答
- 故障排除指南

## 集成要求

### 与现有系统集成
- 不修改现有类的公共接口
- 通过信号槽机制通信
- 配置独立管理

### 扩展性设计
- 支持插件式算法扩展
- 提供标准化接口
- 版本兼容性考虑

## 质量保证

### 代码审查
- 所有代码必须经过审查
- 遵循编码规范
- 性能和安全检查

### 持续集成
- 自动化构建和测试
- 代码质量检查
- 性能回归测试

这些标准确保墙面提取模块的开发质量和与现有系统的良好集成。
