#include "execution/Evaluator.hpp"
#include <filesystem>

bool Evaluator::evaluate(const std::string& id){
    std::filesystem::path folder_path = std::filesystem::path("../temp")/id;
    std::string folder = std::filesystem::absolute(folder).string();
    std::ifstream outtxt(folder + "/output.txt");
    
    

}