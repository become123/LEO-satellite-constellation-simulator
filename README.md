LEO satellite simulator
============
- 衛星位置的預測是base on此C++ SGP4 project
   - https://www.danrw.com/sgp4/
- 有任何問題可以透過email聯絡我: a0981695482@gmail.com
### 編譯流程

---

- 需先安裝修正方位角時會用到的C++ Eigen library:
    
    ```bash
    sudo apt install libeigen3-dev
    ```
    
- 進入到檔案位置sgp4/中進行cmake來產生Makefile，在sgp4/中輸入指令:
    
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
        - 需設定parameter: observerId、otherId
    3. **printRightConnectabilityFile**: 印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
        - 需設定parameter: observerId、acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLrightAngle
    4. **printLeftConnectabilityFile**: 印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
        - 需設定parameter: observerId、acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLleftAngle
    5. **printAllIslConnectionInfoFile**: 印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到./outputFile/資料夾中，檔名會是`acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt`
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLrightAngle、ISLleftAngle
        - 此function需計算約5分鐘才會輸出結果至檔案中
    6. **compareDifferentAcceptableAzimuthDif**: 印出從方位角誤差(acceptableAzimuthDif)80、85、90、...、170、175，所有衛星的左方與右方ISL一天中可連線總時間平均到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、PAT_time、time、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
        - 此function需計算約110分鐘才會輸出結果至檔案中
    7. **compareDifferentPAT_time**: 印出從方PAT_time從5、10、15、...、到60，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
    8. **printBreakingConnectingStatus**: 印出整個衛星群的所有inter ISL的breaking及connecting time到sattrack/output.txt中，output每列為"time (sat1Id, sat2Id) 1或0"，1或0表connecting或breaking
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLrightAngle、ISLleftAngle、PAT_time
    9. **printConstellationStateFile**: 印出某個特定時刻，行星群的連線狀態(112*112)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、PAT_time、time、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
    10. **adjustableISLdevice_printSatellitesDeviceStateOfDay**: 印出所有衛星(相鄰的會排在一起)，一天86400秒的ISL設置state
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、time、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
    11. **printConstellationHopCountFile**: 印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、PAT_time、time、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
    12. **printConstellationHopCountFileAndOutputCertainPath**: 印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、PAT_time、time、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
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
