#include <stdlib.h>
#include "AI.h"
using namespace std;

int RowNum = 31;
int ColumnNum = 31;


int mapAnalyse[35][35];
int BLASTER_ID_4=-1;
int BLASTER_ID=-1;
int BLASTER_ID_2=-1;
int BLASTER_ID_3=-1;



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

            if (row==14 and column==14)
                cerr<<"O ";
            else {
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
        }
        cerr << endl;

    }
}
bool Isempty(World *world ,int i , int j)
{
    vector <Hero *> ourheros=world->getMyHeroes();
    bool res=true;
    Cell inputCell = Cell::NULL_CELL;
    inputCell=world->map().getCell(i,j);
    if (inputCell.isWall())
        return false;
    for (int i = 0; i <4 ; ++i) {
        if (inputCell==ourheros[i]->getCurrentCell())
            res=false;
    }
    return res;
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
        if (HeroName::BLASTER == world->getMyHeroes()[n]->getName())
        {
            if(BLASTER_ID != -1)
            {
                BLASTER_ID = world->getMyHeroes()[n]->getId();
            }
            else if(BLASTER_ID_2 !=-1)
            {
                BLASTER_ID_2 = world->getMyHeroes()[n]->getId();
            }
            else if(BLASTER_ID_3 !=-1)
            {
                BLASTER_ID_3 = world->getMyHeroes()[n]->getId();
            }
            else if(BLASTER_ID_4!=-1)
            {
                BLASTER_ID_4 = world->getMyHeroes()[n]->getId();
            }
        }
    }
}
/*Cell ColsestOBJZONE(World *world ,int Id)
{
    minDistance = 1000;
    minI = -1;
    minJ = -1;

    for (int i = 0; i < 31; ++i) //finding the closest obj zone cell
    {
        for (int j = 0; j < 31; ++j)
        {
            Cell alpha=world->map().getCell(i,j);
            if( alpha.isInObjectiveZone() and !alpha.isWall())
            {
                int len = world->manhattanDistance( world->getHero(Id).getCurrentCell(), world->map().getCell(i,j) );
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
}*/
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
bool checknextmove(World *world , int Id)
{
    Cell C=world->getHero(Id).getCurrentCell();
    bool res=false;
    for (int i = -1; i <=1 ; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if(!(world->map().getCell(C.getRow()+i,C.getColumn())==Cell::NULL_CELL))
                res= false;
        }

    }
    return res;
}
Cell findClosestCell(World *world, int ID)
{
    minDistance = 1000;
    int minrow = -1;
    int mincol = -1;
    int len=0;

    for (int i = 0; i < 31; ++i) //finding the closest obj zone cell
    {
        for (int j = 0; j < 31; ++j)
        {
            if(Isempty(world , i , j) and cellLocator(world,i,j).isInObjectiveZone())
            {
                len = world->manhattanDistance( heroLocator(world,ID), cellLocator(world,i,j) );
                if(len <= minDistance)
                {
                    minDistance = len;
                    minrow = i;
                    mincol = j;
                }
            }
        }
    }
    return cellLocator(world ,minrow ,mincol);
}
void moveToCell(World *world , int ID )  // TODO sometimes their direction is closed by another hero and their do nothing !
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
void SentryAbilityDone(World *world , int ID , AbilityName firstAbilityName , AbilityName secondAbilityName  )
{
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
    int istart=world->getHero(ID).getCurrentCell().getRow()-5;
    int jstart=world->getHero(ID).getCurrentCell().getColumn()-5;
    int iend=istart+10;
    int jend=jstart+10;

    for (int i=istart; i <=iend ; i++) {
        for (int j = jstart; j <= jend ; j++) {
            if (world->getHero(ID).getAbility(firstAbilityName).getCooldown() == 0 and
                world->map().getCell(i, j).isInVision() and HERO_mapAnalyse[i][j] > 1 and
                world->manhattanDistance(heroLocator(world, ID), cellLocator(world, i, j)) >=1 )
            {
                if(cellLocator(world,i,j).isWall())
                    world->castAbility(world->getHero(ID),world->getHero(ID).getAbility(firstAbilityName),cellLocator(world, i, j+1));
                else
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
Cell inrange(World *world,int Id)
{
    vector<Hero *> opheros = world->getOppHeroes();

    Cell AIM=world->map().getCell(0,0);
    int minhp=100;
    for (int i = 0; i < opheros.size(); ++i) {
        if(opheros[i]->getCurrentHP()<minhp and world->manhattanDistance(world->getHero(Id).getCurrentCell(),opheros[i]->getCurrentCell())<6)
            AIM=opheros[i]->getCurrentCell();
    }
    return AIM;
}
Cell minrange(World *world,int Id)
{
    vector<Hero *> opheros = world->getOppHeroes();

    Cell AIM=world->map().getCell(0,0);
    int len=25;
    for (int i = 0; i < opheros.size(); ++i) {
        if(world->manhattanDistance(world->getHero(Id).getCurrentCell(),opheros[i]->getCurrentCell())<len)
        {
            len=world->manhattanDistance(world->getHero(Id).getCurrentCell(),opheros[i]->getCurrentCell());
            AIM=opheros[i]->getCurrentCell();
        }
    }
    return AIM;
}
bool movebydic(World *world , int id, char direction)
{
    Cell startcell=world->getHero(id).getCurrentCell();
    switch (direction)
    {
        case 'f':
            if(Isempty(world,startcell.getRow()-1,startcell.getColumn()) and cellLocator(world ,startcell.getRow()-1,startcell.getColumn()).isInObjectiveZone() )
                world->moveHero(id,RIGHT);
            break;
        case'b':
            if(Isempty(world,startcell.getRow()+1,startcell.getColumn()) and cellLocator(world ,startcell.getRow()+1,startcell.getColumn()).isInObjectiveZone())
                world->moveHero(id,LEFT);
            break;
        case 'r':
            if(Isempty(world,startcell.getRow(),startcell.getColumn()+1) and cellLocator(world ,startcell.getRow(),startcell.getColumn()).isInObjectiveZone()+1)
                world->moveHero(id,DOWN);
            break;
        case 'l':
            if(Isempty(world,startcell.getRow()-1,startcell.getColumn()-1) and cellLocator(world ,startcell.getRow(),startcell.getColumn()).isInObjectiveZone()-1)
                world->moveHero(id,UP);
            break;
        default:
            return false;
    }
    return  true;
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
            world->pickHero(HeroName::GUARDIAN);
            break;
        default:
            world->pickHero(HeroName::BLASTER);
    }
    cnt++;
}
//----------------------------------------- Move -----------------------------------------------------------------------
void AI::move(World *world) {
    Cell lastcellofG=Cell::NULL_CELL;
    cerr << "-move" << endl;
    vector<Hero *> my_heros = world->getMyHeroes();
    vector<Hero *> opp_heros = world->getOppHeroes();
    std::vector<Cell *> objzone = world->map().getObjectiveZone();
    for (int i = 0; i < 4; ++i)
    {
        if (!my_heros[i]->getCurrentCell().isInObjectiveZone())
        {
            Cell targetCell = findClosestCell(world, my_heros[i]->getId());
            targetCellRow[i]    = targetCell.getRow();
            targetCellColumn[i] = targetCell.getColumn();
        }
        else
        {
            if(my_heros[i]->getName()==GUARDIAN)
            {
                int nearest=20,res=0;
                for (int j = 0; j <4 ; ++j) {
                    if (world->manhattanDistance(my_heros[i]->getCurrentCell(),opp_heros[j]->getCurrentCell())<nearest)
                    {
                        nearest=world->manhattanDistance(my_heros[i]->getCurrentCell(),opp_heros[j]->getCurrentCell());
                        res=j;
                    }
                }
                targetCellRow[i]=opp_heros[res]->getCurrentCell().getRow();
                targetCellColumn[i]=opp_heros[res]->getCurrentCell().getColumn();
            }
            else
            {
                if(i==0)
                {
                    targetCellRow[i]=objzone[(objzone.size()/2)-1]->getRow();
                    targetCellColumn[i]=objzone[(objzone.size()/2)-1]->getColumn()+4;
                }
                else if (i==1)
                {
                    targetCellRow[i]=objzone[(objzone.size()/2)-1]->getRow()+2;
                    targetCellColumn[i]=objzone[(objzone.size()/2)-1]->getColumn()+1;
                }
                else if(i==2)
                {
                    targetCellRow[i]=objzone[(objzone.size()/2)-1]->getRow();
                    targetCellColumn[i]=objzone[(objzone.size()/2)-1]->getColumn()-2;
                }
                else if(i==3)
                {
                    targetCellRow[i]=objzone[(objzone.size()/2)-1]->getRow();
                    targetCellColumn[i]=objzone[(objzone.size()/2)-1]->getColumn()+3;
                }
            }
        }
    }
    for (int i = 0; i < 4 ; ++i)
    {
        vector<Direction> _dirs = world->getPathMoveDirections(my_heros[i]->getCurrentCell().getRow(),
                                                               my_heros[i]->getCurrentCell().getColumn(),
                                                               targetCellRow[i],targetCellColumn[i]);

        if ((_dirs.size() != 0 and !my_heros[i]->getCurrentCell().isInObjectiveZone()) or (_dirs.size()!=0 and world->getAP()>80))
            if(my_heros[i]->getName()==GUARDIAN and lastcellofG.getRow()==targetCellRow[i] and lastcellofG.getColumn()==targetCellColumn[i])
                world->moveHero(my_heros[i]->getId(),_dirs[1]);
            else
                world->moveHero(my_heros[i]->getId(),_dirs[0]);
        if(my_heros[i]->getName()==GUARDIAN)
        {
            lastcellofG=cellLocator(world,targetCellRow[i],targetCellColumn[i]);
        }
    }
}

//----------------------------------------- Action ---------------------------------------------------------------------
void AI::action(World *world) {
    HeroAnalyse(world);
    int attack_cell_guardian_row = -1;
    int attack_cell_guardian_column = -1;
    cerr << "-action" << endl;
    for (Hero *my_hero : world->getMyHeroes()) {
        if (my_hero->getName() == HeroName::BLASTER) {
            int bombing_cell_row = -1;
            int bombing_cell_column = -1;
            int attacking_cell_row = -1;
            int attacking_cell_column = -1;
            for (int k = 0; k < 4; ++k) {
                if (world->manhattanDistance(my_hero->getCurrentCell(), world->getOppHeroes()[k]->getCurrentCell()) <=
                    5 and my_hero->getAbilities()[2]->getRemCooldown()==0) {
                    bombing_cell_column = world->getOppHeroes()[k]->getCurrentCell().getColumn();
                    bombing_cell_row = world->getOppHeroes()[k]->getCurrentCell().getRow();
                }
                if (world->manhattanDistance(my_hero->getCurrentCell(), world->getOppHeroes()[k]->getCurrentCell()) <=
                    4) {
                    attacking_cell_column = world->getOppHeroes()[k]->getCurrentCell().getColumn();
                    attacking_cell_row = world->getOppHeroes()[k]->getCurrentCell().getRow();
                }
            }
            if (my_hero->getCurrentHP() <= 100) {
                world->castAbility(my_hero->getId(), BLASTER_DODGE, my_hero->getCurrentCell().getRow() - 2,
                                   my_hero->getCurrentCell().getColumn() - 2);
            }
            if (bombing_cell_column != -1 && bombing_cell_row != -1) {
                cerr << "The BlasterBOMB is doing... \n ";
                world->castAbility(*my_hero, AbilityName::BLASTER_BOMB,
                                   world->map().getCell(bombing_cell_row, bombing_cell_column));
            }
            if (attacking_cell_column != -1 and attacking_cell_row != -1) {
                world->castAbility(*my_hero, AbilityName::BLASTER_ATTACK,
                                   world->map().getCell(attacking_cell_row, attacking_cell_column));
                cerr << "The BlasterAttack is doing... \n ";
            }
        }
        else
        {
            for (int k = 0; k < 4; ++k)
            {
                if (world->manhattanDistance(my_hero->getCurrentCell(), world->getOppHeroes()[k]->getCurrentCell()) <= 1)
                {
                    attack_cell_guardian_column = world->getOppHeroes()[k]->getCurrentCell().getColumn();
                    attack_cell_guardian_row = world->getOppHeroes()[k]->getCurrentCell().getRow();
                }
            }

        }
        if (attack_cell_guardian_column!=-1 and attack_cell_guardian_row!=-1)
        {
            world->castAbility(my_hero->getId(),GUARDIAN_ATTACK,attack_cell_guardian_row,attack_cell_guardian_column);
            cerr<<"Guard attack"<<endl;
        }
        for (int i = 0; i < 4; ++i)
        {
            if(world->getMyHeroes()[i]->getCurrentHP()<=150 and world->manhattanDistance(my_hero->getCurrentCell(),world->getMyHeroes()[i]->getCurrentCell())<=4 and my_hero->getAbilities()[2]->getRemCooldown() ==0)
            {
                world->castAbility(my_hero->getId(),GUARDIAN_FORTIFY,world->getMyHeroes()[i]->getCurrentCell().getRow(),world->getMyHeroes()[i]->getCurrentCell().getColumn());
                cerr<<"G FORITY"<<endl;
            }
        }
    }
}