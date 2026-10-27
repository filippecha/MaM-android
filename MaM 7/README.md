# Might and Magic VII – Android port „MaM 7"

- Engine: [OpenEnroth](https://github.com/OpenEnroth/OpenEnroth), přesný commit v `engine-upstream.txt`, naše změny v `nase-upravy.patch`, `nase-upravy-2-tablet-klavesnice.patch`, `nase-upravy-3-dialog-bez-klavesnice.patch`, `nase-upravy-4-synchronizace-savu.patch`, `nase-upravy-5-teleport-harmondale-ostrov.patch`, `nase-upravy-6-kraj-etapa1.patch`, `nase-upravy-7-kraj-etapa2.patch`, `nase-upravy-8-schody-pod-nohama.patch` `nase-upravy-9-arcomage-chybejici-hospody.patch` `nase-upravy-10-cheat-nesmrtelnost.patch`, `nase-upravy-11-gamepad-autosave.patch`, `nase-upravy-12-stavy-identifikace-vzit-vse.patch` a `nase-upravy-13-hd-grafika.patch` (aplikovat v tomto pořadí).
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
- Test tabletu: vlastní AVD `Test_Tablet` (2000×1250, 240 dpi, datový oddíl 16 GB kvůli 1,1 GB APK) ve složce `avd-test` (není v gitu), spouštět s `ANDROID_AVD_HOME` nastaveným na tu složku.
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

Vývojová smyčka na PC (desktopový build pod Xvfb, jako u MaM 6 a MaM 8): `bash tools/dev.sh <sekundy> "<kroky xdotool>"`, obrazovky a log v `devout7/`. Pomocné proměnné (jen ve vývojové kopii přes `docker-desktop/devhooks.patch`): `OE_DEV_MAP=<číslo mapy>`, `OE_DEV_POS=<x>,<y>,<směr>`, `OE_DEV_QBIT=<bit>`, `OE_DEV_LOGPOS=1`, `OE_DEV_DUMP=1` (přechody mezi mapami), `OE_DEV_CHEST=<číslo truhly>` (otevře truhlu aktuální mapy čtyři sekundy po vstupu do hry). Testy a kontrola stylu: `docker-desktop/test.sh`. Ověřeno na PC oběma směry (hrad → ostrov a ostrov → hrad), APK postavené, na telefonu zatím nevyzkoušené.


## Schody s průchozími čely (Titánská pevnost)

Obří schody v Titánské pevnosti mají čela stupňů označená jako průchozí (`FACE_ETHEREAL`). Družina jimi prošla, ocitla se pod dalším stupněm uvnitř schodiště a obraz zčernal. `BLV_ProcessPartyActions` (`src/Engine/Graphics/Indoor.cpp`) teď družinu, která stojí pod podlahou nejvýš o výšku schodu (128), postaví na ni. Úprava: `nase-upravy-8-schody-pod-nohama.patch`. Ověřeno na PC na uložené pozici z pevnosti (`OE_DEV_POS=-1900,-20300,512`, chůze nahoru i dolů), na telefonu zatím nevyzkoušeno.

Vývojová smyčka k tomu umí nahrát uloženou pozici: soubory `devout7/saves/*.mm7` se nabídnou v okně Nahrát. Další proměnné: `OE_DEV_ACTORS=1` (výpis nestvůr), `OE_DEV_DOORS=1` (dveře a plochy s událostí), `OE_DEV_LOGPOS=2` (poloha každý snímek).


## Arcomage v úspěších

Řádek „Vítězství v Arcomage“ na kartě Úspěchy má za počtem výher v závorce lokace hospod, kde družina ještě nevyhrála, třeba „(chybí: Brakadská poušť, Oblačné město, Jáma)“. Po výhře ve všech třinácti hospodách závorka zmizí. Kód: `missingArcomageTaverns` v `src/GUI/UI/UICharacter.cpp`, úprava `nase-upravy-9-arcomage-chybejici-hospody.patch`. Ověřeno na PC na uložené pozici.


## Cheat: nesmrtelnost a neomezená mana

Deset stisků klávesy Y (tlačítko „Y křik“) za sebou – každý do dvou sekund od předchozího a bez jiné klávesy mezi nimi – zapne cheat: ve stavovém řádku se objeví „Nesmrtelnost a neomezená mana zapnuta“, postavy nedostávají žádné zranění a jejich zdraví i mana se drží na maximu. Dalších deset Y cheat vypne. Nikam se neukládá: načtení pozice, nová hra i vypnutí hry ho zruší. Zapnutý cheat každý snímek ruší i všechny špatné stavy postav (prokletí, slabost, strach, opilost, šílenství, otravu, nemoc, ochromení, bezvědomí, smrt, zkamenění i vymazání). Spánek, zombie a „dobrý“ stav nechává. Předměty se při zranění ani otravě nelámou (`Character::ItemsPotionDmgBreak`). Kód: `cheatImmortal` v `src/Engine/Party.h`, počítání v `src/Io/KeyboardInputHandler.cpp` (klávesa Y je zároveň „Ano“, proto se porovnávají klávesy, ne akce), úpravy `nase-upravy-10-cheat-nesmrtelnost.patch` a `nase-upravy-12-stavy-identifikace-vzit-vse.patch`. Ověřeno na emulátoru (hláška po desátém stisku), samotné nezranitelnosti a rušení stavů jen v kódu.

## Gamepad

Engine má ovládání gamepadem od začátku (levá páčka chůze, pravá rozhlížení, A akce, L1 útok, R1 rychlé kouzlo, Y skok, Start boj po tazích, křížek nahoru/dolů/vlevo poznámky, kniha kouzel, mapa), ale na Androidu se jeho události do hry nedostávaly: SDL je čte jen ze svého vlastního zobrazení, a nad ním leží dotykové ovládání. `OpenEnroth.java` teď tlačítka i páčky gamepadu předává SDL přímo (`dispatchKeyEvent`, `dispatchGenericMotionEvent`). V nabídkách, oknech, dialozích a domech gamepad navíc ovládá kurzor myši: páčky jím hýbou, A je levé tlačítko, X pravé, B je Escape, křížek kurzor posouvá po krocích (`GameWindowHandler::gamepadTick`). Ve 3D pohledu platí původní vazby. Pohyb, útok, rychlé kouzlo a boj po tazích se čtou každý snímek ze stavu kláves (`KeyboardController`), který engine plnil jen z klávesnice – teď do něj zapisuje i tlačítka a páčky gamepadu (páčka vychýlená přes polovinu je držená klávesa). Rozložení po připomínce testera: Y jen skáče, inventář je na křížku nahoru (poznámky mají záložku v knize úkolů). Uložený `openenroth.ini` nese všechny vazby, proto `GameStarter` staré rozložení (inventář i skok na Y) při startu přepíše. Ověřeno testerem na telefonu.

## Automatické ukládání po pěti minutách

Každých pět minut hraní (počítá se jen čas na herní obrazovce, ne v nabídkách a při videích) se hra uloží do `auto01.mm7` až `auto10.mm7` – postupně dokola, nejstarší se přepíše. Pořadí si pamatuje soubor `saves/auto.next`. Pozice se v seznamu načtení jmenují „Automaticky 01“ až „Automaticky 10“, ve stavovém řádku se objeví „Hra automaticky uložena (NN)“. Načtení pozice a nová hra odpočet začnou znovu. Kód: `periodicAutoSave` v `src/Engine/SaveLoad.cpp`, volá se každý snímek přes `frameHook` v `src/Engine/EngineGlobals.cpp`. Ověřeno v PC smyčce.

## Sdílená identifikace předmětů a nestvůr

Dovednosti Identifikace předmětů a Identifikace nestvůr platí pro celou družinu: stačí, když je má jedna živá postava, a ostatní identifikují na stejné úrovni. V dovednostech zůstávají jen u postavy, která se je naučila. Předměty: `Character::CanIdentify` v `src/Engine/Objects/Character.cpp` bere nejlepší součin násobku mistrovství a bodů ze všech živých postav (velmistr kdekoli identifikuje vše). Nestvůry: okno s informacemi o nestvůře (`MonsterPopup_Draw` v `src/GUI/UI/UIPopup.cpp`) zkusí identifikaci každou živou postavou a ukáže sjednocení toho, co která poznala. Mrtvé a vymazané postavy se nepočítají. Úprava `nase-upravy-12-stavy-identifikace-vzit-vse.patch`, ověřeno jen překladem.

## Tlačítko „Vzít vše“ v truhle

Okno truhly má nad tlačítkem „Zavřít“ tlačítko „Vzít vše“, které sebere všechny předměty i zlato stejně jako mezerník na PC (`Chest::GrabItem(true)`). Nová zpráva `UIMSG_CHEST_TakeAll` v `src/GUI/GUIEnums.h`, tlačítko v `src/GUI/UI/UIChest.cpp`. Pozadí tlačítka je textura tlačítka „Zavřít“ (`ib-bcu-c`), která má nápis zapečený v obrázku, proto se při otevření truhly zkopíruje, nápis se překreslí prázdným pruhem destičky a „Vzít vše“ se do kopie vykreslí písmem `create.fnt` (`GUIFont::DrawTextLineToBuff`, teď veřejná), které je stejné jako zapečené nápisy tlačítek, s tenkým tahem a čtvrtinovým vyhlazením okolo, takže vypadá jako „Zavřít“ pod ním. Kopie respektuje HD texturu tlačítka (`GraphicsImage::scale()`). Ověřeno v PC smyčce na Smaragdovém ostrově (truhla 2: předměty zmizely, zlato stouplo o 145).

## HD grafika

Jedna verze pro všechny, bez přepínače, vše v patchi `nase-upravy-13-hd-grafika.patch`:

1. **Vykreslení v rozlišení displeje.** Engine kreslil vše do framebufferu 640×480 a zvětšoval ho bez vyhlazení, souřadnice UI jsou v něm napevno (volba enginu `render_filter = 0` nefunguje, 2D vrstva zůstane v rohu). Framebuffer je teď k‑násobný: `graphics.render_scale`, 0 (výchozí) = nejmenší celé číslo, které pokryje okno (telefon 1440×1080 → 3×, tablet 2000×1250 → 3×, 4K → 4×), obraz se pak na displej zmenšuje, ne roztahuje. 3D pohled dostane skutečné pixely, 2D vrstvu zvětší GPU přes ortho projekci, herní kód, `pViewport` i myš zůstávají v logických 640×480 (`OpenGLRenderer::_renderScale`, `framebufferSize()`, viewport, scissor, čtení pixelů pro náhledy savů).
2. **HD textury stěn, terénu a oblohy.** 1163 textur z `bitmaps.lod` (bez palet, masek a cedulí s písmem, upscaler si vymýšlí písmena) zvětšeno 4× modelem Remacri, `gamedata/mm7/hd/bitmaps/` (JPEG kvalita 90, 12 průhledných PNG). `loadHdBitmap` (`src/Engine/Graphics/HdTextures.cpp`) v `Bitmaps_LOD_Loader` a v generátoru přechodových dlaždic (`TileGenerator::loadTile`), stejné odbarvení jako palety. `graphics.hd_textures`: 0 vypnuto, 2 poloviční, 4 plná, -1 (výchozí) = 4, na Androidu s méně než 6 GB paměti 2. JPEG dekóduje `stb_image` (`thirdparty/stb`, `src/Library/Image/Jpeg.cpp`). Sada musí být úplná, smíchané velikosti dlaždic terénu si v GL polích prohodí vrstvy.
3. **HD UI.** 4205 obrázků z `icons.lod` (rámy, knihy, portréty, paper doll, animace interiérů domů, ikony předmětů) zvětšeno 4× Remacri, `gamedata/mm7/hd/icons/` (JPEG). `applyHdIcon` v loaderech ColorKey, Alpha a PCX nahradí barvy a zmenší je plošným filtrem přesně na `render_scale` (víc pixelů by framebuffer stejně zprůměroval), průhlednost zůstává z původního obrázku, takže klíčované barvy a alfa fungují jako dřív. `GraphicsImage` má `scale()` (pixely textury na logický pixel) a `width()`/`height()` vrací logickou velikost. Minimapy (`getImage_Solid`) se nenahrazují, kód je čte po pixelech.
4. **Sprity a písmo.** Nestvůry, NPC, předměty a dekorace jsou paletové sprity: 9192 z 9221 snímků používá jinou paletu, než sprite nese (barevné varianty nestvůr), a shader přes paletu dělá osvětlení. AI obrázek je barevný a zpět na indexy nejde, proto `Sprites_LOD_Loader` zvětšuje indexy Scale2x/Scale3x (`src/Library/Image/PixelScale.cpp`) podle `graphics.sprite_scale` (0 = podle `render_scale`, na Androidu nejvýš 2, Scale3x je devítinásobek paměti GPU). Totéž dostane atlas písma (`GUIFont::CreateFontTex`).

Výroba balíků (`tools/hd-upscale/`): `extract.py` a `extract_icons.py` vytáhnou obrázky z LOD (průhledné oblasti vyplní inpaintingem), `upscale.py` je zvětší modelem Remacri přes spandrel (Docker image `mm7-upscale-gpu` z `Dockerfile.gpu`, `docker run --gpus all`, `PAD_MODE=wrap` pro dlaždice, `reflect` pro UI, na RTX 5070 obojí za pár minut), `pack.py` udělá JPEG/PNG balík. Ověřeno v PC smyčce (1280×960: ostré 3D, kliknutí do scény i do UI sedí, náhledy savů) a v emulátoru.

## Rozšíření „Stín nad Krajem“ (rozpracované)

Scénář a stav: [`dlc-kraj/SCENAR.md`](dlc-kraj/SCENAR.md). Obsah rozšíření je v `gamedata/mm7/DATA/kraj.lod` (vlastní archiv, původní LOD soubory se nemění), vytváří ho `python dlc-kraj/tools/gen_kraj.py gamedata/mm7/DATA/games.lod gamedata/mm7/DATA/kraj.lod`. Engine čte mapy, tabulky i obrázky nejdřív z `kraj.lod` a pak z původních archivů (`GamesLod`, `ResourceManager::krajData`, `LodTextureCache::openKraj`). Nová mapa má číslo 77 (`MAP_SHIRE`), řádek tabulky map je v `kmapstats.txt`. Úpravy enginu etapy 1: `nase-upravy-6-kraj-etapa1.patch`, etapy 2 (Hobitín: domy, obyvatelé, rozhovory, obrázky interiérů): `nase-upravy-7-kraj-etapa2.patch`. `kraj.lod` se generuje z vlastního `games.lod`, v repozitáři není.
