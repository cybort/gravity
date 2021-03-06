#ifndef _GRAVITY_ENTITY_HH_
#define _GRAVITY_ENTITY_HH_

#include "mesh.hh"

#include <Box2D/Box2D.h>

#include <vector>

using namespace std;

struct TrailPoint {
  TrailPoint() :
    time(0.0)
  {}

  TrailPoint(b2Vec2 pos, float32 time) :
    pos(pos),
    time(time)
  {}

  b2Vec2 pos;
  float32 time;
};

struct Trail {
  Trail() :
    size(0),
    time(0.0)
  {}

  int size;
  float32 time;
  vector<TrailPoint> points;
};

enum class CollectibleType {
  PLUS_SCORE,
  MINUS_SCORE,
  PLUS_TIME,
  MINUS_TIME,
  SPAWN_PLANET,
};

struct Entity {
protected:
  void SaveBody(const b2Body *b, ostream &s) const;
  void SaveTrail(const Trail &t, ostream &s) const;
  b2Body *LoadBody(istream &s, b2World *world);
  Trail LoadTrail(istream &s);

public:
  Entity();
  ~Entity();

  bool hasPhysics;
  b2Body *body;

  bool hasGravity;
  float32 gravityCoeff;

  bool hasTrail;
  Trail trail;

  bool isAffectedByGravity;
  bool isSun;
  bool isEnemy;

  bool isPlanet;
  int planetWhooshChannel;

  bool isCollectible;
  bool hasScore;
  int score;
  bool hasTime;
  int time;
  bool spawnPlanet;

  bool isDrawable;
  Mesh *mesh;

  void Save(ostream &s) const;
  void Load(istream &s, b2World *world);

  static Entity *CreatePlanet(b2World *world,
                              b2Vec2 pos,
                              float32 radius,
                              float32 density,
                              b2Vec2 v0=b2Vec2(0, 0));
  static Entity *CreateSun(b2World *world,
                           b2Vec2 pos,
                           float32 radius,
                           float32 density,
                           float32 gravityCoeff);
  static Entity *CreateCollectible(b2World *world,
                                   b2Vec2 pos,
                                   CollectibleType type);
  static Entity *CreateEnemyShip(b2World *world,
                                 b2Vec2 pos,
                                 b2Vec2 velocity,
                                 float32 angle);
};

#endif /* _GRAVITY_ENTITY_HH_ */
