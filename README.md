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
- 目前有實作以下5個function
    - **printAllSatNeighborId** : Terminal印出每一個衛星的四個連線鄰居衛星編號
    - **printAERfile** : 印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到sattrack/output.txt中
        - 需設定parameter: observerId、otherId
    - **printRightConnectabilityFile**: 印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
        - 需設定parameter: observerId、acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLrightAngle
    - **printLeftConnectabilityFile**: 印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
        - 需設定parameter: observerId、acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLleftAngle
    - **printAllSatConnectionInfoFile**: 印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到./outputFile/資料夾中，檔名會是`acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt`
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、ISLrightAngle、ISLleftAngle
        - 此function需計算約5分鐘才會輸出結果至檔案中
    - **printAvgAvailableTimeFile**:  印出某個特定時刻，行星群的連線狀態(116*116的二維陣列，可以連的話填上距離，不可連的話填上0，自己連自己也是填0)到sattrack/output.txt中
        - 需設定parameter: acceptableAzimuthDif、acceptableElevationDif、acceptableRange、PAT_time、time、ISLrightAngle、ISLleftAngle(目前是當成前後一定可以連線)
        - 此function需計算約60分鐘才會輸出結果至檔案中
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
