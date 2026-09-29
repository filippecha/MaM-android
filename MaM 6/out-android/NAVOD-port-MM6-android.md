# Prompt: vlastní Android port Might and Magic VI (jedno APK s daty, čeština, dotykové ovládání)

Vlož text z bloku PROMPT do Claude Code (desktop aplikace nebo CLI) spuštěného ve své pracovní složce na Windows.
Nejdřív si projdi „Co musíš mít připravené“ a uprav blok „MOJE ÚDAJE“.

**Důležitý rozdíl proti MaM 7:** OpenEnroth sám o sobě umí jen MM7. Podpora MM6 je dopsaná ručně
do naší kopie enginu (desítky commitů: pravidla, kouzla, domy, rozhovory, události map, vzhled oken…).
Tenhle prompt proto nestaví engine od nuly, ale sestaví APK z hotové upravené kopie ve složce
`MaM 6\OpenEnroth` z veřejného repozitáře [MaM-android](https://github.com/filippecha/MaM-android). Na konci je sekce
„Když upravený engine nemáš“ s popisem, jak se MM6 do enginu dopisovala.

---

## Co musíš mít připravené, než prompt pustíš

**Hra**
- Legálně koupená hra Might and Magic VI: The Mandate of Heaven z GOG.com.
- Hra musí být na PC **NAINSTALOVANÁ** (výchozí cesta `C:\GOG Games\Might and Magic 6`), nestačí mít stažený instalátor.
- Ve složce hry musí být `Anims` (Anims1.vid, Anims2.vid), `data` (BITMAPS.LOD, SPRITES.LOD, games.lod, icons.lod),
  `Sounds` (Audio.snd a hudba 2.mp3 až 16.mp3) a `MM6.exe`. Dohromady asi 715 MB.
- `MM6.exe` se do APK přibalí, ale nespouští se. Engine z něj jen čte tabulky (ceny, kouzla, dovednosti, startovní hodnoty postav).

**Upravený engine**
- Klon veřejného repozitáře: `git clone https://github.com/filippecha/MaM-android` (třeba do `C:\Projekty\MaM-android`).
  Je v něm složka `MaM 6` s podsložkami `OpenEnroth` (upravený engine) a `docker-android` (build skript)
  a složka `MaM 7\docker` (Docker image se sdílí s MaM 7).
- Herní data v repozitáři NEJSOU, každý musí mít vlastní koupenou hru. Prompt si je zkopíruje
  z tvé instalace do `MaM 6\gamedata\MM6` (ta složka se do gitu nenahrává).

**Čeština (volitelné)**
- Hra na tomhle PC už česká je (přeložené `data\icons.lod` a `data\games.lod`). Když máš anglickou verzi
  a chceš češtinu, nainstaluj do hry fanouškovský překlad MM6, originální soubory si předtím zazálohuj.
- Texty, které MM6 vůbec nemá (nová okna enginu), jsou v enginu už přeložené, vzaly se z českého MM7.
- Dabing zůstává anglický.

**Počítač**
- Windows 10/11 64bit, ideálně 16 GB RAM, asi 20 GB volného místa (Docker image ~5 GB, build cache, data, APK).
- **Docker Desktop** nainstalovaný, spuštěný a nepozastavený (WSL2). Bez něj build nejde.
- **Git** a **Python 3**.
- Čas: první build včetně stahování závislostí asi 30–60 minut, další pár minut.

**Testování (volitelné, ale doporučené)**
- Android Studio s emulátorem. Emulátor umí jen OpenGL ES 3.1, proto se pro něj staví zvláštní testovací APK.

**Telefon**
- Android 7+ s 64bit ARM procesorem (arm64) a **OpenGL ES 3.2**.
- Asi **1,2 GB volného místa** (APK ~550 MB + rozbalená data).
- Povolená instalace z neznámých zdrojů.

**Synchronizace uložených her (volitelné)**
- Když chceš stejné pozice na telefonu i tabletu, nainstaluj si na obě zařízení z Google Play synchronizační
  aplikaci (třeba Autosync for Google Drive nebo FolderSync). Hra si savy kopíruje do složky, kterou vybereš
  (třeba `Dokumenty/MaM6`), a aplikace tu složku drží stejnou s Google Diskem na všech zařízeních.
- Přímo s Google Diskem se hra nespojuje, nepotřebuješ žádný vývojářský účet u Googlu.

**Právní poznámka**
- Hotové APK obsahuje tvoje herní data z GOG. Je jen pro tebe, nikam ho nenahrávej a nikomu neposílej.

---

## PROMPT (kopíruj odsud dolů)

```text
Chci si pro vlastní potřebu postavit Android port hry Might and Magic VI: The Mandate of Heaven
jako JEDNO samostatné APK, které po instalaci rovnou funguje. Hru vlastním legálně z GOG.com
a mám ji NAINSTALOVANOU. APK bude obsahovat moje herní data, takže je jen pro mě a nesmí se šířit.
Na to mě na konci upozorni.

MOJE ÚDAJE (doplň / uprav):
- Cesta k nainstalované hře: C:\GOG Games\Might and Magic 6
- Klon repozitáře MaM-android: C:\Projekty\MaM-android   (git clone https://github.com/filippecha/MaM-android)
  (MM6 port je ve složce "MaM 6", Docker image pro Android ve "MaM 7\docker")
- Název aplikace v telefonu: MaM 6, ID balíčku: cz.mm6.game (tak je to už v enginu nastavené)
- Jazyk hry: čeština (překlad je v icons.lod a games.lod)  / nebo: angličtina
- Mám Docker Desktop: ano. Mám Android Studio (SDK + emulátor): ano/ne.

ZÁSADNÍ PRAVIDLA:
1. Do složky s originální hrou NIKDY nezapisuj, jen z ní čti.
2. Pracuj jen uvnitř složky "MaM 6" (a čti z "MaM 7\docker"). Jiné porty v repozitáři neměň.
3. Než začneš něco velkého stahovat nebo stavět, napiš mi krátce plán a počkej na moje „ano".
   Potom pracuj samostatně.
4. Nic netvrď bez ověření. U každé věci řekni, jestli je ověřená (a jak), nebo jen předpokládaná.
5. Emulátor mi NEMAŽ (wipe-data), mám v něm vlastní aplikace. Docker nikdy neresetuj do továrního nastavení.
6. Komunikuj česky.

KROK 0: OVĚŘ PŘEDPOKLADY (nic přitom neměň):
- Ve hře existují Anims\Anims1.vid, Anims\Anims2.vid, data\BITMAPS.LOD, data\SPRITES.LOD,
  data\games.lod, data\icons.lod, Sounds\Audio.snd, Sounds\2.mp3 … 16.mp3 a MM6.exe.
- Vytvoř složku "MaM 6\gamedata\MM6" a zkopíruj do ní data ze hry (jen čtení ze hry, zápis
  do repozitáře), jen soubory vyjmenované výše. Ověř, že velikosti sedí s originálem. Tahle složka
  se nesmí dostat do gitu (je v .gitignore), obsahuje moje koupená data.
- Jazyk: MM6 má texty v icons.lod (global.txt). Přečti je malým parserem LOD (hlavička "LOD\0",
  adresář položek, obsah zlib) a řekni mi, jestli je hra česky.
- Docker běží (docker info), je git a Python 3 a aspoň 20 GB místa. Zjisti, jestli mám Android SDK
  a emulátor (%LOCALAPPDATA%\Android\Sdk, emulator -list-avds).

KROK 1: DOCKER IMAGE
- Image "openenroth-android" se staví z "MaM 7\docker\Dockerfile" (Ubuntu 24.04, JDK 17, Android SDK
  s NDK, CMake 3.31 z Kitware, protože Ubuntu má jen 3.28). Když už existuje (docker images), nestav ho znovu.
- Licence Android SDK zkopíruj do "MaM 7\docker\licenses" z mého Android Studia
  (%LOCALAPPDATA%\Android\Sdk\licenses). Když Android Studio nemám, zeptej se mě.
  Neodklikávej je za mě přes „yes |".
- Když Docker Desktop nejde spustit s chybou „…sock.stale: The file cannot be accessed by the system",
  přejmenuj (ne mazat) složky %LOCALAPPDATA%\Docker\run a %LOCALAPPDATA%\docker-secrets-engine,
  když Docker neběží. Když je Docker ručně pozastavený, neobcházej to a řekni mi to.

KROK 2: BUILD APK PRO TELEFON (PowerShell ve složce "MaM 6"):
  docker run --rm -v "${PWD}\OpenEnroth:/src:ro" -v "${PWD}\gamedata:/gamedata:ro" -v "${PWD}\out-android:/out" -v "${PWD}\docker-android:/dd:ro" -v oe6_work:/work -v oe_gradle:/root/.gradle openenroth-android bash -c "sed 's/\r$//' /dd/build.sh > /tmp/b.sh && bash /tmp/b.sh"
- Skript docker-android/build.sh zkopíruje zdrojáky do volume (build na bind-mountu z Windows je
  pomalý), přibalí data bez *.partNNN, *.dll a *.ini, podepíše APK klíčem out-android\keystore.jks
  (když chybí, vytvoří ho) a výsledek uloží jako out-android\MaM6.apk.
- Keystore je potřeba schovat, jinak další verze nepůjde nainstalovat přes starou.

KROK 3: KONTROLA APK (povinná)
- apksigner verify, aapt dump badging (název MaM 6, balíček cz.mm6.game), native-code JEN arm64-v8a,
  v assets/mm6 jsou všechny datové soubory nekomprimované (LOD, vid, snd, mp3, MM6.exe) a žádné *.partNNN.

KROK 3b: SYNCHRONIZACE ULOŽENÝCH HER (zkontroluj, že je v kopii enginu)
- V OpenEnroth\android\openenroth\src\main\java\org\openenroth\game musí být SaveSync.java
  a DataInstallerActivity.java a OpenEnroth.java ji musí volat (popis níže v sekci „Synchronizace uložených her“).
  Když tam není, dopiš ji podle té sekce, jen do Android části, engine v C++ se nemění.
- V APK zkontroluj, že classes*.dex obsahuje SaveSync.

KROK 4: TEST NA EMULÁTORU (když mám Android Studio)
- Emulátor umí jen OpenGL ES 3.1, engine chce 3.2. Postav ZVLÁŠTNÍ testovací APK:
  -e GITHUBARCH=x86_64, výstup do "out-android-emu" (s prebuild.sh zkopírovaným z "MaM 7\out-emu",
  ten jen v pracovní kopii sníží ES 3.2 na 3.1) a jiný volume (oe6emu_work). Tahle úprava se nesmí
  dostat do APK pro telefon, zkontroluj, že out-android\MaM6.apk má jen arm64-v8a.
- Přes adb (input tap/swipe, screencap) mi ukaž: instalaci dat při prvním spuštění, hlavní menu,
  tvorbu družiny s diakritikou, Nové Sorpigal po startu hry, chůzi tlačítkem dopředu, psaní jména
  postavy s klávesnicí Androidu. Když mám tabletový AVD, to samé na tabletu, a rozměr telefonu přes
  adb shell wm size 2400x1080 + wm density 420 (potom wm size reset a wm density reset).
- Synchronizace savů: do /sdcard/Documents/MaM6 dej přes adb push zkušební soubor (třeba pokus.txt),
  při prvním spuštění vyber v dialogu „Synchronizace uložených her" tu složku a povol přístup.
  Po startu hry soubor ze složky smaž, zmáčkni Domů a ukaž mi, že ho tam hra vrátila ze své kopie
  (tím jsou ověřené oba směry). Když máš save z téhle hry, ukaž ho i v nabídce Nahrát hru.
  Pak ověř zástupce: adb shell dumpsys shortcut musí u cz.mm6.game ukázat „Složka pro uložené hry".
- U release buildu nefunguje run-as, stav zjišťuj z logcatu a screenshotů.
- Po testu testovací aplikaci odinstaluj a emulátor vypni. Schránku na mém PC nepřepisuj.

VÝSTUP:
- out-android\MaM6.apk a keystore tamtéž.
- Na konci mi napiš: cestu k APK, jak ho dostat do telefonu a nainstalovat (neznámé zdroje,
  ~1,2 GB místa, první spuštění rozbaluje data), co je ověřené a co ne (skutečný telefon s ES 3.2
  jsi netestoval), co jsi změnil na disku (složky, Docker image a volumes) a že APK nesmím šířit.
```

---

## Ovládání v telefonu

Stejné jako MaM 7 (obrázky `MaM6-telefon.png` a `MaM6-tablet.png` v této složce):

- **Levý pruh:** nahoře pohled (Del dolů, PgDn nahoru, End na střed) a let (Ins, Home, PgUp),
  uprostřed Esc, Y křik, Mezera, Enter, meč (útok) a hůlka (rychlé kouzlo), dole otáčení doleva a doprava.
- **Pravý pruh:** Myš L / Myš P (pravé tlačítko na jeden dotyk, třeba info o předmětu nebo postavě),
  batoh (inventář), mapa, X skok, B přeskočit tah, dole chůze dopředu a dozadu.
- Dotyk do obrazu hry je levé kliknutí myší. Klávesnice Androidu vyskočí jen tam, kde se opravdu píše.
- Na tabletu (16:10) jsou pruhy širší a hra se zúží mezi ně, na telefonu se nic nemění.

---

## Synchronizace uložených her

**Pro hráče:** při prvním spuštění se hra zeptá „Synchronizace uložených her". Klepni na **Vybrat složku**,
vytvoř třeba `Dokumenty/MaM6`, klepni na „Použít tuto složku" a povol přístup. Na tabletu udělej totéž.
V synchronizační aplikaci (Autosync for Google Drive, FolderSync) nastav obousměrnou synchronizaci té složky
se stejnou složkou na Google Disku, na obou zařízeních. Složku změníš podržením ikony hry →
„Složka pro uložené hry". Novou verzi APK vždy instaluj přes starou (stejný keystore), odinstalace
uložené hry smaže. Stávající savy se při prvním výběru složky do ní zkopírují.

**Jak to funguje (pro stavbu):**
- Engine ukládá do soukromé složky aplikace `files/.openenroth/saves`, kam se jiné aplikace nedostanou.
  Třída `SaveSync` (Java, jen Android část) ji porovnává se složkou, kterou hráč jednou vybere přes
  `ACTION_OPEN_DOCUMENT_TREE` (`takePersistableUriPermission`, URI v `SharedPreferences`). Soubory čte a píše
  přes `DocumentsContract` (bez androidx). S Google Diskem se hra sama nespojuje.
- Pro každý soubor si pamatuje stav po poslední synchronizaci (čas změny a velikost na obou stranách, soubor
  `files/save-sync-state.properties`). Změněný jen na jedné straně se zkopíruje na druhou. Změněný na obou
  stranách (i při úplně první synchronizaci): při stejném obsahu se jen zapíše stav, jinak vyhraje novější
  a starší se uloží do podsložky `zaloha` s datem a „telefon"/„slozka" v názvu. Nic se nemaže (pozice
  smazaná ve hře se ze složky vrátí, nejde tak přijít o save omylem). Stažený soubor se píše přes dočasný
  soubor a přejmenování.
- Kdy: v `DataInstallerActivity` po instalaci dat a před startem hry (text „Synchronizuji uložené hry…"),
  v herní aktivitě `OpenEnroth` v `onStop` (odchod do pozadí nebo konec) a v `onRestart`, obojí ve vlákně na pozadí.
- Dialog při prvním spuštění (Vybrat složku / Teď ne). Změnu složky nabízí dynamický zástupce u ikony
  (`ShortcutManager`, od Androidu 7.1), statické `shortcuts.xml` by potřebovalo pevné ID balíčku.
  Texty jsou česky v `res/values-cs/strings.xml` a anglicky v `res/values/strings.xml`.
- Stejný kód jako v MaM 7, kde je ověřený v emulátoru (převzetí save ze složky, nahrání nového save,
  převzetí změněného save, kolize se zálohou) i na telefonu a tabletu s Google Diskem. V MaM 6 je ověřený
  v emulátoru (zkušební soubor ze složky do hry a zpět, zástupce u ikony), na telefonu zatím ne.

---

## Když upravený engine nemáš

Pak je potřeba MM6 do OpenEnroth dopsat. Není to práce na jeden prompt, ale na dlouhé vedení
(„pokračuj, dokud to nebude hotové“). Takhle vznikl tenhle port a osvědčilo se:

- **Stejný základ jako MaM 7:** všechny Android úpravy z návodu k MaM 7 (instalátor dat, dotyková
  tlačítka, klávesnice, tablet, oprava SDL schránky) platí i tady. Nejdřív postav MaM 7, pak kopii
  enginu rozšiřuj o MM6.
- **Rozlišení hry podle dat:** `GameVariant` v `Utility/GameVariant.h` a funkce `isMm6()`. PathResolver
  pozná MM6 podle souborů (MM6 nemá `events.lod`, skripty a tabulky má v `icons.lod`). Každá změna je
  větev `if (isMm6())`, chování MM7 zůstává beze změny.
- **Překlad čísel:** MM6 čísla předmětů, dovedností, profesí a map se při načítání převádějí do rozsahů
  MM7 (`Engine/Objects/Mm6Ids`), aby zbytek enginu fungoval beze změn.
- **Tabulky z MM6.exe:** hodnoty, které nejsou v datech (ceny, statistiky tříd, kouzla), se čtou přímo
  z exe (`Engine/Mm6ExeData`). Adresy jsou z projektu MMExtension (github.com/GrayFace/MMExtension),
  jeho `Scripts/Core` popisuje i formát událostí MM6.
- **Vývoj na PC, ne na telefonu:** desktopový Linux build v Dockeru, spuštěný pod Xvfb s daty MM6,
  ovládaný přes xdotool, se snímky obrazovky (skript `tools/dev.sh`). Jeden cyklus trvá pár minut.
- **Originál jako vzor:** původní MM6.exe běží pod Wine v Dockeru (`docker-wine`, grafika přes
  cnc-ddraw) a každá obrazovka se porovnává se snímkem z originálu, včetně pozic a písma.
- **Po částech:** načtení dat → hlavní menu → tvorba družiny → mapa a pohyb → postava a kouzla →
  domy a obchody → rozhovory a úkoly → konec hry. Po každé části kontrola stylu a unit testy
  (`docker-desktop/test.sh`) a commit.
- **Chybějící texty:** co MM6 nemá v `global.txt`, se doplní z českého MM7 (`tools/gen_mm6_lstr.py`).

---

Pozn.: prompt odkazuje na stav repozitáře a verze nástrojů ze září 2026 (NDK 28.1, CMake 3.31, SDL 3.2.22).
