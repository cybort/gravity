#include "entity.hh"
#include "helpers.hh"

#include <exception>
#include <iostream>

using namespace std;

Entity::Entity() :
  hasPhysics(false),
  body(nullptr),
  hasGravity(false),
  gravityCoeff(0.0),
  hasTrail(false),
  isAffectedByGravity(false),
  isSun(false),
  isPlanet(false),
  isEnemy(false),
  isCollectible(false),
  hasScore(false),
  score(0.0)
{
}

Entity::~Entity() {

}

void Entity::SaveBody(const b2Body *b, ostream &s) const {
  s << (b ? 1 : 0)
    << b->GetType()
    << b->GetPosition()
    << b->GetAngle()
    << b->GetLinearVelocity()
    << b->GetAngularVelocity();

  int fixtureCount = 0;
  for (auto f = b->GetFixtureList(); f; f = f->GetNext())
    fixtureCount++;

  s << fixtureCount;
  for (auto f = b->GetFixtureList(); f; f = f->GetNext()) {
    s << f->GetFriction()
      << f->GetDensity()
      << f->GetRestitution();

    b2CircleShape *shape = (b2CircleShape*) f->GetShape();
    if (shape->GetType() != b2Shape::e_circle)
      throw runtime_error("Only circle shapes are currently supported.");
    s << shape->m_p << shape->m_radius;
  }
}

void Entity::SaveTrail(const Trail &t, ostream &s) const {
  s << t.size << t.time;
  s << t.points.size();
  for (auto it = t.points.begin(); it != t.points.end(); ++it) {
    s << it->pos << it->time;
  }
}

b2Body *Entity::LoadBody(istream &s, b2World *world) {
  int bodyExists;
  s >> bodyExists;
  if (!bodyExists)
    return nullptr;

  b2BodyDef bd;
  int bdtype;
  s >> bdtype
    >> bd.position
    >> bd.angle
    >> bd.linearVelocity
    >> bd.angularVelocity;
  bd.type = (b2BodyType) bdtype;
  this->body = world->CreateBody(&bd);

  int fixtureCount;
  s >> fixtureCount;
  for (int i = 0; i < fixtureCount; ++i) {
    b2FixtureDef fd;
    s >> fd.friction
      >> fd.density
      >> fd.restitution;

    b2CircleShape shape;
    s >> shape.m_p >> shape.m_radius;
    fd.shape = &shape;

    this->body->CreateFixture(&fd);
  }

  this->body->SetUserData(this);
}

Trail Entity::LoadTrail(istream &s) {
  Trail t;
  s >> t.size >> t.time;
  size_t pointCount;
  s >> pointCount;
  for (int i = 0; i < pointCount; ++i) {
    TrailPoint tp;
    s >> tp.pos >> tp.time;
    t.points.push_back(tp);
  }
  return t;
}

void Entity::Save(ostream &s) const {
  s << hasPhysics;
  this->SaveBody(this->body, s);

  s << this->hasGravity << this->gravityCoeff;

  s << this->hasTrail;
  this->SaveTrail(this->trail, s);

  s << this->isAffectedByGravity
    << this->isSun
    << this->isPlanet;
}

void Entity::Load(istream &s, b2World *world) {
  s >> this->hasPhysics;
  this->body = this->LoadBody(s, world);

  s >> this->hasGravity >> this->gravityCoeff;

  s >> this->hasTrail;
  this->trail = this->LoadTrail(s);

  s >> this->isAffectedByGravity
    >> this->isSun
    >> this->isPlanet;
}

Entity *Entity::CreatePlanet(b2World *world,
                             b2Vec2 pos,
                             float32 radius,
                             float32 density)
{
  Entity *e = new Entity;
  e->hasPhysics = true;
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position = pos;
  e->body = world->CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = radius;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = density;
  e->body->CreateFixture(&fd);

  e->hasTrail = true;
  e->trail.size = 30;
  e->trail.time = 1.0;

  e->hasGravity = false;
  e->isAffectedByGravity = true;
  e->isPlanet = true;
  e->isSun = false;
  e->body->SetUserData(e);

  return e;
}

Entity *Entity::CreateSun(b2World *world,
                          b2Vec2 pos,
                          float32 radius,
                          float32 density,
                          float32 gravityCoeff)
{
  Entity *e = new Entity;
  e->hasPhysics = true;
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position = pos;
  e->body = world->CreateBody(&bd);

  b2CircleShape shape;
  shape.m_p.Set(0.0, 0.0);
  shape.m_radius = radius;

  b2FixtureDef fd;
  fd.shape = &shape;
  fd.friction = 0.5;
  fd.restitution = 0.7;
  fd.density = density;
  e->body->CreateFixture(&fd);

  e->hasGravity = true;
  e->gravityCoeff = gravityCoeff;
  e->isAffectedByGravity = false;
  e->isSun = true;
  e->isPlanet = false;
  e->body->SetUserData(e);

  return e;
}

Entity *Entity::CreateScoreCollectible(b2World *world, b2Vec2 pos) {
  Entity *e = new Entity;
  e->hasPhysics = true;
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position = pos;
  e->body = world->CreateBody(&bd);

  b2PolygonShape shape;
  shape.SetAsBox(0.5, 0.5);

  b2FixtureDef fd;
  fd.shape = &shape;
  e->body->CreateFixture(&fd);

  e->isCollectible = true;
  e->hasScore = true;
  e->score = 100;

  e->body->SetUserData(e);

  return e;
}

Entity *Entity::CreateEnemyShip(b2World *world, b2Vec2 pos, b2Vec2 velocity, float32 angle) {
  Entity *e = new Entity;
  e->hasPhysics = true;
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.position = pos;
  bd.linearVelocity = velocity;
  bd.angle = angle;
  e->body = world->CreateBody(&bd);

  b2PolygonShape shape;
  b2Vec2 vertices[5];
  vertices[0].Set(-2, -2);
  vertices[1].Set(-2, 0);
  vertices[2].Set(0, 2);
  vertices[3].Set(2, 0);
  vertices[4].Set(2, -2);
  shape.Set(vertices, 5);

  b2FixtureDef fd;
  fd.shape = &shape;
  e->body->CreateFixture(&fd);

  e->isEnemy = true;

  e->body->SetUserData(e);

  return e;
}
