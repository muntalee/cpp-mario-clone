#include "Game.h"
#include "imgui.h"

#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <cstring>

// chatgpt code for random number
int randomNumber(int lower_bound, int upper_bound)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(lower_bound, upper_bound);

    return distr(gen);
}

Game::Game(const std::string &config)
{
    init(config);
}

// read in config file here
void Game::init(const std::string &path)
{
    // window setup
    std::string name, fin_type;
    int win_isFullScreen, win_w, win_h;
    std::ifstream fin(path);
    fin >> name >> win_w >> win_h >> m_framerate >> win_isFullScreen;

    if (win_isFullScreen)
    {
        m_window.create(sf::VideoMode(win_w, win_h), "Assignment 2", sf::Style::Fullscreen);
    }
    else
    {
        m_window.create(sf::VideoMode(win_w, win_h), "Assignment 2");
    }
    m_window.setFramerateLimit(m_framerate);

    // reading input
    while (fin >> fin_type)
    {
        if (fin_type == "Font")
        {
            // Font F S R G B
            int f_size;
            float f_r, f_g, f_b;
            fin >> name >> f_size >> f_r >> f_g >> f_b;
            if (!m_font.loadFromFile(name))
            {
                std::cerr << "Font cannot be loaded!\n";
                exit(-1);
            }
            m_text.setCharacterSize(f_size);
            m_text.setFillColor(sf::Color(f_r, f_g, f_b));
            m_text.setFont(m_font);
            m_text.setPosition(10, 10);
        }
        else if (fin_type == "Player")
        {
            // Player SR CR S FR FG FB OR OG OB OT V
            fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >>
                m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >>
                m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >>
                m_playerConfig.OT >> m_playerConfig.V;
        }
        else if (fin_type == "Enemy")
        {
            // Enemy SR CR SMIN SMAX OR OG OB OT VMIN VMAX L SI
            fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >>
                m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >>
                m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >>
                m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
        }
        else if (fin_type == "Bullet")
        {
            // Bullet SR CR S FR FG FB OR OG OB OT V L
            fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >>
                m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >>
                m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >>
                m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
        else if (fin_type == "Shield")
        {
            // Shield SR CR OR OG OB OT V CD
            fin >> m_shieldConfig.SR >> m_shieldConfig.CR >>
                m_shieldConfig.OR >> m_shieldConfig.OG >> m_shieldConfig.OB >>
                m_shieldConfig.OT >> m_shieldConfig.V >> m_shieldConfig.CD;
        }
    }

    ImGui::SFML::Init(m_window);

    spawnPlayer();
}


void Game::run()
{
    while (m_running)
    {
        // update the entity manager
        m_entities.update();

        // required update call to imgui
        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        if (!m_pause)
        {
            if (m_sMovement_enable)
            {
                sMovement();
            }
            if (m_sLifeSpan_enable)
            {
                sLifespan();
            }
            if (m_sCollision_enable)
            {
                sCollision();
            }
            if (m_sEnemySpawner_enable)
            {
                sEnemySpawner();
            }
        }

        if (m_sUserInput_enable)
        {
            sUserInput();
        }
        if (m_sGUI_enable)
        {
            sGUI();
        }
        if (m_sRender_enable)
        {
            sRender();
        }

        // increment the current frame
        m_currentFrame++;
    }
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
    auto e = m_entities.addEntity("player");

    // player will spawn in the middle
    e->cTransform = std::make_shared<CTransform>(
                        Vec2((float)m_window.getSize().x / 2, (float)m_window.getSize().y / 2), Vec2(0, 0),
                        0.0f);

    // add all player shape configs
    e->cShape = std::make_shared<CShape>(
                    m_playerConfig.SR, m_playerConfig.V,
                    sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
                    sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
                    m_playerConfig.OT);

    // add all player systems needed
    e->cInput = std::make_shared<CInput>();
    e->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

    // this goes slightly against the EntityManager paradigm, but we use the
    // player so much it's worth it
    m_player = e;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
    auto e = m_entities.addEntity("enemy");
    e->cTransform = std::make_shared<CTransform>(
                        // random position
                        Vec2(randomNumber(m_enemyConfig.CR,
                                          m_window.getSize().x - m_enemyConfig.CR),
                             randomNumber(m_enemyConfig.CR,
                                          m_window.getSize().y - m_enemyConfig.CR)),
                        // random speed
                        Vec2((randomNumber(0, 1) == 0 ? -1 : 1) *
                             randomNumber(m_enemyConfig.SMIN, m_enemyConfig.SMAX),
                             (randomNumber(0, 1) == 0 ? -1 : 1) *
                             randomNumber(m_enemyConfig.SMIN, m_enemyConfig.SMAX)),
                        0.0f);

    // enemy shape
    e->cShape = std::make_shared<CShape>(
                    m_enemyConfig.SR, randomNumber(m_enemyConfig.VMIN, m_enemyConfig.VMAX),
                    // random color
                    sf::Color(randomNumber(0, 255), randomNumber(0, 255),
                              randomNumber(0, 255)),
                    sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
                    m_enemyConfig.OT);

    // add collisions
    e->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
    std::cout << "enemy spawned \n";
}

// spawns the small enemies on an entity e
// will be used when an entity is destroyed
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    int vertices = e->cShape->circle.getPointCount();
    float original_angle = e->cShape->circle.getRotation();
    float speed = sqrt(powf(e->cTransform->velocity.x, 2) +
                       powf(e->cTransform->velocity.y, 2)) /
                  2;

    float angle_interval = (float)360 / vertices;

    // create small entities depending on number of vertices
    for (int i = 0; i < vertices; i++)
    {
        float angleRadians =
            original_angle + (angle_interval * i) * (M_PI / 180.0f);

        auto small_e = m_entities.addEntity("small_enemy");
        small_e->cTransform = std::make_shared<CTransform>(
                                  // location
                                  e->cTransform->pos,
                                  // speed
                                  Vec2(speed * std::cos(angleRadians), speed * std::sin(angleRadians)),
                                  // angle
                                  original_angle + (angle_interval * i));

        // enemy shape
        small_e->cShape = std::make_shared<CShape>(
                              m_enemyConfig.SR / 2, vertices, e->cShape->circle.getFillColor(),
                              e->cShape->circle.getOutlineColor(), m_enemyConfig.OT);
        // add collisions
        small_e->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2);

        // add lifespan
        small_e->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
    }
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2 &target)
{
    // some math to calculate where the bullet should go
    Vec2 D(target.x - entity->cTransform->pos.x,
           target.y - entity->cTransform->pos.y);
    Vec2 N(D.x / D.length(), D.y / D.length());
    Vec2 velocity(m_bulletConfig.S * N.x, m_bulletConfig.S * N.y);

    // create bullet entity
    auto bullet = m_entities.addEntity("bullet");
    bullet->cTransform = std::make_shared<CTransform>(
                             // location
                             entity->cTransform->pos,
                             // speed
                             velocity,
                             // angle
                             0.0f);

    // enemy shape
    bullet->cShape = std::make_shared<CShape>(
                         m_bulletConfig.SR, m_bulletConfig.V,
                         sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
                         sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
                         m_bulletConfig.OT);
    // add collisions
    bullet->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2);

    // add lifespan
    bullet->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
}

// personal special weapon: shield!!!
void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    int shieldFrameRate = m_shieldConfig.CD * m_framerate;

    auto shield = m_entities.addEntity("shield");
    shield->cTransform = std::make_shared<CTransform>(
                             // sheild stays on entity
                             entity->cTransform->pos,
                             // speed default to 0
                             Vec2(0, 0),
                             0.0f);

    // shield shape
    shield->cShape = std::make_shared<CShape>(
                         m_shieldConfig.SR, m_shieldConfig.V,
                         sf::Color::Transparent,
                         sf::Color(m_shieldConfig.OR, m_shieldConfig.OG, m_shieldConfig.OB),
                         m_shieldConfig.OT);

    // add a lifespan
    shield->cLifespan = std::make_shared<CLifespan>(shieldFrameRate);

    // input so that it follows player
    shield->cInput = std::make_shared<CInput>();

    // add collisions
    shield->cCollision = std::make_shared<CCollision>(m_shieldConfig.CR);

    // note down for cooldown
    m_lastShieldSpawnTime = m_currentFrame;
}

// movenment system
void Game::sMovement()
{
    for (auto e : m_entities.getEntities())
    {
        if (e->cInput != nullptr)
        {
            // vertical movement
            if (e->cInput->up)
            {
                e->cTransform->velocity.y = -1 * m_playerConfig.V;
            }
            else if (e->cInput->down)
            {
                e->cTransform->velocity.y = m_playerConfig.V;
            }
            else
            {
                e->cTransform->velocity.y = 0;
            }

            // horizontal movement
            if (e->cInput->left)
            {
                e->cTransform->velocity.x = -1 * m_playerConfig.V;
            }
            else if (e->cInput->right)
            {
                e->cTransform->velocity.x = m_playerConfig.V;
            }
            else
            {
                e->cTransform->velocity.x = 0;
            }
        }
    }

    // implement all entity movement in this function
    for (auto e : m_entities.getEntities())
    {
        // sample movement speed update
        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }
}

// lifespan system
void Game::sLifespan()
{
    // for all entities
    for (auto e : m_entities.getEntities())
    {
        // if entity has no lifespan component, skip it
        if (e->cLifespan == nullptr)
        {
            continue;
        }
        else if (e->cLifespan->remaining > 0)
        {
            // if entity has > 0 remaining lifespan, subtract 1
            e->cLifespan->remaining -= 1;
            // if it has lifespan and its time is up destroy the entity
            if (e->cLifespan->remaining == 0)
            {
                e->destroy();
            }
            else
            {
                // if it has lifespan and is alive scale its alpha channel properly
                sf::Color color = e->cShape->circle.getFillColor();
                sf::Color outline = e->cShape->circle.getOutlineColor();
                color.a = (float)e->cLifespan->remaining / e->cLifespan->total * 255;
                outline.a = (float)e->cLifespan->remaining / e->cLifespan->total * 255;
                e->cShape->circle.setFillColor(color);
                e->cShape->circle.setOutlineColor(outline);
            }
        }
    }
}

// collision system for all entities
void Game::sCollision()
{
    // player xpos-wall collision
    if (m_player->cTransform->pos.x - m_player->cCollision->radius <= 0)
    {
        m_player->cTransform->pos.x = m_player->cCollision->radius;
    }
    else if (m_player->cTransform->pos.x + m_player->cCollision->radius >
             m_window.getSize().x)
    {
        m_player->cTransform->pos.x =
            m_window.getSize().x - m_player->cCollision->radius;
    }

    // player ypos-wall collision
    if (m_player->cTransform->pos.y - m_player->cCollision->radius <= 0)
    {
        m_player->cTransform->pos.y = m_player->cCollision->radius;
    }
    else if (m_player->cTransform->pos.y + m_player->cCollision->radius >
             m_window.getSize().y)
    {
        m_player->cTransform->pos.y =
            m_window.getSize().y - m_player->cCollision->radius;
    }

    // enemy collision
    for (auto e : m_entities.getEntities("enemy"))
    {
        // enemy wall collision
        if (e->cTransform->pos.x - e->cCollision->radius <= 0 ||
                e->cTransform->pos.x + e->cCollision->radius > m_window.getSize().x)
        {
            e->cTransform->velocity.x *= -1;
        }

        if (e->cTransform->pos.y - e->cCollision->radius <= 0 ||
                e->cTransform->pos.y + e->cCollision->radius > m_window.getSize().y)
        {
            e->cTransform->velocity.y *= -1;
        }

        // player-enemy collision
        if (e->collides(m_player))
        {
            m_player->destroy();
            e->destroy();
            spawnSmallEnemies(e);
            spawnPlayer();
        }
    }

    // small enemy collision
    for (auto small_e : m_entities.getEntities("small_enemy"))
    {
        for (auto e : m_entities.getEntities())
        {
            if (small_e->collides(m_player))
            {
                small_e->destroy();
            }
        }
    }

    // bullet collisions
    for (auto bullet : m_entities.getEntities("bullet"))
    {
        for (auto enemy : m_entities.getEntities("enemy"))
        {
            // bullet hitting an enemy (regular points)
            if (bullet->collides(enemy))
            {
                enemy->destroy();
                bullet->destroy();
                spawnSmallEnemies(enemy);
                m_score += enemy->cShape->circle.getPointCount() * 100;
            }
        }
        for (auto small : m_entities.getEntities("small_enemy"))
        {
            // bullet hitting small enemy (double points)
            if (bullet->collides(small))
            {
                small->destroy();
                bullet->destroy();
                m_score += 2 * (small->cShape->circle.getPointCount() * 100);
            }
        }
    }

    // shield collisions
    for (auto shield : m_entities.getEntities("shield"))
    {
        for (auto enemy : m_entities.getEntities("enemy"))
        {
            // shield hitting an enemy (regular points)
            if (shield->collides(enemy))
            {
                enemy->destroy();
                spawnSmallEnemies(enemy);
                m_score += enemy->cShape->circle.getPointCount() * 100;
            }
        }
        for (auto small : m_entities.getEntities("small_enemy"))
        {
            // shield hitting small enemy (double points)
            if (shield->collides(small))
            {
                small->destroy();
                m_score += 2 * (small->cShape->circle.getPointCount() * 100);
            }
        }
    }
}

// enemy spawning system
void Game::sEnemySpawner()
{
    if (m_currentFrame >= m_lastEnemySpawnTime + m_enemyConfig.SI ||
            m_currentFrame % m_enemyConfig.SI == 0)
    {
        spawnEnemy();
    }
}

// gui system
void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");
    // the gui_* vectors are just for the gui text
    std::vector<const char *> gui_entityTags;
    std::vector<const char *> gui_entityIDs;
    // this will be used to store the entity IDs
    std::vector<size_t> entityIDs;

    if (ImGui::BeginTabBar("tabs"))
    {
        // enable/disable systems
        if (ImGui::BeginTabItem("Systems"))
        {
            ImGui::Checkbox("Movement", &m_sMovement_enable);
            ImGui::Checkbox("Lifespan", &m_sLifeSpan_enable);
            ImGui::Checkbox("Collision", &m_sCollision_enable);
            ImGui::Checkbox("Spawning", &m_sEnemySpawner_enable);
            if (m_sEnemySpawner_enable)
            {
                ImGui::Text("Spawn Rate (per frame)");
                ImGui::SliderInt("", &m_enemyConfig.SI, 0, 300);
            }
            ImGui::Checkbox("GUI", &m_sGUI_enable);
            ImGui::Checkbox("Rendering", &m_sRender_enable);
            ImGui::EndTabItem();

            // manually spawn an enemy
            if (ImGui::Button("Manual Spawn"))
            {
                spawnEnemy();
            }
        }
        // modifying entities
        if (ImGui::BeginTabItem("Entities"))
        {

            // get all the different entity tags
            for (auto const &item : m_entities.getEntityMap())
            {
                gui_entityTags.push_back(item.first.c_str());
            }

            ImGui::Combo("Entity Tags", &m_sGUI_entity_tag,
                         gui_entityTags.data(),
                         gui_entityTags.size());

            if (!gui_entityTags.empty())
            {
                auto &entities = gui_entityTags[m_sGUI_entity_tag];

                // get all the id's for the tag selected
                for (auto const &e : m_entities.getEntities(entities))
                {
                    // convert to char*
                    std::string idString = std::to_string(e->id());
                    char *idCString = new char[idString.length() + 1];
                    std::strcpy(idCString, idString.c_str());

                    gui_entityIDs.push_back(idCString);

                    // will be used for adjusting the entity
                    entityIDs.push_back(e->id());
                }

                ImGui::Combo("id", &m_sGUI_entity_id,
                             gui_entityIDs.data(),
                             gui_entityIDs.size());

                if (!gui_entityIDs.empty())
                {
                    // selected entity
                    auto entity = m_entities.getEntityById(entityIDs[m_sGUI_entity_id]);

                    // edit selected entity velocity
                    float velo[2] = {entity->cTransform->velocity.x, entity->cTransform->velocity.y};
                    ImGui::SliderFloat2("Velocity", velo, -8.0f, 8.0f);
                    entity->cTransform->velocity.x = velo[0];
                    entity->cTransform->velocity.y = velo[1];

                    // edit selected entity fill color
                    sf::Color fill_color = entity->cShape->circle.getFillColor();
                    float fill_color_arr[3] = {fill_color.r / 255.0f, fill_color.g / 255.0f, fill_color.b / 255.0f};
                    ImGui::ColorEdit3("Fill Color", fill_color_arr);
                    entity->cShape->circle.setFillColor(sf::Color(fill_color_arr[0] * 255, fill_color_arr[1] * 255, fill_color_arr[2] * 255));

                    // edit selected entity outline color
                    sf::Color out_color = entity->cShape->circle.getOutlineColor();
                    float out_color_arr[3] = {out_color.r / 255.0f, out_color.g / 255.0f, out_color.b / 255.0f};
                    ImGui::ColorEdit3("Outline Color", out_color_arr);
                    entity->cShape->circle.setOutlineColor(sf::Color(out_color_arr[0] * 255, out_color_arr[1] * 255, out_color_arr[2] * 255));

                    // destroy selected entity
                    if (ImGui::Button("Destroy"))
                    {
                        entity->destroy();
                        spawnSmallEnemies(entity);
                    }
                }
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

// render system
void Game::sRender()
{
    m_window.clear();
    m_entities.update();
    m_entities.getEntities();

    for (auto e : m_entities.getEntities())
    {
        // set the position of the shape based on the entity's transform->pos
        e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

        // set the rotation of the shape based on the entity's transform->angle
        e->cTransform->angle += 1.0f;
        e->cShape->circle.setRotation(e->cTransform->angle);

        // draw the entity's sf::CircleShape
        m_window.draw(e->cShape->circle);
    }

    // draw the ui last
    ImGui::SFML::Render(m_window);

    // score display
    m_text.setString("SCORE: " + std::to_string(m_score));
    m_window.draw(m_text);

    m_window.display();
}

// user input system
void Game::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        // pass the event to imgui to be parsed
        ImGui::SFML::ProcessEvent(m_window, event);

        // this event triggers when the window is closed
        if (event.type == sf::Event::Closed)
        {
            m_running = false;
        }

        // this event is triggered when a key is pressed
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            // player moving
            case sf::Keyboard::W:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->up = true;
                    }
                }
                break;
            case sf::Keyboard::A:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->left = true;
                    }
                }
                break;
            case sf::Keyboard::S:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->down = true;
                    }
                }
                break;
            case sf::Keyboard::D:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->right = true;
                    }
                }
                break;

            default:
                break;
            }
        }

        // this event is triggered when a key is released
        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
            // player stopping
            case sf::Keyboard::W:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->up = false;
                    }
                }
                break;
            case sf::Keyboard::A:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->left = false;
                    }
                }
                break;
            case sf::Keyboard::S:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->down = false;
                    }
                }
                break;
            case sf::Keyboard::D:
                for (auto e : m_entities.getEntities())
                {
                    if (e->cInput != nullptr)
                    {
                        e->cInput->right = false;
                    }
                }
                break;

            // pausing
            case sf::Keyboard::P:
                m_pause = m_pause ? false : true;
                break;

            // closing game with Escape
            case sf::Keyboard::Escape:
                m_running = false;
                break;

            default:
                break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (ImGui::GetIO().WantCaptureMouse)
            {
                continue;
            }

            if (event.mouseButton.button == sf::Mouse::Left)
            {
                if (!m_pause)
                {
                    spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
                }
            }

            if (event.mouseButton.button == sf::Mouse::Right)
            {
                if (!m_pause)
                {
                    spawnSpecialWeapon(m_player);
                }
            }
        }
    }
}
