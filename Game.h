#pragma once

#include "Vector2f.h"
#include "Texture.h"
#include <vector>

#include "NeuralNet.h"

struct Asteroid
{
	Point2f position;
	Vector2f velocity;
	float size = 57.5f;
	int hp = 3;

	bool markedForDeletion = false;
};

struct Projectile
{
	Point2f position;
	Vector2f velocity;
	float killTimer = 3.0f;
};

struct Spaceship
{
	int id{ 0 };
	bool didShoot{ false };
	bool didThrust{ false };
	bool didTurnLeft{ false };
	bool didTurnRight{ false };

	Point2f position{};
	Vector2f velocity{};
	float rotation{ 45.f };
	float size{ 12.5f };
	
	float flameTimer{ 0 };
	float shotTimer{ 0 };

	std::vector<Projectile> projectiles{};

	float timeOut{ 0 };
	float rotationSpeed{ 200.f };

	int score{ 0 };
	int bonus{ 0 };
	int lives{ 1 };
};

class Game final
{
public:
	explicit Game( const Window& window );
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game( Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	~Game();

	void Update( float elapsedSec );
	void Draw( ) const;

	// Event handling
	void ProcessKeyDownEvent( const SDL_KeyboardEvent& e );
	void ProcessKeyUpEvent( const SDL_KeyboardEvent& e );
	void ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e );
	void ProcessMouseDownEvent( const SDL_MouseButtonEvent& e );
	void ProcessMouseUpEvent( const SDL_MouseButtonEvent& e );

private:
	// DATA MEMBERS
	const Window m_Window;

	Texture* m_pScoreFont = nullptr;
	Texture* m_pBottomFont = nullptr;
	Texture* m_pGameOverFont = nullptr;

	static const unsigned int m_nrSimulations{ 500 };
	unsigned int m_nrRounds{ 0 };

	std::vector<std::vector<Projectile>> m_Particles;
	std::vector<std::vector<Asteroid>> m_Asteroids;
	std::vector<Spaceship> m_Spaceships;
	std::vector<NeuralNet> m_NeuralNets;
	NeuralNet m_SavedBestNN;
	NeuralNet m_SavedLastNN;
	int m_SavedBestScore{ 0 };

	int m_BestAlivePlayerId{ 0 };
	int m_BestAliveScore{ 0 };

	// FUNCTIONS
	void Initialize(bool skipFirstNN = false);
	void CreateAsteroids(unsigned int nrSimulation);
	void Cleanup( );
	void OverlapWorld(Point2f& position, float size);
	void SpawnParticles(int nrParticles, Point2f position, unsigned int nrSimulation);
	void CalculateNNInput(const Spaceship& spaceship, float* nnInput, unsigned int nrSimulation);
	//void CalculateNNInput(const Spaceship& spaceship);
	void ResetGame(bool skipFirstNN = false);
	void HitAsteroid(Asteroid& asteroid, unsigned int nrSimulation);
	void RotateRight(Spaceship& spaceship, float elapsedSec);
	void RotateLeft(Spaceship& spaceship, float elapsedSec);
	void ThrustForward(Spaceship& spaceship, float elapsedSec);
	void ThrustBackwards(Spaceship& spaceship, float elapsedSec);
	void Shoot(Spaceship& spaceship, float elapsedSec);
	void ClearBackground( ) const;
	void PrepareNextRound();

	void UpdateScore(int score);
};