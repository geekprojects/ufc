#include <iostream>
#include <SDL.h>


#include "display.h"


int main()
{
    //float roll = glm::radians(5.0);
    XPFlightDisplay display;
    display.init();
    bool running = true;
    while (running)
    {
        display.draw();

        SDL_Event event;
        int res = SDL_PollEvent(&event);
        if (res)
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
            }
        }
        //SDL_Delay(50);
    }

    display.close();

    //SDL_DestroyRenderer(renderer);
    return 0;
}
