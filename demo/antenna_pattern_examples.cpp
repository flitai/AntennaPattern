#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include "simCore/EM/AntennaPattern.h"
#include "simCore/Calc/Angle.h"

// 示例1: 使用算法型天线方向图（高斯型）
void example1_algorithmic_pattern()
{
    std::cout << "\n=== 示例1: 高斯型天线方向图 ===\n";
    
    // 创建高斯型天线方向图
    std::unique_ptr<simCore::AntennaPatternGauss> gaussPattern(new simCore::AntennaPatternGauss());
    
    // 设置天线参数
    simCore::AntennaGainParameters params;
    params.refGain_ = 25.0f;                           // 25dB参考增益
    params.hbw_ = simCore::DEG2RAD * 3.0f;            // 3度水平波束宽度
    params.vbw_ = simCore::DEG2RAD * 5.0f;            // 5度垂直波束宽度
    params.freq_ = 10.0e9;                            // 10GHz频率
    params.polarity_ = simCore::POLARITY_VERTICAL;     // 垂直极化
    
    // 计算不同角度的增益
    std::vector<float> angles = {0, 1, 2, 5, 10, 20, 30};  // 度
    
    std::cout << "角度(度)\t增益(dB)\n";
    std::cout << "------------------------\n";
    
    for (float angle_deg : angles) {
        params.azim_ = simCore::DEG2RAD * angle_deg;   // 方位角
        params.elev_ = 0.0f;                           // 仰角为0
        
        float gain = gaussPattern->gain(params);
        std::cout << angle_deg << "\t\t" << std::fixed << std::setprecision(2) << gain << std::endl;
    }
    
    // 获取增益范围
    float minGain, maxGain;
    gaussPattern->minMaxGain(&minGain, &maxGain, params);
    std::cout << "\n增益范围: " << minGain << " ~ " << maxGain << " dB\n";
}

// 示例2: 使用工厂函数加载文件
void example2_load_from_file()
{
    std::cout << "\n=== 示例2: 从文件加载天线方向图 ===\n";
    
    // 使用工厂函数加载不同类型的文件
    std::vector<std::string> testFiles = {
        "radar_antenna.pat",    // 表格型
        "dish_antenna.rel",     // 相对表格型  
        "array_antenna.cru",    // CRUISE格式
        "horn_antenna.nsm"      // NSMA格式
    };
    
    for (const auto& filename : testFiles) {
        std::unique_ptr<simCore::AntennaPattern> pattern(
            simCore::loadPatternFile(filename, 2400.0f)  // 2.4GHz
        );
        
        if (pattern && pattern->valid()) {
            std::cout << "成功加载: " << filename << std::endl;
            std::cout << "类型: " << simCore::antennaPatternTypeString(pattern->type()) << std::endl;
            
            // 计算主瓣方向增益
            simCore::AntennaGainParameters params;
            params.azim_ = 0.0f;
            params.elev_ = 0.0f;
            params.refGain_ = 0.0f;  // 使用文件中的原始增益
            
            float gain = pattern->gain(params);
            std::cout << "主瓣增益: " << gain << " dB\n" << std::endl;
        } else {
            std::cout << "加载失败: " << filename << std::endl;
        }
    }
}

// 示例3: 单脉冲天线方向图
void example3_monopulse_pattern()
{
    std::cout << "\n=== 示例3: 单脉冲天线方向图 ===\n";
    
    simCore::AntennaPatternMonopulse monopulsePattern;
    
    // 加载单脉冲天线数据文件
    int result = monopulsePattern.readPat("monopulse_antenna.mon", 5.6e9);  // 5.6GHz
    
    if (result == 0 && monopulsePattern.valid()) {
        std::cout << "单脉冲天线加载成功\n";
        
        simCore::AntennaGainParameters params;
        params.refGain_ = 30.0f;    // 30dB参考增益
        params.freq_ = 5.6e9;       // 5.6GHz
        
        // 测试和通道 (Sum Channel)
        params.delta_ = false;
        std::cout << "\n=== 和通道增益 ===\n";
        std::cout << "角度(度)\t增益(dB)\n";
        std::cout << "------------------------\n";
        
        for (int angle = -10; angle <= 10; angle += 2) {
            params.azim_ = simCore::DEG2RAD * angle;
            params.elev_ = 0.0f;
            
            float sumGain = monopulsePattern.gain(params);
            std::cout << angle << "\t\t" << std::fixed << std::setprecision(2) << sumGain << std::endl;
        }
        
        // 测试差通道 (Delta Channel)
        params.delta_ = true;
        std::cout << "\n=== 差通道增益 ===\n";
        std::cout << "角度(度)\t增益(dB)\n";
        std::cout << "------------------------\n";
        
        for (int angle = -10; angle <= 10; angle += 2) {
            params.azim_ = simCore::DEG2RAD * angle;
            params.elev_ = 0.0f;
            
            float deltaGain = monopulsePattern.gain(params);
            std::cout << angle << "\t\t" << std::fixed << std::setprecision(2) << deltaGain << std::endl;
        }
    } else {
        std::cout << "单脉冲天线加载失败，错误码: " << result << std::endl;
    }
}

// 示例4: 多极化天线方向图 (NSMA格式)
void example4_polarization_pattern()
{
    std::cout << "\n=== 示例4: 多极化天线方向图 ===\n";
    
    simCore::AntennaPatternNSMA nsmaPattern;
    
    if (nsmaPattern.readPat("multi_pol_antenna.nsm") == 0) {
        std::cout << "NSMA天线加载成功\n";
        
        simCore::AntennaGainParameters params;
        params.azim_ = simCore::DEG2RAD * 0.0f;     // 主瓣方向
        params.elev_ = simCore::DEG2RAD * 0.0f;
        params.refGain_ = 0.0f;  // 使用天线本身的增益
        
        // 测试不同极化
        std::vector<std::pair<simCore::PolarityType, std::string>> polarizations = {
            {simCore::POLARITY_HORIZONTAL, "水平极化(HH)"},
            {simCore::POLARITY_VERTICAL, "垂直极化(VV)"},
            {simCore::POLARITY_HORZVERT, "水平-垂直极化(HV)"},
            {simCore::POLARITY_VERTHORZ, "垂直-水平极化(VH)"}
        };
        
        std::cout << "\n不同极化的主瓣增益:\n";
        std::cout << "极化类型\t\t增益(dB)\n";
        std::cout << "--------------------------------\n";
        
        for (const auto& pol : polarizations) {
            params.polarity_ = pol.first;
            float gain = nsmaPattern.gain(params);
            std::cout << pol.second << "\t" << std::fixed << std::setprecision(2) << gain << std::endl;
            
            // 获取该极化的增益范围
            float minGain, maxGain;
            nsmaPattern.minMaxGain(&minGain, &maxGain, params);
            std::cout << "    增益范围: " << minGain << " ~ " << maxGain << " dB\n";
        }
    } else {
        std::cout << "NSMA天线加载失败\n";
    }
}

// 示例5: 扫描整个空域的增益分布
void example5_gain_pattern_scan()
{
    std::cout << "\n=== 示例5: 天线方向图扫描 ===\n";
    
    // 创建sinc函数天线
    std::unique_ptr<simCore::AntennaPatternSinXX> sincPattern(new simCore::AntennaPatternSinXX());
    
    simCore::AntennaGainParameters params;
    params.refGain_ = 20.0f;                          // 20dB参考增益
    params.hbw_ = simCore::DEG2RAD * 5.0f;           // 5度水平波束宽度
    params.vbw_ = simCore::DEG2RAD * 8.0f;           // 8度垂直波束宽度
    params.firstLobe_ = -13.2f;                       // 第一旁瓣电平
    params.freq_ = 3.0e9;                            // 3GHz
    
    std::cout << "方位角扫描 (仰角=0度):\n";
    std::cout << "方位角(度)\t增益(dB)\t波瓣类型\n";
    std::cout << "----------------------------------------\n";
    
    params.elev_ = 0.0f;  // 固定仰角为0度
    
    for (int azim_deg = -30; azim_deg <= 30; azim_deg += 5) {
        params.azim_ = simCore::DEG2RAD * azim_deg;
        float gain = sincPattern->gain(params);
        
        // 简单判断波瓣类型
        std::string lobeType = "旁瓣";
        if (std::abs(azim_deg) <= 3) {
            lobeType = "主瓣";
        } else if (gain < -15.0f) {
            lobeType = "后瓣";
        }
        
        std::cout << azim_deg << "\t\t" << std::fixed << std::setprecision(2) 
                  << gain << "\t\t" << lobeType << std::endl;
    }
    
    std::cout << "\n仰角扫描 (方位角=0度):\n";
    std::cout << "仰角(度)\t增益(dB)\n";
    std::cout << "------------------------\n";
    
    params.azim_ = 0.0f;  // 固定方位角为0度
    
    for (int elev_deg = -20; elev_deg <= 20; elev_deg += 4) {
        params.elev_ = simCore::DEG2RAD * elev_deg;
        float gain = sincPattern->gain(params);
        std::cout << elev_deg << "\t\t" << std::fixed << std::setprecision(2) << gain << std::endl;
    }
}

// 示例6: 频率相关的天线方向图 (CRUISE格式)
void example6_frequency_dependent_pattern()
{
    std::cout << "\n=== 示例6: 频率相关天线方向图 ===\n";
    
    simCore::AntennaPatternCRUISE cruisePattern;
    
    if (cruisePattern.readPat("wideband_antenna.cru") == 0) {
        std::cout << "CRUISE天线加载成功\n";
        
        simCore::AntennaGainParameters params;
        params.azim_ = 0.0f;     // 主瓣方向
        params.elev_ = 0.0f;
        params.refGain_ = 0.0f;
        
        // 测试不同频率下的增益
        std::vector<double> frequencies = {8.0e9, 10.0e9, 12.0e9, 14.0e9, 16.0e9};  // GHz
        
        std::cout << "\n不同频率下的主瓣增益:\n";
        std::cout << "频率(GHz)\t增益(dB)\n";
        std::cout << "------------------------\n";
        
        for (double freq : frequencies) {
            params.freq_ = freq;
            float gain = cruisePattern.gain(params);
            std::cout << freq/1e9 << "\t\t" << std::fixed << std::setprecision(2) << gain << std::endl;
        }
        
        // 在固定频率下扫描方位角
        params.freq_ = 10.0e9;  // 固定10GHz
        params.elev_ = 0.0f;
        
        std::cout << "\n10GHz频率下的方位角扫描:\n";
        std::cout << "方位角(度)\t增益(dB)\n";
        std::cout << "------------------------\n";
        
        for (int azim_deg = -15; azim_deg <= 15; azim_deg += 3) {
            params.azim_ = simCore::DEG2RAD * azim_deg;
            float gain = cruisePattern.gain(params);
            std::cout << azim_deg << "\t\t" << std::fixed << std::setprecision(2) << gain << std::endl;
        }
    } else {
        std::cout << "CRUISE天线加载失败\n";
    }
}

// 示例7: 创建自定义表格天线方向图
void example7_create_custom_table_pattern()
{
    std::cout << "\n=== 示例7: 创建自定义表格天线方向图 ===\n";
    
    simCore::AntennaPatternTable tablePattern;
    
    // 手动设置天线数据 (模拟从二进制文件加载)
    tablePattern.setValid(true);
    tablePattern.setType(false);  // 角度单位为弧度
    tablePattern.setFilename("custom_antenna.pat");
    
    // 设置方位角数据 (简单的余弦平方方向图)
    for (int angle_deg = -180; angle_deg <= 180; angle_deg += 10) {
        float angle_rad = simCore::DEG2RAD * angle_deg;
        float gain_db = -3.0f * (angle_deg * angle_deg) / (90.0f * 90.0f);  // 简单的抛物线
        if (gain_db < -30.0f) gain_db = -30.0f;  // 限制最小增益
        
        tablePattern.setAzimData(angle_rad, gain_db);
    }
    
    // 设置仰角数据
    for (int angle_deg = -90; angle_deg <= 90; angle_deg += 10) {
        float angle_rad = simCore::DEG2RAD * angle_deg;
        float gain_db = -1.5f * (angle_deg * angle_deg) / (45.0f * 45.0f);  // 更窄的波束
        if (gain_db < -20.0f) gain_db = -20.0f;
        
        tablePattern.setElevData(angle_rad, gain_db);
    }
    
    std::cout << "自定义天线方向图创建完成\n";
    std::cout << "类型: " << simCore::antennaPatternTypeString(tablePattern.type()) << std::endl;
    std::cout << "文件名: " << tablePattern.filename() << std::endl;
    
    // 测试计算
    simCore::AntennaGainParameters params;
    params.refGain_ = 25.0f;
    params.hbw_ = simCore::DEG2RAD * 10.0f;
    params.vbw_ = simCore::DEG2RAD * 15.0f;
    params.weighting_ = true;  // 使用加权平均
    
    std::cout << "\n自定义天线增益测试:\n";
    std::cout << "角度(度)\t增益(dB)\n";
    std::cout << "------------------------\n";
    
    for (int angle = -20; angle <= 20; angle += 5) {
        params.azim_ = simCore::DEG2RAD * angle;
        params.elev_ = 0.0f;
        
        float gain = tablePattern.gain(params);
        std::cout << angle << "\t\t" << std::fixed << std::setprecision(2) << gain << std::endl;
    }
}

// 主函数：运行所有示例
int main()
{
    std::cout << "天线方向图库调用示例\n";
    std::cout << "====================\n";
    
    try {
        example1_algorithmic_pattern();
        example2_load_from_file();
        example3_monopulse_pattern();
        example4_polarization_pattern();
        example5_gain_pattern_scan();
        example6_frequency_dependent_pattern();
        example7_create_custom_table_pattern();
        
        std::cout << "\n所有示例运行完成！\n";
        
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}

// 编译命令示例:
// g++ -std=c++11 -I/path/to/simCore/include antenna_examples.cpp -L/path/to/simCore/lib -lsimCore -o antenna_examples