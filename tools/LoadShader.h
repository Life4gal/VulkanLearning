#ifndef VULKANLEARNING_LOADSHADER_H
#define VULKANLEARNING_LOADSHADER_H

#include <vector>
#include <fstream>

std::vector<char> loadSpv(std::string filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw std::runtime_error("Failed to open file!");
    }

    auto fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, std::vector<char> code){
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

VkShaderModule createShaderModuleFromFile(VkDevice device, std::string filename){
    return createShaderModule(device, loadSpv(filename));
}

#endif //VULKANLEARNING_LOADSHADER_H
