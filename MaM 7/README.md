# Might and Magic VII – Android port „MaM 7"

- Engine: [OpenEnroth](https://github.com/OpenEnroth/OpenEnroth), přesný commit v `engine-upstream.txt`, naše změny v `nase-upravy.patch`, `nase-upravy-2-tablet-klavesnice.patch`, `nase-upravy-3-dialog-bez-klavesnice.patch`, `nase-upravy-4-synchronizace-savu.patch` a `nase-upravy-5-teleport-harmondale-ostrov.patch` (aplikovat v tomto pořadí).
- Zdrojáky s úpravami: `OpenEnroth/`
- Herní data z vlastní GOG instalace (v repozitáři nejsou, zkopíruj je sám): `gamedata/mm7/`. Čeština je fanouškovský překlad (`Events.lod`, `ICONS.LOD`).
- Balíček `cz.mm7.game`, ikona gryfa z `MM7-Rel.exe`, podpisový klíč `out/keystore.jks` (vytvoří ho první build, v repozitáři není).
- Návod pro ostatní: `PROMPT-port-MM7-android.md`, `out/NAVOD-port-MM7-android.md`.

## Změny proti OpenEnroth

- `DataInstallerActivity`: při prvním spuštění rozbalí data z APK.
- `TouchControls`: ovládání v černých pruzích (viz `out/MaM7-ovladani.png`). Vlevo nahoře pohled Del/PgDn/End a let Ins/Home/PgUp, pod nimi Esc, Y křik, Mezera, Enter, meč (A), hůlka (S) a dole otáčení. Vpravo Myš L/P, batoh (I), mapa (M), X skok, B skip a dole dopředu/dozadu.
- Oprava pádu SDL 3.2.22 při změně schránky.
- Tablety (poměr stran pod 1,9:1): pruhy s ovládáním mají aspoň 150 dp a obraz hry se zmenší mezi ně. Meč a hůlka jsou na tabletu hned nad šipkami otáčení. Systémové lišty se skrývají přes `WindowInsetsController`. Pokud lišta zůstane (třeba panel úloh na tabletu), hra i ovládání se rozloží jen do viditelné plochy. Telefony (2:1 a širší) se chovají jako dřív.
- Zadávání textu (jméno postavy, název uložené hry, částka v bance) zobrazí klávesnici Androidu (`PlatformWindow::setTextInputActive`, sladěné se stavem psaní v `globalProcessMessages`).
- Test tabletu: vlastní AVD `Test_Tablet` (2000×1250, 240 dpi) ve složce `avd-test` (není v gitu), spouštět s `ANDROID_AVD_HOME` nastaveným na tu složku.
- Čeština: tolerance DOSového konce souboru (0x1A) v tabulkách a chybné velikosti v hlavičkách obrázků z překladu.
- Vypnuté střídání ročních období (`seasons_change`), jinak je na začátku hry sníh, který MM7 nikdy nemělo.

## Build (PowerShell, z této složky)

Herní data nejsou v repozitáři: zkopíruj složky `ANIMS`, `DATA`, `MUSIC` a `SOUNDS` z vlastní instalace do `gamedata\mm7`.

```
docker build -t openenroth-android docker
docker run --rm -v "${PWD}\OpenEnroth:/src:ro" -v "${PWD}\gamedata:/gamedata:ro" -v "${PWD}\out:/out" -v oe_work:/work -v oe_gradle:/root/.gradle openenroth-android
```

Výsledek: `out/MaM7.apk` (arm64, OpenGL ES 3.2).

## Test na emulátoru

Emulátor umí jen OpenGL ES 3.1. Build pro něj: přidat `-e GITHUBARCH=x86_64` a výstup do `out-emu`, kde `prebuild.sh` dočasně sníží verzi GLES.
**Před dalším buildem pro telefon smazat `/work/src/android/openenroth/build` ve volume `oe_work`**, jinak se do APK přimíchá x86_64 knihovna.

## Synchronizace uložených her

Při prvním spuštění se hra zeptá na složku pro uložené hry (lze odložit a později změnit podržením ikony hry → „Složka pro uložené hry“). Savy si hra dál ukládá do své soukromé složky (`files/.openenroth/saves`), při spuštění a při odchodu do pozadí je ale porovná s vybranou složkou (`SaveSync.java`):

- změněný jen na jedné straně se zkopíruje na druhou,
- změněný na obou stranách: vyhraje novější, starší jde do podsložky `zaloha` (s datem a „telefon“ nebo „slozka“ v názvu),
- nic se nemaže (smazaná pozice se ze složky vrátí).

Synchronizaci složky s Google Diskem dělá samostatná aplikace (např. Autosync for Google Drive nebo FolderSync). Ověřeno v emulátoru: převzetí save ze složky, nahrání nového save do složky, převzetí změněného save, kolize se zálohou. Na telefonu a tabletu s Google Diskem vyzkoušeno a funguje (28. 9. 2026).

## Teleport Hrad Harmondale ↔ Smaragdový ostrov

Na Smaragdový ostrov se v původní hře po odplutí nedá vrátit. Naše kopie enginu přidává dvě propojené plošiny (`updatePartyTeleporters` v `src/Application/Game.cpp`):

- **Hrad Harmondale:** před fontánou v sále za vchodem (souřadnice -5120, -180), družina přijde kousek před ni, čelem k fontáně.
- **Smaragdový ostrov:** na cestě, kde molo přechází v pevninu (12552, 1500), družina přijde na konec mola čelem k vesnici.
- Plošinu označuje modrá záře. Po šlápnutí na ni se ukáže běžné okno přechodu mezi mapami („Přejete si opustit …?“, nahoře název cíle), klepnutí do obrazu nebo OK přenese, Zavřít ne. Po příchodu se plošina nespustí, dokud z ní družina nesejde.
- Plošina na ostrově funguje až po opuštění ostrova (úkolový bit 136), jinak by šlo přeskočit úvodní úkoly.
- Herní data ani formát uložených her se nemění.

Vývojová smyčka na PC (desktopový build pod Xvfb, jako u MaM 6 a MaM 8): `bash tools/dev.sh <sekundy> "<kroky xdotool>"`, obrazovky a log v `devout7/`. Pomocné proměnné (jen ve vývojové kopii přes `docker-desktop/devhooks.patch`): `OE_DEV_MAP=<číslo mapy>`, `OE_DEV_POS=<x>,<y>,<směr>`, `OE_DEV_QBIT=<bit>`, `OE_DEV_LOGPOS=1`, `OE_DEV_DUMP=1` (přechody mezi mapami). Testy a kontrola stylu: `docker-desktop/test.sh`. Ověřeno na PC oběma směry (hrad → ostrov a ostrov → hrad), APK postavené, na telefonu zatím nevyzkoušené.

