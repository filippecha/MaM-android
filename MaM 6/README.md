# Might and Magic VI – Android port „MaM 6"

MM6 je dopsaná přímo do enginu [OpenEnroth](https://github.com/OpenEnroth/OpenEnroth), stejně jako MaM 7. Žádná emulace Windows.

- Zdrojáky enginu s úpravami: `OpenEnroth/` (výchozí stav je stejný commit jako MaM 7, viz `../MaM 7/engine-upstream.txt`; historie úprav je v gitu tohoto repozitáře).
- Herní data z vlastní GOG instalace (v repozitáři nejsou, zkopíruj je sám): `gamedata/MM6/`. Engine z `MM6.exe` čte jen tabulky (ceny, kouzla, dovednosti…), nespouští ho.
- Balíček `cz.mm6.game`, ikona draka z `MM6.exe`, podpisový klíč `out-android/keystore.jks` (vytvoří ho první build, v repozitáři není).
- Návod pro Claude Code, jak APK sestavit: `out-android/NAVOD-port-MM6-android.md`.
- `docker-wine/` spouští originální `MM6.exe` pod Wine na PC (`tools/ref.sh`), jen jako vzor pro porovnávání při vývoji. Grafiku mu dělá [cnc-ddraw](https://github.com/FunkyFr3sh/cnc-ddraw) v `docker-wine/cnc-ddraw/`. Do APK se nic z toho nedostane.

## Co je v enginu pro MM6

Rozlišení hry `isMm6()` (`Utility/GameVariant.h`), podle dat se pozná samo. Hlavní části:

- tabulky z `MM6.exe` (`Engine/Mm6ExeData`), MM6 čísla předmětů, dovedností, profesí a map (`Engine/Objects/Mm6Ids`), MM6 pravidla postav (`Mm6CharacterRules`), kouzla (`Engine/Spells/Mm6Spells`), lektvary (`Engine/Objects/Mm6Potions`),
- MM6 příkazy a proměnné v událostech map (`Engine/Evt`), hádanky a hesla,
- domy: hospody, obchody, gildy, trénink, chrámy, doprava, radnice (lov monster), aréna, věštec, orákulum, hrady a trůnní sály,
- rozhovory na ulici (pozdravy podle slávy a reputace, prosit / vyhrožovat / podplatit, téma dne, novinky, najímání) a učitelé (`GUI/UI/Mm6NpcTalk`, `Mm6NpcTopics`),
- MM6 vzhled panelů (`GUI/UI/Mm6Dialogue`), knihy (úkoly, poznámky, mapa, kalendář, Town Portal, Lloydův maják), herní menu, ukládání, nastavení, cestování pěšky mezi mapami,
- konec hry (videa, certifikát výhry, obrazovka prohry bez Rituálu prázdnoty), zvuky kouzel, hlášek a dveří podle tabulek MM6,
- české texty: co MM6 nemá, se bere z českého MM7 (`tools/gen_mm6_lstr.py`).

## Vývoj na PC (Docker)

- `bash tools/dev.sh <sekundy> "<kroky xdotool>"` přeloží desktopovou verzi a spustí hru s `gamedata/MM6`, snímky a log jsou v `devout/`. Proměnné pro testy: `OE_DEV_MAP`, `OE_DEV_HOUSE`, `OE_DEV_EVENT`, `OE_DEV_POS`, `OE_DEV_HOURS`, `OE_DEV_TALK`, `OE_DEV_ITEMS`, `OE_DEV_SPELL`, `OE_DEV_GOLD`, `OE_DEV_EXP`, `OE_DEV_REP`, `OE_DEV_AWARDS`, `OE_DEV_SPELL_SWEEP` (sešle postupně všechna kouzla), `OE_DEV_MAP_TOUR` (projde všechny mapy), popis v `Application/Game.cpp`. `DEV_GDB=1` vypíše backtrace výjimek, `DEV_GDB=Soubor.cpp:řádek` zastaví na daném místě, výsledek je v `devout/gdb.log`.
- `bash tools/house-tour.sh <čísla domů>` vyfotí domy.
- Kontrola stylu a unit testy: `docker-desktop/test.sh` (výsledek `devout/test.log`).

## Build APK (PowerShell, z této složky)

Herní data nejsou v repozitáři: zkopíruj je z vlastní instalace do `gamedata\MM6` (seznam v návodu). Image `openenroth-android` je stejný jako pro MaM 7 (`..\MaM 7\docker`).

```
docker run --rm -v "${PWD}\OpenEnroth:/src:ro" -v "${PWD}\gamedata:/gamedata:ro" -v "${PWD}\out-android:/out" -v "${PWD}\docker-android:/dd:ro" -v oe6_work:/work -v oe_gradle:/root/.gradle openenroth-android bash -c "sed 's/\r$//' /dd/build.sh > /tmp/b.sh && bash /tmp/b.sh"
```

Výsledek: `out-android/MaM6.apk` (arm64, OpenGL ES 3.2, asi 550 MB). Na telefonu potřebuje asi 1,2 GB volného místa.

Pro emulátor (umí jen OpenGL ES 3.1): přidat `-e GITHUBARCH=x86_64`, výstup do `out-android-emu` s `prebuild.sh` z `..\MaM 7\out-emu` a volume `oe6emu_work`.

## Ovládání

Stejné jako MaM 7 (`out-android/MaM6-telefon.png`, `out-android/MaM6-tablet.png`): vlevo klávesy, meč, hůlka a otáčení, vpravo myš L/P, batoh, mapa, skok, skip a chůze. Dotyk do obrazu hry je kliknutí myší.

## Synchronizace uložených her

Stejná jako u MaM 7 (`OpenEnroth/android/.../SaveSync.java`, popis v `../MaM 7/README.md`). Při prvním spuštění se hra zeptá na složku (třeba `Dokumenty/MaM6`), změna podržením ikony hry → „Složka pro uložené hry“. Savy se kopírují při spuštění a při odchodu do pozadí, při změně na obou stranách vyhraje novější a starší jde do podsložky `zaloha`. Složku s Google Diskem drží synchronizační aplikace (Autosync, FolderSync). Ověřeno v emulátoru (převzetí souboru ze složky a jeho vrácení po odchodu ze hry), na telefonu zatím ne.
