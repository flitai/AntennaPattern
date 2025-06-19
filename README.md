## 代码功能

### 主要功能

该代码实现了一个**天线方向图计算库**，用于计算不同类型天线在各个方向上的增益特性。主要功能包括：

1. **多种天线方向图模型支持**
2. **天线增益计算**
3. **方向图文件解析**
4. **插值和查表功能**

### 代码特点和技术要点

### 1. **设计模式**

- 使用**工厂模式**通过文件扩展名自动选择合适的解析器
- 采用**策略模式**，不同天线类型有不同的增益计算算法
- **模板特化**用于不同数据类型的插值表

### 2. **数值计算技术**

- **双线性插值**用于表格数据的平滑查找
- **角度归一化**处理（`angFixPI`, `angFix2PI`等）
- **分贝和线性值转换**（`dB2Linear`, `linear2dB`）
- **复数运算**支持单脉冲天线的幅度相位计算

### 3. **工程实用性**

- **多种文件格式支持**，兼容不同仿真软件
- **频率相关性**处理，支持宽带天线建模
- **极化特性**完整支持
- **错误处理**和边界条件检查完善

### 4. **性能优化**

- **缓存机制**：存储上次计算的波束宽度，避免重复计算min/max增益
- **查找表**：预计算数据用于快速插值
- **内存管理**：合理的动态内存分配和释放

这个库主要用于**雷达系统仿真**、**通信链路分析**、**电磁兼容性评估**等应用场景，是电磁建模仿真系统的核心组件。



## 支持的天线方向图类型

### 1. 算法型天线方向图
这些是基于数学公式计算的天线方向图：

#### AntennaPatternGauss (高斯型)
- **用途**: 模拟高斯分布的天线方向图
- **特点**: 主瓣呈高斯分布，旁瓣较低
- **公式**: 基于高斯函数 exp(-0.5 * (ln2 / sin²(vbw/2)) * sin²(elev))

#### AntennaPatternCscSq (余割平方型)
- **用途**: 适用于扇形波束天线
- **特点**: 在仰角方向有特殊的余割平方特性
- **应用**: 常用于雷达天线设计

#### AntennaPatternSinXX (sinc函数型)
- **用途**: 经典的sinc函数方向图
- **特点**: 具有明显的主瓣和旁瓣结构
- **公式**: 基于 sin(x)/x 函数

#### AntennaPatternPedestal (基座型)
- **用途**: 具有平台特性的方向图
- **特点**: 在一定角度范围内保持较平的增益

#### AntennaPatternOmni (全向型)
- **用途**: 全向辐射天线
- **特点**: 各个方向增益相同

### 2. 表格型天线方向图
这些从文件中读取实测或仿真数据：

#### AntennaPatternTable (.pat文件)
- **数据格式**: 标准表格格式
- **支持**: 不同对称性 (1折、2折、4折对称)
- **内容**: 方位角和仰角的增益数据

#### AntennaPatternRelativeTable (.rel文件)
- **特点**: 相对增益数据
- **格式**: 方位角和仰角分别存储

#### AntennaPatternCRUISE (.cru文件)
- **来源**: CRUISE建模软件
- **特点**: 包含频率相关的增益数据
- **存储**: 电压增益(需平方转换为功率增益)

#### AntennaPatternMonopulse (.mon文件)
- **用途**: 单脉冲雷达天线
- **特点**: 包含和通道(sum)和差通道(diff)
- **数据**: 复数形式(幅度和相位)

#### AntennaPatternBiLinear (.bil文件)
- **插值**: 双线性插值
- **特点**: 支持频率选择

#### AntennaPatternNSMA (.nsm文件)
- **标准**: 美国国家频谱管理协会格式
- **极化**: 支持HH、HV、VV、VH极化
- **数据**: 包含方位角和仰角数据

#### AntennaPatternEZNEC (.ezn文件)
- **来源**: EZNEC电磁仿真软件
- **特点**: 支持垂直、水平、总增益
- **角度**: 支持两种角度约定(罗盘/数学)

#### AntennaPatternXFDTD (.xfd文件)
- **来源**: XFDTD时域有限差分软件
- **格式**: UAN格式
- **数据**: 包含θ和φ分量的增益和相位



## 核心数据结构

### AntennaGainParameters 类

**作用**: 存储天线增益计算所需的所有参数

#### 输入参数

```cpp
float azim_;           // 相对方位角 (弧度)
float elev_;           // 相对仰角 (弧度)  
PolarityType polarity_; // 极化类型
float hbw_;            // 水平波束宽度 (弧度)
float vbw_;            // 垂直波束宽度 (弧度)
float refGain_;        // 参考增益 (dB)
float firstLobe_;      // 第一旁瓣电平 (dB)
float backLobe_;       // 后瓣电平 (dB)
double freq_;          // 频率 (Hz)
bool weighting_;       // 是否使用加权平均
bool delta_;           // 单脉冲天线: true=差通道, false=和通道
```

### AntennaPattern 抽象基类

**作用**: 所有天线方向图类的基类

#### 核心方法

```cpp
// 计算指定参数下的天线增益
virtual float gain(const AntennaGainParameters &params) = 0;

// 获取方向图的最小和最大增益
virtual void minMaxGain(float *min, float *max, 
                       const AntennaGainParameters &params) = 0;

// 获取天线方向图类型
virtual AntennaPatternType type() const = 0;
```

#### 通用属性

```cpp
bool valid_;           // 数据有效性标志
float minGain_;        // 最小增益值 (dB)
float maxGain_;        // 最大增益值 (dB)
PolarityType polarity_; // 天线极化类型
std::string filename_; // 数据文件名
```

### 查找表相关类型定义

```cpp
// 复数天线方向图查找表 (用于单脉冲天线)
typedef InterpTable<std::complex<double>> SymmetricAntennaPattern;

// 实数增益查找表 (用于一般天线)
typedef InterpTable<double> SymmetricGainAntPattern;

// 浮点增益查找表
typedef InterpTable<float> GainData;
```

### 工厂函数

```cpp
// 根据文件扩展名自动创建相应的天线方向图对象
AntennaPattern* loadPatternFile(const std::string& filename, float freqMHz);

// 字符串到枚举类型转换
AntennaPatternType antennaPatternType(const std::string& antPatStr);
std::string antennaPatternTypeString(AntennaPatternType antPatType);
```



## 输入输出

### 主要输入

#### 1. 几何参数

- **方位角 (azim_)**: 目标相对于天线的水平角度 (弧度)
- **仰角 (elev_)**: 目标相对于天线的垂直角度 (弧度)
- **波束宽度**: 水平和垂直方向的3dB波束宽度 (弧度)

#### 2. 电磁参数  

- **频率 (freq_)**: 工作频率 (Hz)
- **极化类型**: 
  - POLARITY_HORIZONTAL (水平极化)
  - POLARITY_VERTICAL (垂直极化)  
  - POLARITY_RIGHTCIRC (右旋圆极化)
  - POLARITY_LEFTCIRC (左旋圆极化)
  - POLARITY_HORZVERT (水平-垂直极化)
  - POLARITY_VERTHORZ (垂直-水平极化)

#### 3. 增益参数

- **参考增益 (refGain_)**: 天线最大增益 (dB)
- **旁瓣电平**: 第一旁瓣和后瓣的相对电平 (dB)

#### 4. 文件输入格式示例

##### .pat 文件格式 (AntennaPatternTable)

```
// 注释行
1 2        // 类型(0/1) 对称性(1/2/4)  
36         // 数据点数
-180.0 -23.2   // 角度(度) 增益(dB)
-175.0 -22.8
...
```

##### .rel 文件格式 (AntennaPatternRelativeTable)  

```
72 36      // 方位角数据点数 仰角数据点数
-180.0 -25.3   // 方位角度 增益(dB)
...
-90.0 -15.2    // 仰角度 增益(dB)
...
```

##### .nsm 文件格式 (AntennaPatternNSMA)

```
制造商名称
型号
注释
FCC ID
...
2400-2500      // 频率范围(MHz)
15.2           // 中频增益(dB)
25.0           // 半功率波束宽度(度)
HH 36          // HH极化 数据点数
-180.0 -20.1   // 角度 增益
...
```

### 主要输出

#### 1. 增益计算结果

```cpp
float gain = antennaPattern->gain(params);
// 返回: 指定方向的天线增益 (dB)
```

#### 2. 增益范围

```cpp
float minGain, maxGain;
antennaPattern->minMaxGain(&minGain, &maxGain, params);
// 返回: 该方向图的最小和最大增益值 (dB)
```

#### 3. 状态信息

```cpp
bool isValid = antennaPattern->valid();
// 返回: 天线方向图数据是否有效

AntennaPatternType type = antennaPattern->type();  
// 返回: 天线方向图类型枚举值

std::string filename = antennaPattern->filename();
// 返回: 数据文件名称
```

### 典型使用流程

#### 1. 创建和初始化

```cpp
// 方式1: 工厂函数自动创建
AntennaPattern* pattern = loadPatternFile("antenna.pat", 2400.0f);

// 方式2: 直接创建特定类型
AntennaPatternGauss* gaussPattern = new AntennaPatternGauss();
```

#### 2. 设置计算参数

```cpp
AntennaGainParameters params;
params.azim_ = DEG2RAD * 30.0;    // 30度方位角
params.elev_ = DEG2RAD * 10.0;    // 10度仰角  
params.freq_ = 2.4e9;             // 2.4GHz
params.refGain_ = 20.0;           // 20dB参考增益
params.hbw_ = DEG2RAD * 3.0;      // 3度水平波束宽度
params.vbw_ = DEG2RAD * 5.0;      // 5度垂直波束宽度
params.polarity_ = POLARITY_VERTICAL;
```

#### 3. 计算增益

```cpp
if (pattern && pattern->valid()) {
    float gainValue = pattern->gain(params);
    std::cout << "天线增益: " << gainValue << " dB" << std::endl;
    
    float minGain, maxGain;
    pattern->minMaxGain(&minGain, &maxGain, params);
    std::cout << "增益范围: " << minGain << " ~ " << maxGain << " dB" << std::endl;
}
```

### 错误处理

- 文件读取失败时返回错误码
- 无效角度或参数时返回 `SMALL_DB_VAL` (通常为-999dB)
- 通过 `valid()` 方法检查数据完整性
- 异常情况通过 `SIM_ERROR` 宏输出错误信息
