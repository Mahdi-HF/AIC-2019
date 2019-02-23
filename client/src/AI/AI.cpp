#include <stdlib.h>
#include "AI.h"

using namespace std;

// We have 2 blaster and 1 healer and 1 sentry
int RowNum = 31;
int ColumnNum = 31;

bool WAR_IN_OBJECTIVE_ZONE = true;

int mapAnalyse[35][35];
int SENTRY_ID;
int BLASTER_ID;
int BLASTER_ID_2;
int BLASTER_ID_3;

Cell SENTRY_CELL;
Cell BLASTER_CELL;
Cell BLASTER_CELL_2;
Cell BLASTER_CELL_3;

int currentHeal_Sentry=120;
int currentHeal_Blaster_1=250;
int currentHeal_Blaster_2=250;
int currentHeal_Blaster_3=250;
int minHeal=600;

int HERO_mapAnalyse[35][35];

int targetCellRow[4];
int targetCellColumn[4];

int minDistance;
int minI;
int minJ;

vector<Direction> Mypath;

//----------------------------------------- Functions ------------------------------------------------------------------
void printMap(World *world)
{
    Map map = world->map();
    for (int row = 0; row < map.getRowNum(); row++)
    {
        for (int column = 0; column < map.getColumnNum(); column++)
        {
            Cell cell = map.getCell(row, column);
            char cur;
            if (world->getMyHero(row, column) != Hero::NULL_HERO)
                cur = static_cast<char>('0' + world->getMyHero(row, column).getId() % 10);
            else if (world->getOppHero(row, column) != Hero::NULL_HERO)
                cur = static_cast<char>('0' + world->getOppHero(row, column).getId() % 10);
            else if (cell.isWall())
                cur = '#';
            else if (cell.isInVision())
                cur = '+';
            else
                cur = '.';
            cerr << cur << ' ';
        }
        cerr << endl;
    }
}

void mapAnalysingByHeroesView(World *world)
{
    for (int n = 0; n < 4; ++n)
    {
        int i = world->getOppHeroes()[n]->getCurrentCell().getRow();
        int j = world->getOppHeroes()[n]->getCurrentCell().getColumn();

        if (HeroName::BLASTER == world->getOppHeroes()[n]->getName())
        {
            HERO_mapAnalyse[i][j] = 2;
        }
        else if (HeroName::SENTRY == world->getOppHeroes()[n]->getName())
        {
            HERO_mapAnalyse[i][j] = 3;
        }
        else if (HeroName::GUARDIAN == world->getOppHeroes()[n]->getName())
        {
            HERO_mapAnalyse[i][j] = 4;
        }
        else if (HeroName::HEALER == world->getOppHeroes()[n]->getName())
        {
            HERO_mapAnalyse[i][j] = 5;
        }
    }

    for (int i = 0; i < RowNum; ++i)
    {
        for (int j = 0; j < ColumnNum; ++j)
        {
            int k = HERO_mapAnalyse[i][j];
            if (world->map().getCell(i, j).isInVision() and k != 2 and k != 3 and k != 4 and k != 5)
            {
                k = 1;
            }
            else if (k != 1 and k != 2 and k != 3 and k != 4 and k != 5)
            {
                k = -1;
            }
            HERO_mapAnalyse[i][j] = k;
        }
    }
}

void HeroAnalyse(World *world)
{
    for (int n = 0; n < 4; n++)
    {
        if (HeroName::SENTRY == world->getMyHeroes()[n]->getName())
        {
            SENTRY_ID = world->getMyHeroes()[n]->getId();
        }
        else if (HeroName::BLASTER == world->getMyHeroes()[n]->getName())
        {
            if(BLASTER_ID == 0)
            {
                BLASTER_ID = world->getMyHeroes()[n]->getId();
            }
            else if(BLASTER_ID_2 == 0)
            {
                BLASTER_ID_2 = world->getMyHeroes()[n]->getId();
            }
            else if(BLASTER_ID_3 == 0)
            {
                BLASTER_ID_3 = world->getMyHeroes()[n]->getId();
            }
        }
    }
}

void mapAnalyser(World *world)
{
    unsigned long ObjectiveZoneSize = world->map().getObjectiveZone().size(); //objective zone
    for (int l = 0; l < ObjectiveZoneSize; l++)
    {
        int i = world->map().getObjectiveZone()[l]->getRow();
        int j = world->map().getObjectiveZone()[l]->getColumn();
        mapAnalyse[i][j] = 2;
    }

    unsigned long RespawnZoneSize = world->map().getMyRespawnZone().size(); //my respawn zone
    for (int l = 0; l < RespawnZoneSize; l++)
    {
        int i = world->map().getMyRespawnZone()[l]->getRow();
        int j = world->map().getMyRespawnZone()[l]->getColumn();
        mapAnalyse[i][j] = 3;
    }

    unsigned long oppRespawnZoneSize = world->map().getOppRespawnZone().size(); // opponent respawn zone
    for (int l = 0; l < oppRespawnZoneSize; l++)
    {
        int i = world->map().getOppRespawnZone()[l]->getRow();
        int j = world->map().getOppRespawnZone()[l]->getColumn();
        mapAnalyse[i][j] = 4;
    }

    for (int i = 0; i < RowNum; i++)
    {
        for (int j = 0; j < ColumnNum; j++)
        {
            int k = mapAnalyse[i][j];
            if (world->getMap().getCell(i, j).isWall()) // is wall
            {
                mapAnalyse[i][j] = -1;
            }
            else if (k != 2 and k != 3 and k != 4) //Normal zone
            {
                mapAnalyse[i][j] = 1;
            }
        }
    }
}

Cell heroLocator(World *world, int ID)
{
    return world->getHero(ID).getCurrentCell();
}

Cell cellLocator(World *world, int i, int j)
{
    return world->map().getCell(i, j);
}

void findClosestCell(World *world, int ID)
{
    minDistance = 1000;
    minI = -1;
    minJ = -1;

    SENTRY_CELL = heroLocator(world,SENTRY_ID);
    BLASTER_CELL = heroLocator(world,BLASTER_ID);
    BLASTER_CELL_2 = heroLocator(world,BLASTER_ID_2);
    BLASTER_CELL_3 = heroLocator(world,BLASTER_ID_3);

    for (int i = 0; i < 31; ++i) //finding the closest obj zone cell
    {
        for (int j = 0; j < 31; ++j)
        {
            if( cellLocator(world,i,j).isInObjectiveZone() and cellLocator(world,i,j) != SENTRY_CELL and
                    cellLocator(world,i,j) != BLASTER_CELL and cellLocator(world,i,j) != BLASTER_CELL_2 and
                    cellLocator(world,i,j) != BLASTER_CELL_3 )
            {
                int len = world->manhattanDistance( heroLocator(world,ID), cellLocator(world,i,j) );
                if(len <= minDistance)
                {
                    minDistance = len;
                    minI = i;
                    minJ = j;
                    continue;
                }
            }
        }
    }
}

void moveToCell(World *world , int ID ) // find the hero and locate the current cell and target cell
{
    findClosestCell(world , ID );  //changes minDistance and minI and minJ
    Mypath.clear();
    Mypath = world->getPathMoveDirections( heroLocator(world,ID).getRow(), heroLocator(world,ID).getColumn(),
                                           cellLocator(world,minI,minJ).getRow(), cellLocator(world,minI,minJ).getColumn() );
    world->moveHero( ID ,Mypath[0] );
}

void fullAnalyse(World *world)
{
    mapAnalyser(world);
    HeroAnalyse(world);
    mapAnalysingByHeroesView(world);
}

void SentryAbilityDone(World *world , int ID , AbilityName firstAbilityName , AbilityName secondAbilityName  ) {
    for (int k = 0; k < 4; ++k) {
        if (world->getMyHeroes()[k]->getId() == ID) {
            for (int i = 0; i < 31; i++) {
                for (int j = 0; j < 31; j++) {
//                    cerr<<"i : "<<i<<" j :"<<j<<endl;
                    if (world->getHero(ID).getAbility(firstAbilityName).getCooldown() == 0 and
                        (mapAnalyse[i][j] != -1) and world->map().getCell(i, j).isInVision() and
                        HERO_mapAnalyse[i][j] > 1 and
                        world->manhattanDistance(heroLocator(world, ID), cellLocator(world, i, j)) > 7) {
                        world->castAbility(world->getHero(ID), world->getHero(ID).getAbility(firstAbilityName),
                                           cellLocator(world, i, j));
//                        cerr<<"Centry Ray Done";
                        break;
                    }

                    if (world->getHero(ID).getAbility(secondAbilityName).getCooldown() == 0 and
                        (mapAnalyse[i][j] != -1) and world->map().getCell(i, j).isInVision() and
                        HERO_mapAnalyse[i][j] > 1 and
                        world->manhattanDistance(heroLocator(world, ID), cellLocator(world, i, j)) <= 7) {
                        world->castAbility(world->getHero(ID), world->getHero(ID).getAbility(secondAbilityName),
                                           cellLocator(world, i, j));
//                        cerr<<"Centry Attack Done";
                        break;
                    }
                }
            }
        }
    }
}

    void BlasterAbilityDone(World *world , int ID , AbilityName firstAbilityName , AbilityName secondAbilityName )
    {
        for (int k = 0; k < 4 ; ++k)
        {
            if (world->getMyHeroes()[k]->getId() == ID)
            {
                for (int i = 0; i < 31 ; i++) {
                    for (int j = 0; j < 31 ; j++) {
                        if (world->getHero(ID).getAbility(firstAbilityName).getCooldown() == 0 and (mapAnalyse[i][j] != -1) and
                            world->map().getCell(i, j).isInVision() and HERO_mapAnalyse[i][j] > 1 and
                            world->manhattanDistance(heroLocator(world, ID), cellLocator(world, i, j)) == 5)
                        {
                            world->castAbility(world->getHero(ID),world->getHero(ID).getAbility(firstAbilityName),cellLocator(world, i, j));
                            break;
                        }

                        if (world->getHero(ID).getAbility(secondAbilityName).getCooldown() == 0 and(mapAnalyse[i][j] != -1) and
                            world->map().getCell(i, j).isInVision() and HERO_mapAnalyse[i][j] > 1 and
                            world->manhattanDistance(heroLocator(world, ID), cellLocator(world, i, j)) <= 4)
                        {
                            world->castAbility(world->getHero(ID), world->getHero(ID).getAbility(secondAbilityName), cellLocator(world, i, j));
                            break;
                        }
                    }
                }
            }
        }
    }

//----------------------------------------- PreProcess -----------------------------------------------------------------
void AI::preProcess(World *world)
{
    srand(time(0));
}

//----------------------------------------- Pick -----------------------------------------------------------------------
void AI::pick(World *world)
{
    cerr << "-pick" << endl;
    static int cnt = 0;

    switch (cnt)
    {
    case 0:
        world->pickHero(HeroName::BLASTER);
        break;
    case 1:
        world->pickHero(HeroName::BLASTER);
        break;
    case 2:
        world->pickHero(HeroName::BLASTER);
        break;
    case 3:
        world->pickHero(HeroName::SENTRY);
        break;
    default:
        world->pickHero(HeroName::BLASTER);
    }
    cnt++;
}

//----------------------------------------- Move -----------------------------------------------------------------------
void AI::move(World *world)
{
    cerr << "-move" << endl;

    fullAnalyse(world);

    if(world->getAP() > 80)
    {
        moveToCell(world, BLASTER_ID);
        moveToCell(world, BLASTER_ID_2);
        moveToCell(world, BLASTER_ID_3);

        moveToCell(world, SENTRY_ID); //TODO implement the vision array for sentry
    }
    /*
    static int targetRefreshPeriod = 0;
    if (targetRefreshPeriod <= 0) {
        srand(time(0) + world->getMyHeroes()[0]->getId());//made this so we can test two clients with this exact AI code
        for (int i = 0; i < 4; ++i) {
            if (WAR_IN_OBJECTIVE_ZONE) {
                std::vector<Cell *> obj_list = world->map().getObjectiveZone();

                Cell *targetCell = obj_list.at(rand() % obj_list.size());
                targetCellRow[i] = targetCell->getRow();
                targetCellColumn[i] = targetCell->getRow();
            } else {
                while (1) {
                    targetCellRow[i] = rand() % world->map().getRowNum();
                    targetCellColumn[i] = rand() % world->map().getColumnNum();

                    //Make sure the target is not a wall!
                    if (!world->map().getCell(targetCellRow[i], targetCellColumn[i]).isWall())
                        break;
                }
            }
        }
        targetRefreshPeriod = 10 * 6;// Change target locations after 20 complete moves
    } else
        targetRefreshPeriod--;

    vector<Hero *> my_heros = world->getMyHeroes();
    for (int i = 0; i < 4; ++i) {
        vector<Direction> _dirs = world->getPathMoveDirections(my_heros[i]->getCurrentCell().getRow(), my_heros[i]->getCurrentCell().getColumn(),
                                                               targetCellRow[i], targetCellColumn[i]);
        if (_dirs.size() == 0)//ALWAYS check if there is a path to that target!!!!
            continue;
        world->moveHero(my_heros[i]->getId(),_dirs[0]);
    }
*/
}

//----------------------------------------- Action ---------------------------------------------------------------------
void AI::action(World *world)
{
    cerr << "-action" << endl;

    fullAnalyse(world);

    SentryAbilityDone(world , SENTRY_ID , SENTRY_RAY , SENTRY_ATTACK );

    BlasterAbilityDone(world , BLASTER_ID , BLASTER_BOMB , BLASTER_ATTACK );
    BlasterAbilityDone(world , BLASTER_ID_2 , BLASTER_BOMB , BLASTER_ATTACK );
    BlasterAbilityDone(world , BLASTER_ID_3 , BLASTER_BOMB , BLASTER_ATTACK );


    }

    /*
for (Hero *my_hero : world->getMyHeroes())
{
    if (my_hero->getName() == HeroName::BLASTER)
    {
        //Find the closest bombing target
        Cell bombing_cell = Cell::NULL_CELL;
        int min_dist = 10000;
        for (Hero *opp_hero : world->getOppHeroes())
        {
            if (opp_hero->getCurrentCell().isInVision()) //if hero is seen
            {
                if (min_dist > world->manhattanDistance(opp_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell()))
                {
                    min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell());
                    bombing_cell = opp_hero->getCurrentCell();
                }
            }
        }
        //Perform the bombing
        if (bombing_cell != Cell::NULL_CELL)
        {
            world->castAbility(*my_hero, AbilityName::BLASTER_BOMB, bombing_cell);
        }
    }
    else if (my_hero->getName() == HeroName::GUARDIAN)
    {
        //Find the closest attacking target
        Cell attack_cell = Cell::NULL_CELL;
        int min_dist = 10000;
        for (Hero *opp_hero : world->getOppHeroes())
        {
            if (opp_hero->getCurrentCell().isInVision()) //if hero is seen
            {
                if (min_dist > world->manhattanDistance(opp_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell()))
                {
                    min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell());
                    attack_cell = opp_hero->getCurrentCell();
                }
            }
        }
        //Perform the attack
        if (attack_cell != Cell::NULL_CELL)
        {
            world->castAbility(*my_hero, AbilityName::GUARDIAN_ATTACK, attack_cell);
        }
    }
    else if (my_hero->getName() == HeroName::SENTRY)
    {
        //Find the closest shooting target
        Cell shoot_cell = Cell::NULL_CELL;
        int min_dist = 10000;
        for (Hero *opp_hero : world->getOppHeroes())
        {
            if (opp_hero->getCurrentCell().isInVision()) //if hero is seen
            {
                if (min_dist > world->manhattanDistance(opp_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell()))
                {
                    min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell());
                    shoot_cell = opp_hero->getCurrentCell();
                }
            }
        }
        //Perform the shooting
        if (shoot_cell != Cell::NULL_CELL)
        {
            world->castAbility(*my_hero, AbilityName::SENTRY_RAY, shoot_cell);
        }
    }
    else if (my_hero->getName() == HeroName::HEALER)
    {
        //Find the closest healing target
        Cell target_heal_cell = Cell::NULL_CELL;
        int min_dist = 10000;
        for (Hero *_hero : world->getMyHeroes())
        {
            if (min_dist > world->manhattanDistance(_hero->getCurrentCell(), my_hero->getCurrentCell()) and
                _hero->getRemRespawnTime() == 0 and
                _hero->getCurrentHP() != _hero->getMaxHP())
            {
                min_dist = world->manhattanDistance(_hero->getCurrentCell(),
                                                    my_hero->getCurrentCell());
                target_heal_cell = _hero->getCurrentCell();
            }
        }
        //Do the attack
        if (target_heal_cell != Cell::NULL_CELL)
        {
            world->castAbility(*my_hero, AbilityName::HEALER_HEAL, target_heal_cell);
        }
    }
}

*/

 printMap(world);

}
