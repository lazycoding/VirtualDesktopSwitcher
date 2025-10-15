# 鼠标侧键虚拟桌面切换器架构设计

## 1. 总体架构
采用分层架构设计，分为以下核心模块：
- 输入监听层 (MouseHook)
- 业务逻辑层 (GestureAnalyzer) 
- 桌面控制层 (DesktopManager)
- 用户界面层 (OverlayUI, TrayIcon)
- 配置管理层 (Settings)

## 2. 模块详细设计

### 2.1 输入监听层
- **MouseHook模块** (Singleton模式)
  - 职责：使用Windows Hook API捕获鼠标侧键事件(XBUTTON1)和移动轨迹
  - 技术：SetWindowsHookEx(WH_MOUSE_LL)实现低级鼠标钩子
  - 特性：支持多个回调函数注册，事件分发机制
  - 输出：标准化的鼠标事件数据(WPARAM, LPARAM)

### 2.2 业务逻辑层
- **GestureAnalyzer模块**
  - 职责：分析鼠标滑动轨迹，检测手势方向
  - 算法：基于移动向量分析，最小滑动距离阈值(50像素)
  - 输出：Direction枚举(Left/Right/Up/Down/None)
  - 特性：支持手势进度状态跟踪

### 2.3 桌面控制层
- **DesktopManager模块**
  - 职责：虚拟桌面切换操作
  - 技术：模拟Win+Ctrl+左右方向键组合
  - 功能：单方向桌面切换(true=右, false=左)
  - 特性：遵循Rule of Five原则，禁用拷贝和移动操作

### 2.4 用户界面层
- **OverlayUI模块**
  - 职责：使用Direct2D渲染手势轨迹可视化
  - 技术：Direct2D工厂模式，HWND渲染目标
  - 特性：可配置颜色和样式，支持实时更新

- **TrayIcon模块**
  - 职责：系统托盘图标管理和通知显示
  - 技术：Shell API (NOTIFYICONDATAW)，自定义窗口过程
  - 功能：动态菜单项管理，气泡通知显示
  - 特性：支持回调函数注册，资源自动清理

### 2.5 配置管理层
- **Settings模块**
  - 职责：JSON配置文件持久化管理
  - 存储：config.json文件，使用nlohmann/json库
  - 功能：手势灵敏度配置，覆盖层颜色设置
  - 特性：热重载支持，类型安全的数据访问

## 3. 核心交互流程
```mermaid
sequenceDiagram
    participant MH as MouseHook(Singleton)
    participant GA as GestureAnalyzer
    participant DM as DesktopManager
    participant OV as OverlayUI
    participant TI as TrayIcon
    participant ST as Settings
    
    MH->>GA: XBUTTONDOWN事件(开始手势)
    GA->>GA: 清空位置记录，添加起始点
    MH->>GA: MOUSEMOVE事件(记录轨迹)
    GA->>GA: 持续添加位置点
    MH->>GA: XBUTTONUP事件(结束手势)
    GA->>GA: 分析手势方向
    GA->>DM: 切换指令(Left/Right)
    DM->>TI: 桌面切换完成通知
    TI->>用户: 显示气泡通知
    
    Note over ST: 配置参数流向所有模块
    ST->>GA: 手势灵敏度设置
    ST->>OV: 覆盖层颜色配置
```
## 4. 技术栈特性
- **C++标准**: 遵循C++17标准，使用现代C++特性
- **设计模式**: Singleton、Observer、Factory等模式应用
- **内存管理**: RAII原则，智能指针，自动资源清理
- **异常处理**: 结构化异常处理与错误码结合
- **跨平台准备**: Windows API抽象，便于未来扩展