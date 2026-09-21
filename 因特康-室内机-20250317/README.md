# two-wire-indoor


#### 介绍
博科两线室内机

#### 软件架构
软件架构说明


#### 安装教程

1.  xxxx
2.  xxxx
3.  xxxx

#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)

## 20250117
1.优化wifi连接卡顿问题，具体原因调用system时未后台执行，导致需要等待命令结束才会回到主进程中，导致卡顿，且直接system调用字符串常亮可能导致命令后面的&未执行，导致阻塞
2.优化wifi配置文件wpa_supplicant丢失问题，etc/config目录下的wpa_supplicant文件只做拷贝覆盖，不直接删除，间接避免删除后拷贝新的配置文件失败，导致文件丢失从而wifi无法搜索
3.重构wifi_connection_status_sucess函数，添加获取信息超时退出机制，及在msg_task任务检测wifi连接状态时使用wifi_connection_status_sucess函数
4.实现升级门口机时呼叫不应答
5.修复涂鸦CCTV切换卡住，原因:切换cctv过程中线程未来得及关闭，导致新开的码流失败返回；修改方式:线程做阻塞返回，不适用分离接口
6.用户数据处理线程添加看门狗
7.修复涂鸦OTA升级下载后未执行升级操作及升级过程中关闭看门狗
8.修复涂鸦ID切换过程中重启，涂鸦ID切换前关闭看门狗，切换完成重新打开
9.优化涂鸦id切换卡顿过长，将fopen涂鸦id缓存文件由a打开方式更换成w;xls表格库在识别xls文件时可能存在解析错误行数，导致创建过多无效的数据，添加后续切换解析ini文件负担；解决方案：判断每行数据是否有效，若存在无效数据则不更新至ini配置文件;优化涂鸦id切换tuya_xls_head_valid_check接口，判断第一次检测到该行中uuid或key数据为空时，修改有效行数，避免解析出错误的行数导致切换id时返回文件错误等异常；及在tuya_uuid_and_key_swtch函数中添加对tuya_xls_head_valid_check函数的调用
10.删除standby_task函数下息屏时待机黑屏控件创建代码，新添standby_black_screen函数接口，调用该接口时，根据对应参数状态，刷新黑屏背景，控制背光；在layout_standby.c文件中替换backlight_open调用
11.修复视频播放界面快速切换后导致播放异常，将播放音视频线程改成阻塞释放
12.修复SD卡分区内存不足时，清除内存操作异常，导致循环进入清除操作，占用大量CPU资源，其原因是代码函数SD_card_space_clear在做清除数据类型判断时，未添加else，导致在某个判断成立后，指定删除的类型未退出判断代码，进行下一步的其他类型选择；解决方案：在每次判断选择清除数据的类型后添加else，避免if过后继续执行下一段代码
13.修复关闭移动侦测预览状态下，移动侦测录像时进入移动侦测录制文件列表界面，存在偶尔丢失当前录制文件问题，其根源是不同线程对文件操作函数接口的调用，同时访问操作一个缓存数组，导致冲突，在其数据修改后，因其他线程操作导致旧数据重新覆盖掉新数据，因此丢失当前的录制文件；目前修改方式：在进行扫描SD卡文件时，添加判断是否进行视频录制中，当进行录制则等待录制完毕，并优化相关UI显示逻辑 
14.室内机cctv通道切换显示效果异常，cctv分辨率与其他通道不一致且未重启解码导致，每次切换重启解码
15.修复软重启时画面显示异常
16.视频解码修改为join阻塞关闭，避免detach分离关闭未及时释放导致下次开启解码异常

## 20250214
1.修复监控状态下自动退出后，待机界面时钟显示异常，根源在于监控定时器做页面切换后未return出定时器，定时器继续执行错误逻辑导致。

## 20250301
1.添加wifi扫描热点异常处理，添加至用户数据进程检测