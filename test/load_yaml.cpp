
#include <iostream>
#include <yaml-cpp/yaml.h>

/*running: /run.sh load_yaml.cpp -lyaml-cpp*/

struct test1{
    double r1;
    double r2;
};

int main() {



    YAML::Node config = YAML::LoadFile("../configs/config.yaml");


    std::cout<< config<<"\n";

    printf("address: %s\n", config["address"].as<std::string>().c_str());
    printf("count_subcarriers: %s\n", config["ofdm_parameters"]["count_subcarriers"].as<std::string>().c_str());
    
    if(config["address11212"])
        printf("address: %s\n", config["address11212"].as<std::string>().c_str());
    


    YAML::Node cfg = config["ofdm_parameters"];

    printf("==========\n");
    std::cout<< cfg<<"\n";
    printf("==========\n");
    test1 t1 = {cfg["pilot"][0].as<double>(),
            cfg["pilot"][1].as<double>()};
    printf("t1: %f %f\n", t1.r1, t1.r2);
    printf("End\n");
}




















