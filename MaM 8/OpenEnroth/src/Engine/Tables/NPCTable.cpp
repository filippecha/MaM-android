#include "NPCTable.h"

#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Objects/Mm6Ids.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Objects/NPCEnumFunctions.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Random/Random.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/GameVariant.h"
#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::array<NPCTopic, 1100> pNPCTopics;
NPCStats *pNPCStats = nullptr;

int NPCStats::dword_AE336C_LastMispronouncedNameFirstLetter = -1;
int NPCStats::dword_AE3370_LastMispronouncedNameResult = -1;

//----- (00476977) --------------------------------------------------------
void NPCStats::InitializeNPCText(const Blob &npcText) {
    // npctext.txt table structure: index | text (localized) | dev notes | npc name (localized, not used).
    for (std::string_view line : split(npcText.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]) - 1; // File indices are 1-based, array is 0-based.
        pNPCTopics[i].pText = unquote(tokens[1]);
    }
}

void NPCStats::InitializeNPCTopics(const Blob &npcTopics) {
    // npctopic.txt table structure: index | topic (localized) | ??? (not used) | dev notes | text index (not used) |
    //                               npc name (not localized, not used) | npc index (not used).
    for (std::string_view line : split(npcTopics.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]);
        pNPCTopics[i].pTopic = unquote(tokens[1]);
    }
}

void NPCStats::InitializeNPCDist(const Blob &npcDist) {
    // npcdist.txt table structure: profession (localized, not used) | area profession chance values...
    for (auto [line, prof] : split(npcDist.str()).by("\r\n").drop(2).skip("").zip(Segment(NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST_MM7)))
        for (auto [token, map] : split(line).by('\t').drop(1).zip(allMaps()))
            pProfessionChance[map].chanceByProfession[prof] = fromString<int>(token);

    for (MapId map : allMaps())
        for (NpcProfession prof : allNpcProfessions())
            pProfessionChance[map].total += pProfessionChance[map].chanceByProfession[prof];
}

//----- (00476CB5) --------------------------------------------------------
void NPCStats::InitializeNPCData(const Blob &npcData) {
    // npcdata.txt table structure: index | name (localized) | portrait id | groups (not used) | house | profession |
    //                              greeting index | can join (y/n) | event ids 1-6 | dev notes |
    //                              map id (optional, not used).
    for (std::string_view line : split(npcData.str()).by("\r\n").drop(2).skip("").take(NPC_CAPACITY - 1)) {
        std::array<std::string_view, 16> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]); // File indices are 1-based.
        pNPCUnicNames[i] = unquote(tokens[1]);
        pOriginalNPCData[i].name = pNPCUnicNames[i];
        pOriginalNPCData[i].portraitId = fromString<int>(tokens[2]);
        pOriginalNPCData[i].house = static_cast<HouseId>(fromString<int>(tokens[6]));
        pOriginalNPCData[i].profession = static_cast<NpcProfession>(fromString<int>(tokens[7]));
        pOriginalNPCData[i].greetingIndex = fromString<int>(tokens[8]);
        pOriginalNPCData[i].canJoin = tokens[9][0] == 'y' ? 1 : 0;
        pOriginalNPCData[i].dialogue_1_evt_id = fromString<int>(tokens[10]);
        pOriginalNPCData[i].dialogue_2_evt_id = fromString<int>(tokens[11]);
        pOriginalNPCData[i].dialogue_3_evt_id = fromString<int>(tokens[12]);
        pOriginalNPCData[i].dialogue_4_evt_id = fromString<int>(tokens[13]);
        pOriginalNPCData[i].dialogue_5_evt_id = fromString<int>(tokens[14]);
        pOriginalNPCData[i].dialogue_6_evt_id = fromString<int>(tokens[15]);
    }
    uNumNewNPCs = isMm8() ? NPC_CAPACITY : 501;
}

void NPCStats::InitializeNPCGreets(const Blob &npcGreets) {
    // npcgreet.txt table structure: index | greeting 1 (localized) | greeting 2 (localized) |
    //                               notes (not localized, not used) | owner (not localized, not used).
    for (std::string_view line : split(npcGreets.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 3> tokens = split(line).by('\t');
        if (tokens[0].empty())
            continue; // Trailing orphan row with no index column.

        int i = fromString<int>(tokens[0]); // File indices are 1-based.
        pNPCGreetings[i].pGreeting1 = unquote(tokens[1]);
        pNPCGreetings[i].pGreeting2 = unquote(tokens[2]);
    }
}

void NPCStats::InitializeNPCGroups(const Blob &npcGroups) {
    // npcgroup.txt table structure: group index | news index | dev notes.
    for (std::string_view line : split(npcGroups.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]); // File indices are 0-based.
        pOriginalGroups[i] = fromString<int>(tokens[1]);
    }
}

void NPCStats::InitializeNPCNews(const Blob &npcNews) {
    // npcnews.txt table structure: index | text (localized) | dev notes.
    for (std::string_view line : split(npcNews.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]); // File indices are 0-based.
        pCatchPhrases[i] = unquote(tokens[1]);
    }
}

static bool startsWithDigit(std::string_view s) {
    return !s.empty() && s[0] >= '0' && s[0] <= '9';
}

void NPCStats::InitializeMm6(ResourceManager *resourceManager) {
    // npcdata.txt: index | name | portrait | state | fame | rep | house | profession | join | news | event A-C | notes.
    for (std::string_view line : split(resourceManager->eventsData("npcdata.txt").str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 13> tokens = split(line).by('\t');
        if (!startsWithDigit(tokens[0]))
            continue;
        int i = fromString<int>(tokens[0]);
        if (i <= 0 || i >= static_cast<int>(pOriginalNPCData.size()))
            continue;
        NPCData &npc = pOriginalNPCData[i];
        pNPCUnicNames[i] = unquote(tokens[1]);
        npc.name = pNPCUnicNames[i];
        npc.portraitId = fromString<int>(tokens[2]);
        npc.fame = fromString<int>(tokens[4]);
        npc.rep = fromString<int>(tokens[5]);
        npc.house = static_cast<HouseId>(fromString<int>(tokens[6]));
        npc.profession = npcProfessionFromMm6(fromString<int>(tokens[7]));
        npc.flags = NpcFlags(static_cast<uint32_t>(fromString<int>(tokens[3]) & 0x7F));
        npc.canJoin = tokens[8] == "1" || (!tokens[8].empty() && (tokens[8][0] == 'y' || tokens[8][0] == 'Y'));
        npc.field_24 = tokens[9] == "1" || (!tokens[9].empty() && (tokens[9][0] == 'y' || tokens[9][0] == 'Y'));
        npc.dialogue_1_evt_id = fromString<int>(tokens[10]);
        npc.dialogue_2_evt_id = fromString<int>(tokens[11]);
        npc.dialogue_3_evt_id = fromString<int>(tokens[12]);
    }
    uNumNewNPCs = 501;

    // npcnews.txt: index | map id | topic | text.
    mm6News.clear();
    for (std::string_view line : split(resourceManager->eventsData("npcnews.txt").str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 4> tokens = split(line).by('\t');
        if (!startsWithDigit(tokens[0]))
            continue;
        mm6News.push_back({startsWithDigit(tokens[1]) ? fromString<int>(tokens[1]) : 0, std::string(unquote(tokens[2])), std::string(unquote(tokens[3]))});
    }

    // npctext.txt: index | text | notes.
    for (std::string_view line : split(resourceManager->eventsData("npctext.txt").str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        if (!startsWithDigit(tokens[0]))
            continue;
        int i = fromString<int>(tokens[0]) - 1;
        if (i >= 0 && i < static_cast<int>(pNPCTopics.size()))
            pNPCTopics[i].pText = unquote(tokens[1]);
    }

    // npctopic.txt: index | topic.
    for (std::string_view line : split(resourceManager->eventsData("npctopic.txt").str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        if (!startsWithDigit(tokens[0]))
            continue;
        int i = fromString<int>(tokens[0]);
        if (i >= 0 && i < static_cast<int>(pNPCTopics.size()))
            pNPCTopics[i].pTopic = unquote(tokens[1]);
    }

    InitializeNPCNames(resourceManager->eventsData("npcnames.txt"));

    // npcbtb.txt: message number or Beg/Bribe/Threat | notes | a column for every personality, see MM6.exe 0x468120.
    static constexpr std::array<int, Mm6NpcTalkTable::PERSONALITY_COUNT> btbColumns = {7, 12, 11, 3, 10, 9, 0, 8, 5, 2, 1, 6, 4};
    for (std::string_view line : split(resourceManager->eventsData("npcbtb.txt").str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2 + Mm6NpcTalkTable::PERSONALITY_COUNT> tokens = split(line).by('\t').resize(2 + Mm6NpcTalkTable::PERSONALITY_COUNT, "");
        std::string_view key = trim(tokens[0]);
        int row = key == "Beg" ? 0 : key == "Bribe" ? 1 : key == "Threat" ? 2 : -1;
        int message = startsWithDigit(key) ? fromString<int>(key) : 0;
        for (int column = 0; column < Mm6NpcTalkTable::PERSONALITY_COUNT; column++) {
            std::string_view cell = tokens[2 + column];
            if (row >= 0)
                mm6Talk.allowed[row][btbColumns[column]] = trim(cell) == "1";
            else if (message >= 1 && message <= Mm6NpcTalkTable::MESSAGE_COUNT)
                mm6Talk.messages[message][btbColumns[column]] = unquote(cell);
        }
    }

    // proftext.txt: profession id | name | topic and text for every day of the week, starting with Sunday.
    for (std::string_view line : split(resourceManager->eventsData("proftext.txt").str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 16> tokens = split(line).by('\t').resize(16, "");
        if (!startsWithDigit(tokens[0]))
            continue;
        NpcProfession prof = npcProfessionFromMm6(fromString<int>(tokens[0]));
        if (prof == NoProfession)
            continue;
        for (int day = 0; day < 7; day++) {
            mm6DayTopics[prof][day] = unquote(tokens[2 + 2 * day]);
            mm6DayTexts[prof][day] = unquote(tokens[3 + 2 * day]);
        }
    }

    // Personalities in the order of MM6.exe 0x468C42. The exe only knows the English names, the Czech translation of
    // npcprof.txt uses its own.
    static const std::map<std::string, int, ascii::NoCaseLess> personalities = {
        {"Adventurer", 0}, {"Fanatic", 1}, {"Guard", 2}, {"Merchant", 3}, {"Noble", 4}, {"Official", 5}, {"Paladin", 6},
        {"Peasant", 7}, {"Priest", 8}, {"Scholar", 9}, {"Sorcerer", 10}, {"Thief", 11}, {"Monster", 12},
        {"Dobrodruh", 0}, {"Fanatik", 1}, {"Str\xe1\x9e", 2}, {"Kupec", 3}, {"\x8alechtic", 4}, {"Ceremoni\xe1\xf8", 5},
        {"Drnohryz", 7}, {"Kn\xec\x9e", 8}, {"U\xe8" "enec", 9}, {"\xc8" "arod\xecj", 10}, {"Zlod\xecj", 11},
    };
    constexpr int defaultPersonality = 11; // Thief, MM6.exe 0x468DB8.

    // npcprof.txt: id | name | random chance | hire price | personality | action text | benefit | join text.
    // MM6 has no npcdist.txt, the random chance column applies to every map.
    for (std::string_view line : split(resourceManager->eventsData("npcprof.txt").str()).by("\r\n").drop(4)) {
        std::array<std::string_view, 8> tokens = split(line).by('\t');
        if (!startsWithDigit(tokens[0]))
            continue;
        NpcProfession prof = npcProfessionFromMm6(fromString<int>(tokens[0]));
        if (prof == NoProfession)
            continue;
        pProfessions[prof].uHirePrice = startsWithDigit(tokens[3]) ? fromString<int>(tokens[3]) : 0;
        mm6Personality[prof] = valueOr(personalities, std::string(trim(tokens[4])), defaultPersonality);
        pProfessions[prof].pActionText = unquote(tokens[5]);
        pProfessions[prof].pBenefits = unquote(tokens[6]);
        pProfessions[prof].pJoinText = unquote(tokens[7]);
        int chance = startsWithDigit(tokens[2]) ? fromString<int>(tokens[2]) : 0;
        for (MapId map : allMaps())
            pProfessionChance[map].chanceByProfession[prof] = chance;
    }
    for (MapId map : allMaps()) {
        pProfessionChance[map].total = 0;
        for (NpcProfession prof : allNpcProfessions())
            pProfessionChance[map].total += pProfessionChance[map].chanceByProfession[prof];
    }
    uNumNPCProfessions = std::to_underlying(NPC_PROFESSION_LAST) + 1;
}

//----- (0047702F) --------------------------------------------------------
void NPCStats::Initialize(ResourceManager *resourceManager) {
    pOriginalNPCData.fill(NPCData());
    if (isMm6()) {
        InitializeMm6(resourceManager);
        return;
    }
    InitializeNPCData(resourceManager->eventsData("npcdata.txt"));
    InitializeNPCGreets(resourceManager->eventsData("npcgreet.txt"));
    InitializeNPCGroups(resourceManager->eventsData("npcgroup.txt"));
    InitializeNPCNews(resourceManager->eventsData("npcnews.txt"));
    InitializeNPCText(resourceManager->eventsData("npctext.txt"));
    InitializeNPCTopics(resourceManager->eventsData("npctopic.txt"));
    InitializeNPCDist(resourceManager->eventsData("npcdist.txt"));
    InitializeNPCNames(resourceManager->eventsData("npcnames.txt"));
    InitializeNPCProfs(resourceManager->eventsData("npcprof.txt"));
}

void NPCStats::InitializeNPCNames(const Blob &npcNames) {
    // npcnames.txt table structure: male name (localized) | female name (localized).
    // Female column runs out before the male column.
    uNewlNPCBufPos = 0;
    pNPCNames.fill({});
    for (std::string_view line : split(npcNames.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        if (!tokens[0].empty())
            pNPCNames[SEX_MALE].emplace_back(unquote(tokens[0]));
        if (!tokens[1].empty())
            pNPCNames[SEX_FEMALE].emplace_back(unquote(tokens[1]));
    }
}

void NPCStats::InitializeNPCProfs(const Blob &npcProfs) {
    // npcprof.txt table structure: profession id | profession name (localized, not used) | hire price |
    //                              action text (localized) | benefit description (localized) |
    //                              join text (localized) | dismiss text (localized).
    for (std::string_view line : split(npcProfs.str()).by("\r\n").drop(4)) {
        std::array<std::string_view, 7> tokens = split(line).by('\t');
        if (tokens[0].empty())
            continue; // Trailing pure-tab orphan rows past the last entry.

        NpcProfession prof = static_cast<NpcProfession>(fromString<int>(tokens[0]));
        pProfessions[prof].uHirePrice = fromString<int>(tokens[2]);
        pProfessions[prof].pActionText = unquote(tokens[3]);
        pProfessions[prof].pBenefits = unquote(tokens[4]);
        pProfessions[prof].pJoinText = unquote(tokens[5]);
        pProfessions[prof].pDismissText = unquote(tokens[6]);
    }
    uNumNPCProfessions = 59;
}

//----- (0047732C) --------------------------------------------------------
void NPCStats::InitializeAdditionalNPCs(NPCData *pNPCDataBuff, MonsterId npc_uid,
                                        HouseId uLocation2D, MapId uMapId) {
    int rep_gen;
    int uGeneratedPortret;    // ecx@23
    int test_prof_summ;       // ecx@37
    int max_prof_cap;         // edx@37
                              // signed int result; // eax@39
    Race uRace;                // [sp+Ch] [bp-Ch]@1
    bool break_gen;           // [sp+10h] [bp-8h]@1
    signed int gen_attempts;  // [sp+14h] [bp-4h]@1
    int uPortretMin;          // [sp+24h] [bp+Ch]@1
    int uPortretMax;

    MonsterType monsterType = monsterTypeForMonsterId(npc_uid);
    Sex uNPCSex = sexForMonsterType(monsterType);
    uRace = raceForMonsterType(monsterType);
    pNPCDataBuff->sex = uNPCSex;
    pNPCDataBuff->name = grng->randomSample(pNPCNames[uNPCSex]);

    gen_attempts = 0;
    break_gen = false;

    do {
        switch (uRace) {
            case RACE_HUMAN:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 2;
                    uPortretMax = 100;
                } else {
                    uPortretMin = 201;
                    uPortretMax = 250;
                }
            case RACE_ELF:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 400;
                    uPortretMax = 430;
                } else {
                    uPortretMin = 460;
                    uPortretMax = 490;
                }
                break;
            case RACE_GOBLIN:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 500;
                    uPortretMax = 520;
                } else {
                    uPortretMin = 530;
                    uPortretMax = 550;
                }
                break;
            case RACE_DWARF:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 300;
                    uPortretMax = 330;
                } else {
                    uPortretMin = 360;
                    uPortretMax = 387;
                }

                break;
        }

        uGeneratedPortret =
            uPortretMin + grng->random(uPortretMax - uPortretMin + 1);
        if (CheckPortretAgainstSex(uGeneratedPortret, uNPCSex))
            break_gen = true;
        ++gen_attempts;
        if (gen_attempts >= 4) {
            uGeneratedPortret = uPortretMin;
            break_gen = true;
        }
    } while (!break_gen);

    pNPCDataBuff->portraitId = uGeneratedPortret;
    pNPCDataBuff->flags = 0;
    pNPCDataBuff->fame = 0;
    // generate reputation
    rep_gen = grng->random(100) + 1;

    if (rep_gen >= 60) {
        if (rep_gen >= 90) {
            if (rep_gen >= 95) {
                if (rep_gen >= 98)
                    pNPCDataBuff->rep = -600;
                else
                    pNPCDataBuff->rep = 400;
            } else {
                pNPCDataBuff->rep = -300;
            }
        } else {
            pNPCDataBuff->rep = 200;
        }
    } else {
        pNPCDataBuff->rep = 0;
    }

    max_prof_cap = grng->random(pProfessionChance[uMapId].total);
    test_prof_summ = 0;
    pNPCDataBuff->profession = isMm6() ? NPC_PROFESSION_LAST : NPC_PROFESSION_LAST_MM7;
    for (NpcProfession i : allNpcProfessions()) {
        test_prof_summ += pProfessionChance[uMapId].chanceByProfession[i];
        if (test_prof_summ > max_prof_cap) {
            pNPCDataBuff->profession = i;
            break;
        }
    }
    pNPCDataBuff->house = uLocation2D;
    pNPCDataBuff->field_24 = 1;
    pNPCDataBuff->canJoin = 1;
    pNPCDataBuff->dialogue_1_evt_id = 0;
    pNPCDataBuff->dialogue_2_evt_id = 0;
    pNPCDataBuff->dialogue_3_evt_id = 0;
    pNPCDataBuff->dialogue_4_evt_id = 0;
    pNPCDataBuff->dialogue_5_evt_id = 0;
    pNPCDataBuff->dialogue_6_evt_id = 0;
}

//----- (00495366) --------------------------------------------------------
const std::string &NPCStats::sub_495366_MispronounceName(char firstLetter, Sex gender) {
    int pickedName;

    // TODO(captainurist): Caching in kinda wrong? Revisit when working on #mm6.
    //                     See "O Ho! %13! Er, %13. I think. Whatever..."

    if (firstLetter == dword_AE336C_LastMispronouncedNameFirstLetter) {
        pickedName = dword_AE3370_LastMispronouncedNameResult;
    } else {
        dword_AE336C_LastMispronouncedNameFirstLetter = firstLetter;
        const std::vector<std::string> &names = this->pNPCNames[gender];

        std::vector<int> matches;
        for (int i = 0; i < static_cast<int>(names.size()); ++i)
            if (tolower(names[i][0]) == tolower(firstLetter))
                matches.push_back(i);

        if (!matches.empty())
            pickedName = vrng->randomSample(matches);
        else
            pickedName = vrng->random(names.size()); // No name with this letter - pick any.
    }
    dword_AE3370_LastMispronouncedNameResult = pickedName;
    return this->pNPCNames[gender][pickedName];
}
