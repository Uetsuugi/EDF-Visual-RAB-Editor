//
// Created by ruko on 18/08/22.
//

#ifndef TESTING_UI_H
#define TESTING_UI_H


#include <imgui.h>
#include <memory>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

class UI
{
public:
    typedef std::shared_ptr<UI> Ptr;
    static Ptr instance;
    static Ptr getUI();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    std::vector<std::string> rabFile;
    std::string archiveName;
    sf::Texture texture;

public:
    void load(float inputTick);
    void setup();
    void ConfigSetup();

};


#endif //TESTING_UI_H
