#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <yaml-cpp/yaml.h>
#include <iostream>
#include <opencv2/core.hpp>

#include "FindHole.h"
#include "FindScissors.h"
#include "WiseComm.h"
#include "HiwinSDK.h"
#include "SPAData.h"
#include "SenseCamera.h"

class Process {
   public:
    Process(const std::string arm_ip, int port, const std::string wise_ip, const std::string yaml_path);
    ~Process();

    int ProcessInit();
    void ProcessRun();

    void CameraProcess();
    void FindScissorsProcess();
    void FindHoleProcess();
    void SuckerProcess();
    void ArmProcess();

    void EasyProcess();

   private:
    SenseData senseData;
    PlanData planData;
    ActData actData;

    //yaml
    YAML::Node Node;
    std::string yamlPath;

    // camera
    SenseCamera camera;

    // findscissor
    FindScissors findScissors;

    // findhole
    FindHole findhole;

    // sucker
    WiseComm wise;
    std::string wiseIp;

    // arm
    HiwinSDK hiwin;
    std::string armIp;
    int armPort;

    // process
    bool keep;
    static void onMouse(int event, int x, int y, int flags, void* ptr);

    struct xyz {
        double x;
        double y;
        double z;
    };

    xyz camera2sucker;
    double camera2suckerFoamDepth;
    // process end

};

#endif

