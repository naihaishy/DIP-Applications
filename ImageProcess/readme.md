数字图像处理专题
1.图像去雾
打开程序 读取图像 在“去雾菜单”栏里点击相应条目。
图像去雾总控制面板提供了用户自定义相关参数的交互界面。
本程序基于Qt开源跨平台框架开发，图像处理等基本操作使用OpenCV 3.4,核心代码根据何凯文的论文提供的原理进行编写。



环境说明 
1.编译器 Qt Creatot 4.5 
2.框架 Qt5.10 Opencv 3.4.0(包括contrib) 4.5 
3.相关下载 
Qt  5.10  https://download.qt.io/official_releases/qt/
OpenCV 3.4.0(Win 10x64 vs2017编译完成) https://blog.zhfsky.com/2018/01/16/opencv-3-4-0-contrib-download.html
4.说明：visual studio2017下有Qt的官方插件，方便编译和调试Qt应用，但前提是安装了Qt Creator的QT核心库

编译指南

1.将下载完成的opencv_xx 重命名为opencv 放入代码项目的Libs文件夹下,应该是类似这样的结构
/Libs/opencv/bin

2.打开.pro文件

3.配置好编译器 MSVC17 Release

</div>

