#ifndef _GRAVITY_HIGH_SCORES_SCREEN_HH_
#define _GRAVITY_HIGH_SCORES_SCREEN_HH_

#include "screen.hh"
#include "label-widget.hh"

class HighScoresScreen : public Screen {
protected:
  vector<int> scores;
  int currentScoreIndex;

  vector<LabelWidget*> labels;

public:
  HighScoresScreen(SDL_Window *window);
  virtual ~HighScoresScreen();

  virtual void SwitchScreen(const map<string, string> &lastState);
  virtual void HandleEvent(const SDL_Event &e);
  virtual void HandleWidgetEvent(int event_type, Widget *widget);

  virtual void Reset();
  virtual void Save(ostream &s) const;
  virtual void Load(istream &s);

  virtual void Advance(float dt);
  virtual void Render(Renderer *renderer);
};


#endif /* _GRAVITY_HIGH_SCORES_SCREEN_HH_ */
