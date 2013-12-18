
#include "game/game.h"
#include "mge/ModularEngine.h"

int main(int argc, char *argv[])
{
printf(__FILE__" %d\n",__LINE__);
    //Later, we'll add support for opening a file when you start the engine
	//Create the engine
	ModularEngine::init(getSDLVideoFlags());
    ModularEngine::get()->setWorldEngine(createWorldEngine());
    ModularEngine::get()->setPhysicsEngine(createPhysicsEngine());
    ModularEngine::get()->setRenderEngine(createRenderEngine());
    ModularEngine::get()->setAudioEngine(createAudioEngine());

    //Build game world
    initWorld();

	//Run the game
	ModularEngine::get()->run();

    //Clean the parts of the game not cleaned by the engines
    cleanWorld();

    cleanPhysicsEngine();
    cleanRenderEngine();
    cleanAudioEngine();
	cleanWorldEngine();
	ModularEngine::clean();
	return 0;
}
