
# CrystalMelody
## Full Dynamic Theme
### Filter color follow thumbnails
## Only one application in flight

## Sounds fall off when close window

# CrystalExplorer
## File Explorer use IndexableTree
## Custom Adaptable Layouts


// BUGFIX
- 修复专辑图像问题: 扩展存读 crystalPackage 的动作: 打包时提取专辑图片作为隐藏文件存入(也可用户指定添加), 读取时读取此隐藏图片作为专辑封面
- 打开 .MKV 时卡死 --10 bit视频和FLV视频的不支持
- [Solved] 拉动进度条无法回到最初播放位置
- 暂停时拉动进度条到底会导致且换失败且死循环	(不稳定复现)
-  Shuffle 会在小范围循环

## DEV
- 设计 For Explorer UI.
- ogg 格式支持
- flv 格式支持
- 参数启动限制多开
- Dynamic Theme 设计
- 左右方向键的重载
- 高斯效果改为边缘扩张
- 字体颜色自动改变
- 设计 LowFocus 为滑动条 (WindowOpacity)