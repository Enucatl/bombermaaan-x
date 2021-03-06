/************************************************************************************

    Copyright (C) 2000-2002, 2007 Thibaut Tollemer
    Copyright (C) 2007, 2008 Bernd Arnold
	Copyright (C) 2008 Jerome Bigot

    This file is part of Bombermaaan.

    Bombermaaan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bombermaaan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bombermaaan.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************************/


/**
 *  \file CLevel.cpp
 *  \brief Handling a level
 */

#include "STDAFX.H"
#include "CLevel.h"
//#include "COptions.h"
//#include "CInput.h"
#include "CArena.h"
#include <sstream>
#include "../third-party/simpleini/SimpleIni.h"

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

CLevel::CLevel ( std::string filename_full, std::string filename_short )
{
    m_Filename_full = filename_full;
    m_Filename_short = filename_short;

    int i, j;

    //--m_LevelsData = NULL;
    for ( i = 0; i < ARENA_WIDTH; i++ )
    {
        for ( j = 0; j < ARENA_HEIGHT; j++ )
        {
            m_ArenaData[ i ][ j ] = BLOCKTYPE_HARDWALL;
        }
    }

	//--m_NumberOfItemsInWalls = NULL;
    for ( i = 0; i < NUMBER_OF_ITEMS; i++ )
    {
        m_NumberOfItemsInWalls[ i ] = 0;
    }

	//--m_InitialBomberSkills = NULL;
    for ( i = 0; i < NUMBER_OF_BOMBERSKILLS; i++ )
    {
        m_InitialBomberSkills[ i ] = 0;
    }
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

CLevel::~CLevel (void)
{
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

/**
 *  @param  filename_full    The full file name of the level file
 *  @param  filename_short   The file name of the level file without path
 */

bool CLevel::LoadFromFile()
{
    bool ErrorOccurred = false;    

    // Open the existing level file for reading
    ifstream in;
    in.open( m_Filename_full.c_str(), ios_base::in );

    // If it failed
    if (!in.is_open())
    {
        theLog.WriteLine ("Options         => Loading level file %s failed.", m_Filename_full.c_str() );
        // Stop loading levels
        return false;
    }


    // This is the first line for the level files beginning with version 2 (therefore "V2plus")
    string headerV2plus( "; Bombermaaan level file version=" );

    string s;
    getline( in, s );
    int LevelVersion;

    // When header string is found at the beginning of the string, find() returns 0 (offset 0)
    if ( s.find( headerV2plus ) == 0 ) {
        // We can look for the level version now
        LevelVersion = atoi( s.substr( headerV2plus.length() ).c_str() );
    }
    else
    {
        LevelVersion = 1;
    }
    
    switch ( LevelVersion ) {

        case 1:
            if (!LoadVersion1( in ) ) {
                ErrorOccurred = true;
            }
            break;

        case 2:
            if (!LoadVersion2( m_Filename_full ) ) {
                ErrorOccurred = true;
            }
            break;

        default:
            theLog.WriteLine ("Options         => !!! Unsupported version of level file %s.", m_Filename_short.c_str());
            ErrorOccurred = true;
            break;

    }

	// Close the level file
    in.close();
    
    // Validate this level if no error occurred so far
    if (!ErrorOccurred)
    {
        ErrorOccurred = !Validate();
    }

    // If there wasn't any problem
    if (!ErrorOccurred)
    {
        theLog.WriteLine ("Options         => Level file %s was successfully loaded (version %d).", m_Filename_short.c_str(), LevelVersion);
    }
    // If there was a problem
    else
    {
        theLog.WriteLine ("Options         => !!! Could not load level file %s (version %d).", m_Filename_short.c_str(), LevelVersion);
    }

    // If we had to stop then there is a problem.
    if (ErrorOccurred)
        return false;

    // Everything went right
    return true;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CLevel::LoadVersion1( ifstream& File ) {

    bool StopReadingFile = false;
    filebuf *pbuf = File.rdbuf();
    
    // go to the beginning
    pbuf->pubseekpos (0,ios::in);

    // For each line of characters to read
    for (int y = 0 ; y < ARENA_HEIGHT ; y++)
    {
        // Buffer where we'll store one line of characters. We'll read the two EOL characters as well.
        string Line;
        int ReadBytes;
    
        // Read one line of characters (including the EOL chars)
        if (File.good())
        {
            getline( File, Line );
            ReadBytes = Line.size();
        }
        else
        {
            ReadBytes = 0;
        }

        // Check if all the characters were read
        if (ReadBytes < ARENA_WIDTH)
        {
            // Log there is a problem
            theLog.WriteLine ("Options         => !!! Level file is incorrect (Line: %d, Length: %d).", y+1, ReadBytes);
        
            // Close the level file
            File.close();

            // Stop loading levels
            StopReadingFile = true;
            break;
        }

        // For each character representing a block in this line
        for (int x = 0 ; x < ARENA_WIDTH ; x++)
        {
            // According to the character value, store the corresponding block type in the current position and level
            switch(Line.c_str()[x])
            {
                case '*' : m_ArenaData[x][y] = BLOCKTYPE_HARDWALL;    break;
                case '-' : m_ArenaData[x][y] = BLOCKTYPE_RANDOM;      break;
                case ' ' : m_ArenaData[x][y] = BLOCKTYPE_FREE;        break;
                case '1' : m_ArenaData[x][y] = BLOCKTYPE_WHITEBOMBER; break;
                case '2' : m_ArenaData[x][y] = BLOCKTYPE_BLACKBOMBER; break;
                case '3' : m_ArenaData[x][y] = BLOCKTYPE_REDBOMBER;   break;
                case '4' : m_ArenaData[x][y] = BLOCKTYPE_BLUEBOMBER;  break;
                case '5' : m_ArenaData[x][y] = BLOCKTYPE_GREENBOMBER; break;
                case 'R' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_RIGHT; break;
                case 'D' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_DOWN;  break;
                case 'L' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_LEFT;  break;
                case 'U' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_UP;    break;
                default  : 
                {
                    // Log there is a problem
                    theLog.WriteLine ("Options         => !!! Level file is incorrect (unknown character %c).", Line[x]);
                
                    // Close the level file
                    File.close();

                    // Stop loading levels
                    StopReadingFile = true;
                    break;
                }
            }
        }

        // If there was a problem
        if (StopReadingFile)
        {
            // Stop reading this level file
            break;
        }
    }

    m_NumberOfItemsInWalls[ITEM_BOMB] = INITIAL_ITEMBOMB;
    m_NumberOfItemsInWalls[ITEM_FLAME] = INITIAL_ITEMFLAME;
    m_NumberOfItemsInWalls[ITEM_KICK] = INITIAL_ITEMKICK;
    m_NumberOfItemsInWalls[ITEM_ROLLER] = INITIAL_ITEMROLLER;
    m_NumberOfItemsInWalls[ITEM_SKULL] = INITIAL_ITEMSKULL;
    m_NumberOfItemsInWalls[ITEM_THROW] = INITIAL_ITEMTHROW;
    m_NumberOfItemsInWalls[ITEM_PUNCH] = INITIAL_ITEMPUNCH;
	m_NumberOfItemsInWalls[ITEM_REMOTE] = INITIAL_ITEMREMOTE;

    m_InitialBomberSkills[ BOMBERSKILL_FLAME ] = INITIAL_FLAMESIZE;
    m_InitialBomberSkills[ BOMBERSKILL_BOMBS ] = INITIAL_BOMBS;
    m_InitialBomberSkills[ BOMBERSKILL_BOMBITEMS ] = INITIAL_BOMBITEMS;
    m_InitialBomberSkills[ BOMBERSKILL_FLAMEITEMS ] = INITIAL_FLAMEITEMS;
    m_InitialBomberSkills[ BOMBERSKILL_ROLLERITEMS ] = INITIAL_ROLLERITEMS;
    m_InitialBomberSkills[ BOMBERSKILL_KICKITEMS ] = INITIAL_KICKITEMS;
    m_InitialBomberSkills[ BOMBERSKILL_THROWITEMS ] = INITIAL_THROWITEMS;
    m_InitialBomberSkills[ BOMBERSKILL_PUNCHITEMS ] = INITIAL_PUNCHITEMS;
	m_InitialBomberSkills[ BOMBERSKILL_REMOTEITEMS ] = INITIAL_REMOTEITEMS;
    
    return !StopReadingFile;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CLevel::LoadVersion2( std::string filename )
{
    // Define INI file
    CSimpleIniA iniFile(false, false, false);

    // Load INI file
    SI_Error rc = iniFile.LoadFile( filename.c_str() );
    if (rc<0) return false;

    std::string s;
    std::stringstream default_value;
    int value;

    // Read the width of the map and check whether it is allowed
    // At the moment the width is fix, but maybe the width can be changed in the future

    default_value.str(std::string());
    default_value << ARENA_WIDTH;
    std::stringstream width_line;
    width_line << iniFile.GetValue( "General", "Width", default_value.str().c_str() );
    width_line >> value;
    if ( value != ARENA_WIDTH ) {
        theLog.WriteLine ("Options         => !!! Invalid arena width %d. Only %d is allowed.", value, ARENA_WIDTH );
        return false;
    }

    // Read the height of the map and check whether it is allowed
    // At the moment the height is fix, but maybe the height can be changed in the future
    default_value.str(std::string());
    default_value << ARENA_HEIGHT;
    std::stringstream height_line;
    height_line << iniFile.GetValue( "General", "Height", default_value.str().c_str() );
    height_line >> value;
    if ( value != ARENA_HEIGHT ) {
        theLog.WriteLine ("Options         => !!! Invalid arena height %d. Only %d is allowed.", value, ARENA_HEIGHT );
        return false;
    }

    // Read the maximum number of players allowed with this level
    // At the moment this must be set to 5
    // Maybe this is changed in the future
    std::stringstream line;
    line << iniFile.GetValue( "General", "MaxPlayers", "5" );
    line >> value;
    if ( value != 5 ) {
        theLog.WriteLine ("Options         => !!! Invalid maximum players %d. Only %d is allowed.", value, 5 );
        return false;
    }

    // Read the maximum number of players allowed with this level
    // Currently this must be set to 1, though a game with 1 player is not possible
    // Maybe this is changed in the future
    std::stringstream min_players_line;
    min_players_line << iniFile.GetValue( "General", "MinPlayers", "1" );
    min_players_line >> value;
    if ( value != 1 ) {
        theLog.WriteLine ("Options         => !!! Invalid minimum players %d. Only %d is allowed.", value, 1 );
        return false;
    }

    // Check if there is a line with the creator
    // The creator can be empty, it's not stored anywhere at the moment
    std::string creator = iniFile.GetValue( "General", "Creator", "" );

    // Priority line following
    // The priority setting is not used currently
    // For future use:
    // - The levels are first sorted by priority and then by the file name
    std::stringstream priority_line;
    priority_line << iniFile.GetValue( "General", "Priority", "0" );
    priority_line >> value;

    // Comment line following (not used currently)
    std::string comment = iniFile.GetValue( "General", "Comment", "" );

    // Description line following (not used currently)
    std::string description = iniFile.GetValue( "General", "Description", "" );

    // For each line of characters to read
    for (int y = 0 ; y < ARENA_HEIGHT ; y++)
    {
        std::ostringstream oss;
        oss << "Line." << y;
        std::string keyName = oss.str();

        theLog.WriteLine (keyName.c_str());
        std::string arenaLine = iniFile.GetValue( "Map", keyName.c_str(), "" );

        if ( arenaLine.length() != ARENA_WIDTH ) {
            theLog.WriteLine ("Options         => !!! Level file is incorrect (Line.%d wrong length %d).", y, arenaLine.length() );
            return false;
        }

        // For each character representing a block in this line
        for (int x = 0 ; x < ARENA_WIDTH ; x++)
        {
            // According to the character value, store the corresponding block type in the current position and level
            switch(arenaLine.at(x))
            {
                case '*' : m_ArenaData[x][y] = BLOCKTYPE_HARDWALL;    break;
                case '-' : m_ArenaData[x][y] = BLOCKTYPE_RANDOM;      break;
                case ' ' : m_ArenaData[x][y] = BLOCKTYPE_FREE;        break;
                case '1' : m_ArenaData[x][y] = BLOCKTYPE_WHITEBOMBER; break;
                case '2' : m_ArenaData[x][y] = BLOCKTYPE_BLACKBOMBER; break;
                case '3' : m_ArenaData[x][y] = BLOCKTYPE_REDBOMBER;   break;
                case '4' : m_ArenaData[x][y] = BLOCKTYPE_BLUEBOMBER;  break;
                case '5' : m_ArenaData[x][y] = BLOCKTYPE_GREENBOMBER; break;
                case 'R' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_RIGHT; break;
                case 'D' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_DOWN;  break;
                case 'L' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_LEFT;  break;
                case 'U' : m_ArenaData[x][y] = BLOCKTYPE_MOVEBOMB_UP;    break;
                default  : 
                {
                    // Log there is a problem
                    theLog.WriteLine ("Options         => !!! Level file is incorrect (unknown character %c).", arenaLine.at(x) );
                    return false;
                }
            }
        }

    }

    //---------------------
    // Read the ItemsInWalls values
    //---------------------

    default_value.str(std::string());
    default_value << ITEM_BOMB;
    std::stringstream bombs_line;
    bombs_line << iniFile.GetValue( "Settings", "ItemsInWalls.Bombs", default_value.str().c_str() );
    bombs_line >> m_NumberOfItemsInWalls[ITEM_BOMB];

    default_value.str(std::string());
    default_value << ITEM_FLAME;
    std::stringstream flames_line;
    flames_line << iniFile.GetValue( "Settings", "ItemsInWalls.Flames", default_value.str().c_str() );
    flames_line >> m_NumberOfItemsInWalls[ITEM_FLAME];

    default_value.str(std::string());
    default_value << ITEM_KICK;
    std::stringstream kicks_line;
    kicks_line << iniFile.GetValue( "Settings", "ItemsInWalls.Kicks", default_value.str().c_str() );
    kicks_line >> m_NumberOfItemsInWalls[ITEM_KICK];

    default_value.str(std::string());
    default_value << ITEM_ROLLER;
    std::stringstream rollers_line;
    rollers_line << iniFile.GetValue( "Settings", "ItemsInWalls.Rollers", default_value.str().c_str() );
    rollers_line >> m_NumberOfItemsInWalls[ITEM_ROLLER];

    default_value.str(std::string());
    default_value << ITEM_SKULL;
    std::stringstream skulls_line;
    skulls_line << iniFile.GetValue( "Settings", "ItemsInWalls.Skulls", default_value.str().c_str() );
    skulls_line >> m_NumberOfItemsInWalls[ITEM_SKULL];

    default_value.str(std::string());
    default_value << ITEM_THROW;
    std::stringstream throws_line;
    throws_line << iniFile.GetValue( "Settings", "ItemsInWalls.Throws", default_value.str().c_str() );
    throws_line >> m_NumberOfItemsInWalls[ITEM_THROW];

    default_value.str(std::string());
    default_value << ITEM_PUNCH;
    std::stringstream punches_line;
    punches_line << iniFile.GetValue( "Settings", "ItemsInWalls.Punches", default_value.str().c_str() );
    punches_line >> m_NumberOfItemsInWalls[ITEM_PUNCH];

    default_value.str(std::string());
    default_value << ITEM_REMOTE;
    std::stringstream remotes_line;
    remotes_line << iniFile.GetValue( "Settings", "ItemsInWalls.Remotes", default_value.str().c_str() );
    remotes_line >> m_NumberOfItemsInWalls[ITEM_REMOTE];

    
    //---------------------
    // Read the BomberSkillsAtStart values
    //---------------------

    default_value.str(std::string());
    default_value << INITIAL_FLAMESIZE;
    std::stringstream skill_flame_size_line;
    skill_flame_size_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.FlameSize", default_value.str().c_str() ); 
    skill_flame_size_line >> m_InitialBomberSkills[BOMBERSKILL_FLAME];

    default_value.str(std::string());
    default_value << INITIAL_BOMBS;
    std::stringstream skill_initial_bombs_line;
    skill_initial_bombs_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.InitialBombs", default_value.str().c_str() ); 
    skill_initial_bombs_line >> m_InitialBomberSkills[BOMBERSKILL_BOMBS];

    default_value.str(std::string());
    default_value << INITIAL_BOMBITEMS;
    std::stringstream skill_bomb_items_line;
    skill_bomb_items_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.BombItems", default_value.str().c_str() ); 
    skill_bomb_items_line >> m_InitialBomberSkills[BOMBERSKILL_BOMBITEMS];

    default_value.str(std::string());
    default_value << INITIAL_FLAMEITEMS;
    std::stringstream skill_flame_line;
    skill_flame_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.FlameItems", default_value.str().c_str() ); 
    skill_flame_line >> m_InitialBomberSkills[BOMBERSKILL_FLAMEITEMS];

    default_value.str(std::string());
    default_value << INITIAL_ROLLERITEMS;
    std::stringstream skill_roller_line;
    skill_roller_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.RollerItems", default_value.str().c_str() ); 
    skill_roller_line >> m_InitialBomberSkills[BOMBERSKILL_ROLLERITEMS];

    default_value.str(std::string());
    default_value << INITIAL_KICKITEMS;
    std::stringstream skill_kick_line;
    skill_kick_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.KickItems", default_value.str().c_str() ); 
    skill_kick_line >> m_InitialBomberSkills[BOMBERSKILL_KICKITEMS];

    default_value.str(std::string());
    default_value << INITIAL_THROWITEMS;
    std::stringstream skill_throw_line;
    skill_throw_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.ThrowItems", default_value.str().c_str() ); 
    skill_throw_line >> m_InitialBomberSkills[BOMBERSKILL_THROWITEMS];

    default_value.str(std::string());
    default_value << INITIAL_PUNCHITEMS;
    std::stringstream skill_punch_line;
    skill_punch_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.PunchItems", default_value.str().c_str() ); 
    skill_punch_line >> m_InitialBomberSkills[BOMBERSKILL_PUNCHITEMS];

    default_value.str(std::string());
    default_value << INITIAL_REMOTEITEMS;
    std::stringstream skill_remote_line;
    skill_remote_line << iniFile.GetValue( "Settings", "BomberSkillsAtStart.RemoteItems", default_value.str().c_str() ); 
    skill_remote_line >> m_InitialBomberSkills[BOMBERSKILL_REMOTEITEMS];


    //---------------------
    // Read the ContaminationsNotUsed setting
    //---------------------

    // This setting controls which contamination should not be used in this level
    // The only one value allowed is "None" at the moment

    std::string contaminationsNotToUse = iniFile.GetValue( "Settings", "ContaminationsNotUsed", "" );


    // Everything went right
    return true;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

/**
 * @brief   check if this level does not exceed the maximum possible number of items
 * @param   sumOfMaxItems   pointer to an integer variable where the sum of max items is counted
 * @return  true if the number of maximum allowed items is not exceeded, false otherwise
 */
bool CLevel::CheckMaxNumberOfItems( unsigned int *sumOfMaxItems )
{
    // check if maximum number of items is not exceeded
    // we do this, because if there is a draw game when many bombers die
    // they all lose their items at the same time.
    *sumOfMaxItems = 0;
    unsigned int i;

    // count items in walls
    for (i = ITEM_NONE + 1; i < NUMBER_OF_ITEMS; i++)
    {
        *sumOfMaxItems += m_NumberOfItemsInWalls[i];
    }

    // count initial bomber skills (note: count the worst case with five players)
    for (i = BOMBERSKILL_DUMMYFIRST + 1; i < NUMBER_OF_BOMBERSKILLS; i++)
    {
        // initial skills like bombs and flames will not be lost
        if (i != BOMBERSKILL_FLAME && i != BOMBERSKILL_BOMBS)
            *sumOfMaxItems += m_InitialBomberSkills[i] * MAX_PLAYERS;
    }
        
    if (*sumOfMaxItems > MAX_ITEMS)
        return false;
    else
        return true;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CLevel::Validate()
{
    unsigned int itemCount;

    // There may be more checks in the future

    if ( !CheckMaxNumberOfItems( &itemCount ) ){
        // Log there is a problem
        theLog.WriteLine ("Options         => !!! Level file is incorrect (Too many items: %d of %d allowed).", itemCount, MAX_ITEMS);
        return false;
    }
    
    return true;
}
