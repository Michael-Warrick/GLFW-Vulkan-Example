#include "application.hpp"

int main() 
{
    Application app;
    try
    {
        app.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 0;
}