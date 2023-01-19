#include "pch.h"
#include "Game.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

Game::Game( const Window& window ) 
	:m_Window{ window },
	m_SavedLastNN { NeuralNet(9, 16, 4) },
	m_SavedBestNN { NeuralNet(9, 16, 4) }
{
	m_pBottomFont = new Texture("2DAE08 - YOMTOV NIR - RL ASTEROIDS", "Resources/Fonts/Hyperspace.otf", 18, Color4f{ 0, 1, 0, 1 });
	m_pGameOverFont = new Texture("GAME OVER", "Resources/Fonts/Hyperspace.otf", 48, Color4f{ 0, 1, 0, 1 });

	Initialize( );
}

Game::~Game( )
{
	Cleanup( );
}

void Game::UpdateScore(int score)
{
	delete m_pScoreFont;
	m_pScoreFont = new Texture(std::to_string(score), "Resources/Fonts/Hyperspace.otf", 32, Color4f{1, 1, 0, 1});
}

void Game::Initialize(bool skipFirstNN)
{
	m_NeuralNets.clear();
	m_Spaceships.clear();
	m_Asteroids.clear();
	m_Particles.clear();

	m_NeuralNets.reserve(m_nrSimulations);
	m_Spaceships.resize(m_nrSimulations);
	m_Asteroids.resize(m_nrSimulations);
	m_Particles.resize(m_nrSimulations);
	for (unsigned int i{ 0 }; i < m_nrSimulations; i++)
	{
		if (skipFirstNN)
		{
			if (i % 2 == 0)
				m_NeuralNets.push_back(m_SavedBestNN);
			else
				m_NeuralNets.push_back(m_SavedBestNN);
				//m_NeuralNets.push_back(m_SavedLastNN);

			if (i > 1)
				m_NeuralNets[i].Mutate(0.1f);
		}
		else
		{
			m_NeuralNets.push_back(NeuralNet(9, 16, 4));
		}

		m_Spaceships[i] = {};
		m_Spaceships[i].id = i;
		m_Spaceships[i].position = Point2f{ m_Window.width / 2.f, m_Window.height / 2.f };

		CreateAsteroids(i);
	}

	UpdateScore(0);
}

void Game::CreateAsteroids(unsigned int nrSimulation)
{
	Asteroid asteroid{};
	asteroid.position = Point2f{ m_Window.width - asteroid.size * 2, m_Window.height - asteroid.size * 2 };
	asteroid.velocity = Vector2f{ 25, 12.5 };
	m_Asteroids[nrSimulation].push_back(asteroid);

	asteroid = {};
	asteroid.position = Point2f{ asteroid.size * 2, m_Window.height - asteroid.size * 2 };
	asteroid.velocity = Vector2f{ -12.5f, 25 };
	m_Asteroids[nrSimulation].push_back(asteroid);

	asteroid = {};
	asteroid.position = Point2f{ asteroid.size * 2, asteroid.size * 2 };
	asteroid.velocity = Vector2f{ 12.5f, -25 };
	m_Asteroids[nrSimulation].push_back(asteroid);

	asteroid = {};
	asteroid.position = Point2f{ m_Window.width - asteroid.size * 2, asteroid.size * 2 };
	asteroid.velocity = Vector2f{ -12.5f, -25 };
	m_Asteroids[nrSimulation].push_back(asteroid);

	asteroid = {};
	asteroid.position = Point2f{ m_Window.width - asteroid.size * 2, asteroid.size * 2 };
	asteroid.velocity = (m_Spaceships[nrSimulation].position - asteroid.position).Normalized() * 25.f;
	m_Asteroids[nrSimulation].push_back(asteroid);

	for (auto& asteroid : m_Asteroids[nrSimulation])
	{
		if (utils::IsOverlapping(Circlef{ m_Spaceships[nrSimulation].position, m_Spaceships[nrSimulation].size }, Circlef{ asteroid.position, asteroid.size }))
		{
			asteroid.markedForDeletion = true;
		}
	}
}

void Game::Cleanup( )
{
	delete m_pBottomFont;
	delete m_pScoreFont;
	delete m_pGameOverFont;
}

void Game::OverlapWorld(Point2f& position, float size)
{
	if (position.x > m_Window.width + size)
	{
		position.x = -size;
	}
	else if (position.x < -size)
	{
		position.x = m_Window.width + size;
	}

	if (position.y > m_Window.height + size)
	{
		position.y = -size;
	}
	else if (position.y < -size)
	{
		position.y = m_Window.height + size;
	}
}

void Game::SpawnParticles(int nrParticles, Point2f position, unsigned int nrSimulation)
{
	Projectile projectile{};
	projectile.killTimer = .75f;
	projectile.position = position;
	for (int i = 0; i < nrParticles; i++)
	{
		int angle = (std::rand() % 361);
		projectile.velocity = (std::rand() % 45) / 100.f * Vector2f{ cos(angle * float(M_PI) / 180.f), sin(angle * float(M_PI) / 180.f) };
		m_Particles[nrSimulation].push_back(projectile);
	}
}

void Game::CalculateNNInput(const Spaceship& spaceship, float* nnInput, unsigned int nrSimulation)
{
	//float nnInput[9] = {};

	Vector2f direction;
	Point2f p1, p2;
	Circlef circle;
	std::vector<Point2f> asteroidVertices;
	for (int i = 0; i < 8; i++)
	{
		direction = Vector2f{ cos(spaceship.rotation * float(M_PI) / 180.f + i * (float(M_PI) / 4)), sin(spaceship.rotation * float(M_PI) / 180.f + i * (float(M_PI) / 4)) };
		p1 = spaceship.position + direction * spaceship.size;
		p2 = p1 + direction * m_Window.width;

		//glBegin(GL_LINES);
		//{
		//	glVertex2f(p1.x, p1.y);
		//	glVertex2f(p2.x, p2.y);
		//}
		//glEnd();

		for (const Asteroid& asteroid : m_Asteroids[nrSimulation])
		{
			circle.center = asteroid.position;
			circle.radius = asteroid.size;
			asteroidVertices.clear();
			utils::GetVerticesFromCircle(circle, asteroidVertices);

			utils::HitInfo hitInfo;
			if (utils::Raycast(asteroidVertices, p1, p2, hitInfo))
			{
				nnInput[i] = 1 / utils::GetDistance(p1, hitInfo.intersectPoint);
			}
		}
	}

	// if spaceship can shoot and there's an asteroid in front of it - set input weight to 1 to make sure it shoots it
	if (spaceship.shotTimer <= 0 && nnInput[0] != 0)
	{
		nnInput[8] = 1;
	}

	////std::cout << "nnInput[0-9] = (";
	//for (int i = 0; i < 9; i++)
	//{
	//	std::cout << nnInput[i] << ", ";
	//}
	//std::cout << "\n";
}

void Game::HitAsteroid(Asteroid& asteroid, unsigned int nrSimulation)
{
	SpawnParticles(10, asteroid.position, nrSimulation);

	Vector2f temp;
	Asteroid asteroid2;
	switch (asteroid.hp)
	{
	case 3:
		asteroid.hp -= 1;
		m_Spaceships[nrSimulation].score += 20;
		//UpdateScore(m_Spaceships[nrSimulation].score);

		asteroid.size /= 1.5f;
		temp = asteroid.velocity;
		asteroid.velocity.x = temp.x * cos(1.57f) - temp.y * sin(1.57f);
		asteroid.velocity.y = temp.x * sin(1.57f) + temp.y * cos(1.57f);
		asteroid.velocity *= 1.25f;

		asteroid2 = asteroid;
		asteroid2.velocity = -asteroid2.velocity;

		asteroid.position += asteroid.velocity.Normalized() * asteroid.size;
		asteroid2.position += asteroid2.velocity.Normalized() * asteroid2.size;

		m_Asteroids[nrSimulation].push_back(asteroid2);
		break;
	case 2:
		asteroid.hp -= 1;
		m_Spaceships[nrSimulation].score += 50;
		//UpdateScore(m_Spaceships[nrSimulation].score);

		asteroid.size /= 2.f;
		temp = asteroid.velocity;
		asteroid.velocity.x = temp.x * cos(1.57f) - temp.y * sin(1.57f);
		asteroid.velocity.y = temp.x * sin(1.57f) + temp.y * cos(1.57f);
		asteroid.velocity *= 1.75f;

		asteroid2 = asteroid;
		asteroid2.velocity = -asteroid2.velocity;

		asteroid.position += asteroid.velocity.Normalized() * asteroid.size;
		asteroid2.position += asteroid2.velocity.Normalized() * asteroid2.size;

		m_Asteroids[nrSimulation].push_back(asteroid2);
		break;
	case 1:
		asteroid.hp -= 1;
		m_Spaceships[nrSimulation].score += 100;
		//UpdateScore(m_Spaceships[nrSimulation].score);

		asteroid.markedForDeletion = true;

		break;
	}
}

void Game::RotateRight(Spaceship& spaceship, float elapsedSec)
{
	spaceship.rotation -= elapsedSec * spaceship.rotationSpeed;

	spaceship.didTurnRight = true;
}

void Game::RotateLeft(Spaceship& spaceship, float elapsedSec)
{
	spaceship.rotation += elapsedSec * spaceship.rotationSpeed;

	spaceship.didTurnLeft = true;
}

void Game::ThrustForward(Spaceship& spaceship, float elapsedSec)
{
	spaceship.velocity += elapsedSec * Vector2f{ cos(spaceship.rotation * float(M_PI) / 180.f), sin(spaceship.rotation * float(M_PI) / 180.f) };

	if (spaceship.flameTimer > 0.075f)
	{
		spaceship.flameTimer = 0;
	}

	spaceship.didThrust = true;
}

void Game::ThrustBackwards(Spaceship& spaceship, float elapsedSec)
{
	spaceship.velocity -= elapsedSec * Vector2f{ cos(spaceship.rotation * float(M_PI) / 180.f), sin(spaceship.rotation * float(M_PI) / 180.f) };
}

void Game::Shoot(Spaceship& spaceship, float elapsedSec)
{
	if (spaceship.shotTimer <= 0)
	{
		spaceship.shotTimer = .666f;
		Projectile projectile{};
		projectile.position = spaceship.position;
		projectile.velocity = spaceship.velocity + 2.5f * Vector2f{ cos(spaceship.rotation * float(M_PI) / 180.f), sin(spaceship.rotation * float(M_PI) / 180.f) };
		spaceship.projectiles.push_back(projectile);
	}

	spaceship.didShoot = true;
}

void Game::Update(float elapsedSec)
{	
	// Check keyboard state
	const Uint8* pStates = SDL_GetKeyboardState(nullptr);

	/*
	if (m_Spaceship.lives > 0) m_Spaceship.timeOut -= elapsedSec;
	if (m_Spaceship.timeOut <= 0)
	{
		if (pStates[SDL_SCANCODE_RIGHT])
		{
			RotateRight(m_Spaceship, elapsedSec);
		}
		if (pStates[SDL_SCANCODE_LEFT])
		{
			RotateLeft(m_Spaceship, elapsedSec);
		}

		if (pStates[SDL_SCANCODE_UP])
		{
			ThrustForward(m_Spaceship, elapsedSec);
		}

		if (pStates[SDL_SCANCODE_DOWN])
		{
			ThrustBackwards(m_Spaceship, elapsedSec);
		}

		if (pStates[SDL_SCANCODE_SPACE])
		{
			Shoot(m_Spaceship, elapsedSec);
		}
	}
	*/

	auto bestSpaceship = std::max_element(m_Spaceships.begin(), m_Spaceships.end(), [](const Spaceship& ss1, const Spaceship& ss2)
		{
			return (ss1.score < ss2.score);
		});
	m_BestAlivePlayerId = bestSpaceship->id;
	m_BestAliveScore = bestSpaceship->score;
	UpdateScore(m_BestAliveScore);

	if (m_BestAliveScore > 1000 && m_BestAliveScore > m_SavedBestScore + 500)
		PrepareNextRound();

	int nrDead{ 0 };
	for (unsigned int s{ 0 }; s < m_nrSimulations; s++)
	{
		if (m_Spaceships[s].lives <= 0)
		{
			nrDead++;
			continue;
		}

		/*
		m_Spaceships[s].timeOut -= elapsedSec;

		if (m_Spaceships[s].timeOut < -10.f)
		{
			if (!m_Spaceships[s].didThrust && !m_Spaceships[s].didTurnLeft || !m_Spaceships[s].didTurnRight)
				m_Spaceships[s].lives = 0;
		}
		*/
		
		m_Spaceships[s].flameTimer += elapsedSec;
		m_Spaceships[s].shotTimer -= elapsedSec;

		m_Spaceships[s].position += m_Spaceships[s].velocity;
		OverlapWorld(m_Spaceships[s].position, m_Spaceships[s].size);

		// NN start
		float nnInput[9] = {};
		CalculateNNInput(m_Spaceships[s], &nnInput[0], s);

		float nnOutput[4] = {};
		m_NeuralNets[s].CalculateOutput(&nnInput[0], &nnOutput[0]);

		//Outputs:
		// 0 = Move Forward
		// 1 = Rotate Left
		// 2 = Rotate Right
		// 3 = Shoot

		if (nnOutput[0] > 0.8f)
		{
			// Move Forward
			ThrustForward(m_Spaceships[s], elapsedSec);
		}

		if (nnOutput[1] > 0.8f)
		{
			// Rotate Left
			RotateLeft(m_Spaceships[s], elapsedSec);
		}
		else if (nnOutput[2] > 0.8f)
		{
			// Rotate Right
			RotateRight(m_Spaceships[s], elapsedSec);
		}
		if (nnOutput[3] > 0.8f)
		{
			// Shoot
			Shoot(m_Spaceships[s], elapsedSec);
		}
		// NN End

		if (m_Asteroids[s].empty())
		{
			CreateAsteroids(s);
		}

		for (auto& projectile : m_Spaceships[s].projectiles)
		{
			projectile.position += projectile.velocity;
			projectile.killTimer -= elapsedSec;
		}
		m_Spaceships[s].projectiles.erase(
			std::remove_if(m_Spaceships[s].projectiles.begin(), m_Spaceships[s].projectiles.end(),
				[](Projectile const& p) {
					return p.killTimer <= 0;
				}
		), m_Spaceships[s].projectiles.end());

		for (auto& particle : m_Particles[s])
		{
			particle.position += particle.velocity;
			particle.killTimer -= elapsedSec;
		}
		m_Particles[s].erase(
			std::remove_if(m_Particles[s].begin(), m_Particles[s].end(),
				[](Projectile const& p) {
					return p.killTimer <= 0;
				}
		), m_Particles[s].end());

		for (auto& asteroid : m_Asteroids[s])
		{
			asteroid.position += asteroid.velocity * elapsedSec;
			OverlapWorld(asteroid.position, asteroid.size);
		}
		m_Asteroids[s].erase(
			std::remove_if(m_Asteroids[s].begin(), m_Asteroids[s].end(),
				[](Asteroid const& a) {
					return a.markedForDeletion;
				}
		), m_Asteroids[s].end());

		if (m_Spaceships[s].timeOut <= 0)
		{
			for (auto& asteroid : m_Asteroids[s])
			{
				if (utils::IsOverlapping(Circlef{ m_Spaceships[s].position, m_Spaceships[s].size }, Circlef{ asteroid.position, asteroid.size }))
				{
					HitAsteroid(asteroid, s);

					m_Spaceships[s].lives--;
					m_Spaceships[s].timeOut = 3.f;
					m_Spaceships[s].position = Point2f{ m_Window.width / 2, m_Window.height / 2 };
					m_Spaceships[s].rotation = 45;
					m_Spaceships[s].velocity = Vector2f{ 0,0 };

					break;
				}
			}
		}

		for (auto& projectile : m_Spaceships[s].projectiles)
		{
			for (auto& asteroid : m_Asteroids[s])
			{
				if (utils::IsPointInCircle(projectile.position, Circlef{ asteroid.position, asteroid.size }))
				{
					projectile.killTimer = 0;
					HitAsteroid(asteroid, s);
					break;
				}
			}
		}
	}

	if (nrDead == m_nrSimulations)
	{
		PrepareNextRound();
	}
}

void Game::PrepareNextRound()
{
	/*
	for (auto& ss : m_Spaceships)
	{
		if (ss.didThrust)
			ss.bonus += 250;

		if (ss.didTurnLeft)
			ss.bonus += 250;

		if (ss.didTurnRight)
			ss.bonus += 250;

		if (ss.didTurnLeft && ss.didTurnRight)
			ss.bonus += 500;
	}
	*/

	auto bestSpaceship = std::max_element(m_Spaceships.begin(), m_Spaceships.end(), [](const Spaceship& ss1, const Spaceship& ss2)
		{
			return (ss1.score + ss1.bonus) < (ss2.score + ss2.bonus);
		});
	auto bestSpaceshipId = bestSpaceship - m_Spaceships.begin();

	m_SavedLastNN = m_NeuralNets[bestSpaceship->id];

	if (bestSpaceship->score > m_SavedBestScore)
	{
		m_SavedBestScore = bestSpaceship->score + bestSpaceship->bonus;
		m_SavedBestNN = m_NeuralNets[bestSpaceship->id];
	}

	std::cout << "Round #" << ++m_nrRounds << " over! Highest score this round = " << bestSpaceship->score << std::endl; //<< ", ever = " << m_SavedBestScore << std::endl;

	bool skipFirstNN{ true };
	ResetGame(skipFirstNN);
}

void Game::Draw( ) const
{
	ClearBackground( );

	//debug draw spaceship hitbox
	//utils::DrawEllipse(m_Spaceship.position, m_Spaceship.size, m_Spaceship.size, 1.f);

	unsigned int s{ 0 };
	for (s = 0; s < m_nrSimulations; s++)
	{
		if (m_Spaceships[s].lives <= 0 || s == m_BestAlivePlayerId)
		{
			//if (m_Spaceships[s].score > 0) std::cout << "Spaceship#" << s << " score: " << m_Spaceships[s].score << std::endl;
			continue;
			//m_pGameOverFont->Draw(Point2f{ m_Window.width / 2 - m_pGameOverFont->GetWidth() / 2 , m_Window.height / 2 - m_pGameOverFont->GetHeight() / 2 });
		}

		if (m_Spaceships[s].timeOut <= 0)
		{
			glPushMatrix();
			glTranslatef(m_Spaceships[s].position.x, m_Spaceships[s].position.y, 0);
			glRotatef(m_Spaceships[s].rotation, 0, 0, 1);
			//draw spaceship
			glBegin(GL_LINE_LOOP);
			{
				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex2f(m_Spaceships[s].size, 0);
				glVertex2f(-m_Spaceships[s].size, m_Spaceships[s].size);
				glVertex2f(-m_Spaceships[s].size / 1.75f, 0);
				glVertex2f(-m_Spaceships[s].size, -m_Spaceships[s].size);
			}
			glEnd();

			if (m_Spaceships[s].flameTimer == 0)
			{
				glLineWidth(2.f);
				glBegin(GL_LINES);
				{
					glVertex2f(-m_Spaceships[s].size / 1.4f, m_Spaceships[s].size / 2.f);
					glVertex2f(-m_Spaceships[s].size * 1.75f, 0);

					glVertex2f(-m_Spaceships[s].size * 1.75f, 0);
					glVertex2f(-m_Spaceships[s].size / 1.4f, -m_Spaceships[s].size / 2.f);
				}
				glEnd();
				glLineWidth(1.f);
			}
			glPopMatrix();
			glFlush();
		}

		//Draw score
		//m_pScoreFont->Draw(Point2f{ 64, m_Window.height - 64 });

		//Draw lives
		/*
		for (int i = 0; i < m_Spaceships[s].lives; i++)
		{
			glPushMatrix();
			glTranslatef(m_Spaceships[s].size * 2.5f * i + 128 - m_Spaceships[s].size, m_Window.height - 64 - m_Spaceships[s].size, 0);
			glRotatef(90, 0, 0, 1);

			glBegin(GL_LINE_LOOP);
			{
				glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
				glVertex2f(m_Spaceships[s].size, 0);
				glVertex2f(-m_Spaceships[s].size, m_Spaceships[s].size);
				glVertex2f(-m_Spaceships[s].size / 1.75f, 0);
				glVertex2f(-m_Spaceships[s].size, -m_Spaceships[s].size);
			}
			glEnd();
			glPopMatrix();
		}
		glFlush();
		*/

		for (auto& asteroid : m_Asteroids[s])
		{
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			utils::DrawEllipse(asteroid.position, asteroid.size, asteroid.size, 1.f);
		}

		for (auto& projectile : m_Spaceships[s].projectiles)
		{
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			utils::DrawPoint(projectile.position, 2.5f);
		}

		for (auto& particle : m_Particles[s])
		{
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			utils::DrawPoint(particle.position, 2.5f);
		}
	}

	//Draw bottom text
	m_pBottomFont->Draw(Point2f{ m_Window.width / 2 - m_pBottomFont->GetWidth() / 2 , 32 });

	s = m_BestAlivePlayerId;
	if (m_Spaceships[s].lives > 0)
	{
		glPushMatrix();
		glTranslatef(m_Spaceships[s].position.x, m_Spaceships[s].position.y, 0);
		glRotatef(m_Spaceships[s].rotation, 0, 0, 1);
		//draw spaceship
		glBegin(GL_LINE_LOOP);
		{
			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex2f(m_Spaceships[s].size, 0);
			glVertex2f(-m_Spaceships[s].size, m_Spaceships[s].size);
			glVertex2f(-m_Spaceships[s].size / 1.75f, 0);
			glVertex2f(-m_Spaceships[s].size, -m_Spaceships[s].size);
		}
		glEnd();

		if (m_Spaceships[s].flameTimer == 0)
		{
			glLineWidth(2.f);
			glBegin(GL_LINES);
			{
				glVertex2f(-m_Spaceships[s].size / 1.4f, m_Spaceships[s].size / 2.f);
				glVertex2f(-m_Spaceships[s].size * 1.75f, 0);

				glVertex2f(-m_Spaceships[s].size * 1.75f, 0);
				glVertex2f(-m_Spaceships[s].size / 1.4f, -m_Spaceships[s].size / 2.f);
			}
			glEnd();
			glLineWidth(1.f);
		}
		glPopMatrix();
		glFlush();
	

		//Draw score
		//m_pScoreFont->Draw(Point2f{ 64, m_Window.height - 64 });

		for (auto& asteroid : m_Asteroids[s])
		{
			glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

			utils::DrawEllipse(asteroid.position, asteroid.size, asteroid.size, 1.f);
		}

		for (auto& projectile : m_Spaceships[s].projectiles)
		{
			glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

			utils::DrawPoint(projectile.position, 2.5f);
		}

		for (auto& particle : m_Particles[s])
		{
			glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

			utils::DrawPoint(particle.position, 2.5f);
		}
	}

	m_pScoreFont->Draw(Point2f{ 64, m_Window.height - 64 });
}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	//std::cout << "KEYDOWN event: " << e.keysym.sym << std::endl;
}

void Game::ResetGame(bool skipFirstNN)
{
	Initialize(skipFirstNN);
}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{
	//std::cout << "KEYUP event: " << e.keysym.sym << std::endl;
	switch (e.keysym.sym)
	{
	case SDLK_r:
		//ResetGame();
		PrepareNextRound();
		break;
	}
	//case SDLK_RIGHT:
	//	//std::cout << "`Right arrow key released\n";
	//	break;
	//case SDLK_1:
	//case SDLK_KP_1:
	//	//std::cout << "Key 1 released\n";
	//	break;
	//}
}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONDOWN event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONUP event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ClearBackground( ) const
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}