# Might and Magic VIII – Android port „MaM 8"

MM8 je dopsaná přímo do enginu [OpenEnroth](https://github.com/OpenEnroth/OpenEnroth), stejně jako MaM 6. Žádná emulace Windows.

- Zdrojáky enginu s úpravami: `OpenEnroth/` (vychází z MaM 6, umí MM6, MM7 i MM8, historie úprav je v gitu tohoto repozitáře).
- Herní data z vlastní GOG instalace (v repozitáři nejsou, zkopíruj je sám): `gamedata/MM8/`. Engine z `MM8-Rel.exe` čte jen tabulky (třídy, dovednosti, kouzla, ceny, domy…), nespouští ho.
- Balíček `cz.mm8.game`, ikona draka, podpisový klíč `out-android/keystore.jks` (vytvoří ho první build, v repozitáři není).
- Návod pro Claude Code, jak APK sestavit: `out-android/NAVOD-port-MM8-android.md`.
- `docker-wine/` spouští originální MM8 pod Wine na PC (kontejner `mm8ref`), jen jako vzor pro porovnávání při vývoji. Do APK se nic z toho nedostane.

## Stav

Hratelné na PC i v emulátoru (tablet i rozměr telefonu), skutečný telefon zatím nevyzkoušený a hra není dohraná do konce.

Hotové: načtení dat MM8, hlavní menu, tvorba postavy, družina 1–5 postav a najímání v hostinci, všech 61 map, domy, obchody, učitelé, rozhovory a úkoly, rasové schopnosti a draci, kouzla, knihy (úkoly, poznámky, mapa, Town Portal, Lloydův maják), ukládání, aréna, Arcomage, artefakty, kořist z příšer, zvuky, konec hry.

Zbývá: projít hlavní příběh hraním a vyzkoušet APK na skutečném telefonu.

## Co je v enginu pro MM8

Rozlišení hry `isMm8()` (`Utility/GameVariant.h`), podle dat se pozná samo. Hlavní části:

- čísla předmětů, dovedností, tříd, kouzel a map MM8, tabulky z `MM8-Rel.exe` (`Engine/Objects/Mm8Ids`, `Mm8Roster`),
- MM8 příkazy a proměnné v událostech map (`Engine/Evt`), dungeony a mapy ve formátu MM8,
- rasy a rasové schopnosti (temní elfové, upíři, draci), liche, minotauři, trolové,
- artefakty, skupiny příšer, kořist a díly pro úkoly podle `MM8-Rel.exe`,
- MM8 vzhled oken (herní obrazovka, panáček postavy `GUI/UI/Mm8Paperdoll`, knihy, domy, hostinec),
- MM8 zvuky (jiná čísla než MM7), Arcomage v hospodách 107–117, certifikát na konci hry.

## Vývoj na PC (Docker)

- `bash tools/dev.sh <sekundy> "<kroky xdotool>"` přeloží desktopovou verzi a spustí hru s `gamedata/MM8`, snímky a log jsou v `devout/`. Proměnné pro testy: `OE_DEV_MAP`, `OE_DEV_HOUSE`, `OE_DEV_EVENT`, `OE_DEV_POS`, `OE_DEV_TALK`, `OE_DEV_ITEMS`, `OE_DEV_OFFHAND`, `OE_DEV_SPELL`, `OE_DEV_SPELL_SWEEP`, `OE_DEV_ROSTER`, `OE_DEV_MAP_TOUR`, `OE_DEV_DECOR` a další, popis v `Application/Game.cpp`. Nová hra: kroky v `tools/newgame-steps.txt`.
- `bash tools/crash.sh` spustí hru pod gdb a vypíše backtrace.
- `tools/lod.py` vypíše a rozbalí soubory z LOD archivů, `tools/pe.py` čte data z `MM8-Rel.exe`.
- Kontrola stylu a unit testy: `docker-desktop/test.sh` (výsledek `devout/test.log`).

## Build APK (PowerShell, z této složky)

Herní data nejsou v repozitáři: zkopíruj je z vlastní instalace do `gamedata\MM8` (seznam v návodu). Image `openenroth-android` je stejný jako pro MaM 7 (`..\MaM 7\docker`).

```
docker run --rm -v "${PWD}\OpenEnroth:/src:ro" -v "${PWD}\gamedata:/gamedata:ro" -v "${PWD}\out-android:/out" -v "${PWD}\docker-android:/dd:ro" -v oe8_work:/work -v oe_gradle:/root/.gradle openenroth-android bash -c "sed 's/\r$//' /dd/build.sh > /tmp/b.sh && bash /tmp/b.sh"
```

Výsledek: `out-android/MaM8.apk` (arm64, OpenGL ES 3.2, asi 660 MB). Na telefonu potřebuje asi 1,5 GB volného místa.

Pro emulátor (umí jen OpenGL ES 3.1): přidat `-e GITHUBARCH=x86_64`, výstup do `out-android-emu` s `prebuild.sh` z `..\MaM 7\out-emu` a volume `oe8emu_work`.

## Ovládání

Stejné jako MaM 7 a MaM 6 (`out-android/MaM8-telefon.png`, `MaM8-telefon-hra.png`, `MaM8-tablet.png`, `MaM8-tablet-rozhovor.png`): vlevo klávesy, meč, hůlka a otáčení, vpravo myš L/P, batoh, mapa, skok, skip a chůze. Dotyk do obrazu hry je kliknutí myší.

## Synchronizace uložených her

Stejná jako u MaM 7 (`OpenEnroth/android/.../SaveSync.java`, popis v `../MaM 7/README.md`). Při prvním spuštění se hra zeptá na složku (třeba `Dokumenty/MaM8`), změna podržením ikony hry → „Složka pro uložené hry“. Savy se kopírují při spuštění a při odchodu do pozadí, při změně na obou stranách vyhraje novější a starší jde do podsložky `zaloha`. Složku s Google Diskem drží synchronizační aplikace (Autosync, FolderSync). Ověřeno v emulátoru (převzetí souboru ze složky a jeho vrácení po odchodu ze hry), na telefonu zatím ne.
