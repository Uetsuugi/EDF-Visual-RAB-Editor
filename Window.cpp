#include "Window.hpp"

Window::Ptr Window::instance{nullptr};
Window::Ptr Window::getWindow()
{
  if(!instance)
  {
    instance = std::make_shared<Window>();
  }
  return instance;
}

void Window::setup(bool fullscreen)
{
   sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  if(fullscreen)
  {
    window.create(sf::VideoMode(1920/2, 1080/2, desktop.bitsPerPixel),m_title, sf::Style::Fullscreen); //sf::Style::Fullscreen fixes the render latency

    sf::View view_main(sf::FloatRect(-1920.f/2.f, -1080.f/2.f, 1920.f, 1080.f));
    view_main.move(m_offsetX, m_offsetY);
    window.setView(view_main);
    window.requestFocus();

    if(!w) w = &window;
    //Engine::Host::getHost()->setWindow(w);
  }
  else
  {
    window.create(sf::VideoMode(1920/2, 1080/2, desktop.bitsPerPixel),m_title, sf::Style::Default);

    sf::View view_main(sf::FloatRect(-1920.f/2.f, -1080.f/2.f, 1920.f, 1080.f));
    view_main.move(m_offsetX, m_offsetY);
    window.setView(view_main);
    window.requestFocus();

    if(!w) w = &window;
  }
}

sf::RenderWindow &Window::getRenderWindow()
{
  if(!w) w = &window;
  return *w;
}
