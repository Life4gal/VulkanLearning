#### Vulkan的学习记录

<del>又是一个三分钟热度的库</del>

目前在看 [vulkan-tutorial.com](https://vulkan-tutorial.com/) 的教程

#### 这个库使用的开发环境:

* <del>Visual Studio2019</del>
* <del>CXX17 标准</del>
* <del>[VulkanSDK-1.2.154.1](https://vulkan.lunarg.com/sdk/home#sdk/downloadConfirm/1.2.154.1/windows/VulkanSDK-1.2.154.1-Installer.exe)</del>
* <del>[GLM 0.9.9.8](https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.7z)</del>
* <del>[glfw-3.3.2.bin.WIN64](https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip)</del>
* <del>Windows操作系统</del>


* CLion
* CXX17 标准
* Pop-OS(Linux)
```bash
sudo apt install vulkan-tools
sudo apt install libvulkan-dev
sudo apt install vulkan-validationlayers-dev spirv-tools

# vkcube should work well
vkcube

# GLFW
sudo apt install libglfw3-dev

# GLM
sudo apt install libglm-dev

# glslc
# https://storage.googleapis.com/shaderc/badges/build_link_linux_clang_release.html
cp ./install/bin/glslc /use/local/bin/glslc
```

