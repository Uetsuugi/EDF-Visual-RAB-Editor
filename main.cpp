#include <iostream>
#include <fstream>
#include <vector>
#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "utility.h"
#include "CMPLHandler.h"
#include "Window.hpp"
#include "UI.h"

int main() {
    Window::getWindow()->setup(false);
    sf::RenderWindow *window = &Window::getWindow()->getRenderWindow();

    sf::View view_main(sf::FloatRect(-1920.f/2.f, -1080.f/2, 1920.f, 1080.f));
    window->setView(view_main);

    signed int frameLimit{144};
    std::cout << "frame limited to: " << frameLimit << " fps" << std::endl;
    window->setFramerateLimit(frameLimit); //for non-dedicated gpu
    ImGui::SFML::Init(*window);
    UI::getUI()->setup();
    UI::getUI()->ConfigSetup();

    sf::Color bgColour;
    float color[3] = { 0.078f, 0.392f, 0.392f };
    bgColour.r = static_cast<sf::Uint8>(color[0] * 255.f);
    bgColour.g = static_cast<sf::Uint8>(color[1] * 255.f);
    bgColour.b = static_cast<sf::Uint8>(color[2] * 255.f);


    sf::Clock deltaTime;
    while(window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window->close();
            static bool isF11 = false;
            static bool isFullscreen = false;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::F11))
            {
                if(isF11 == true && isFullscreen == false)
                {
                    Window::getWindow()->setup(true);
                    isF11 = false;
                    isFullscreen = true;
                }
                else if(isF11 == true && isFullscreen == true)
                {
                    Window::getWindow()->setup(false);
                    isF11 = false;
                    isFullscreen = false;
                }
            }
            else
                isF11 = true;
            ImGui::SFML::ProcessEvent(event);
        }
        static float tick = (deltaTime.restart().asSeconds());

        ImGui::SFML::Update(*window, deltaTime.restart());
        UI::getUI()->load(tick);
        window->clear(bgColour);
        ImGui::SFML::Render(*window);
        window->display();
    }

    window->setActive(false);
    ImGui::SFML::Shutdown();

    return 0;
}
