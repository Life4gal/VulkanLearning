#include "VulkanApplication.h"

int main()
{
    VulkanApplication app(800, 600);
    try 
    {
        app.InitInstance();
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    


    _CrtCheckMemory();
	
    return 0;
}
