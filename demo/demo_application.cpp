#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <memory>
#include <iomanip>
#include "simCore/EM/AntennaPattern.h"
#include "simCore/Calc/Angle.h"

/**
 * 实际应用场景：雷达系统覆盖范围分析
 * 
 * 这个示例展示如何使用天线方向图库来：
 * 1. 分析雷达的探测覆盖范围
 * 2. 计算不同目标方位的接收功率
 * 3. 评估天线增益对系统性能的影响
 * 4. 生成覆盖范围可视化数据
 */

// 雷达系统参数结构
struct RadarSystemParams {
    double transmitPower;      // 发射功率 (W)
    double frequency;          // 工作频率 (Hz)
    double antennaGain;        // 天线增益 (dB)
    double systemLosses;       // 系统损耗 (dB)
    double noiseTemperature;   // 噪声温度 (K)
    double detectionThreshold; // 探测门限 (dB)
    std::string antennaFile;   // 天线方向图文件
};

// 目标参数结构
struct TargetParams {
    double range;              // 距离 (m)
    double azimuth;           // 方位角 (度)
    double elevation;         // 仰角 (度)
    double rcs;               // 雷达截面积 (m²)
};

// 雷达方程计算类
class RadarEquationCalculator {
private:
    static constexpr double BOLTZMANN_CONSTANT = 1.38064852e-23;  // J/K
    static constexpr double LIGHT_SPEED = 2.99792458e8;           // m/s
    
    RadarSystemParams radarParams_;
    std::unique_ptr<simCore::AntennaPattern> antennaPattern_;
    
public:
    RadarEquationCalculator(const RadarSystemParams& params) 
        : radarParams_(params) {
        // 加载天线方向图
        antennaPattern_.reset(simCore::loadPatternFile(
            radarParams_.antennaFile, 
            static_cast<float>(radarParams_.frequency / 1e6)
        ));
        
        if (!antennaPattern_ || !antennaPattern_->valid()) {
            throw std::runtime_error("无法加载天线方向图文件: " + radarParams_.antennaFile);
        }
        
        std::cout << "成功加载天线方向图: " << radarParams_.antennaFile << std::endl;
        std::cout << "天线类型: " << simCore::antennaPatternTypeString(antennaPattern_->type()) << std::endl;
    }
    
    // 计算天线在指定方向的增益
    double getAntennaGain(double azimuth_deg, double elevation_deg) {
        simCore::AntennaGainParameters params;
        params.azim_ = simCore::DEG2RAD * azimuth_deg;
        params.elev_ = simCore::DEG2RAD * elevation_deg;
        params.refGain_ = static_cast<float>(radarParams_.antennaGain);
        params.freq_ = radarParams_.frequency;
        params.hbw_ = simCore::DEG2RAD * 3.0f;  // 假设3度波束宽度
        params.vbw_ = simCore::DEG2RAD * 5.0f;  // 假设5度波束宽度
        
        return antennaPattern_->gain(params);
    }
    
    // 计算接收功率 (雷达方程)
    double calculateReceivedPower(const TargetParams& target) {
        // 获取发射和接收方向的天线增益
        double txGain = getAntennaGain(target.azimuth, target.elevation);
        double rxGain = txGain;  // 对于单站雷达，收发天线相同
        
        // 波长
        double wavelength = LIGHT_SPEED / radarParams_.frequency;
        
        // 雷达方程: Pr = (Pt * Gt * Gr * λ² * σ) / ((4π)³ * R⁴ * L)
        double range4 = std::pow(target.range, 4);
        double lambda2 = wavelength * wavelength;
        double pi4_cubed = std::pow(4.0 * M_PI, 3);
        
        // 转换增益从dB到线性
        double txGainLinear = std::pow(10.0, txGain / 10.0);
        double rxGainLinear = std::pow(10.0, rxGain / 10.0);
        double lossesLinear = std::pow(10.0, radarParams_.systemLosses / 10.0);
        
        double receivedPower = (radarParams_.transmitPower * txGainLinear * rxGainLinear * 
                               lambda2 * target.rcs) / (pi4_cubed * range4 * lossesLinear);
        
        return receivedPower;  // 返回线性功率值 (W)
    }
    
    // 计算信噪比
    double calculateSNR(const TargetParams& target) {
        double receivedPower = calculateReceivedPower(target);
        double noisePower = BOLTZMANN_CONSTANT * radarParams_.noiseTemperature * 1e6; // 假设1MHz带宽
        
        return 10.0 * std::log10(receivedPower / noisePower);  // dB
    }
    
    // 判断目标是否可探测
    bool isTargetDetectable(const TargetParams& target) {
        double snr = calculateSNR(target);
        return snr >= radarParams_.detectionThreshold;
    }
    
    // 计算最大探测距离
    double calculateMaxRange(double azimuth_deg, double elevation_deg, double rcs) {
        TargetParams target = {1000.0, azimuth_deg, elevation_deg, rcs}; // 初始距离1km
        
        // 二分查找最大探测距离
        double minRange = 1000.0;    // 1km
        double maxRange = 500000.0;  // 500km
        double epsilon = 100.0;      // 精度100m
        
        while (maxRange - minRange > epsilon) {
            double midRange = (minRange + maxRange) / 2.0;
            target.range = midRange;
            
            if (isTargetDetectable(target)) {
                minRange = midRange;
            } else {
                maxRange = midRange;
            }
        }
        
        return minRange;
    }
};

// 覆盖范围分析器
class CoverageAnalyzer {
private:
    RadarEquationCalculator& calculator_;
    
public:
    CoverageAnalyzer(RadarEquationCalculator& calc) : calculator_(calc) {}
    
    // 生成方位角覆盖图
    void generateAzimuthCoverage(const std::string& filename, double rcs = 1.0) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法创建文件: " << filename << std::endl;
            return;
        }
        
        file << "# 方位角覆盖分析结果\n";
        file << "# 方位角(度), 天线增益(dB), 最大探测距离(km)\n";
        
        std::cout << "\n=== 方位角覆盖分析 ===\n";
        std::cout << "方位角(度)\t天线增益(dB)\t最大距离(km)\n";
        std::cout << "--------------------------------------------\n";
        
        for (int azimuth = -180; azimuth <= 180; azimuth += 5) {
            double antennaGain = calculator_.getAntennaGain(azimuth, 0.0);
            double maxRange = calculator_.calculateMaxRange(azimuth, 0.0, rcs);
            
            file << azimuth << ", " << std::fixed << std::setprecision(2) 
                 << antennaGain << ", " << maxRange/1000.0 << "\n";
            
            if (azimuth % 20 == 0) {  // 每20度输出一次
                std::cout << azimuth << "\t\t" << std::fixed << std::setprecision(2)
                         << antennaGain << "\t\t" << maxRange/1000.0 << std::endl;
            }
        }
        
        file.close();
        std::cout << "方位角覆盖数据已保存到: " << filename << std::endl;
    }
    
    // 生成仰角覆盖图
    void generateElevationCoverage(const std::string& filename, double rcs = 1.0) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法创建文件: " << filename << std::endl;
            return;
        }
        
        file << "# 仰角覆盖分析结果\n";
        file << "# 仰角(度), 天线增益(dB), 最大探测距离(km)\n";
        
        std::cout << "\n=== 仰角覆盖分析 ===\n";
        std::cout << "仰角(度)\t天线增益(dB)\t最大距离(km)\n";
        std::cout << "--------------------------------------------\n";
        
        for (int elevation = -30; elevation <= 90; elevation += 5) {
            double antennaGain = calculator_.getAntennaGain(0.0, elevation);
            double maxRange = calculator_.calculateMaxRange(0.0, elevation, rcs);
            
            file << elevation << ", " << std::fixed << std::setprecision(2) 
                 << antennaGain << ", " << maxRange/1000.0 << "\n";
            
            if (elevation % 10 == 0) {  // 每10度输出一次
                std::cout << elevation << "\t\t" << std::fixed << std::setprecision(2)
                         << antennaGain << "\t\t" << maxRange/1000.0 << std::endl;
            }
        }
        
        file.close();
        std::cout << "仰角覆盖数据已保存到: " << filename << std::endl;
    }
    
    // 生成2D覆盖热力图数据
    void generate2DCoverage(const std::string& filename, double rcs = 1.0) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法创建文件: " << filename << std::endl;
            return;
        }
        
        file << "# 2D覆盖热力图数据\n";
        file << "# 方位角(度), 仰角(度), 天线增益(dB), 最大探测距离(km)\n";
        
        std::cout << "\n=== 生成2D覆盖热力图数据 ===\n";
        
        int totalPoints = 0;
        int processedPoints = 0;
        
        // 计算总点数
        for (int azimuth = -180; azimuth <= 180; azimuth += 10) {
            for (int elevation = -30; elevation <= 90; elevation += 5) {
                totalPoints++;
            }
        }
        
        for (int azimuth = -180; azimuth <= 180; azimuth += 10) {
            for (int elevation = -30; elevation <= 90; elevation += 5) {
                double antennaGain = calculator_.getAntennaGain(azimuth, elevation);
                double maxRange = calculator_.calculateMaxRange(azimuth, elevation, rcs);
                
                file << azimuth << ", " << elevation << ", " 
                     << std::fixed << std::setprecision(2) << antennaGain << ", " 
                     << maxRange/1000.0 << "\n";
                
                processedPoints++;
                if (processedPoints % 100 == 0) {
                    std::cout << "进度: " << (100 * processedPoints / totalPoints) << "%" << std::endl;
                }
            }
        }
        
        file.close();
        std::cout << "2D覆盖数据已保存到: " << filename << std::endl;
    }
    
    // 分析特定目标
    void analyzeSpecificTargets(const std::vector<TargetParams>& targets) {
        std::cout << "\n=== 特定目标分析 ===\n";
        std::cout << "目标\t距离(km)\t方位(度)\t仰角(度)\tRCS(m²)\t接收功率(dBm)\t信噪比(dB)\t可探测\n";
        std::cout << "--------------------------------------------------------------------------------\n";
        
        for (size_t i = 0; i < targets.size(); ++i) {
            const auto& target = targets[i];
            
            double receivedPower = calculator_.calculateReceivedPower(target);
            double receivedPower_dBm = 10.0 * std::log10(receivedPower * 1000.0);  // 转换为dBm
            double snr = calculator_.calculateSNR(target);
            bool detectable = calculator_.isTargetDetectable(target);
            
            std::cout << "T" << (i+1) << "\t" 
                     << std::fixed << std::setprecision(1) << target.range/1000.0 << "\t\t"
                     << target.azimuth << "\t\t" << target.elevation << "\t\t"
                     << target.rcs << "\t" << std::setprecision(2) << receivedPower_dBm << "\t\t"
                     << snr << "\t\t" << (detectable ? "是" : "否") << std::endl;
        }
    }
};

// 主程序：雷达覆盖范围分析
int main() {
    std::cout << "雷达系统覆盖范围分析程序\n";
    std::cout << "========================\n";
    
    try {
        // 定义雷达系统参数
        RadarSystemParams radarParams;
        radarParams.transmitPower = 1000000.0;      // 1MW
        radarParams.frequency = 10.0e9;             // 10GHz (X波段)
        radarParams.antennaGain = 30.0;             // 30dB
        radarParams.systemLosses = 6.0;             // 6dB系统损耗
        radarParams.noiseTemperature = 290.0;       // 290K噪声温度
        radarParams.detectionThreshold = 13.0;      // 13dB探测门限
        radarParams.antennaFile = "radar_antenna.pat"; // 天线方向图文件
        
        std::cout << "雷达系统参数:\n";
        std::cout << "发射功率: " << radarParams.transmitPower/1e6 << " MW\n";
        std::cout << "工作频率: " << radarParams.frequency/1e9 << " GHz\n";
        std::cout << "天线增益: " << radarParams.antennaGain << " dB\n";
        std::cout << "系统损耗: " << radarParams.systemLosses << " dB\n";
        std::cout << "探测门限: " << radarParams.detectionThreshold << " dB\n\n";
        
        // 创建雷达方程计算器
        RadarEquationCalculator calculator(radarParams);
        CoverageAnalyzer analyzer(calculator);
        
        // 1. 生成方位角覆盖分析
        analyzer.generateAzimuthCoverage("azimuth_coverage.csv", 1.0);  // 1m² RCS
        
        // 2. 生成仰角覆盖分析
        analyzer.generateElevationCoverage("elevation_coverage.csv", 1.0);
        
        // 3. 生成2D覆盖热力图数据
        analyzer.generate2DCoverage("2d_coverage.csv", 1.0);
        
        // 4. 分析特定目标
        std::vector<TargetParams> testTargets = {
            {50000.0,   0.0,  0.0, 1.0},    // 50km, 正前方, 1m² RCS
            {100000.0, 30.0,  5.0, 0.5},    // 100km, 右前方, 0.5m² RCS
            {200000.0, 45.0, 10.0, 2.0},    // 200km, 右侧, 2m² RCS
            {150000.0, 90.0,  0.0, 1.5},    // 150km, 右侧, 1.5m² RCS
            {80000.0, 180.0,  0.0, 0.8},    // 80km, 后方, 0.8m² RCS
            {120000.0, -45.0, 15.0, 1.2}    // 120km, 左侧上方, 1.2m² RCS
        };
        
        analyzer.analyzeSpecificTargets(testTargets);
        
        // 5. 主瓣方向性能分析
        std::cout << "\n=== 主瓣方向性能分析 ===\n";
        double mainBeamGain = calculator.getAntennaGain(0.0, 0.0);
        double maxRange_1m2 = calculator.calculateMaxRange(0.0, 0.0, 1.0);
        double maxRange_10m2 = calculator.calculateMaxRange(0.0, 0.0, 10.0);
        double maxRange_01m2 = calculator.calculateMaxRange(0.0, 0.0, 0.1);
        
        std::cout << "主瓣增益: " << std::fixed << std::setprecision(2) << mainBeamGain << " dB\n";
        std::cout << "最大探测距离:\n";
        std::cout << "  RCS = 0.1 m²: " << maxRange_01m2/1000.0 << " km\n";
        std::cout << "  RCS = 1.0 m²: " << maxRange_1m2/1000.0 << " km\n";
        std::cout << "  RCS = 10.0 m²: " << maxRange_10m2/1000.0 << " km\n";
        
        // 6. 不同RCS目标的覆盖对比
        std::cout << "\n=== 不同RCS目标覆盖对比 ===\n";
        std::vector<double> rcsValues = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
        std::cout << "RCS(m²)\t主瓣最大距离(km)\t侧向最大距离(km)\t后向最大距离(km)\n";
        std::cout << "----------------------------------------------------------------\n";
        
        for (double rcs : rcsValues) {
            double mainBeamRange = calculator.calculateMaxRange(0.0, 0.0, rcs);
            double sideRange = calculator.calculateMaxRange(90.0, 0.0, rcs);
            double backRange = calculator.calculateMaxRange(180.0, 0.0, rcs);
            
            std::cout << rcs << "\t" << std::fixed << std::setprecision(1)
                     << mainBeamRange/1000.0 << "\t\t\t" 
                     << sideRange/1000.0 << "\t\t\t"
                     << backRange/1000.0 << std::endl;
        }
        
        std::cout << "\n分析完成！结果文件已生成：\n";
        std::cout << "- azimuth_coverage.csv: 方位角覆盖数据\n";
        std::cout << "- elevation_coverage.csv: 仰角覆盖数据\n";
        std::cout << "- 2d_coverage.csv: 2D覆盖热力图数据\n";
        std::cout << "\n可使用Python/MATLAB等工具进行数据可视化。\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}

// 编译命令:
// g++ -std=c++11 -O2 -I/path/to/simCore/include radar_coverage_analysis.cpp -L/path/to/simCore/lib -lsimCore -o radar_coverage_analysis

/*
使用说明:
1. 准备天线方向图文件 (radar_antenna.pat)
2. 编译并运行程序
3. 程序将生成三个CSV文件用于进一步分析和可视化
4. 可以修改雷达参数来分析不同系统的性能

Python可视化示例代码:
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 读取数据
azimuth_data = pd.read_csv('azimuth_coverage.csv', comment='#')
elevation_data = pd.read_csv('elevation_coverage.csv', comment='#')

# 绘制方位角覆盖图
plt.figure(figsize=(12, 6))
plt.subplot(121)
plt.plot(azimuth_data.iloc[:, 0], azimuth_data.iloc[:, 2])
plt.xlabel('方位角 (度)')
plt.ylabel('最大探测距离 (km)')
plt.title('方位角覆盖图')
plt.grid(True)

# 绘制仰角覆盖图
plt.subplot(122)
plt.plot(elevation_data.iloc[:, 0], elevation_data.iloc[:, 2])
plt.xlabel('仰角 (度)')
plt.ylabel('最大探测距离 (km)')
plt.title('仰角覆盖图')
plt.grid(True)

plt.tight_layout()
plt.show()
*/