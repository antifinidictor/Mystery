#ifndef GAME_H
#define GAME_H

class AudioEngine;
class RenderEngine;
class PhysicsEngine;
class WorldEngine;

WorldEngine   *createWorldEngine();
PhysicsEngine *createPhysicsEngine();
RenderEngine  *createRenderEngine();
AudioEngine   *createAudioEngine();
void cleanWorldEngine();
void cleanPhysicsEngine();
void cleanRenderEngine();
void cleanAudioEngine();

int getSDLVideoFlags();

void buildWorld();

#endif
