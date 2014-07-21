#include "main-menu-screen.hh"
#include "button-widget.hh"
#include "label-widget.hh"
#include "image-button-widget.hh"
#include "resource-cache.hh"

MainMenuScreen::MainMenuScreen(SDL_Window *window) :
  Screen(window),
  background(window, ResourceCache::GetTexture("background"))
{
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("new-game"),
                                                0.0, 0.45, 0.08,
                                                TextAnchor::CENTER, TextAnchor::TOP,
                                                {255, 0, 0, 255},
                                                {255, 255, 255, 255}));
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("high-scores"),
                                                0.0, 0.55, 0.08,
                                                TextAnchor::CENTER, TextAnchor::TOP,
                                                {255, 0, 0, 255},
                                                {255, 255, 255, 255}));
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("exit"),
                                                0.0, 0.65, 0.08,
                                                TextAnchor::CENTER, TextAnchor::TOP,
                                                {255, 0, 0, 255},
                                                {255, 255, 255, 255}));
  this->widgets.push_back(new LabelWidget(this,
                                          "Main Menu",
                                          0.0, 0.1, 0.2,
                                          TextAnchor::CENTER, TextAnchor::TOP,
                                          {255, 0, 0}));

  this->Reset();
}

MainMenuScreen::~MainMenuScreen() {

}

void MainMenuScreen::SwitchScreen(const map<string, string> &lastState) {
  this->state.clear();
  this->state["name"] = "menu-ongoing";

  for (auto w : this->widgets)
    w->Reset();
}

void MainMenuScreen::HandleEvent(const SDL_Event &e) {
  for (auto w : this->widgets)
    w->HandleEvent(e);
}

void MainMenuScreen::HandleWidgetEvent(int event_type, Widget *widget) {
  switch (event_type) {
  case BUTTON_CLICK:
    if (widget == this->widgets[0]) { // New Game
      this->state["name"] = "menu-new-game-selected";
    }
    if (widget == this->widgets[1]) { // High Scores
      this->state["name"] = "menu-highscores-selected";
    }
    else if (widget == this->widgets[2]) { // Exit
      this->state["name"] = "menu-exit-selected";
    }

    break;
  }
}

void MainMenuScreen::Reset() {
  this->state.clear();
  this->state["name"] = "menu-ongoing";
}

void MainMenuScreen::Save(ostream &s) const {

}

void MainMenuScreen::Load(istream &s) {

}

void MainMenuScreen::Advance(float dt) {
  for (auto w : this->widgets)
    w->Advance(dt);
}

void MainMenuScreen::Render(Renderer *renderer) {
  this->background.Draw();

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}
