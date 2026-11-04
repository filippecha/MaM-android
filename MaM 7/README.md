# Might and Magic VII – Android port „MaM 7"

- Engine: [OpenEnroth](https://github.com/OpenEnroth/OpenEnroth), přesný commit v `engine-upstream.txt`, naše změny v `nase-upravy.patch`, `nase-upravy-2-tablet-klavesnice.patch`, `nase-upravy-3-dialog-bez-klavesnice.patch`, `nase-upravy-4-synchronizace-savu.patch`, `nase-upravy-5-teleport-harmondale-ostrov.patch`, `nase-upravy-6-kraj-etapa1.patch`, `nase-upravy-7-kraj-etapa2.patch`, `nase-upravy-8-schody-pod-nohama.patch` `nase-upravy-9-arcomage-chybejici-hospody.patch` `nase-upravy-10-cheat-nesmrtelnost.patch`, `nase-upravy-11-gamepad-autosave.patch`, `nase-upravy-12-stavy-identifikace-vzit-vse.patch`, `nase-upravy-13-hd-grafika.patch`, `nase-upravy-14-mapa-oko-cheat-kouzla.patch`, `nase-upravy-15-cheat-svitek-golema.patch`, `nase-upravy-16-truhla-ukol-0-lamani.patch`, `nase-upravy-17-panacek-truhla-cheat.patch`, `nase-upravy-18-ikony-cestina-cheat.patch` a `nase-upravy-19-pochodne-na-zdi.patch` (aplikovat v tomto pořadí).
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

Deset stisků klávesy Y (tlačítko „Y křik“) za sebou – každý do dvou sekund od předchozího a bez jiné klávesy mezi nimi – zapne cheat: ve stavovém řádku se objeví „Nesmrtelnost, mana a posílení zapnuty“, postavy nedostávají žádné zranění a jejich zdraví i mana se drží na maximu. Zapnutý cheat drží i kouzla Požehnání (+15 k útoku), Osud (60) a Ochranu (místo smrti bezvědomí) na každé postavě a Hrdinství (+15 k poškození) a Den bohů (+60 ke všem vlastnostem), Pochodeň (síla 4, největší dosvit), Čarodějovo oko (na velmistra ukazuje na minimapě i velké mapě nestvůry, předměty i modré klikací stěny), Štít, Kamennou kůži (+15 k obraně), Ochranu před magií (na velmistra, deset zablokovaných kouzel, doplňuje se každý snímek), všech šest odolností (+40), Zrychlení (po vypršení cheatového nenastane slabost, pozná se podle `isGM`, `Engine.cpp`), Zjisti život (u nestvůr ukazuje zdraví), Pád pírka, Obětování (ohnivá aura na velmistra, pod cheatem pálí jen nepřátele, vesničané a stráže ve městech zůstanou mimo, `Party::immolationAffectedActors`), Létání a Chůzi po vodě na družině (bez sesilatele a bez úbytku many, jako od najatého NPC), síla odpovídá velmistrovi s dovedností 10 (`keepCheatBuff` v `src/Engine/Party.cpp`). Kouzla se každý snímek prodlouží na hodinu herního času, po vypnutí cheatu tedy do hodiny vyprší. Silnější kouzlo seslané družinou se nepřepíše. Osud na postavě OpenEnroth při útoku nepoužívá (počítá ho jen u nestvůr), u postav se tedy jen zobrazuje. Dalších deset Y cheat vypne. Nikam se neukládá: načtení pozice, nová hra i vypnutí hry ho zruší. Zapnutý cheat každý snímek ruší i všechny špatné stavy postav (prokletí, slabost, strach, opilost, šílenství, otravu, nemoc, ochromení, bezvědomí, smrt, zkamenění i vymazání). Spánek, zombie a „dobrý“ stav nechává. Předměty se nelámou: ani při zranění a otravě (`Character::ItemsPotionDmgBreak`), ani speciálním útokem nestvůr, který jinak láme zbraně a zbroje, proklíná a krade (`Character::ReceiveSpecialAttackEffect` pod cheatem nic neudělá), ani nepovedeným Očarováním předmětu (kouzlo jen selže, `CastSpellInfo.cpp`). Kód: `cheatImmortal` v `src/Engine/Party.h`, počítání v `src/Io/KeyboardInputHandler.cpp` (klávesa Y je zároveň „Ano“, proto se porovnávají klávesy, ne akce), úpravy `nase-upravy-10-cheat-nesmrtelnost.patch`, `nase-upravy-12-stavy-identifikace-vzit-vse.patch` `nase-upravy-14-mapa-oko-cheat-kouzla.patch` , `nase-upravy-15-cheat-svitek-golema.patch` , `nase-upravy-17-panacek-truhla-cheat.patch` a `nase-upravy-18-ikony-cestina-cheat.patch`. Kouzla ověřena v PC smyčce (hák `OE_DEV_CHEAT=1` zapne cheat, v panelu svítí Hrdinství, Den bohů, Pochodeň a Čarodějovo oko). Ověřeno na emulátoru (hláška po desátém stisku), samotné nezranitelnosti a rušení stavů jen v kódu.

## Svitek s díly golema

Kupní smlouva z úkolu na povýšení na čaroděje (Thomas Grey, Brakadská poušť) vypisuje, kam byly díly golema doručeny. Řádek je zelený, když družina má všechny jeho díly (v inventáři kterékoli postavy, v ruce nebo už vestavěné do golema na hradě), žlutý, když má jeden ze dvou (Deyja: obě nohy, Brakada: dvě hlavy). Golem potřebuje jen jednu hlavu, proto je řádek Brakady zelený, jakmile je některá hlava vestavěná. Přiřazení dílů k řádkům je ověřené v datech: truhly v mapách (Mohylové údolí trup, Tatalie pravá ruka, Avlee levá ruka, Deyja obě nohy, Brakada obě hlavy) a questbity 66–72, které golem nastavuje při vestavění (`global.evt`). Kód: `colorGolemDeliveries` v `src/GUI/UI/UIMessageScroll.cpp`, úprava `nase-upravy-15-cheat-svitek-golema.patch`. Ověřeno v PC smyčce (háky `OE_DEV_ITEMS=<id>,<id>` dá předměty první postavě, `OE_DEV_SCROLL=728` otevře svitek, `OE_DEV_QBIT` nastaví vestavěný díl).

## Čarodějovo oko na velké mapě

Když je aktivní Čarodějovo oko (kouzlo, najatý kartograf nebo `debug.wizard_eye`), kniha map (tlačítko mapy v knihovničce, ne minimapa) ukazuje totéž co minimapa, ale po celé mapě a jen v prozkoumaných částech: nestvůry červeně, vesničany zeleně, mrtvoly, od experta předměty na zemi a od mistra modře klikací stěny (dveře, páky) v podzemí. Venku rozhoduje políčko odkryté mapy (`IsMapCellPartiallyRevealed`), uvnitř sektor, jehož podlahu nebo stěnu družina viděla (`FACE_SeenByParty`). Minimapa ukazuje jen nestvůry poblíž (`ACTOR_NEARBY`), kniha map všechny v prozkoumané části. Kód: `drawWizardEyeMarkers` v `src/GUI/UI/Books/MapBook.cpp`, `nase-upravy-14-mapa-oko-cheat-kouzla.patch`. Ověřeno v PC smyčce na Smaragdovém ostrově a v Hradu Harmondale.

## Gamepad

Engine má ovládání gamepadem od začátku (levá páčka chůze, pravá rozhlížení, A akce, L1 útok, R1 rychlé kouzlo, Y skok, Start boj po tazích, křížek nahoru/dolů/vlevo poznámky, kniha kouzel, mapa), ale na Androidu se jeho události do hry nedostávaly: SDL je čte jen ze svého vlastního zobrazení, a nad ním leží dotykové ovládání. `OpenEnroth.java` teď tlačítka i páčky gamepadu předává SDL přímo (`dispatchKeyEvent`, `dispatchGenericMotionEvent`). V nabídkách, oknech, dialozích a domech gamepad navíc ovládá kurzor myši: páčky jím hýbou, A je levé tlačítko, X pravé, B je Escape, křížek kurzor posouvá po krocích (`GameWindowHandler::gamepadTick`). Ve 3D pohledu platí původní vazby. Pohyb, útok, rychlé kouzlo a boj po tazích se čtou každý snímek ze stavu kláves (`KeyboardController`), který engine plnil jen z klávesnice – teď do něj zapisuje i tlačítka a páčky gamepadu (páčka vychýlená přes polovinu je držená klávesa). Rozložení po připomínce testera: Y jen skáče, inventář je na křížku nahoru (poznámky mají záložku v knize úkolů). Uložený `openenroth.ini` nese všechny vazby, proto `GameStarter` staré rozložení (inventář i skok na Y) při startu přepíše. Ověřeno testerem na telefonu.

## Automatické ukládání po pěti minutách

Každých pět minut hraní (počítá se jen čas na herní obrazovce, ne v nabídkách a při videích) se hra uloží do `auto01.mm7` až `auto10.mm7` – postupně dokola, nejstarší se přepíše. Pořadí si pamatuje soubor `saves/auto.next`. Pozice se v seznamu načtení jmenují „Automaticky 01“ až „Automaticky 10“, ve stavovém řádku se objeví „Hra automaticky uložena (NN)“. Načtení pozice a nová hra odpočet začnou znovu. Kód: `periodicAutoSave` v `src/Engine/SaveLoad.cpp`, volá se každý snímek přes `frameHook` v `src/Engine/EngineGlobals.cpp`. Ověřeno v PC smyčce.

## Sdílená identifikace předmětů a nestvůr

Dovednosti Identifikace předmětů a Identifikace nestvůr platí pro celou družinu: stačí, když je má jedna živá postava, a ostatní identifikují na stejné úrovni. V dovednostech zůstávají jen u postavy, která se je naučila. Předměty: `Character::CanIdentify` v `src/Engine/Objects/Character.cpp` bere nejlepší součin násobku mistrovství a bodů ze všech živých postav (velmistr kdekoli identifikuje vše). Nestvůry: okno s informacemi o nestvůře (`MonsterPopup_Draw` v `src/GUI/UI/UIPopup.cpp`) zkusí identifikaci každou živou postavou a ukáže sjednocení toho, co která poznala. Mrtvé a vymazané postavy se nepočítají. Úprava `nase-upravy-12-stavy-identifikace-vzit-vse.patch`, ověřeno jen překladem.

## Tlačítko „Vzít vše“ v truhle

Okno truhly má nad tlačítkem „Zavřít“ tlačítko „Vzít vše“, které sebere všechny předměty i zlato stejně jako mezerník na PC (`Chest::GrabItem(true)`). Nová zpráva `UIMSG_CHEST_TakeAll` v `src/GUI/GUIEnums.h`, tlačítko v `src/GUI/UI/UIChest.cpp`. Pozadí tlačítka je textura tlačítka „Zavřít“ (`ib-bcu-c`), která má nápis zapečený v obrázku, proto se při otevření truhly zkopíruje, nápis se překreslí prázdným pruhem destičky a „Vzít vše“ se do kopie vykreslí písmem `create.fnt` (`GUIFont::DrawTextLineToBuff`, teď veřejná), které je stejné jako zapečené nápisy tlačítek, s tenkým tahem a čtvrtinovým vyhlazením okolo, takže vypadá jako „Zavřít“ pod ním. Kopie respektuje HD texturu tlačítka (`GraphicsImage::scale()`). `scale()` texturu nejdřív načte: dřív vracela 1, dokud se textura nenačetla, a u první truhly po nahrání hry nebo po změně rámu (volba cesty) vyšla kopie na telefonu 3× větší a její ledový nebo kamenný okraj zakryl obě tlačítka (`nase-upravy-16-truhla-ukol-0-lamani.patch`). Mezi „Vzít vše“ (400..435) a „Zavřít“ (445..480) prosvítal deset bodů vysoký tmavý pruh hlavního rozhraní, kopie je proto o deset řádků vyšší a vyplní je čistý kámen (nebo led) destičky kolem tlačítka: zrcadlené spodní řádky 31..34 a horní 0..4 (nase-upravy-17-panacek-truhla-cheat.patch, ověřeno v PC smyčce s `render_scale = 3` v Titánské pevnosti). Ověřeno v PC smyčce na Smaragdovém ostrově (truhla 2: předměty zmizely, zlato stouplo o 145).

## Klikání na panáčka postavy

Předměty na panáčkovi (zbroj, zbraně, boty…) se berou podle neprůhledných pixelů jejich obrázku (`HitMap` v `src/Engine/HitMap.h`). Bod kliknutí je v logických pixelech 640×480, ale HD obrázek předmětu má `scale()` pixelů na jeden logický, takže se testovala jen levá horní třetina každého předmětu a sundat něco bylo skoro nemožné. `HitMap` teď bod přepočítá do pixelů textury. Navíc klik do čtyř bodů vedle předmětu vezme nejbližší předmět (pro prst na telefonu). Totéž platí pro informace pravým tlačítkem na panáčkovi (nase-upravy-17-panacek-truhla-cheat.patch). Ověřeno v PC smyčce s `render_scale = 3`: klik do pravého dolního rohu zbroje ji sundá.

## Ikony kouzel u portrétů bez čtverce pozadí

Požehnání, Ochrana, Kladivové pěsti a Odražení bolesti mají u portrétu ikonu `isg-01..04-a/b/c` (pro každý rám). Originál je neprůhledný čtverec 17×17 vyříznutý z rámu u třetího portrétu, se stínem předmětu namalovaným na kameni (ledu, tmě). U ostatních portrétů pozadí nesedí a v HD byl vidět světlý čtverec a šedý stín. `tools/hd-upscale/isg_alpha.py <ICONS.LOD> <hd/icons>` k HD JPEG ikonám vyrobí PNG, ve kterém je průhledné jen pozadí a zůstane předmět se svým stínem (originál porovnaný s rámem v místě výřezu: pozadí, stín = ztmavený rám, zbytek předmět; hrana v HD podle barev, kotouč jako konvexní obal modrých pixelů). Nejdřív byl pryč i stín, bez něj ale ikony vypadaly divně. `applyHdIcon` v `src/Engine/Graphics/HdTextures.cpp` bere menší z průhlednosti originálu a HD souboru (nase-upravy-18-ikony-cestina-cheat.patch). Ověřeno v PC smyčce s `render_scale = 3`.

## Pochodně na zdi nezastaví družinu (Velechrám Měsíce)

Nástěnné pochodně `torch05`, `torch06` a `torch07` mají v `ddeclist.bin` (i v anglickém originále) poloměr kolize 100, stojací `torch01-03` jen 8. Pochodeň visící na zdi tak zabírala kruh dva metry do místnosti. Ve Velechrámu Měsíce na Evenmornu (d19) se po stisku tlačítka (událost 19) odsune kamenný blok (dveře 15) a otevře mezeru širokou 95, pochodeň na rohu (2068, 3524) ale družinu zastavila uprostřed mezery. Po načtení tabulky dekorací v `src/Engine/Engine.cpp` dostanou tyto tři pochodně poloměr 8 (nase-upravy-19-pochodne-na-zdi.patch). Ověřeno v PC smyčce: družina mezerou projde až k tlačítku. Ladicí háky `OE_DEV_GEOM=1` (geometrie mapy do `devout7/geom.txt`) a `OE_DEV_PROBE=1` (každý snímek poslední kolize a výška podlahy před družinou) jsou v `docker-desktop/devhooks.patch`.

## Čeština hlášek, které česká data nemají

OpenEnroth má pár textů napsaných přímo v kódu anglicky (`Localization::initialize` v `src/Engine/Localization.cpp`): zranění od Obětování („Obětování: 25 zranění, zasažené cíle: 2“), krádež, doba zotavení, nabitá hůlka, rychlé uložení, speciální útoky a schopnosti nestvůr v informacích o nestvůře a další. Teď jsou česky v cp1250 (nase-upravy-18-ikony-cestina-cheat.patch).

## Úkol „0“ v knize úkolů

Questbity 271 až 512 jsou v `quests.txt` nepoužité a mají místo textu „0“. Kniha úkolů je zobrazuje u každého nastaveného questbitu s textem, takže po dodání golema na hrad se objevil úkol „0“ (je to i v původní hře). `initializeQuests` v `src/Engine/Tables/QuestTable.cpp` text „0“ nechá prázdný a kniha ho vynechá (`nase-upravy-16-truhla-ukol-0-lamani.patch`).

## HD grafika

Jedna verze pro všechny, bez přepínače, vše v patchi `nase-upravy-13-hd-grafika.patch`:

1. **Vykreslení v rozlišení displeje.** Engine kreslil vše do framebufferu 640×480 a zvětšoval ho bez vyhlazení, souřadnice UI jsou v něm napevno (volba enginu `render_filter = 0` nefunguje, 2D vrstva zůstane v rohu). Framebuffer je teď k‑násobný: `graphics.render_scale`, 0 (výchozí) = nejmenší celé číslo, které pokryje okno (telefon 1440×1080 → 3×, tablet 2000×1250 → 3×, 4K → 4×), obraz se pak na displej zmenšuje, ne roztahuje. 3D pohled dostane skutečné pixely, 2D vrstvu zvětší GPU přes ortho projekci, herní kód, `pViewport` i myš zůstávají v logických 640×480 (`OpenGLRenderer::_renderScale`, `framebufferSize()`, viewport, scissor, čtení pixelů pro náhledy savů).
2. **HD textury stěn, terénu a oblohy.** 1163 textur z `bitmaps.lod` (bez palet, masek a cedulí s písmem, upscaler si vymýšlí písmena) zvětšeno 4× modelem Remacri, `gamedata/mm7/hd/bitmaps/` (JPEG kvalita 90, 12 průhledných PNG). `loadHdBitmap` (`src/Engine/Graphics/HdTextures.cpp`) v `Bitmaps_LOD_Loader` a v generátoru přechodových dlaždic (`TileGenerator::loadTile`), stejné odbarvení jako palety. `graphics.hd_textures`: 0 vypnuto, 2 poloviční, 4 plná, -1 (výchozí) = 4, na Androidu s méně než 6 GB paměti 2. JPEG dekóduje `stb_image` (`thirdparty/stb`, `src/Library/Image/Jpeg.cpp`). Sada musí být úplná, smíchané velikosti dlaždic terénu si v GL polích prohodí vrstvy. Souřadnice textur na stěnách jsou v datech hry v pixelech původní textury, proto `GraphicsImage` u HD bitmapy vrací logickou velikost (`scale()` = kolikrát je textura větší), renderer nahrává skutečné pixely (`rgba()`), stěna si pamatuje `BLVFace::texscale` a shadery `glbspshader`/`gloutbuild` dostanou poměr v bitech 20–22 atributů, aby UV i tok vody a lávy počítaly v logických pixelech. Bez toho byly vnitřní mapy a domy zvětšené 4× (první verze na telefonu, „rozbité prostředí“). Upscaler mírně posouvá tón (tmavší, kontrastnější), `pack.py` proto srovná průměr každého kanálu s originálem.
3. **HD UI.** 4205 obrázků z `icons.lod` (rámy, knihy, portréty, paper doll, animace interiérů domů, ikony předmětů) zvětšeno 4× Remacri, `gamedata/mm7/hd/icons/` (JPEG). Extraktor bere průhlednost jako engine: index palety 0 jen s příznakem 512 v hlavičce, jinak tyrkysový klíč (index 0 bývá černá písma a stínů, jeho zamalování dělalo nápisy bledé). `applyHdIcon` v loaderech ColorKey, Alpha a PCX nahradí barvy a zmenší je plošným filtrem přesně na `render_scale` (víc pixelů by framebuffer stejně zprůměroval), průhlednost zůstává z původního obrázku, takže klíčované barvy a alfa fungují jako dřív. `GraphicsImage` má `scale()` (pixely textury na logický pixel) a `width()`/`height()` vrací logickou velikost. HD dostávají i neprůhledné obrázky (`getImage_Solid`: obrázky kouzel v knize, pozadí oken), jen minimapy ne (`getImage_Solid(..., false)`), kód je čte po pixelech. Průhledné texely jsou černé a lineární filtrování zvětšeného obrázku z nich dělalo tmavý obrys (čáry v knize kouzel), `GraphicsImage::renderId` proto u nepaletových obrázků nahraje kopii, kde průhledné texely u okraje dostanou průměrnou barvu sousedů (alfa zůstává 0), a `DrawTwodVerts` kreslí s `GL_CLAMP_TO_EDGE`, aby se do okraje nemíchal protější okraj. Čáry 2D vrstvy (mapa podzemí v minimapě a v knize map) kreslí `GL_LINES` vždy jedním pixelem framebufferu, při zvětšeném framebufferu by byly tenké a bledé, `EndLines2D` je proto převádí na pásky široké jeden logický pixel.
4. **Sprity a písmo.** Nestvůry, NPC, předměty a dekorace jsou paletové sprity: 9192 z 9221 snímků používá jinou paletu, než sprite nese (barevné varianty nestvůr), a shader přes paletu dělá osvětlení. AI obrázek je barevný a zpět na indexy nejde, proto `Sprites_LOD_Loader` zvětšuje indexy Scale2x/Scale3x (`src/Library/Image/PixelScale.cpp`) podle `graphics.sprite_scale` (0 = podle `render_scale`, na Androidu nejvýš 2, Scale3x je devítinásobek paměti GPU). Totéž dostane atlas písma (`GUIFont::CreateFontTex`).

Výroba balíků (`tools/hd-upscale/`): `extract.py` a `extract_icons.py` vytáhnou obrázky z LOD (umí i MM8, postup v `../MaM 8/README.md`, průhledné oblasti vyplní inpaintingem), `upscale.py` je zvětší modelem Remacri přes spandrel (Docker image `mm7-upscale-gpu` z `Dockerfile.gpu`, `docker run --gpus all`, `PAD_MODE=wrap` pro dlaždice, `reflect` pro UI, na RTX 5070 obojí za pár minut), `pack.py` udělá JPEG/PNG balík. Ověřeno v PC smyčce (1280×960: ostré 3D, kliknutí do scény i do UI sedí, náhledy savů) a v emulátoru.

## Rozšíření „Stín nad Krajem“ (rozpracované)

Scénář a stav: [`dlc-kraj/SCENAR.md`](dlc-kraj/SCENAR.md). Obsah rozšíření je v `gamedata/mm7/DATA/kraj.lod` (vlastní archiv, původní LOD soubory se nemění), vytváří ho `python dlc-kraj/tools/gen_kraj.py gamedata/mm7/DATA/games.lod gamedata/mm7/DATA/kraj.lod`. Engine čte mapy, tabulky i obrázky nejdřív z `kraj.lod` a pak z původních archivů (`GamesLod`, `ResourceManager::krajData`, `LodTextureCache::openKraj`). Nová mapa má číslo 77 (`MAP_SHIRE`), řádek tabulky map je v `kmapstats.txt`. Úpravy enginu etapy 1: `nase-upravy-6-kraj-etapa1.patch`, etapy 2 (Hobitín: domy, obyvatelé, rozhovory, obrázky interiérů): `nase-upravy-7-kraj-etapa2.patch`. `kraj.lod` se generuje z vlastního `games.lod`, v repozitáři není.
