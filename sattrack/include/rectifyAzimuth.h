#ifndef RECTIF_YAZIMUTH
#define RECTIF_YAZIMUTH
#include <Eigen/Dense>

namespace rectifyAzimuth
{
    //parameter ECI: 衛星的XYZ座標向量
    Eigen::Vector3d getNorthDir(Eigen::Vector3d &ECI);

    //計算v1轉幾度可以到達v2（順時鐘為正），counterClockwiseDir向量的用意是標示哪一個方向是逆時針（右手定則）
    double angleDiff(Eigen::Vector3d &v1, Eigen::Vector3d &v2, Eigen::Vector3d &counterClockwiseDir);
}
#endif