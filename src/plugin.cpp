#include <llapi/EventAPI.h>
#include <llapi/HookAPI.h>
#include <llapi/DynamicCommandAPI.h>
#include <llapi/LLAPI.h>
#include <llapi/LoggerAPI.h>
#include "version.h"

#include <Nlohmann/json.hpp>

#include <filesystem>

Logger logger (PLUGIN_NAME);

using json_t = nlohmann::json;

struct
{
    std::vector<std::string> allowList;
    std::vector<std::string> blackList;
    bool enableOverworld = false;
    bool enableNether = false;
    bool enableTheEnd = false;
    bool useRegex = false;
    std::string reloadCommandName = "voidreload";
    bool enableDebug = false;
    enum
    {
        LT_allow ,
        LT_black
    }type;
}settings;


void read ( )
{
    std::string d , t;
    std::fstream fin ("./plugins/void/settings.json" , std::ios::in);
    while ( std::getline (fin , t) )
    {
        d += t;
    }
    try
    {
        auto sf = json_t::parse (d , nullptr , true , true);
        auto t = sf ["allow_list"].get<json_t::array_t> ( );
        for ( auto &i : t )
        {
            settings.allowList.push_back (i.get<std::string> ( ));
        }
        t = sf ["black_list"].get<json_t::array_t> ( );
        for ( auto &i : t )
        {
            settings.blackList.push_back (i.get<std::string> ( ));
        }
        settings.enableOverworld = sf ["enable_overworld"].get<bool> ( );
        settings.enableNether = sf ["enable_nether"].get<bool> ( );
        settings.enableTheEnd = sf ["enable_the_end"].get<bool> ( );
        settings.useRegex = sf ["use_regex"].get<bool> ( );
        settings.reloadCommandName = sf ["reload_command_name"].get<std::string> ( );
        settings.enableDebug = sf ["enable_debug"].get<bool> ( );
        if ( sf ["list_type"].get<std::string> ( ) == "allow" )
        {
            settings.type = settings.LT_allow;
        }
        if ( sf ["list_type"].get<std::string> ( ) == "black" )
        {
            settings.type = settings.LT_black;
        }
    }
    catch ( json_t::exception &e )
    {
        logger.error (e.what ( ));
        return;
    }

}

#include <regex>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/BlockTypeRegistry.hpp>

#include <thread>


std::atomic_bool isFirstCallMatch = true;

std::vector<std::string> allowBlockList;
std::vector<std::string> blackBlockList;

bool match (const std::string &blockType)
{
    if ( isFirstCallMatch )
    {
        logger.debug ("onFirstCallMatch");
        if ( !settings.useRegex )
        {
            if ( settings.type == settings.LT_allow )
                for ( auto block : settings.allowList )
                {
                    allowBlockList.push_back (block);
                }
            if ( settings.type == settings.LT_black )
                for ( auto block : settings.blackList )
                {
                    blackBlockList.push_back (block);
                }
        }
        else
        {
            auto btr = Global<Level>->getBlockRegistry ( ).lock ( );
            if ( btr )
            {
                btr->forEachBlock ([ ] (const BlockLegacy &b)
                    {
                        if ( settings.useRegex )
                            if ( settings.type == settings.LT_allow )
                                for ( auto r : settings.allowList )
                                {
                                    std::regex _r (r);
                                    if ( std::regex_match (b.getNamespace ( ) + ":" + b.getRawNameId ( ) , _r) )
                                    {
                                        allowBlockList.push_back (b.getNamespace ( ) + ":" + b.getRawNameId ( ));
                                        logger.debug ("allow:");
                                        logger.debug (b.getNamespace ( ) + ":" + b.getRawNameId ( ));
                                    }
                                }
                            else if ( settings.type == settings.LT_black )
                                for ( auto r : settings.blackList )
                                {
                                    std::regex _r (r);
                                    if ( std::regex_match (b.getNamespace ( ) + ":" + b.getRawNameId ( ) , _r) )
                                    {
                                        blackBlockList.push_back (b.getNamespace ( ) + ":" + b.getRawNameId ( ));
                                        logger.debug ("black:");
                                        logger.debug (b.getNamespace ( ) + ":" + b.getRawNameId ( ));
                                    }
                                }
                        return true;
                    });
            }
        }
        isFirstCallMatch = false;
    }
    if ( settings.type == settings.LT_allow )
        for ( auto b : allowBlockList )
        {
            if ( blockType == b )
                return true;
        }
    if ( settings.type == settings.LT_black )
        for ( auto b : blackBlockList )
        {
            if ( blockType == b )
                return false;
        }
    if ( settings.type == settings.LT_allow )
        return false;
    else
        return true;
}

void PluginInit ( )
{
    logger.info ("void loaded");
    std::filesystem::create_directories ("./plugins/void/");
    if ( std::filesystem::is_regular_file ("./plugins/void/settings.json") )
        read ( );
    else
    {
        std::fstream fout ("./plugins/void/settings.json" , std::ios::out | std::ios::ate);
        fout << "{\"allow_list\":[],\"black_list\":[],\"use_regex\":false,\"enable_overworld\":true,\"enable_nether\":true,\"enable_the_end\":true,\"reload_command_name\":\"voidreload\",\"enable_debug\":false,\"list_type\":\"allow\"}";
    }
    if ( settings.enableDebug )
    {
        logger.consoleLevel = logger.debug.level;
    }
    using ParamType = DynamicCommand::ParameterType;
    using Param = DynamicCommand::ParameterData;
    DynamicCommand::setup (settings.reloadCommandName , "reload allow block list" , { {} } , {} , { {} } , [ ] (DynamicCommand const &cmd , CommandOrigin const &origin , CommandOutput &output , std::unordered_map<std::string , DynamicCommand::Result> &results)
        {
            settings.allowList.clear ( );
            read ( );
            allowBlockList.clear ( );
            blackBlockList.clear ( );
            isFirstCallMatch = true;
            if ( settings.enableDebug )
            {
                logger.consoleLevel = logger.debug.level;
            }
            else
            {
                logger.consoleLevel = logger.info.level;
            }
            output.success ("reload successfully");
        });
}
#define ENABLE_VIRTUAL_FAKESYMBOL_OVERWORLDGENERATORMULTINOISE
// #include <llapi/mc/WorldGenerator.hpp>
#define ENABLE_VIRTUAL_FAKESYMBOL_NETHERGENERATOR
// #include <llapi/mc/NetherGenerator.hpp>
#define ENABLE_VIRTUAL_FAKESYMBOL_THEENDGENERATOR
// #include <llapi/mc/TheEndGenerator.hpp>
#include <llapi/mc/BlockSource.hpp>
#include <llapi/mc/Biome.hpp>
#include <llapi/mc/LevelChunk.hpp>
#include <llapi/mc/Random.hpp>
#include <llapi/mc/ChunkBlockPos.hpp>
#include <llapi/mc/Block.hpp>
#include <llapi/mc/ChunkPos.hpp>
#include <llapi/mc/BiomeDecorationSystem.hpp>

#include <unordered_map>
#include <chrono>

THook (void , "?decorateWorldGenPostProcess\@OverworldGeneratorMultinoise\@\@EEBAXAEAVBiome\@\@AEAVLevelChunk\@\@AEAVBlockSource\@\@AEAVRandom\@\@\@Z" , void *_this , class Biome *biome , class LevelChunk *chunk , class BlockSource *bs , class Random *random)
{
    auto st = std::time (nullptr);
    if ( !settings.enableOverworld )
        original (_this , biome , chunk , bs , random);
    else
        for ( char x = 0;x < 16;x++ )
            for ( short y = -64;y < 300;y++ )
                for ( char z = 0;z < 16;z++ )
                {
                    if ( !match (chunk->getBlock ({ x,y,z }).getName ( )) )
                    {
                        chunk->setBlockSimple (ChunkBlockPos { x, y ,z } , *Block::create ("minecraft:air" , 0));
                    }
                }
    auto et = std::time (nullptr);
    logger.debug ("load chunk time:{}s,at(x:{},z:{},dim:{})" , et - st , chunk->getPosition ( ).x , chunk->getPosition ( ).z , "overworld");
}

THook (void , "?decorateWorldGenPostProcess\@NetherGenerator\@\@MEBAXAEAVBiome\@\@AEAVLevelChunk\@\@AEAVBlockSource\@\@AEAVRandom\@\@\@Z" , void *_this , class Biome *biome , class LevelChunk *chunk , class BlockSource *bs , class Random *random)
{
    auto st = std::time (nullptr);
    if ( !settings.enableNether )
        original (_this , biome , chunk , bs , random);
    else
        for ( char x = 0;x < 16;x++ )
            for ( short y = 0;y < 128;y++ )
                for ( char z = 0;z < 16;z++ )
                {
                    if ( !match (chunk->getBlock ({ x,y,z }).getName ( )) )
                    {
                        chunk->setBlockSimple (ChunkBlockPos { x, y ,z } , *Block::create ("minecraft:air" , 0));
                    }
                }
    auto et = std::time (nullptr);
    logger.debug ("load chunk time:{}s,at(x:{},z:{},dim:{})" , et - st , chunk->getPosition ( ).x , chunk->getPosition ( ).z , "nether");
}

THook (void , "?decorateWorldGenPostProcess\@TheEndGenerator\@\@MEBAXAEAVBiome\@\@AEAVLevelChunk\@\@AEAVBlockSource\@\@AEAVRandom\@\@\@Z" , void *_this , class Biome *biome , class LevelChunk *chunk , class BlockSource *bs , class Random *random)
{
    auto st = std::time (nullptr);
    if ( !settings.enableTheEnd )
        original (_this , biome , chunk , bs , random);
    else
        for ( char x = 0;x < 16;x++ )
            for ( short y = 0;y < 128;y++ )
                for ( char z = 0;z < 16;z++ )
                {
                    if ( !match (chunk->getBlock ({ x,y,z }).getName ( )) )
                    {
                        chunk->setBlockSimple (ChunkBlockPos { x, y ,z } , *Block::create ("minecraft:air" , 0));
                    }
                }
    auto et = std::time (nullptr);
    logger.debug ("load chunk time:{}s,at(x:{},z:{},dim:{})" , et - st , chunk->getPosition ( ).x , chunk->getPosition ( ).z , "the_end");
}
