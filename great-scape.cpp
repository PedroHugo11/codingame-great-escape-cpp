#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stack>
#include <time.h>

using namespace std;

struct Wall
{
    enum Orientation
    {
        Horizontal,
        Vertical
    };

    int x;
    int y;
    Orientation orientation;
};

struct Player
{
    int x; // coordenada x do jogador
    int y; // coordenada y do jogador
    int wallsLeft; // numero de walls disponiveis para o jogador
};


int w; // width do board
int h; // height do board
int playerCount; // counter de jogadores (2 or 3)
int myId; // id do meu boneco (0 = primero player, 1 = segundo player, ...)

int turns;

vector<Wall> walls;
vector<Player> players;

vector<vector<pair<int, int>>> storedPaths;

int target;

vector<string> phrases = {
    "You shall not pass!",
    "Stop right there!",
    "The way is shut!",
    "HALT!",
    "STOP!",
    "Don't move!",
    "Don't go there!",
    "Path blocked."
};

bool isGoalNode(int id, int x, int y)
{
    assert(id >= 0 && id <= 2);

    switch (id)
    {
    case 0:
        if (x == w-1)
            return true;
        else
            return false;

    case 1:
        if (x == 0)
            return true;
        else
            return false;

    case 2:
        if (y == h-1)
            return true;
        else
            return false;
    }
}

bool hasVerticalWall(int x, int y)
{
    for (auto& wall : walls)
    {
        if ((wall.orientation == Wall::Orientation::Vertical) && (wall.x == x))
        {
            if (wall.y == y)
                return true;
        }
    }
    return false;
}

bool hasHorizontalWall(int x, int y)
{
    for (auto& wall : walls)
    {
        if ((wall.orientation == Wall::Orientation::Horizontal) && (wall.y == y))
        {
            if (wall.x == x)
                return true;
        }
    }
    return false;
}

vector<pair<int, int>> getNeighbors(pair<int, int> node)
{
    vector<pair<int, int>> neighbors;

    if ((node.first > 0)
     && (!hasVerticalWall(node.first, node.second - 1))
     && (!hasVerticalWall(node.first, node.second)))
    {
        neighbors.push_back({node.first - 1, node.second});
    }

    if ((node.second > 0)
     && (!hasHorizontalWall(node.first - 1, node.second))
     && (!hasHorizontalWall(node.first, node.second)))
    {
        neighbors.push_back({node.first, node.second - 1});
    }

    if ((node.first < w-1)
     && (!hasVerticalWall(node.first + 1, node.second - 1))
     && (!hasVerticalWall(node.first + 1, node.second)))
    {
        neighbors.push_back({node.first + 1, node.second});
    }

    if ((node.second < h-1)
     && (!hasHorizontalWall(node.first - 1, node.second + 1))
     && (!hasHorizontalWall(node.first, node.second + 1)))
    {
        neighbors.push_back({node.first, node.second + 1});
    }

    return neighbors;
}

vector<pair<int, int>> dijkstra(pair<int, int> source, int id)
{
    auto dist = vector<vector<int>>(h, vector<int>(w, numeric_limits<int>::max()));
    auto prev = vector<vector<pair<int, int>>>(h, vector<pair<int, int>>(w, {numeric_limits<int>::max(), numeric_limits<int>::max()}));

    vector<pair<int, int>> unvisited;

    // inicializacao
    dist[source.second][source.first] = 0;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
            unvisited.push_back({x, y});
    }

    while (!unvisited.empty())
    {
        // Encontre o nó com a menor distância
        vector<pair<int, int>>::iterator minIt = unvisited.begin();
        int minDist = numeric_limits<int>::max();
        for (auto it = unvisited.begin(); it != unvisited.end(); ++it)
        {
            if (dist[it->second][it->first] < minDist)
            {
                minDist = dist[it->second][it->first];
                minIt = it;
            }
        }

        // Retorne uma lista vazia quando não houver maneira de chegar à meta
        if (minDist == numeric_limits<int>::max())
            return {};

        // Remova e retorne o melhor vértice
        pair<int, int> target = *minIt;
        unvisited.erase(minIt);

        // Check se é o nó da meta
        if (isGoalNode(id, target.first, target.second))
        {
            stack<pair<int, int>> pathStack;
            pair<int, int> node = target;
            while (prev[node.second][node.first] != pair<int, int>{numeric_limits<int>::max(), numeric_limits<int>::max()})
            {
                pathStack.push(node);
                node = prev[node.second][node.first];
            }

            vector<pair<int, int>> path;
            while (!pathStack.empty())
            {
                path.push_back(pathStack.top());
                pathStack.pop();
            }

            return path;
        }

        vector<pair<int, int>> neighbors = getNeighbors(target);
        for (pair<int, int>& neighbor : neighbors)
        {
            int alt = dist[target.second][target.first] + 1;
            if (alt < dist[neighbor.second][neighbor.first])
            {
                dist[neighbor.second][neighbor.first] = alt;
                prev[neighbor.second][neighbor.first] = target;
            }
        }
    }

    return {};
}

void move(int x, int y)
{
    assert(players[myId].x == x || players[myId].y == y);

    if (x > players[myId].x)
        cout << "RIGHT" << endl;
    else if (x < players[myId].x)
        cout << "LEFT" << endl;
    else
    {
        if (y > players[myId].y)
            cout << "DOWN" << endl;
        else if (y < players[myId].y)
            cout << "UP" << endl;
    }
}

int calculateMovesDiff()
{
    auto& paths = storedPaths;

    paths.clear();
    paths.resize(playerCount);

    for (int i = 0; i < playerCount; ++i)
    {
        if (players[i].x != -1)
        {
            paths[i] = dijkstra({players[i].x, players[i].y}, i);
            if (paths[i].empty())
                return -numeric_limits<int>::max();
        }
    }

    int movesDiff = numeric_limits<int>::max();
    for (int i = 0; i < playerCount; ++i)
    {
        if ((i != myId) && (players[i].x != -1))
            movesDiff = min((int)paths[i].size() - (int)paths[myId].size(), movesDiff);
    }

    return movesDiff;
}

bool checkValidWallPlacement(Wall wall)
{
    if ((wall.x == -1) || (wall.y == -1)
     || (wall.x == w) || (wall.y == h)
     || (wall.x == w-1 && wall.orientation == Wall::Orientation::Horizontal)
     || (wall.y == h-1 && wall.orientation == Wall::Orientation::Vertical)
     || (wall.x == 0 && wall.orientation == Wall::Orientation::Vertical)
     || (wall.y == 0 && wall.orientation == Wall::Orientation::Horizontal))
        return false;

    if (wall.orientation == Wall::Orientation::Vertical)
    {
        if (hasVerticalWall(wall.x, wall.y-1) || hasVerticalWall(wall.x, wall.y)
         || hasVerticalWall(wall.x, wall.y+1) || hasHorizontalWall(wall.x-1, wall.y+1))
        {
            return false;
        }
    }
    else
    {
        if (hasHorizontalWall(wall.x-1, wall.y) || hasHorizontalWall(wall.x, wall.y)
         || hasHorizontalWall(wall.x+1, wall.y) || hasVerticalWall(wall.x+1, wall.y-1))
        {
            return false;
        }
    }

    return true;
}

pair<Wall, int> findBestWall(int movesDiff, int turn = 1)
{
    Wall wall{-1, -1, Wall::Orientation::Horizontal};

    // Calcule os caminhos dos oponentes
    int i;
    int smallestPath = myId;
    vector<vector<pair<int, int>>> paths(playerCount);
    for (i = 0; i < playerCount; ++i)
    {
        if ((i != myId) && (players[i].x != -1))
        {
            if (isGoalNode(i, players[i].x, players[i].y))
                return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};

            paths[i] = dijkstra({players[i].x, players[i].y}, i);
            assert(!paths[i].empty());

            if (smallestPath == myId)
            {
                smallestPath = i;
            }
            else
            {
                if (paths[i].size() < paths[smallestPath].size())
                    smallestPath = i;
                else if (paths[i].size() == paths[smallestPath].size())
                {
                    if (rand() % 2)
                        smallestPath = i;
                }
            }

            paths[i].insert(paths[i].begin(), {players[i].x, players[i].y});
        }
    }

    // pegue um alvo
    if (target == myId)
    {
        // Se houver 3 jogadores e nós formos os últimos, tente o segundo lugar
        if ((playerCount == 3) && (players[3 - myId - smallestPath].x != -1)
         && (players[smallestPath].x != -1)
         && (players[3 - myId - smallestPath].x != -1)
         && (storedPaths[myId].size() + 1 > paths[smallestPath].size())
         && (storedPaths[myId].size() + 1 > paths[3 - myId - smallestPath].size()))
        {
            i = 3 - myId - smallestPath;
        }
        else // atacar o oponente atualmente vencedor
        {
            i = smallestPath;
        }
    }
    else // alvo já dado na rodada anterior
        i = target;

    auto attemptWall = [&](int x, int y, Wall::Orientation orientation)
        {
            if (!checkValidWallPlacement(Wall{x, y, orientation}))
                return;

            walls.push_back(Wall{x, y, orientation});

            if (calculateMovesDiff() == -numeric_limits<int>::max())
            {
                walls.pop_back();
                return;
            }

            int tentativeMovesDiff = storedPaths[i].size() - storedPaths[myId].size();
            if (turn < turns)
            {
                if (tentativeMovesDiff > movesDiff)
                {
                    wall = Wall{x, y, orientation};
                    movesDiff = tentativeMovesDiff;
                }

                pair<int, int> oldPositions[playerCount];

                // Movendo outros jogadores
                for (int j = 0; j < playerCount; ++j)
                {
                    if ((j != myId) && (players[j].x != -1))
                    {
                        oldPositions[j] = {players[j].x, players[j].y};

                        auto path = dijkstra({players[j].x, players[j].y}, j);
                        if (path.empty())
                        {
                            for (int k = 0; k < j; ++k)
                            {
                                if ((k != myId) && (players[k].x != -1))
                                {
                                    players[k].x = oldPositions[k].first;
                                    players[k].y = oldPositions[k].second;
                                }
                            }

                            walls.pop_back();
                            return;
                        }

                        players[j].x = path[0].first;
                        players[j].y = path[0].second;
                    }
                }

                int oldTarget = target;
                if (target == myId)
                    target = i;

                tentativeMovesDiff = findBestWall(movesDiff, turn+1).second;
                target = oldTarget;

                // Mova os jogadores de volta para suas posições anteriores
                for (int j = 0; j < playerCount; ++j)
                {
                    if ((j != myId) && (players[j].x != -1))
                    {
                        players[j].x = oldPositions[j].first;
                        players[j].y = oldPositions[j].second;
                    }
                }
            }

            walls.pop_back();

            if (tentativeMovesDiff > movesDiff)
            {
                wall = Wall{x, y, orientation};
                movesDiff = tentativeMovesDiff;
            }
        };

    // Apenas tente colocar paredes em locais que bloqueiem este caminho
    for (int j = 0; j < min<int>(paths[i].size()-1, 5); ++j)
    {
        if (paths[i][j+1].first > paths[i][j].first)
        {
            if (paths[i][j].second != 1)
                attemptWall(paths[i][j].first+1, paths[i][j].second, Wall::Orientation::Vertical);

            if (paths[i][j].second != h-2)
                attemptWall(paths[i][j].first+1, paths[i][j].second-1, Wall::Orientation::Vertical);
        }
        else if (paths[i][j+1].first < paths[i][j].first)
        {
            if (paths[i][j].second != 1)
                attemptWall(paths[i][j].first, paths[i][j].second, Wall::Orientation::Vertical);

            if (paths[i][j].second != h-2)
                attemptWall(paths[i][j].first, paths[i][j].second-1, Wall::Orientation::Vertical);
        }
        else
        {
            if (paths[i][j+1].second > paths[i][j].second)
            {
                if (paths[i][j].first != 1)
                {
                    attemptWall(paths[i][j].first, paths[i][j].second+1, Wall::Orientation::Horizontal);
                }

                if (paths[i][j].first != w-2)
                {
                    attemptWall(paths[i][j].first-1, paths[i][j].second+1, Wall::Orientation::Horizontal);
                }
            }
            else if (paths[i][j+1].second < paths[i][j].second)
            {
                if (paths[i][j].first != 1)
                    attemptWall(paths[i][j].first, paths[i][j].second, Wall::Orientation::Horizontal);

                if (paths[i][j].first != w-2)
                    attemptWall(paths[i][j].first-1, paths[i][j].second, Wall::Orientation::Horizontal);
            }
        }
    }

    if (turn == 1)
    {
        // Coloque a parede ao lado do oponente (espere até que ele chegue ao local proposto primeiro)
        if (wall.x > -1)
        {
            if (wall.orientation == Wall::Orientation::Vertical)
            {
                if (paths[i][1].first > paths[i][0].first)
                {
                    if (((wall.x != paths[i][0].first+1) || (wall.y != paths[i][0].second))
                     && ((wall.x != paths[i][0].first+1) || (wall.y != paths[i][0].second-1)))
                    {
                        return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};
                    }
                }
                else if (paths[i][1].first < paths[i][0].first)
                {
                    if (((wall.x != paths[i][0].first) || (wall.y != paths[i][0].second))
                     && ((wall.x != paths[i][0].first) || (wall.y != paths[i][0].second-1)))
                    {
                        return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};
                    }
                }
                else
                    return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};
            }
            else // wall horizontal 
            {
                if (paths[i][1].second > paths[i][0].second)
                {
                    if (((wall.x != paths[i][0].first) || (wall.y != paths[i][0].second+1))
                     && ((wall.x != paths[i][0].first-1) || (wall.y != paths[i][0].second+1)))
                    {
                        return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};
                    }
                }
                else if (paths[i][1].second < paths[i][0].second)
                {
                    if (((wall.x != paths[i][0].first) || (wall.y != paths[i][0].second))
                     && ((wall.x != paths[i][0].first-1) || (wall.y != paths[i][0].second)))
                    {
                        return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};
                    }
                }
                else
                    return {Wall{-1, -1, Wall::Orientation::Horizontal}, movesDiff};
            }
        }
    }

    return {wall, movesDiff};
}

bool placeWall(int movesDiff)
{
    bool wallFound = false;
    Wall wallToPlace = findBestWall(movesDiff).first;

    if (wallToPlace.x > -1)
        wallFound = true;

    if (wallFound)
    {
        if (wallToPlace.orientation == Wall::Orientation::Vertical)
        {
            cout << wallToPlace.x << " " << wallToPlace.y << " V"
                 << " " << phrases[rand() % phrases.size()] << endl;
        }
        else
        {
            cout << wallToPlace.x << " " << wallToPlace.y << " H"
                 << " " << phrases[rand() % phrases.size()] << endl;
        }

        players[myId].wallsLeft--;
        return true;
    }
    else
    {
        cerr << "oh oh" << endl;
        return false;
    }
}


int main()
{
    srand(time(NULL));

    cin >> w >> h >> playerCount >> myId; cin.ignore();

    // Podemos calcular mais quando há apenas um oponente
    if (playerCount == 2)
        turns = 2;
    else
        turns = 2;

    // Escolha um alvo para se preocupar com paredes
    if (playerCount == 2)
        target = 1 - myId;
    else
    {
        if (myId == 0)
            target = 2 - (rand() % 2);
        else if (myId == 1)
            target = 2 - ((rand() % 2) * 2);
        else
        target = rand() % 2;
    }

    // game loop
    int iteration = 0;
    while (1) {
        iteration++;

        vector<Wall> previousWalls;

        walls.clear();
        players.clear();

        for (int i = 0; i < playerCount; i++) {
            Player player;
            cin >> player.x >> player.y >> player.wallsLeft; cin.ignore();
            players.push_back(player);
        }

        int wallCount; // numeros de walls no board
        cin >> wallCount; cin.ignore();
        for (int i = 0; i < wallCount; i++) {
            Wall wall;
            string wallOrientation; // wall orientation ('H' or 'V')
            cin >> wall.x >> wall.y >> wallOrientation; cin.ignore();

            if (wallOrientation == "H")
                wall.orientation = Wall::Orientation::Horizontal;
            else
                wall.orientation = Wall::Orientation::Vertical;

            walls.push_back(wall);
        }

        // Se um oponente adicionou uma parede, pare de mirar em um único jogador
        if (target != myId)
        {
            for (auto it = walls.begin(), prevIt = previousWalls.begin();
                 (it != walls.end()) && (prevIt != previousWalls.end());
                 ++it, ++prevIt)
            {
                if ((it->x != prevIt->x) || (it->y != prevIt->y) || (it->orientation != prevIt->orientation))
                {
                    target = myId;
                    break;
                }
            }
        }

        int movesDiff = calculateMovesDiff();

        // Check quando mover
        if ((movesDiff >= 0) // vencendo
         || (iteration <= 5) // em nossas primeiras 6 voltas
         || (players[myId].wallsLeft == 0) // quando não houver mais paredes sobrando
         || !placeWall(movesDiff)) // quando não conseguiu encontrar um lugar para colocar uma parede
        {
            auto path = dijkstra({players[myId].x, players[myId].y}, myId);
            assert(!path.empty());
            move(path[0].first, path[0].second);
        }
    }
}