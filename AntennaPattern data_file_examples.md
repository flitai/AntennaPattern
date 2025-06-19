# 天线方向图数据文件格式示例

## 1. .pat 文件格式 (AntennaPatternTable)

### radar_antenna.pat
```
// Radar antenna pattern file
// Type: 1 (beamwidth units), Symmetry: 2 (azim+elev tables)
1 2

// Azimuth table size
36
-180.0  -25.3
-175.0  -24.8
-170.0  -23.9
-165.0  -22.7
...省略...
0.0     0.0
5.0     -0.2
10.0    -1.1
...省略...
175.0   -24.8
180.0   -25.3

// Elevation table size  
18
-90.0   -30.0
-85.0   -28.5
-80.0   -25.2
...省略...
0.0     0.0
5.0     -0.5
10.0    -2.3
...省略...
85.0    -28.5
90.0    -30.0
```

## 2. .rel 文件格式 (AntennaPatternRelativeTable)

### dish_antenna.rel
```
// Relative antenna pattern
// Azimuth and elevation pattern data
// Angles in degrees, gains in relative dB

72 36

// Azimuth data (72 points)
-180.0  -23.2
-175.0  -22.8
-170.0  -21.9
...省略...
0.0     0.0
5.0     -0.3
...省略...
175.0   -22.8
180.0   -23.2

// Elevation data (36 points)
-90.0   -25.5
-85.0   -24.1
...省略...
0.0     0.0
5.0     -0.8
...省略...
85.0    -24.1
90.0    -25.5
```

## 3. .cru 文件格式 (AntennaPatternCRUISE)

### wideband_antenna.cru
```
// CRUISE antenna pattern file
// Azimuth data first, then elevation data

// Azimuth: #angles #frequencies
36 5
// Azimuth limits: min_angle step_angle
-180.0 10.0

// Frequency table (GHz)
8.0 10.0 12.0 14.0 16.0

// Azimuth gain tables (voltage gains)
// Freq 8.0 GHz
0.95 0.94 0.92 0.89 0.85 0.80 0.74 0.67 0.59 0.50 0.41 0.32 0.23 0.15 0.09 0.05 0.03 0.02 0.03 0.05 0.09 0.15 0.23 0.32 0.41 0.50 0.59 0.67 0.74 0.80 0.85 0.89 0.92 0.94 0.95 0.95

// Freq 10.0 GHz  
0.96 0.95 0.93 0.90 0.86 0.81 0.75 0.68 0.60 0.51 0.42 0.33 0.24 0.16 0.10 0.06 0.04 0.03 0.04 0.06 0.10 0.16 0.24 0.33 0.42 0.51 0.60 0.68 0.75 0.81 0.86 0.90 0.93 0.95 0.96 0.96

...省略其他频率...

// Elevation data
36 5
-90.0 5.0

// Frequency table (should match azimuth)
8.0 10.0 12.0 14.0 16.0

// Elevation gain tables
...省略...
```

## 4. .nsm 文件格式 (AntennaPatternNSMA)

### multi_pol_antenna.nsm
```
ACME Antenna Corporation
Model XYZ-2400
Dual polarization patch antenna
FCC-ID-12345
REV-001
2023-01-15
ACME-001

2400-2500
12.5
25.0

HH 36
-180.0  -20.1
-175.0  -19.8
-170.0  -19.2
...省略...
0.0     0.0
5.0     -0.4
...省略...
175.0   -19.8
180.0   -20.1

HV 36
-180.0  -35.2
-175.0  -34.8
...省略...
0.0     -25.0
...省略...
180.0   -35.2

VV 36
-180.0  -18.5
-175.0  -18.1
...省略...
0.0     0.0
...省略...
180.0   -18.5

VH 36
-180.0  -33.7
...省略...
0.0     -23.5
...省略...
180.0   -33.7

ELHH 18
-90.0   -25.0
-85.0   -24.2
...省略...
0.0     0.0
...省略...
90.0    -25.0

ELHV 18
-90.0   -40.0
...省略...
0.0     -28.0
...省略...
90.0    -40.0

ELVV 18
-90.0   -23.8
...省略...
0.0     0.0
...省略...
90.0    -23.8

ELVH 18
-90.0   -38.5
...省略...
0.0     -26.5
...省略...
90.0    -38.5
```

## 5. .mon 文件格式 (AntennaPatternMonopulse)

### monopulse_antenna.mon
```
sum
8.0 12.0 0.5
-30.0 30.0 2.0
-15.0 15.0 1.0

// Frequency 10.0 GHz data
// Azimuth, Elevation, Magnitude(dB), Phase(degrees)
-30.0 -15.0 -15.2  45.3
-30.0 -14.0 -14.8  44.1
...省略sum channel数据...

diff  
8.0 12.0 0.5
-30.0 30.0 2.0
-15.0 15.0 1.0

// Frequency 10.0 GHz data
// Azimuth, Elevation, Magnitude(dB), Phase(degrees)
-30.0 -15.0 -25.8  125.7
-30.0 -14.0 -25.2  124.3
...省略diff channel数据...
```

## 6. .bil 文件格式 (AntennaPatternBiLinear)

### horn_antenna.bil
```
bilinear
8.0 12.0 0.5
-45.0 45.0 2.0
-30.0 30.0 1.5

// Frequency 10.0 GHz gain data (dB only)
-45.0 -30.0 -18.5
-45.0 -28.5 -17.8
-45.0 -27.0 -17.1
...省略...
0.0 0.0 0.0
...省略...
45.0 30.0 -18.5
```

## 7. .ezn 文件格式 (AntennaPatternEZNEC)

### array_antenna.ezn
```
EZNEC ver. 7.0

Frequency = 2.45 GHz

Maximum Gain = 15.2 dBi

Azimuth Pattern for Elevation Angle = 0 deg
"Deg","V dB","H dB","Tot dB"
0,15.2,14.8,15.5
5,14.9,14.5,15.2
10,13.8,13.4,14.1
...省略...
355,14.9,14.5,15.2

Azimuth Pattern for Elevation Angle = 10 deg
"Deg","V dB","H dB","Tot dB"
0,14.8,14.4,15.1
5,14.5,14.1,14.8
...省略...
```

## 8. .xfd 文件格式 (AntennaPatternXFDTD)

### patch_antenna.xfd
```
begin_<parameters>
format free
phi_min 0
phi_max 360
phi_inc 5
theta_min 0
theta_max 180
theta_inc 5
complex
mag_phase
pattern gain
magnitude dB
maximum_gain 8.5
phase degrees
direction degrees
polarization theta_phi
end_<parameters>

0.0 0.0 8.5 7.2 0.0 90.0
0.0 5.0 8.3 7.0 -5.2 88.5
0.0 10.0 7.8 6.5 -12.8 85.3
...省略数据...
```

## 使用这些文件的C++代码示例

```cpp
// 读取不同格式的文件
void loadDifferentFormats() {
    // 自动识别格式并加载
    auto pattern1 = simCore::loadPatternFile("radar_antenna.pat", 2400.0f);
    auto pattern2 = simCore::loadPatternFile("dish_antenna.rel", 2400.0f);  
    auto pattern3 = simCore::loadPatternFile("wideband_antenna.cru", 2400.0f);
    auto pattern4 = simCore::loadPatternFile("multi_pol_antenna.nsm", 2400.0f);
    
    // 手动加载特定格式
    simCore::AntennaPatternMonopulse mono;
    mono.readPat("monopulse_antenna.mon", 10.0e9);
    
    simCore::AntennaPatternBiLinear bilin;
    bilin.readPat("horn_antenna.bil", 10.0e9);
    
    simCore::AntennaPatternEZNEC eznec;
    eznec.readPat("array_antenna.ezn");
    
    simCore::AntennaPatternXFDTD xfdtd;
    xfdtd.readPat("patch_antenna.xfd");
}
```
