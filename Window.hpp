#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>

class Window
{
  public:
    typedef std::shared_ptr<Window> Ptr;
    static Ptr instance;
    static Ptr getWindow();

    sf::RenderWindow window;
    sf::RenderWindow *w{nullptr};
  public:
    void setup(bool fullscreen);
    sf::RenderWindow &getRenderWindow();
    int m_offsetX{0};
    int m_offsetY{0};
    std::string m_title = "EDF Visual RAB Editor | ver 1.0.0 (abandoned)";
};
