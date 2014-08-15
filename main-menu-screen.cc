#include "main-menu-screen.hh"
#include "button-widget.hh"
#include "label-widget.hh"
#include "number-widget.hh"
#include "resource-cache.hh"
#include "helpers.hh"

MainMenuScreen::MainMenuScreen(SDL_Window *window) :
  Screen(window),
  background(window, ResourceCache::GetTexture("background"))
{
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("new-game"),
                                                0.1, 0.15, 0.08,
                                                TextAnchor::RIGHT, TextAnchor::TOP,
                                                {255, 0, 0, 128},
                                                {255, 255, 255, 128}));
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("high-scores"),
                                                0.0, 0.3, 0.08,
                                                TextAnchor::CENTER, TextAnchor::TOP,
                                                {255, 0, 0, 128},
                                                {255, 255, 255, 128}));
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("exit"),
                                                0.1, 0.45, 0.08,
                                                TextAnchor::LEFT, TextAnchor::TOP,
                                                {255, 0, 0, 128},
                                                {255, 255, 255, 128}));
  this->muteButton = new ImageButtonWidget(this,
                                           ResourceCache::GetTexture("mute"),
                                           0.05, 0.05, 0.08,
                                           TextAnchor::LEFT, TextAnchor::BOTTOM,
                                           {255, 128, 128, 255},
                                           {255, 255, 255, 128});
  this->widgets.push_back(this->muteButton);
  this->widgets.push_back(new ImageWidget(this,
                                          ResourceCache::GetTexture("splash"),
                                          -0.15, 0.0, 0.6,
                                          TextAnchor::RIGHT, TextAnchor::BOTTOM));

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
    PlaySound("button-click");

    if (widget == this->widgets[0]) { // New Game
      this->state["name"] = "menu-new-game-selected";
    }
    if (widget == this->widgets[1]) { // High Scores
      this->state["name"] = "menu-highscores-selected";
    }
    else if (widget == this->widgets[2]) { // Exit
      this->state["name"] = "menu-exit-selected";
    }
    else if (widget == this->muteButton) { // Toggle Mute
      mute = !mute;
      if (mute)
        this->muteButton->SetTexture(ResourceCache::GetTexture("unmute"));
      else
        this->muteButton->SetTexture(ResourceCache::GetTexture("mute"));
    }

    break;

  case BUTTON_MOUSE_ENTER:
    PlaySound("mouse-over");
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
