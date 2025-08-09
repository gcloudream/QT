# 修复方案与实施说明

## 问题1：复杂点云下线段标注位置偏移

根因：
- 点击坐标到渲染视口的映射没有考虑QLabel中pixmap的等比缩放与居中偏移(letterbox)
- 坐标转换过程中，视口尺寸与视图边界没有和渲染器使用的值保持强一致

修复：
1) 在渲染完成设置QLabel的pixmap时，记录缩放后尺寸和居中偏移：
   - 变量：m_lastScaledPixmapSize、m_lastPixmapTopLeft
2) 新增坐标映射方法：
   - mapLabelPosToViewport(labelLocal, viewportPos)
   - 利用上述尺寸和偏移，把label内点击位置映射到渲染视口坐标系
3) 鼠标事件(mousePress/Move/Release)中不再用"event->pos()-label->topLeft"，而是用map后的viewport坐标传给LineDrawingTool。
4) 渲染尺寸/布局变化时重置映射缓存以避免失真。

额外：保留基于点云实际Bounds的accurateWorldToScreen/accurateScreenToWorld用于渲染叠加时的坐标一致性。

## 问题2：清除功能需要点两次

根因：
- clearPointCloud只清数据没有清空QLabel的pixmap与映射缓存
- updateRenderView当点云为空时未强制清空显示，后续定时器或其它更新又触发渲染尝试

修复：
1) clearPointCloud中：清空pixmap、文字、统计，并调用clearLineSegmentData
2) updateRenderView：当点云为空时，立即清空QLabel pixmap、文字并重置m_lastScaledPixmapSize/m_lastPixmapTopLeft
3) 确认load/生成数据路径先清标注，再渲染

## 验证
- 多建筑物大范围点云：点击某建筑边缘和中心多处，验证线段落点与点击吻合
- 调整窗口大小和工具栏展开/收起后再点击，坐标仍然正确
- 一次点击“清除数据”后，点云与线段即时全部清空；再次点击不应有新的变化

## 后续
- 如仍有细微偏移，继续把LineDrawingTool中的screenToWorld也切换为使用mapLabelPosToViewport后的视口坐标 + m_currentViewBounds 来反解世界坐标。

