
#include <iostream>
#include <yaml-cpp/yaml.h>

int main() {



    YAML::Node config = YAML::LoadFile("../configs/config.yaml");


    std::cout<< config<<"\n";

    printf("address: %s\n", config["address"].as<std::string>().c_str());
    printf("count_subcarriers: %s\n", config["ofdm_parameters"]["count_subcarriers"].as<std::string>().c_str());
    
    if(config["address11212"])
        printf("address: %s\n", config["address11212"].as<std::string>().c_str());
    

    printf("End\n");
}




















