#include "entity.hh"
#include "helpers.hh"
#include "resource-cache.hh"

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
  score(0.0),
  isDrawable(false),
  mesh(nullptr)
{
}

Entity::~Entity() {
  delete this->mesh;
}

void Entity::SaveBody(const b2Body *b, ostream &s) const {
  int hasBody = b ? 1 : 0;
  WRITE(hasBody, s);

  auto bType = b->GetType();
  WRITE(bType, s);

  auto bPos = b->GetPosition();
  WRITE(bPos, s);

  auto bAngle = b->GetAngle();
  WRITE(bAngle, s);

  auto bLinVel = b->GetLinearVelocity();
  WRITE(bLinVel, s);

  auto bAngVel = b->GetAngularVelocity();
  WRITE(bAngVel, s);

  int fixtureCount = 0;
  for (auto f = b->GetFixtureList(); f; f = f->GetNext())
    fixtureCount++;

  WRITE(fixtureCount, s);
  for (auto f = b->GetFixtureList(); f; f = f->GetNext()) {
    auto fFriction = f->GetFriction();
    WRITE(fFriction, s);

    auto fDensity = f->GetDensity();
    WRITE(fDensity, s);

    auto fRestitution = f->GetRestitution();
    WRITE(fRestitution, s);

    b2CircleShape *shape = (b2CircleShape*) f->GetShape();
    if (shape->GetType() != b2Shape::e_circle)
      throw runtime_error("Only circle shapes are currently supported.");
    WRITE(shape->m_p, s);
    WRITE(shape->m_radius, s);
  }
}

void Entity::SaveTrail(const Trail &t, ostream &s) const {
  WRITE(t.size, s);
  WRITE(t.time, s);

  size_t pointCount = t.points.size();
  WRITE(pointCount, s);
  for (auto it = t.points.begin(); it != t.points.end(); ++it) {
    WRITE(it->pos, s);
    WRITE(it->time, s);
  }
}

b2Body *Entity::LoadBody(istream &s, b2World *world) {
  int bodyExists;
  READ(bodyExists, s);
  if (!bodyExists)
    return nullptr;

  b2BodyDef bd;
  int bdtype;
  READ(bdtype, s);
  READ(bd.position, s);
  READ(bd.angle, s);
  READ(bd.linearVelocity, s);
  READ(bd.angularVelocity, s);
  bd.type = (b2BodyType) bdtype;
  this->body = world->CreateBody(&bd);

  int fixtureCount;
  READ(fixtureCount, s);
  for (int i = 0; i < fixtureCount; ++i) {
    b2FixtureDef fd;
    READ(fd.friction, s);
    READ(fd.density, s);
    READ(fd.restitution, s);

    b2CircleShape shape;
    READ(shape.m_p, s);
    READ(shape.m_radius, s);
    fd.shape = &shape;

    this->body->CreateFixture(&fd);
  }

  this->body->SetUserData(this);
}

Trail Entity::LoadTrail(istream &s) {
  Trail t;
  READ(t.size, s);
  READ(t.time, s);

  size_t pointCount;
  READ(pointCount, s);
  for (int i = 0; i < pointCount; ++i) {
    TrailPoint tp;
    READ(tp.pos, s);
    READ(tp.time, s);
    t.points.push_back(tp);
  }
  return t;
}

void Entity::Save(ostream &s) const {
  WRITE(this->hasPhysics, s);
  if (this->hasPhysics)
    this->SaveBody(this->body, s);

  WRITE(this->hasGravity, s);
  WRITE(this->gravityCoeff, s);

  WRITE(this->hasTrail, s);
  if (this->hasTrail)
    this->SaveTrail(this->trail, s);

  WRITE(this->isAffectedByGravity, s);
  WRITE(this->isSun, s);
  WRITE(this->isPlanet, s);
}

void Entity::Load(istream &s, b2World *world) {
  READ(this->hasPhysics, s);
  if (this->hasPhysics)
    this->body = this->LoadBody(s, world);

  READ(this->hasGravity, s);
  READ(this->gravityCoeff, s);

  READ(this->hasTrail, s);
  if (this->hasTrail)
    this->trail = this->LoadTrail(s);

  READ(this->isAffectedByGravity, s);
  READ(this->isSun, s);
  READ(this->isPlanet, s);
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

  // Create mesh.
  GLfloat vertexData[] = {
    // triangle 1
    /* coord */ -radius, -radius, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ -radius,  radius, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  radius, -radius, /* tex_coord */ 1.0f, 0.0f,

    // triangle 2
    /* coord */ -radius,  radius, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  radius,  radius, /* tex_coord */ 1.0f, 1.0f,
    /* coord */  radius, -radius, /* tex_coord */ 1.0f, 0.0f,
  };

  e->mesh = new Mesh(vertexData, 6, ResourceCache::GetTexture("planet"));
  e->isDrawable = true;

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

  // Create mesh.
  GLfloat vertexData[] = {
    // triangle 1
    /* coord */ -radius, -radius, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ -radius,  radius, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  radius, -radius, /* tex_coord */ 1.0f, 0.0f,

    // triangle 2
    /* coord */ -radius,  radius, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  radius,  radius, /* tex_coord */ 1.0f, 1.0f,
    /* coord */  radius, -radius, /* tex_coord */ 1.0f, 0.0f,
  };

  e->mesh = new Mesh(vertexData, 6, ResourceCache::GetTexture("sun"));
  e->isDrawable = true;

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
  shape.SetAsBox(1.5, 1.5);

  b2FixtureDef fd;
  fd.shape = &shape;
  e->body->CreateFixture(&fd);

  e->isCollectible = true;
  e->hasScore = true;
  e->score = 100;

    // Create mesh.
  GLfloat vertexData[] = {
    // triangle 1
    /* coord */ -1.5f, -1.5f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ -1.5f,  1.5f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  1.5f, -1.5f, /* tex_coord */ 1.0f, 0.0f,

    // triangle 2
    /* coord */ -1.5f,  1.5f, /* tex_coord */ 0.0f, 1.0f,
    /* coord */  1.5f,  1.5f, /* tex_coord */ 1.0f, 1.0f,
    /* coord */  1.5f, -1.5f, /* tex_coord */ 1.0f, 0.0f,
  };

  e->mesh = new Mesh(vertexData, 6, ResourceCache::GetTexture("plus-score"));
  e->isDrawable = true;

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

  // Create mesh.
  GLfloat vertexData[] = {
    // triangle 1
    /* coord */ -2.0f, -2.0f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */ -2.0f,  0.0f, /* tex_coord */ 0.0f, 0.5f,
    /* coord */  0.0f,  2.0f, /* tex_coord */ 0.5f, 1.0f,

    // triangle 2
    /* coord */ -2.0f, -2.0f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */  0.0f,  2.0f, /* tex_coord */ 0.5f, 1.0f,
    /* coord */  2.0f,  0.0f, /* tex_coord */ 1.0f, 0.5f,

    // triangle 3
    /* coord */ -2.0f, -2.0f, /* tex_coord */ 0.0f, 0.0f,
    /* coord */  2.0f,  0.0f, /* tex_coord */ 1.0f, 0.5f,
    /* coord */  2.0f, -2.0f, /* tex_coord */ 1.0f, 0.0f,
  };

  e->mesh = new Mesh(vertexData, 9, ResourceCache::GetTexture("enemy"));
  e->isDrawable = true;

  e->body->SetUserData(e);

  return e;
}
