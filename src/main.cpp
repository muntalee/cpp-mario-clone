#include <SFML/Graphics.hpp>
#include "GameEngine.h"

int main()
{
    GameEngine g("assets/assets.txt");
    g.run();

    return 0;
}
