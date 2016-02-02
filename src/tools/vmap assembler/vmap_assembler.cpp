/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>
#include <iostream>

#include "threading/Threading.h"
#include <g3dlite\G3DAll.h>
#include <vmaplib\VMapLib.h>

#define WaitForInput() { char cmd[300]; memset( cmd, 0, sizeof( cmd ) ); fgets( cmd, 300, stdin ); }

//=======================================================
int main(int argc, char* argv[])
{
    sLog.Init(7);
    std::string src, tiledest, objdest;
    if(argc == 4)
    {
        src = argv[1];
        tiledest = argv[2];
        objdest = argv[3];
    }
    else
    {
        std::cout << "Incorrect arg count, requesting input" << std::endl;
        std::cout << "Source folder" << std::endl;
        std::cin >> src;
        for(size_t len = 0; len < src.size(); len++)
            src[len] = tolower(src[len]);
        if(strcmp(src.c_str(), "exit") == 0)
            return 1;

        std::cout << "Tile output folder" << std::endl;
        std::cin >> tiledest;
        for(size_t len = 0; len < tiledest.size(); len++)
            tiledest[len] = tolower(tiledest[len]);
        if(strcmp(tiledest.c_str(), "exit") == 0)
            return 1;

        std::cout << "Object output folder" << std::endl;
        std::cin >> objdest;
        for(size_t len = 0; len < objdest.size(); len++)
            objdest[len] = tolower(objdest[len]);
        if(strcmp(objdest.c_str(), "exit") == 0)
            return 1;
    }

    std::cout << "using " << src << " as source directory and writing tile output to " << tiledest << " with object output to " << objdest <<  std::endl;

    VMAP::TileAssembler* ta = new VMAP::TileAssembler(src, tiledest, objdest);

    if(!ta->convertWorld2())
    {
        std::cout << "exit with errors" << std::endl;
        delete ta;
        WaitForInput();
        return 1;
    }

    delete ta;
    std::cout << "Ok, all done" << std::endl;
    WaitForInput();
    return 0;
}
