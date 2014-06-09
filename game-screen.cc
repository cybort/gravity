#include "game-screen.hh"
#include "entity.hh"

#include <sstream>
#include <iomanip>
#include <functional>

using std::placeholders::_1;

using namespace std;

ContactListener::ContactListener(GameScreen *screen) :
  screen(screen),
  inContact(false)
{}

void ContactListener::BeginContact(b2Contact *contact) {
  if (!this->inContact)
    this->screen->timeRemaining -= 10;
  this->inContact = true;
}

void ContactListener::EndContact(b2Contact *contact) {
  this->inContact = false;
}

b2Body *GetBodyFromPoint(b2Vec2 p, b2World *world) {
  for (b2Body *b = world->GetBodyList(); b; b = b->GetNext()) {
    for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) {
      if (f->TestPoint(p))
        return b;
    }
  }

  return nullptr;
}

GameScreen::GameScreen(SDL_Window *window) :
  Screen(window),
  world(b2Vec2(0.0, 0.0)),
  timer(bind(&GameScreen::TimerCallback, this, _1)),
  contactListener(this)
{
  this->timer.Set(1.0, true);

  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position.Set(0.0, 0.0);
  b2Body *body = world.CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = 6.0;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = 1000;
  b2Fixture *fixture = body->CreateFixture(&fd);

  Entity *e = new Entity { body, true, 130000.0 };
  body->SetUserData(e);

  bd.type = b2_dynamicBody;
  bd.position.Set(20.0, 20.0);
  body = world.CreateBody(&bd);

  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = 2.0;

  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = 1.0;
  fixture = body->CreateFixture(&fd);

  e = new Entity { body, false, 0.0 };
  body->SetUserData(e);

  // trail
  this->trail.size = 30;
  this->trail.time = 1.0;
  this->trail.body = world.GetBodyList();

  this->world.SetContactListener(&contactListener);

  this->Reset();
}

GameScreen::~GameScreen() {

}

void GameScreen::HandleEvent(const SDL_Event &e) {
  int x, y;

  if (e.type == SDL_MOUSEBUTTONDOWN) {
    if (e.button.button == SDL_BUTTON_LEFT) {
      SDL_GetMouseState(&x, &y);
      b2Vec2 p = this->camera.PointToWorld(x, y, this->window);
      b2Body *b = GetBodyFromPoint(p, &this->world);
      if (b) {
        Entity *e = (Entity*) b->GetUserData();
        if (e->isGravitySource) {
          this->draggingBody = b;
          this->draggingOffset = p - b->GetPosition();
        }
      }
    }
  }
  else if (e.type == SDL_MOUSEMOTION) {
    if (this->draggingBody) {
      SDL_GetMouseState(&x, &y);
      b2Vec2 p = this->camera.PointToWorld(x, y, this->window);
      this->draggingBody->SetTransform(p - this->draggingOffset, 0.0);
    }
  }
  else if (e.type == SDL_MOUSEBUTTONUP) {
    if (e.button.button == SDL_BUTTON_LEFT) {
      this->draggingBody = nullptr;
    }
  }
  else if (e.type == SDL_KEYDOWN) {
    switch (e.key.keysym.sym) {
    case SDLK_q:
      SDL_Event quitEvent;
      quitEvent.type = SDL_QUIT;
      SDL_PushEvent(&quitEvent);
      break;
    case SDLK_p:
      this->paused = !this->paused;
      Timer::TogglePauseAll();
      break;
    case SDLK_n:
      this->stepOnce = true;
      break;
    case SDLK_f:
      auto flags = SDL_GetWindowFlags(window);
      if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(window, SDL_FALSE);
      else
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
      break;
    }
  }
  else if (e.type == SDL_WINDOWEVENT) {
    if (e.window.event == SDL_WINDOWEVENT_RESIZED)
      this->FixCamera(this->world.GetBodyList());
  }
}

void GameScreen::Reset() {
  this->score = 0;
  this->timeRemaining = 120;
  this->paused = true;
  this->startTime = SDL_GetTicks() / 1000.0;

  this->camera.pos.Set(-50.0, -50.0);
  this->camera.ppm = 10.0;

  this->draggingBody = nullptr;
  this->stepOnce = false;

  Timer::PauseAll();
  this->FixCamera(this->world.GetBodyList());
}

void GameScreen::Save(ostream &s) const {
  s << this->score
    << this->timeRemaining
    << this->paused
    << this->camera.pos.x
    << this->camera.pos.y
    << this->camera.ppm;
}

void GameScreen::Load(istream &s) {
  s >> this->score
    >> this->timeRemaining
    >> this->paused
    >> this->camera.pos.x
    >> this->camera.pos.y
    >> this->camera.ppm;

  this->startTime = SDL_GetTicks() / 1000.0;

  if (this->paused)
    Timer::PauseAll();
  else
    Timer::UnpauseAll();
}

void GameScreen::Advance(float dt) {
  if (this->paused && !this->stepOnce)
    return;

  Timer::CheckAll();

  this->FixCamera(this->world.GetBodyList());

  // Update the trails.
  UpdateTrail(world.GetBodyList(), &this->trail, SDL_GetTicks() / 1000.0 - this->startTime);

  // Update score.
  auto body = this->world.GetBodyList();
  auto sun = body->GetNext();
  auto v = body->GetLinearVelocityFromWorldPoint(body->GetPosition()).Length();
  auto d = (body->GetPosition() - sun->GetPosition()).Length();
  if (d > 100) d = 0;
  int diff = v * d / 100;
  this->score += diff;

  // Apply forces.
  vector<Entity*> gravitySources;

  for (b2Body *b = this->world.GetBodyList(); b; b = b->GetNext()) {
    Entity *e = (Entity*) b->GetUserData();
    if (e->isGravitySource) {
      gravitySources.push_back(e);
    }
  }

  for (b2Body *b = this->world.GetBodyList(); b; b = b->GetNext()) {
    b2Vec2 gravity;
    Entity *e = (Entity*) b->GetUserData();
    if (!e->isGravitySource) {
      gravity.Set(0.0, 0.0);
      for (auto e : gravitySources) {
        b2Vec2 n = e->body->GetPosition() - b->GetPosition();
        float32 r2 = n.LengthSquared();
        n.Normalize();
        gravity += e->gravityCoeff / r2 * n;
      }

      b->ApplyForce(gravity, b->GetWorldCenter(), true);
    }

    // Advance physics.
    this->world.Step(dt, 10, 10);
    this->stepOnce = false;
  }
}

void GameScreen::Render(Renderer *renderer) const {
  renderer->SetCamera(this->camera);

  renderer->DrawBackground();
  renderer->DrawGrid();

  renderer->DrawTrail(&this->trail);

  for (const b2Body *b = this->world.GetBodyList(); b; b = b->GetNext()) {
    renderer->DrawEntity((Entity*) b->GetUserData());
  }

  // Draw HUD.

  // Draw origin.
  renderer->DrawLine(b2Vec2(0, 1), b2Vec2(0, -1), 255, 0, 0, 255);
  renderer->DrawLine(b2Vec2(1, 0), b2Vec2(-1, 0), 255, 0, 0, 255);

  // Draw score.
  stringstream ss;
  ss << setw(6) << setfill('0') << this->score;
  renderer->DrawText(ss.str(), SDL_Color {0, 0, 0, 128}, 10, 10, false, true);

  // Draw time.
  int minutes = this->timeRemaining / 60;
  int seconds = this->timeRemaining % 60;
  stringstream ss2;
  ss2 << setw(2) << setfill('0') << minutes
      << setw(0) << ":"
      << setw(2) << setfill('0') << seconds;
  renderer->DrawText(ss2.str(), SDL_Color {0, 0, 0, 128}, 10, 10);

  renderer->PresentScreen();
}

void GameScreen::FixCamera(b2Body *body) {
  const float32 min_width = 150;
  const float32 min_height = 75;
  const float32 max_width = 300;
  const float32 max_height = 150;

  int winw, winh;
  SDL_GetWindowSize(this->window, &winw, &winh);
  float32 ratio = ((float32) winw) / winh;

  float32 width, height;

  b2Vec2 upper(this->camera.pos.x + winw / this->camera.ppm,
               this->camera.pos.y + winh / this->camera.ppm);
  b2Vec2 lower(this->camera.pos.x, this->camera.pos.y);

  auto r = body->GetFixtureList()->GetShape()->m_radius;

  auto pos = body->GetPosition();

  float32 maxx, maxy, minx, miny;

  if (pos.x + r + 2 > upper.x ||
      pos.y + r + 2 > upper.y ||
      pos.x - r - 2 < lower.x ||
      pos.y - r - 2 < lower.y)
  {
    maxx = max(pos.x + r + 2, upper.x);
    maxy = max(pos.y + r + 2, upper.y);
    minx = min(pos.x - r - 2, lower.x);
    miny = min(pos.y - r - 2, lower.y);
  }
  else {
    maxx = min(pos.x + r + 2, upper.x);
    maxy = min(pos.y + r + 2, upper.y);
    minx = max(pos.x - r - 2, lower.x);
    miny = max(pos.y - r - 2, lower.y);
  }

  auto halfx = max(abs(maxx), abs(minx));
  auto halfy = max(abs(maxy), abs(miny));

  auto width1 = 2 * halfx;
  auto height1 = width1 / ratio;

  auto height2 = 2 * halfy;
  auto width2 = height2 * ratio;

  if (width1 > width2) {
    width = width1;
    height = height1;
  }
  else {
    width = width2;
    height = height2;
  }

  if (width > max_width) {
    width = max_width;
    height = width / ratio;
  }
  if (width < min_width) {
    width = min_width;
    height = width / ratio;
  }
  if (height > max_height) {
    height = max_height;
    width = height * ratio;
  }
  if (height < min_height) {
    height = min_height;
    width = height * ratio;
  }

  this->camera.pos.x = - (width / 2.0);
  this->camera.pos.y = - (height / 2.0);

  this->camera.ppm = winw / width;
}

void GameScreen::UpdateTrail(b2Body *b, Trail *t, float32 currentTime) {
  // Remove all the points not in the desired time window.
  t->points.erase(remove_if(t->points.begin(), t->points.end(),
                            [=](const TrailPoint &p) -> bool {
                              return p.time < currentTime - t->time;
                            }),
                  t->points.end());

  // Add current position to the trail.
  t->points.push_back(TrailPoint { b->GetPosition(), currentTime });
}

void GameScreen::TimerCallback(float elapsed) {
  if (this->timeRemaining > 0)
    this->timeRemaining--;
}