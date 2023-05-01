LEO satellite simulator
============
- 衛星位置的預測是base on此C++ SGP4 project
   - https://www.danrw.com/sgp4/
- 有任何問題可以透過email聯絡我: a0981695482@gmail.com
### 編譯流程

---

- 安裝編譯會用到的build-essential:
    ```bash
    sudo apt-get update
    sudo apt-get install build-essential
    ```

- 安裝編譯會用到的cmake:
    ```bash
    sudo apt update
    sudo apt install cmake
    ```

- 安裝修正方位角時會用到的C++ Eigen library:
    
    ```bash
    sudo apt install libeigen3-dev
    ```
    
- 進入到檔案位置LEO-satellite-constellation-simulator/中進行cmake來產生Makefile，在LEO-satellite-constellation-simulator/中輸入指令:
    
    ```bash
    cmake .
    ```
    
- 使用產生的Makefile編譯檔案:
    
    ```bash
    make
    ```
    

### 執行流程

---

- 設定parameter.txt 中的參數
    - parameter.txt位於sgp4/sattrack中
- 目前有實作以下function
    1. **printAllSatNeighborId** : Terminal印出每一個衛星的四個連線鄰居衛星編號
    2. **printAERfile** : 印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到sattrack/output.txt中
        - 需設定parameter: observerId, otherId, outputFileName
    3. **printRightConnectabilityFile**: 印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
        - 需設定parameter: observerId, acceptableAzimuthDif, acceptableElevationDif, acceptableRange, ISLrightAngle, outputFileName
    4. **printLeftConnectabilityFile**: 印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
        - 需設定parameter: observerId, acceptableAzimuthDif, acceptableElevationDif, acceptableRange, ISLleftAngle, outputFileName
    5. **printAllIslConnectionInfoFile**: 印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif, acceptableElevationDif, acceptableRange, ISLrightAngle, ISLleftAngle
        - 此function需計算約5分鐘才會輸出結果至檔案中
    6. **printConstellationStateFile**: 印出某個特定時刻，行星群的連線狀態(112*112)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif, acceptableElevationDif, acceptableRange, PAT_time, time, ISLrightAngle, ISLleftAngle(目前是當成前後一定可以連線), outputFileName
    7. **printConstellationHopCountFile**: 印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif, acceptableElevationDif, acceptableRange, PAT_time, time, ISLrightAngle, ISLleftAngle(目前是當成前後一定可以連線), outputFileName
    8. **printConstellationHopCountFileAndOutputCertainPath**: 印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
        - 需設定parameter: acceptableAzimuthDif, acceptableElevationDif, acceptableRange, PAT_time, time, ISLrightAngle, ISLleftAngle(目前是當成前後一定可以連線), outputFileName
    9. **printConstellationDistanceFile**: 印出某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif, acceptableElevationDif, acceptableRange, PAT_time, time, ISLrightAngle, ISLleftAngle(目前是當成前後一定可以連線), outputFileName
    10. **printConstellationDistanceAndOutputCertainPath**: 印出某個特定時刻，行星群的shortest path狀態，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
        - 需設定parameter: acceptableAzimuthDif, acceptableElevationDif, acceptableRange, PAT_time, time, ISLrightAngle, ISLleftAngle(目前是當成前後一定可以連線), outputFileName
    11. **printStationAllSatConnectionTime**:印出根據parameter.txt設置位置的地面站，與星群中每一個衛星一天中有那些時間是可以連線的
        - 需設定parameter: stationLatitude, stationLongitude, stationAltitude, groundStationAcceptableElevation, groundStationAcceptableDistance, round, printSecond, outputFileName
    12. **printConstellationISLdeviceInfo**:印出自parameter.txt中的startTime到endTime每秒的衛星間方位角關係及角度差較小的ISL裝置設置角度
        - 需設定parameter: startTime, endTime, ISLrightAngle, ISLleftAngle, outputFileName
    13. **printStationCoverSatsPerSecond**:印出根據parameter.txt設置位置的地面站，一天中的每一秒有哪些衛星是可以連線的
        - 需設定parameter: stationLatitude, stationLongitude, stationAltitude, groundStationAcceptableElevation, groundStationAcceptableDistance, round, outputFileName  
    14. **printDifferentLatitudeCoverTimeOfDay**:印出不同緯度的地面站86400秒中，有幾秒是有被衛星覆蓋的
        - 需設定parameter: stationLongitude, stationAltitude, groundStationAcceptableElevation, groundStationAcceptableDistance, minLatitude, maxLatitude, round, outputFileName
    15. **printDifferentLatitudeConnectedCountOfDay**:印出不同緯度的地面站86400秒中，平均/最小/最大的可連線衛星數量是多少
        - 需設定parameter: stationLongitude, stationAltitude, groundStationAcceptableElevation, groundStationAcceptableDistance, minLatitude, maxLatitude, round, outputFileName
    16. **printGroundStationConnectingInfo**:印出地面站對各個衛星一天中對星群中各個衛星的可連線時間總合，總連線時間最長的衛星，總連線時間最短的衛星，以及各個衛星總連線時間的平均
        - 需設定parameter: stationLatitude, stationLongitude, stationAltitude, groundStationAcceptableElevation, groundStationAcceptableDistance, round, outputFileName
    17. **printAreaConnectingInfo**:印出設定區域(多個地面站)內對各個衛星一天中對星群中各個衛星的可連線時間總合，總連線時間最長的衛星，總連線時間最短的衛星，以及各個衛星總連線時間的平均，多個地面站中只要任一個可以連上，就算那一秒鐘可以連上
        - 需設定parameter: areaStationLatitudes, areaStationLongitudes, areaStationAltitudes, groundStationAcceptableElevation, groundStationAcceptableDistance, round, outputFileName 
    18. **printAreaAllSatConnectionTime**:  印出根據parameter.txt設置的區域(多個地面站)，與星群中每一個衛星一天中有那些時間是可以連線的
        - 需設定parameter: areaStationLatitudes, areaStationLongitudes, areaStationAltitudes, groundStationAcceptableElevation, groundStationAcceptableDistance, round, printSecond, outputFileName
    19. **printRightSatAERdiff**: 印出編號observerId衛星觀察右方衛星一天中的AER差異數值(用於判斷可否連線)到sattrack/output.txt
        - 需設定parameter: observerId, outputFileName, ISLrightAngle
    20. **printLeftSatAERdiff**: 印出編號observerId衛星觀察左方衛星一天中的AER差異數值(用於判斷可否連線)到sattrack/output.txt
        - 需設定parameter: observerId, outputFileName, ISLleftAngle 
    21. **simulateLinkbreakingStatistics**: 模擬計算連結失效率，根據所設置的模擬次數，模擬星群的Link要損壞多少個才會發生連結失效，並將最後的分布統計數據印到所設定的output檔案中
        - 需設定parameter: ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle, TLE_inputFileName, outputFileName, closeLinkSimulateTime, acceptableAzimuthDif, acceptableElevationDif, acceptableRange, time, PAT_time
    22. **simulateSatFailStatistics**: 模擬計算衛星隨機壞掉的連結失效率，根據所設置的模擬次數，模擬星群的衛星要損壞多少個才會發生連結失效，並將最後的分布統計數據印到所設定的output檔案中
        - 需設定parameter: ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle, TLE_inputFileName, outputFileName, satFailSimulateTime, acceptableAzimuthDif, acceptableElevationDif, acceptableRange, time, PAT_time
- 進入到sgp4/sattrack中執行sattrack
    
    ```bash
    ./sattrack
    ```
SGP4 library
============

[![Build Status](https://travis-ci.org/dnwrnr/sgp4.svg?branch=master)](https://travis-ci.org/dnwrnr/sgp4)

License
-------

    Copyright 2017 Daniel Warner

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
