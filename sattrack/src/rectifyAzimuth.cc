#include "rectifyAzimuth.h"

namespace rectifyAzimuth
{
    //parameter ECI: 衛星的XYZ座標向量
    Eigen::Vector3d getNorthDir(Eigen::Vector3d &ECI){
        Eigen::Vector3d earthCenter(0,0,0); //地球質心、ECI原點
        Eigen::Vector3d zDirection(0,0,1); //Z軸、地球北極方向

        Eigen::ParametrizedLine<double, 3> earthCenterToSatelliteLine(earthCenter, ECI.normalized());//地心指向衛星當前位置的三維直線	
        Eigen::Vector3d projectedPoint = earthCenterToSatelliteLine.projection(zDirection);

        Eigen::Vector3d northDir = zDirection-projectedPoint;
        // cout<<northDir.transpose();
        return northDir;
    }


    //計算v1轉幾度可以到達v2（順時鐘為正），counterClockwiseDir向量的用意是標示哪一個方向是逆時針（右手定則）
    double angleDiff(Eigen::Vector3d &v1, Eigen::Vector3d &v2, Eigen::Vector3d &counterClockwiseDir){ 
        // cout<<v1.transpose()<<"\n"<<v2.transpose()<<"\n";
        const Eigen::Vector3d crossVector = v1.cross(v2);//用來檢查夾角是否超過180度(超過的話，cos-1算出來的銳角要再轉換成鈍角)

        double dot = v1.dot(v2); 
        double len1 = v1.norm();
        double len2 = v2.norm();
        double angle = acos(dot/(len1 * len2))*180/M_PI; //兩向量所夾銳角

        //若v1 cross v2的向量方向是逆時鐘，代表角度超過180度
        return crossVector.dot(counterClockwiseDir) > 0 ? 360-angle: angle;
    }
}
