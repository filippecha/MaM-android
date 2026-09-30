# Prompt: vlastní Android port Might and Magic VII (jedno APK s daty, čeština, dotykové ovládání)

Vlož celý text níže do Claude Code (desktop aplikace nebo CLI) spuštěného ve své pracovní složce na Windows.
Prompt staví port od nuly z veřejného OpenEnroth. Hotovou upravenou kopii enginu i s build skripty najdeš
ve veřejném repozitáři [MaM-android](https://github.com/filippecha/MaM-android) ve složce `MaM 7` (naše změny jsou tam i jako `nase-upravy*.patch`),
herní data v něm nejsou, ta musí mít každý vlastní.
Nejdřív si projdi „Co musíš mít připravené" a uprav blok „MOJE ÚDAJE".

---

## Co musíš mít připravené, než prompt pustíš

**Hra**
- Legálně koupená hra Might and Magic VII: For Blood and Honor z GOG.com.
- Hra musí být na PC **NAINSTALOVANÁ**, nestačí mít stažený instalační soubor `setup_might_and_magic_7….exe`.
  Spusť instalátor a nech hru nainstalovat (výchozí cesta je `C:\GOG Games\Might and Magic 7`).
- Ve složce hry musí být podsložky `ANIMS`, `DATA`, `MUSIC` a `SOUNDS` (dohromady asi 634 MB). Z nich se berou herní data.
- Hru stačí mít nainstalovanou, nemusí na PC jít spustit.

**Čeština (volitelné)**
- GOG prodává hru jen anglicky. Pokud chceš češtinu, sežeň si fanouškovský překlad MM7 a nainstaluj ho do hry na PC
  běžným způsobem, tj. přepíše soubory `DATA\Events.lod` a `DATA\ICONS.LOD`.
- Před počeštěním si původní anglické `Events.lod` a `ICONS.LOD` někam zazálohuj.
- Dabing zůstává anglický, čeština jsou texty a obrázky tlačítek.

**Počítač**
- Windows 10/11 64bit, ideálně 16 GB RAM a víc, asi 25 GB volného místa na disku
  (Docker image ~5 GB, zdrojáky a cache buildu, dvě kopie herních dat, hotová APK).
- **Docker Desktop** nainstalovaný, spuštěný a nepozastavený (WSL2 backend). Bez něj build nejde.
- **Git** a **Python 3** (s balíčkem Pillow, kvůli ikoně). Když chybí, řekni to Claudovi, poradí.
- Připojení k internetu, stahuje se zhruba 3 GB (Docker image, Android SDK/NDK, zdrojáky, závislosti).
- Čas: první průchod včetně stahování a testů zabere zhruba hodinu, samotný build pár minut.

**Testování (volitelné, ale doporučené)**
- Android Studio s vytvořeným emulátorem (AVD). Emulátor je x86_64 a umí jen OpenGL ES 3.1, prompt s tím počítá.
- Bez emulátoru dostaneš APK ověřené jen „na papíře" (obsah, podpis) a první skutečný test bude až na telefonu.

**Telefon**
- Android s 64bit ARM procesorem (arm64, to je prakticky každý telefon zhruba od roku 2017) a podporou **OpenGL ES 3.2**.
- Asi **1,3 GB volného místa** (APK ~640 MB + rozbalená data ~634 MB).
- Povolená instalace z neznámých zdrojů (Android se zeptá sám při instalaci).
- Do složky `Android/data` se nikam sahat nemusí, APK si data rozbalí samo.

**Synchronizace uložených her (volitelné)**
- Když chceš stejné pozice na telefonu i tabletu, nainstaluj si z Google Play synchronizační aplikaci
  (třeba Autosync for Google Drive nebo FolderSync). Hra si pak savy kopíruje do složky, kterou vybereš,
  a aplikace tu složku drží stejnou s Google Diskem na všech zařízeních.

**Právní poznámka**
- Hotové APK obsahuje tvoje herní data z GOG. Je jen pro tebe, nikam ho nenahrávej a nikomu neposílej.
  Kdo chce port taky, musí mít vlastní kopii hry a postavit si vlastní APK tímhle promptem.

---

## PROMPT (kopíruj odsud dolů)

```text
Chci si pro vlastní potřebu postavit Android port hry Might and Magic VII: For Blood and Honor
jako JEDNO samostatné APK, které po instalaci rovnou funguje. Vlastním legální kopii z GOG.com
a mám ji NAINSTALOVANOU na tomhle PC. APK bude obsahovat moje herní data, takže je jen pro mě
a nikam se nesmí nahrávat ani sdílet. Na to mě na konci upozorni.

MOJE ÚDAJE (doplň / uprav):
- Cesta k nainstalované hře: C:\GOG Games\Might and Magic 7
- Pracovní složka projektu: D:\Projekty\MaM7   (ideálně BEZ mezer v cestě)
- Název aplikace v telefonu: MaM 7
- ID balíčku: cz.mm7.game
- Jazyk hry: čeština (fanouškovský překlad už mám nainstalovaný ve hře, tj. přepsané
  DATA\Events.lod a DATA\ICONS.LOD)  / nebo: angličtina (originál GOG)
- Mám Docker Desktop: ano. Mám Android Studio (SDK + emulátor): ano/ne.

ZÁSADNÍ PRAVIDLA:
1. Do složky s originální hrou NIKDY nezapisuj, jen z ní čti. Pracuj na kopii dat ve složce projektu.
2. Než začneš cokoli velkého stahovat, kopírovat nebo stavět, napiš mi krátce plán a počkej
   na moje „ano". Potom už pracuj samostatně a neptej se na každý krok.
3. Nic netvrď bez ověření. U každé věci mi řekni, jestli je ověřená (a jak), nebo jen předpokládaná.
4. V ničem, co uvidím v telefonu (název, ikona, ID balíčku, název APK souboru), nesmí být
   „OpenEnroth". Je to můj projekt. Engine je LGPL, na to mě jen upozorni.
5. Komunikuj česky.

KROK 0: OVĚŘ PŘEDPOKLADY (nic přitom neměň). Když něco chybí, zastav se a řekni mi, co mám doplnit:
- Hra je opravdu NAINSTALOVANÁ, nejen stažený instalátor: ve složce hry existují podsložky
  ANIMS, DATA, MUSIC, SOUNDS a v nich soubory anims\magic7.vid, anims\might7.vid,
  data\bitmaps.lod, data\events.lod, data\games.lod, data\icons.lod, data\sprites.lod,
  data\d3dbitmap.hwl, data\d3dsprite.hwl, sounds\audio.snd a hudba music\*.mp3.
  Pokud najdeš jen setup_might_and_magic_7*.exe, řekni mi, ať hru nejdřív nainstaluju.
- Jazyk dat: přečti global.txt z DATA\Events.lod (viz sekce E) a řekni mi, jestli je hra
  česky nebo anglicky. Pokud chci češtinu a data jsou anglická, zastav se.
- Docker Desktop běží a není pozastavený (docker info / docker ps projde).
- Je k dispozici git a Python 3 s Pillow. Je dost místa na disku (aspoň ~25 GB).
- Zjisti, jestli mám Android SDK a emulátor (%LOCALAPPDATA%\Android\Sdk, emulator -list-avds).

TECHNICKÝ POSTUP (ověřená cesta, drž se jí):

A) Engine
- Použij open-source engine OpenEnroth (github.com/OpenEnroth/OpenEnroth), klonuj mělce včetně
  submodulů. Android projekt je ve složce android/ hlavního repa (samostatné repo
  OpenEnroth_Android je archivované, to nepoužívej).
- Engine na Androidu hledá data mimo jiné v INTERNÍM úložišti aplikace (getFilesDir).
  Ověř si to v src/Application/Startup/PathResolver.cpp a GameStarter.cpp. Díky tomu není
  potřeba sahat do Android/data, kam novější Androidy uživatele nepustí.

B) Build: jen v Dockeru, ne nativně na Windows
- Nativní build na Windows je slepá ulička: Gradle 7.4 + AGP 7.3.1 potřebuje JDK 17 (ne 21),
  NDK má problém s mezerami v cestě a LuaJIT si při cross-kompilaci staví hostitelské nástroje
  (minilua, buildvm), což build skripty umí jen na Linuxu/macOS.
- Udělej Docker image z ubuntu:24.04 s: openjdk-17, git, ninja, build-essential, rsync, unzip,
  wget; Android cmdline-tools + sdkmanager: platform-tools, platforms;android-31,
  build-tools;30.0.3, ndk;28.1.13356709 (verzi NDK ověř v android/openenroth/build.gradle).
- CMake z Ubuntu (3.28) NESTAČÍ, submodul ztd_text chce 3.31+. Stáhni oficiální tarball
  CMake 3.31.x od Kitware do /opt/cmake a dej ho na PATH před systémový.
- Licence Android SDK neodklikávej za mě přes „yes |". Pokud mám Android Studio, zkopíruj moje
  už odsouhlasené soubory z %LOCALAPPDATA%\Android\Sdk\licenses do image. Jinak se mě zeptej.
- Build skript v kontejneru: rsync repa z read-only mountu /src do pojmenovaného volume /work
  (build na bind-mountu z Windows je pomalý), opravit CRLF v gradlew (sed) a spouštět ho přes
  „bash ./gradlew", ne „sh". Gradle cache drž v dalším volume.
- Závislosti (SDL3, FFmpeg, OpenAL…) si CMake stáhne předkompilované z OpenEnroth_Dependencies,
  nestav je ručně.
- Stav jen ABI arm64-v8a (proměnná prostředí GITHUBARCH=arm64-v8a), task assembleRelease.
- Podepisování: vygeneruj keytoolem vlastní keystore do výstupní složky a předej ho přes
  SIGNING_KEYSTORE / SIGNING_STORE_PASSWORD / SIGNING_KEY_ALIAS / SIGNING_KEY_PASSWORD.
  Keystore si musím schovat, jinak příští verze nepůjde nainstalovat přes starou.
- Pokud Docker Desktop nejde spustit s chybou „…sock.stale: The file cannot be accessed by the
  system", jsou to osiřelé AF_UNIX sockety. Nejdou smazat, ale jde PŘEJMENOVAT celé složky
  %LOCALAPPDATA%\Docker\run a %LOCALAPPDATA%\docker-secrets-engine (když Docker neběží).
  Nikdy neklikej na „Reset to factory defaults". Pokud je Docker ručně pozastavený (paused),
  neobcházej to a řekni mi to.

C) Data uvnitř APK + instalátor
- Z instalace zkopíruj jen složky ANIMS, DATA, MUSIC, SOUNDS (cca 634 MB) do
  <projekt>\gamedata\mm7\. Do build.gradle přidej volitelnou property GAME_DATA_DIR, která
  přidá tuhle složku do sourceSets.main.assets.srcDirs, a aaptOptions noCompress pro
  lod/LOD, vid/VID, snd/SND, hwl/HWL, mp3/MP3 (rychlé rozbalování, openFd funguje).
- Napiš novou Activity „DataInstallerActivity" (čistá Java, bez XML, minSdk 16) a nastav ji
  v manifestu jako MAIN/LAUNCHER místo herní aktivity. Při prvním spuštění: zkontroluje volné
  místo, rekurzivně zkopíruje assets/mm7/* do getFilesDir() s ukazatelem průběhu v MB
  (české texty ve values-cs), drží zapnutou obrazovku, zapíše značkový soubor vázaný na
  lastUpdateTime balíčku a spustí herní aktivitu. Když značka existuje, spouští hru rovnou.

D) Název, ID, ikona
- app_name = můj název, applicationId = moje ID (namespace Java tříd nech org.openenroth.game,
  kvůli JNI/SDL), výstupní soubor pojmenuj podle mého názvu.
- Ikonu vytáhni z originálního MM7-Rel.exe (skupina ikon obsahuje 64×64 v plných barvách, gryf).
  Nejjednodušší je Python + ctypes (LoadLibraryEx jako datafile, RT_GROUP_ICON/RT_ICON) + Pillow.
  Zvětšuj nejdřív NEAREST na 512 px, pak LANCZOS na cílové velikosti, ať zůstane pixel-art ostrý.
  Vygeneruj mipmap-*/ic_launcher.png, ic_launcher_round.png a ic_launcher_foreground.png
  (108dp plátno, ikona v 66dp bezpečné zóně) a v mipmap-anydpi-v26 přepni foreground na
  @mipmap/ic_launcher_foreground, pozadí černé.

E) Čeština (pokud ji chci)
- GOG verze je jen anglická. Ověř jazyk tak, že si napíšeš malý parser LOD archivu
  (hlavička „LOD\0", adresář na 0x100, položky po 32 bajtech, obsah zlib) a přečteš global.txt
  z Events.lod. Neprohledávej binárku textově, obsah je komprimovaný.
- Fanouškovské překlady shazují OpenEnroth na dvou věcech, které původní hra tolerovala.
  Oprav je V ENGINU, ne v datech:
  1) Textové tabulky končí DOSovým znakem 0x1A, hlásí se to jako „'' is not a number".
     V ResourceManager::eventsData() u souborů *.txt odřízni koncový bajt 0x1A.
  2) Přeložené obrázky v ICONS.LOD mají v hlavičce pole size = velikost celého BMP souboru
     místo šířka×výška, hlásí se to jako „Cannot decode LOD entry … as LOD image".
     V lod::detectImage() změň podmínku header.size == w*h na header.size >= w*h
     (skutečný počet pixelů se stejně kontroluje po dekompresi).
- Než pustíš build, porovnej si česká a anglická data strukturálně (počty sloupců, kontrolní
  znaky, hlavičky obrázků a fontů), ať neladíš pád po pádu. Jiný překlad může mít jiné chyby.

F) Pád SDL při změně schránky
- SDL 3.2.22 padá (strcmp NULL v nativním clipboard callbacku), když se za běhu změní schránka.
  V herní aktivitě (extends SDLActivity) po super.onCreate() odregistruj mClipboardHandler
  z ClipboardManageru. Hra schránku nepotřebuje.

G) Dotykové ovládání v černých pruzích
- Hra je 4:3 uprostřed širokého displeje, vlevo a vpravo jsou černé pruhy. Šířka pruhu
  = (šířka − výška×4/3)/2, minimálně ~110 dp. Napiš vlastní ViewGroup „TouchControls" (čistá Java),
  přidej ji v onCreate do SDLActivity.mLayout nad herní surface. Samotný kontejner nesmí
  zachytávat dotyky, aby dotyk do hry dál fungoval jako levé kliknutí.
- Tlačítka: zaoblené obdélníky, poloprůhledné bílé, při stisku zlaté. Klávesy posílej přes
  SDLActivity.onNativeKeyDown/Up (stisk = down, puštění = up). Ikony udělej jako vektorové
  drawably (res/drawable, 24dp viewport), kvůli nim nastav minSdkVersion 24 (engine stejně
  potřebuje OpenGL ES 3.2, tedy Android 7+).
- LEVÝ pruh, tři skupiny nad sebou:
  1) nahoře 2 řádky po 3 tlačítkách, přesně v tomto pořadí:
     1. řádek (pohled): Del (pohled dolů), PgDn (pohled nahoru), End (vycentrovat pohled)
     2. řádek (let):    Ins (let dolů), Home (přistát), PgUp (let nahoru)
  2) uprostřed 2 sloupce: Esc, „Y křik" (Y = zavolání postav) / Mezera, Enter /
     ikona meče (klávesa A = útok), ikona hůlky (S = rychlé kouzlo)
  3) dole 2 sloupce: šipka doleva, šipka doprava (otáčení)
- PRAVÝ pruh:
  nahoře: „Myš L", „Myš P" / ikona batohu (I = inventář), ikona mapy (M = mapa) / „X skok", „B skip"
  dole uprostřed: šipka nahoru (dopředu) a přímo pod ní bez mezery šipka dolů (dozadu),
  samostatně, s volným místem kolem sebe.
- Ikony jsou bez písmen (jen obrázek). Popisky s textem mají jen klávesy bez ikony.
- Velikost písma popisku se přizpůsobí šířce tlačítka (nejvýš ~84 % šířky), jinak se „Home"
  a „PgDn" v užších tlačítkách po třech nevejdou.
- „Myš L" / „Myš P": přepínač, výchozí je L. Po zmáčknutí „Myš P" zůstane svítit a zapne se
  průhledná celoplošná vrstva. Další dotyk do hry se pošle jako PRAVÉ tlačítko myši přes
  SDLActivity.onNativeMouse (BUTTON_SECONDARY: ACTION_DOWN, při pohybu ACTION_MOVE, při
  puštění state 0 + ACTION_UP) a drží po dobu držení prstu, po puštění se „Myš P" sama vypne.
- Tablety (poměr stran pod 1,9:1, třeba 16:10): přirozené černé pruhy jsou na tlačítka moc
  úzké. Pruh tam má aspoň 150 dp a herní surface zúžíš okraji vlevo a vpravo (MarginLayoutParams,
  okraj = šířka pruhu), engine si 4:3 obraz vejde mezi pruhy sám. Pozadí mLayout nastav černé.
  Telefony (2:1 a širší) nech beze změny, pruh = (šířka − výška×4/3)/2, minimálně 110 dp.
- Na tabletu je v levém pruhu pod prostřední skupinou volné místo. Meč (A) a hůlku (S) tam
  proto vyjmi z prostřední skupiny a polož je jako samostatný řádek hned nad šipky otáčení
  (mezera jen jako mezi řádky). Esc, „Y křik", Mezera a Enter zůstanou nahoře. Na telefonu
  zůstanou meč a hůlka v prostřední skupině.
- Systémové lišty: SDL je skrývá starými příznaky, které tablety (panel úloh, navigační lišta)
  ignorují a lišta pak zakryje spodek hry i spodní tlačítka. V onWindowFocusChanged je skryj
  i přes WindowInsetsController (hide(systemBars()), BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE)
  a na mLayout nastav OnApplyWindowInsetsListener, který dá mLayout padding podle
  insets.getInsets(systemBars()). Výřez displeje (displayCutout) do paddingu NEPOČÍTEJ,
  SDL kreslí i do výřezu a černé pruhy ho zakryjí, jinak se rozbije rozložení na telefonu.
- Dotyk přes celoplošnou vrstvu pro pravé tlačítko myši přepočítej do souřadnic herní surface
  (x + vrstva.getLeft() − surface.getLeft()), jinak na tabletu klikáš vedle.
- Výchozí klávesy enginu pro kontrolu: šipky pohyb a otáčení, A útok, S rychlé kouzlo,
  I inventář, M mapa, X skok, B přeskočit tah v tahovém režimu, Y křik, Mezera interakce,
  Enter tahový režim, Del pohled dolů, PgDn pohled nahoru, End vycentrovat pohled, Ins let dolů,
  Home přistát, PgUp let nahoru, Esc zavírá okna, T kalendář.

G1) Klávesnice pro psaní textu
- Jméno postavy při tvorbě družiny, název uložené hry a částka v bance se píšou klávesnicí.
  Engine má vlastní textový vstup přes klávesové události (KeyboardInputHandler::StartTextInput),
  sám klávesnici Androidu neukáže. Přidej do PlatformWindow setTextInputActive/isTextInputActive
  (SdlWindow: SDL_StartTextInput/SDL_StopTextInput/SDL_TextInputActive, Proxy a Null okno taky)
  a v detail::globalProcessMessages() je v každém průchodu slaď se stavem
  keyboardInputHandler (psaní probíhá = inputType je Text nebo Number). SDL pak klávesnici
  zobrazí i schová a ASCII znaky z ní posílá jako klávesy, takže je engine zpracuje bez další úpravy.
- Pozor na dialog postavy, která má jen text bez voleb (třeba stráž, GUIWindow_BranchlessDialogue):
  engine v něm spouští textový vstup jen proto, aby ho zavřela libovolná klávesa, a klávesnice by
  pak vyskočila zbytečně. Přidej do TextInputType hodnotu AnyKey (znaky sbírá jako Text, ale
  nepočítá se jako psaní) a dialog ať ji používá, kromě otázky EVENT_InputString, kde se opravdu píše.

G2) Sníh na začátku hry
- OpenEnroth má volbu graphics „seasons_change" (stromy a zem podle ročního období), která je
  ve výchozím stavu zapnutá. V originálním MM7 nikdy nebyla (jen v MM6). Hra začíná 1. ledna 1168,
  takže Smaragdový ostrov je pod sněhem. Změň výchozí hodnotu v src/Application/GameConfig.h
  na false. Datum startu (pondělí 1. 1. 1168, 9:00) je správně, neměň ho.

G3) Synchronizace uložených her přes vybranou složku
- Engine ukládá do soukromé složky aplikace (files/.openenroth/saves), kam se jiné aplikace nedostanou.
  Přidej třídu SaveSync (Java, jen Android část, engine se nemění), která savy porovná se složkou,
  kterou si hráč jednou vybere přes ACTION_OPEN_DOCUMENT_TREE (takePersistableUriPermission,
  URI ulož do SharedPreferences). Přímé napojení na Google Disk nedělej, složku s Diskem drží
  samostatná synchronizační aplikace. Soubory čti a piš přes DocumentsContract (bez androidx).
- Pravidla: pro každý soubor si pamatuj stav po poslední synchronizaci (čas změny a velikost na obou
  stranách, v souboru ve files/). Změněný jen na jedné straně zkopíruj na druhou. Změněný na obou
  stranách (i při úplně první synchronizaci): když je obsah stejný, jen zapiš stav, jinak vyhraje
  novější a starší ulož do podsložky „zaloha" s datem a „telefon"/„slozka" v názvu. Nikdy nic nemaž
  (pozice smazaná ve hře se ze složky vrátí, tak nejde přijít o save omylem). Stažený soubor piš přes
  dočasný soubor a přejmenování.
- Kdy: v úvodní aktivitě po instalaci dat a před startem hry (s textem „Synchronizuji uložené hry…"),
  v herní aktivitě v onStop (odchod do pozadí nebo ukončení) a v onRestart, obojí ve vlákně na pozadí.
- Při prvním spuštění se zeptej dialogem „Synchronizace uložených her" (Vybrat složku / Teď ne).
  Změnu složky později nabídni dynamickým zástupcem u ikony (ShortcutManager, „Složka pro uložené
  hry", od Androidu 7.1), statické shortcuts.xml by potřebovalo pevné ID balíčku.
- Stávající savy z předchozí verze APK se musí zachovat: novou verzi instaluj PŘES starou (stejný
  keystore), nikdy neodinstalovávej, odinstalace savy smaže. Při prvním výběru složky se savy
  z telefonu do složky zkopírují.

G4) Teleport Hrad Harmondale ↔ Smaragdový ostrov (volitelné rozšíření)
- Na Smaragdový ostrov se v původní hře po odplutí nedá vrátit. Přidej do src/Application/Game.cpp dvě propojené
  plošiny: hrad Harmondale (mapa MAP_CASTLE_HARMONDALE, d29.blv) před fontánou na (-5120, -180, 1), cíl
  na ostrově (12552, 1250, 193) se směrem 512. Ostrov (MAP_EMERALD_ISLAND, out01.odm) na (12552, 1500, 193),
  cíl kousek před fontánou (-5120, -480, 1) se směrem 512. Fontána nemá v d29.blv událost ani dekoraci, je to
  nádrž s texturou vody wtrtyl kolem bodu (-5120, 736). Start nové hry je na molu (12552, 800).
- Každý snímek hry (vedle UpdateUserInput_and_MapSpecificStuff): nad aktivní plošinou pár částic
  (ParticleType_Bitmap | Rotating | Ascending, textura effpar03 ze SpellFxRenderer, modrá barva). Když družina
  vstoupí do kruhu o poloměru 96, otevři pDialogueWindow = GUIWindow_IndoorEntryExit(HOUSE_INVALID, 1,
  MapDestination(cílová mapa, PartyPlacement(cíl, směr, 0, 0)), soubor cílové mapy). Do okna přidej pole
  _titleMap, aby nahoře byl název cíle, text „Přejete si opustit …?" je hotový lokalizovaný řetězec hry.
- Plošina se nespustí hned po příchodu na mapu ani opakovaně, dokud z ní družina nesejde, ani v tahovém
  režimu nebo když je otevřené jiné okno. Plošina na ostrově funguje až s úkolovým bitem
  QBIT_ESCAPED_EMERALD_ISLE, jinak by šly přeskočit úvodní úkoly.
- Ověř to na PC: desktopový build v Dockeru pod Xvfb ovládaný xdotool, s pomocnými proměnnými jen ve vývojové
  kopii (skok na mapu, pozice, úkolový bit, výpis pozice). Obě cesty, okno s textem a návrat.

H) Testování: povinné, než mi APK předáš
- Rozeber hotové APK a ověř: podpis (apksigner verify), název a ID (aapt dump badging),
  native-code JEN arm64-v8a, všech 29 datových souborů uložených bez komprese, správné LOD uvnitř.
- Pokud mám emulátor z Android Studia: je x86_64 a umí jen OpenGL ES 3.1, kdežto engine vyžaduje
  ES 3.2 (pád „assertion context failed" v initializeOpenGLContext). Pro emulátor proto postav
  ZVLÁŠTNÍ testovací variantu (GITHUBARCH=x86_64, do jiné výstupní složky) s jednorázovým
  patchem aplikovaným jen na pracovní kopii v kontejneru: versionMinor 2→1 v ES větvi
  OpenGLRenderer.cpp a „#version 320 es" → „#version 310 es + #extension GL_EXT_texture_buffer
  : enable" v OpenGLShader.cpp. Tenhle patch se NESMÍ dostat do APK pro telefon: po každém
  emulátorovém buildu smaž android/openenroth/build v pracovním volume, jinak se x86_64
  knihovna přimíchá do telefonního APK. Před předáním to zkontroluj.
- Na emulátoru projdi přes adb (input tap/swipe + screencap) a ukaž mi screenshoty:
  instalační obrazovku, hlavní menu, tvorbu družiny (kontrola diakritiky), 3D scénu po startu
  nové hry (bez sněhu, zelené stromy), tlačítko dopředu (družina se pohne), Del/PgDn/End
  (pohled dolů, nahoru a zpět na střed), ikonu mapy,
  Esc, kalendář přes adb keyevent 48 (musí ukázat 1. ledna 1168), a „Myš P" podrženou na portrétu postavy
  (má se ukázat info okno a po puštění se má přepínač vypnout). U release buildu nefunguje
  „run-as", stav zjišťuj z logcatu a screenshotů. První spuštění může překrýt systémová
  bublina „Viewing full screen", odklikni ji.
- Tablet: pokud nemám tablet v emulátoru, vytvoř si vlastní AVD (třeba 2000×1250, 240 dpi,
  6 GB data) ve složce projektu (ANDROID_AVD_HOME, složku dej do .gitignore) a do lišty zapni
  tříbarvovou navigaci (adb shell cmd overlay enable-exclusive --category
  com.android.internal.systemui.navbar.threebutton). Ukaž mi hlavní menu na tabletu, psaní
  jména postavy s klávesnicí, klik na postavu jen s textem (klávesnice se NESMÍ ukázat)
  a totéž na rozměru telefonu (adb shell wm size 2400x1080,
  wm density 420, pak wm size reset a wm density reset).
- Synchronizace savů na emulátoru: do /sdcard/Documents/MaM7 dej přes adb push nějaký save
  (třeba z OpenEnroth/test/Data jako save005.mm7), při prvním spuštění tu složku vyber a ukaž mi,
  že je v nabídce Nahrát hru. Pak ulož novou pozici, zmáčkni Domů a ukaž, že je ve složce.
  Pak ve složce save005 nahraď jiným a po novém spuštění ukaž, že se ve hře změnil. Nakonec
  kolize: změň ve složce save, který zároveň přepíšeš ve hře, a ukaž zálohu v podsložce zaloha.
- Emulátor mi NEMAŽ (wipe-data), mám v něm vlastní aplikace. Když v něm dojde místo, testovací
  aplikaci odinstaluj a emulátor restartuj. Po testech aplikaci odinstaluj a emulátor vypni.
- Schránku na mém PC nepřepisuj (emulátor ji sdílí).

VÝSTUP:
- Jedno APK ve složce <projekt>\out\, plus keystore tamtéž.
- Na konci mi napiš: přesnou cestu k APK, jak ho dostat do telefonu a nainstalovat (neznámé
  zdroje, cca 1,3 GB volného místa, první spuštění rozbaluje data), co přesně je ověřené a co ne
  (hlavně: skutečný telefon s ES 3.2 jsi netestoval), co jsi změnil na mém disku (složky,
  Docker image a volumes) a že APK s mými daty nesmím šířit.
```

---

Pozn.: prompt odkazuje na konkrétní verze (NDK 28.1, CMake 3.31, SDL 3.2.22, Gradle 7.4) platné v září 2026.
Až se OpenEnroth posune dál, některé body přestanou platit. Prompt proto na několika místech říká „ověř si to v repu".
