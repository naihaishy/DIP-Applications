#include "gmm.h"

GMM::GMM()
{


}

//初始化EM算法的参数
void GMM::InitParamaters(){
    double alpha = 0.5;//背景建模alpha值
    double std = 20;//初始化标准差
    double var = std*std;//初始化方差
    double lamda = 2.5*1.2;//背景更新参数
}

//void GMM::EStep(){

//}

//void GMM::MStep(){

//}

//void GMM::Expectation(){

//}


//void GMM::Maximization(){

//}
