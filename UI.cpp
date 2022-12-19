#include <SFML/Graphics/RenderWindow.hpp>
#include "utility.h"
#include "UI.h"
#include "RAB.h"
#include "Window.hpp"
#include <json/json.h>
#include <fstream>
#include <iostream>
#include "portable-file-dialogs.h"

Json::Value root;
std::ifstream fs;
Json::CharReaderBuilder builder;
Json::StreamWriterBuilder streamBuilder;
JSONCPP_STRING errs;
std::ofstream outputFileStream("config.json");
const std::unique_ptr<Json::StreamWriter> writer(streamBuilder.newStreamWriter());

static int item_current_idx = 0;
static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
static ImGuiInputTextFlags text_flags = ImGuiInputTextFlags_AllowTabInput;
static bool *p_open;
static bool show_app_stack_tool = false;
static std::wstring latestSelected;
static bool fileContextOpen = false;
static int fileNamePos;
static int fileContentPos;
static int fileContentSize;
sf::RenderWindow *window = &Window::getWindow()->getRenderWindow();

UI::Ptr UI::instance{nullptr};
UI::Ptr UI::getUI()
{
    if(!instance)
        instance = std::make_shared<UI>();
    return instance;
}

void UI::setup()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDockingWithShift = true;
    io.ConfigWindowsResizeFromEdges = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

static std::string EXPORTS_DIR = "";
static std::string IMPORTS_DIR = "";
static char EXPORTS_BUF[512] = "";
static char IMPORTS_BUF[512] = "";

void UI::ConfigSetup()
{
    fs.open("config.json", std::ios_base::binary);
    parseFromStream(builder, fs, &root, &errs);
    EXPORTS_DIR = root["export_dir"].asString();
    IMPORTS_DIR = root["input_dir"].asString();
    system("mkdir tmp");
}

void UI::load(float inputTick) {
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGuiIO &io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", p_open, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    static bool packWindow = false;
    static bool exportWindow = false;
    static bool pathsWindow = false;
    static bool useExportDirAsImportDir = true;
    static bool hasScanned = false;


    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("Stack Tool", NULL, &show_app_stack_tool);
        if (show_app_stack_tool) { ImGui::ShowStackToolWindow(&show_app_stack_tool); }
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::Selectable("Open RAB"))
            {
                auto pfd = pfd::open_file("Select RAB Archive", ".", {"RAB Archive", "*.rab *.RAB"});
                rabFile = pfd.result();
                if (!rabFile[0].empty())
                {
                    window->setTitle(Window::getWindow()->m_title + " | " + rabFile[0]);
                    std::string tmp = rabFile[0];
                    std::size_t revcut = tmp.find_last_of('/');
                    tmp.erase(tmp.begin(), tmp.begin()+revcut+1);
                    tmp.erase(tmp.end()-4, tmp.end());
                    RAB::getRAB()->SetArchiveName(tmp);
                    archiveName = tmp;
                    RAB::getRAB()->Read(rabFile[0]);
                }
            }
            ImGui::Selectable("Pack RAB", &packWindow);
            ImGui::Selectable("Export RAB", &exportWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            ImGui::Selectable("Paths", &pathsWindow);
            ImGui::EndMenu();
        }
    }
    if(packWindow)
    {
        if(ImGui::Begin("PACK YOUR FILES"), packWindow)
        {
            if(useExportDirAsImportDir)
            {
                ImGui::TextColored(ImVec4(sf::Color::Cyan), "USING EXPORT PATH TO PACK *.RAB");
                if(ImGui::Button("PACK!")) {  }
            }
            else if(IMPORTS_DIR.empty())
            {
                ImGui::TextColored(ImVec4(sf::Color::Red), "IMPORT PATH REQUIRED");
            }
            else
            {
                ImGui::TextColored(ImVec4(sf::Color::Yellow), "USING IMPORT PATH TO PACK *.RAB");
                if(ImGui::Button("PACK!")) {  }
            }
        }
    }
    if(exportWindow)
    {
        if(ImGui::Begin("EXPORT YOUR FILES"), packWindow)
        {
            //if(ImGui::Button("Cancel")) { packWindow = false; }
            if(EXPORTS_DIR.empty()) { ImGui::TextColored(ImVec4(sf::Color::Red), "EXPORT PATH DOESN'T EXIST"); ImGui::Text("\tSettings > Paths"); }
            if(!EXPORTS_DIR.empty())
            {
                ImGui::TextColored(ImVec4(sf::Color::Cyan), "PATH EXISTS!");
                if(ImGui::Button("EXPORT!")) {  }
            }
        }
    }
    if(pathsWindow)
    {
        if(ImGui::Begin("Settings > Paths"), pathsWindow)
        {
            ImGui::Text("Current EXPORT Path: "); if(EXPORTS_DIR.empty()) { ImGui::PushStyleColor(0, ImVec4(sf::Color::Red)); ImGui::SameLine(); ImGui::Text("NO PATH DETECTED!"); ImGui::PopStyleColor();} ImGui::SameLine();
            ImGui::PushStyleColor(0, ImVec4(sf::Color::Red)); ImGui::TextWrapped("%s", EXPORTS_DIR.c_str()); ImGui::PopStyleColor();
            ImGui::Text("EXPORT DIRECTORY");
            ImGui::InputText("###exportPath", EXPORTS_BUF, IM_ARRAYSIZE(EXPORTS_BUF));
            ImGui::SameLine(); if(ImGui::Button("OK###exp")) { EXPORTS_DIR = EXPORTS_BUF; root["export_dir"] = EXPORTS_DIR; writer->write(root, &outputFileStream); root.clear(); }
            ImGui::Text("Current IMPORT Path: "); if(IMPORTS_DIR.empty()) { ImGui::PushStyleColor(0, ImVec4(sf::Color::Red)); ImGui::SameLine(); ImGui::Text("NO PATH DETECTED!"); ImGui::PopStyleColor();} ImGui::SameLine();
            ImGui::PushStyleColor(0, ImVec4(sf::Color::Red)); ImGui::TextWrapped("%s", IMPORTS_DIR.c_str()); ImGui::PopStyleColor();
            ImGui::Text("IMPORT DIRECTORY");
            if(!useExportDirAsImportDir) ImGui::InputText("###importPath", IMPORTS_BUF, IM_ARRAYSIZE(IMPORTS_BUF));
            ImGui::SameLine(); if(ImGui::Button("OK###imp")) { IMPORTS_DIR = IMPORTS_BUF; root["input_dir"] = IMPORTS_DIR; writer->write(root, &outputFileStream); root.clear(); }
            ImGui::Checkbox("Keep same as export directory?", &useExportDirAsImportDir);

        }
    }
    if(ImGui::Begin(("IMG"), (bool*)nullptr))
    {
        ImGui::Image(texture, ImVec2(200, 200));
    }
    if (ImGui::Begin("FILE CONTEXT") && !latestSelected.empty())
    {
        ImGui::Text("%s", WcharToString(latestSelected).c_str());
        std::string cc = "Import before [" + std::to_string(fileContentPos) + "]";
        if(ImGui::Button(cc.c_str()))
        {
            auto pfd = pfd::open_file("Select Image", ".", {"Image", "*.png *.PNG *.dds *DDS"});
            std::vector<std::string> image = pfd.result();
            std::string tmp = image[0];
            if (!tmp.empty())
            {
                std::size_t revcut = tmp.find_last_of('/');
                tmp.erase(tmp.begin(), tmp.begin()+revcut+1); //tmp.erase(tmp.end()-4, tmp.end());
                std::string newtmp;
                for(auto c : tmp)
                {
                    newtmp += c;
                    newtmp += '_';
                }
                newtmp += '_';
                newtmp += '_';
                RAB::getRAB()->Insert(image[0], fileContentPos, newtmp, fileNamePos); //dokutah ara ara~ //we need to update the filename table (files are stored in order of name)
                RAB::getRAB()->Scan(); //reload the file *.rab (because we just inserted something duh!)
            }
        }
        cc = "Import after [" + std::to_string(fileContentPos + fileContentSize) + "]";
        if(ImGui::Button(cc.c_str()))
        {

        }
        /*cc = "Reimport";
        if(ImGui::Button(cc.c_str())) {}
        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_LeftAlt))
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            std::string s = "Imports the exported file from it's exported location\n  (use this if you want easy importing of files)";
            ImGui::TextUnformatted(s.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }*/
    }
    if(ImGui::Begin(("Main"), (bool*)nullptr))
    {
        //ImGui::TextWrapped("(Could be wrong)Regardless of the nametable filename ordering, these files are stored in alphabetical order.\n*Keep this in mind when inserting and removing content");
        for(auto& f : RAB::getRAB()->FOLDER)
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode((void*)(intptr_t)f.first.size(), WcharToString(f.first).c_str(), f.first.size()))
            {
                for(int x{0}; x < f.second.size(); ++x)
                {
                    std::string place = WcharToString(f.second[x].filename + L" | C: " + std::to_wstring(f.second[x].ContentStartPos) + L" FN: " + std::to_wstring(f.second[x].NameStartPos) + L"###" + std::to_wstring(x) + f.first);
                    ImGui::Selectable(place.c_str(), f.second[x].selected);
                    if (ImGui::IsItemClicked() && ImGui::IsItemActivated())
                    {
                        if(f.second[x].selected) f.second[x].selected = false; else f.second[x].selected = true;
                        latestSelected = f.second[x].filename;
                        fileNamePos = f.second[x].NameStartPos;
                        fileContentPos = f.second[x].ContentStartPos;
                        fileContentSize = f.second[x].Size;
                        std::wcout << latestSelected << L" \n";
                        std::cout << f.second[x].Size << "\n"; //RAB::getRAB()->FILE[latestSelected].size() << "\n";
                        std::string alttmp = WcharToString(f.second[x].filename);
                        alttmp.erase(alttmp.end()-4, alttmp.end());
                        texture.loadFromFile("tmp/" + archiveName + "/" + WcharToString(f.first) + "/" + alttmp + ".png");
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_LeftAlt))
                    {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        std::string s = "Content POS: " + std::to_string(f.second[x].ContentStartPos);
                        s += "\nName POS: " + std::to_string(f.second[x].NameStartPos);
                        s += "\nCMPL Size: " + std::to_string(f.second[x].Size);
                        ImGui::TextUnformatted(s.c_str());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                }
                ImGui::TreePop();
            }
        }
    }
}

//FOLDERNAMEfilename*.ext create a std::unordered_map consisting of just this name which is their unique tag
//the unique tag tooltip is shown when the item matching this rule is hovered.