# Prompt: vlastní Android port Might and Magic VIII (jedno APK s daty, čeština, dotykové ovládání)

Vlož text z bloku PROMPT do Claude Code (desktop aplikace nebo CLI) spuštěného ve své pracovní složce na Windows.
Nejdřív si projdi „Co musíš mít připravené“ a uprav blok „MOJE ÚDAJE“.

**Důležitý rozdíl proti MaM 7:** OpenEnroth sám o sobě umí jen MM7. Podpora MM8 je dopsaná ručně
do naší kopie enginu (desítky commitů: data, pravidla, rasy a draci, kouzla, domy, rozhovory, události map,
vzhled oken…). Kopie vychází z MaM 6, takže umí MM6, MM7 i MM8 a hru pozná podle dat.
Tenhle prompt proto nestaví engine od nuly, ale sestaví APK z hotové upravené kopie ve složce
`MaM 8\OpenEnroth` z veřejného repozitáře [MaM-android](https://github.com/filippecha/MaM-android). Na konci je sekce
„Když upravený engine nemáš“ s popisem, jak se MM8 do enginu dopisovala.

---

## Co musíš mít připravené, než prompt pustíš

**Hra**
- Legálně koupená hra Might and Magic VIII: Day of the Destroyer z GOG.com.
- Hra musí být na PC **NAINSTALOVANÁ** (výchozí cesta `C:\GOG Games\Might and Magic 8`), nestačí mít stažený instalátor.
- Ve složce hry musí být `Anims` (Magicdod.vid, mightdod.vid), `Data` (EnglishD.lod, EnglishT.lod, bitmaps.lod,
  games.lod, icons.lod, sprites.lod, d3dbitmap.hwl, d3dsprite.hwl), `Music` (2.mp3 až 15.mp3) a `MM8-Rel.exe`.
  Dohromady asi 790 MB.
- `MM8-Rel.exe` se do APK přibalí, ale nespouští se. Engine z něj jen čte tabulky (třídy, startovní hodnoty,
  dovednosti, kouzla, ceny, obchody, domy).

**Upravený engine**
- Klon veřejného repozitáře: `git clone https://github.com/filippecha/MaM-android` (třeba do `C:\Projekty\MaM-android`).
  Je v něm složka `MaM 8` s podsložkami `OpenEnroth` (upravený engine) a `docker-android` (build skript)
  a složka `MaM 7\docker` (Docker image se sdílí s MaM 7).
- Herní data v repozitáři NEJSOU, každý musí mít vlastní koupenou hru. Prompt si je zkopíruje
  z tvé instalace do `MaM 8\gamedata\MM8` (ta složka se do gitu nenahrává).

**Čeština (volitelné)**
- Hra na tomhle PC už česká je (texty v `Data\EnglishT.lod`, obrázky a zvuky v `Data\EnglishD.lod`).
  Když máš anglickou verzi a chceš češtinu, nainstaluj do hry fanouškovský překlad MM8, originální soubory
  si předtím zazálohuj.
- Dabing zůstává anglický.

**Počítač**
- Windows 10/11 64bit, ideálně 16 GB RAM, asi 20 GB volného místa (Docker image ~5 GB, build cache, data, APK).
- **Docker Desktop** nainstalovaný, spuštěný a nepozastavený (WSL2). Bez něj build nejde.
- **Git** a **Python 3**.
- Čas: první build včetně stahování závislostí asi 30–60 minut, další pár minut.

**Testování (volitelné, ale doporučené)**
- Android Studio s emulátorem. Emulátor umí jen OpenGL ES 3.1, proto se pro něj staví zvláštní testovací APK.
- Emulátor potřebuje asi 3 GB volné paměti. Když Docker a WSL zabírají skoro všechno, nespustí se.

**Telefon**
- Android 7+ s 64bit ARM procesorem (arm64) a **OpenGL ES 3.2**.
- Asi **1,5 GB volného místa** (APK ~660 MB + rozbalená data).
- Povolená instalace z neznámých zdrojů.

**Synchronizace uložených her (volitelné)**
- Když chceš stejné pozice na telefonu i tabletu, nainstaluj si na obě zařízení z Google Play synchronizační
  aplikaci (třeba Autosync for Google Drive nebo FolderSync). Hra si savy kopíruje do složky, kterou vybereš
  (třeba `Dokumenty/MaM8`), a aplikace tu složku drží stejnou s Google Diskem na všech zařízeních.
- Přímo s Google Diskem se hra nespojuje, nepotřebuješ žádný vývojářský účet u Googlu.

**Právní poznámka**
- Hotové APK obsahuje tvoje herní data z GOG. Je jen pro tebe, nikam ho nenahrávej a nikomu neposílej.

---

## PROMPT (kopíruj odsud dolů)

```text
Chci si pro vlastní potřebu postavit Android port hry Might and Magic VIII: Day of the Destroyer
jako JEDNO samostatné APK, které po instalaci rovnou funguje. Hru vlastním legálně z GOG.com
a mám ji NAINSTALOVANOU. APK bude obsahovat moje herní data, takže je jen pro mě a nesmí se šířit.
Na to mě na konci upozorni.

MOJE ÚDAJE (doplň / uprav):
- Cesta k nainstalované hře: C:\GOG Games\Might and Magic 8
- Klon repozitáře MaM-android: C:\Projekty\MaM-android   (git clone https://github.com/filippecha/MaM-android)
  (MM8 port je ve složce "MaM 8", Docker image pro Android ve "MaM 7\docker")
- Název aplikace v telefonu: MaM 8, ID balíčku: cz.mm8.game (tak je to už v enginu nastavené)
- Jazyk hry: čeština (texty v EnglishT.lod, obrázky v EnglishD.lod)  / nebo: angličtina
- Mám Docker Desktop: ano. Mám Android Studio (SDK + emulátor): ano/ne.

ZÁSADNÍ PRAVIDLA:
1. Do složky s originální hrou NIKDY nezapisuj, jen z ní čti.
2. Pracuj jen uvnitř složky "MaM 8" (a čti z "MaM 7\docker"). Jiné porty v repozitáři neměň.
3. Než začneš něco velkého stahovat nebo stavět, napiš mi krátce plán a počkej na moje „ano".
   Potom pracuj samostatně.
4. Nic netvrď bez ověření. U každé věci řekni, jestli je ověřená (a jak), nebo jen předpokládaná.
5. Emulátor mi NEMAŽ (wipe-data), mám v něm vlastní aplikace. Docker nikdy neresetuj do továrního nastavení.
6. Komunikuj česky.

KROK 0: OVĚŘ PŘEDPOKLADY (nic přitom neměň):
- Ve hře existují Anims\Magicdod.vid, Anims\mightdod.vid, Data\EnglishD.lod, Data\EnglishT.lod,
  Data\bitmaps.lod, Data\games.lod, Data\icons.lod, Data\sprites.lod, Data\d3dbitmap.hwl,
  Data\d3dsprite.hwl, Music\2.mp3 … 15.mp3 a MM8-Rel.exe.
- Vytvoř složku "MaM 8\gamedata\MM8" a zkopíruj do ní data ze hry (jen čtení ze hry, zápis
  do repozitáře), jen soubory vyjmenované výše. Ověř, že velikosti sedí s originálem. Tahle složka
  se nesmí dostat do gitu (je v .gitignore), obsahuje moje koupená data.
- Jazyk: MM8 má texty v EnglishT.lod (global.txt). LOD má jména položek dlouhá 64 znaků a obsah
  zabalený zlib. Přečti je malým parserem (nebo MaM 8\tools\lod.py) a řekni mi, jestli je hra česky.
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

KROK 2: BUILD APK PRO TELEFON (PowerShell ve složce "MaM 8"):
  docker run --rm -v "${PWD}\OpenEnroth:/src:ro" -v "${PWD}\gamedata:/gamedata:ro" -v "${PWD}\out-android:/out" -v "${PWD}\docker-android:/dd:ro" -v oe8_work:/work -v oe_gradle:/root/.gradle openenroth-android bash -c "sed 's/\r$//' /dd/build.sh > /tmp/b.sh && bash /tmp/b.sh"
- Pozor: "gamedata" se mountuje jako /gamedata a MUSÍ v ní být podsložka MM8. Se špatným mountem
  build projde, ale APK nebude mít žádná data.
- Skript docker-android/build.sh zkopíruje zdrojáky do volume (build na bind-mountu z Windows je
  pomalý), přibalí data bez *.partNNN, *.dll a *.ini, podepíše APK klíčem out-android\keystore.jks
  (když chybí, vytvoří ho) a výsledek uloží jako out-android\MaM8.apk.
- Keystore je potřeba schovat, jinak další verze nepůjde nainstalovat přes starou.

KROK 3: KONTROLA APK (povinná)
- apksigner verify, aapt dump badging (název MaM 8, balíček cz.mm8.game), native-code JEN arm64-v8a,
  v assets/mm8 jsou všechny datové soubory, LOD, hwl, vid a mp3 nekomprimované (MM8-Rel.exe
  komprimovaný být smí, je malý) a žádné *.partNNN.

KROK 3b: SYNCHRONIZACE ULOŽENÝCH HER (zkontroluj, že je v kopii enginu)
- V OpenEnroth\android\openenroth\src\main\java\org\openenroth\game musí být SaveSync.java
  a DataInstallerActivity.java a OpenEnroth.java ji musí volat (popis níže v sekci „Synchronizace uložených her“).
  Když tam není, dopiš ji podle té sekce, jen do Android části, engine v C++ se nemění.
- V APK zkontroluj, že classes*.dex obsahuje SaveSync.

KROK 4: TEST NA EMULÁTORU (když mám Android Studio)
- Emulátor umí jen OpenGL ES 3.1, engine chce 3.2. Postav ZVLÁŠTNÍ testovací APK:
  -e GITHUBARCH=x86_64, výstup do "out-android-emu" (s prebuild.sh zkopírovaným z "MaM 7\out-emu",
  ten jen v pracovní kopii sníží ES 3.2 na 3.1) a jiný volume (oe8emu_work). Tahle úprava se nesmí
  dostat do APK pro telefon, zkontroluj, že out-android\MaM8.apk má jen arm64-v8a.
- Emulátor potřebuje asi 3 GB volné paměti. Když se nespustí, protože Docker/WSL zabírá skoro všechno,
  uvolni mezipaměť Dockeru (docker run --rm --privileged alpine sh -c "sync; echo 3 > /proc/sys/vm/drop_caches")
  a zastav nepotřebné kontejnery. Docker neresetuj.
- Přes adb (input tap/swipe, screencap) mi ukaž: instalaci dat při prvním spuštění, intro a hlavní menu,
  tvorbu postavy s diakritikou (i okénko po podržení pravého tlačítka na dovednosti), Dagger Wound
  Island s rozhovorem se S'tonem po startu hry, chůzi tlačítkem dopředu. Když mám tabletový AVD, to samé na tabletu, a rozměr telefonu přes
  adb shell wm size 2400x1080 + wm density 420 (potom wm size reset a wm density reset).
  Po změně rozměru se aplikace zavře, spusť ji znovu.
- Synchronizace savů: do /sdcard/Documents/MaM8 dej přes adb push zkušební soubor (třeba pokus.txt),
  při prvním spuštění vyber v dialogu „Synchronizace uložených her" tu složku a povol přístup.
  Po startu hry soubor ze složky smaž, zmáčkni Domů a ukaž mi, že ho tam hra vrátila ze své kopie
  (tím jsou ověřené oba směry). Když máš save z téhle hry, ukaž ho i v nabídce Nahrát hru.
  Pak ověř zástupce: adb shell dumpsys shortcut musí u cz.mm8.game ukázat „Složka pro uložené hry".
- U release buildu nefunguje run-as, stav zjišťuj z logcatu a screenshotů.
- Po testu testovací aplikaci odinstaluj a emulátor vypni. Schránku na mém PC nepřepisuj.

VÝSTUP:
- out-android\MaM8.apk a keystore tamtéž.
- Na konci mi napiš: cestu k APK, jak ho dostat do telefonu a nainstalovat (neznámé zdroje,
  ~1,5 GB místa, první spuštění rozbaluje data), co je ověřené a co ne (skutečný telefon s ES 3.2
  jsi netestoval), co jsi změnil na disku (složky, Docker image a volumes) a že APK nesmím šířit.
```

---

## Ovládání v telefonu

Stejné jako MaM 7 a MaM 6 (obrázky v této složce: `MaM8-telefon.png` a `MaM8-telefon-hra.png` na telefonu,
`MaM8-tablet.png` a `MaM8-tablet-rozhovor.png` na tabletu):

- **Levý pruh:** nahoře pohled (Del dolů, PgDn nahoru, End na střed) a let (Ins, Home, PgUp),
  uprostřed Esc, Y křik, Mezera, Enter, meč (útok) a hůlka (rychlé kouzlo), dole otáčení doleva a doprava.
- **Pravý pruh:** Myš L / Myš P (pravé tlačítko na jeden dotyk, třeba info o předmětu nebo postavě),
  batoh (inventář), mapa, X skok, B přeskočit tah, dole chůze dopředu a dozadu.
- Dotyk do obrazu hry je levé kliknutí myší. Klávesnice Androidu vyskočí jen tam, kde se opravdu píše.
- Na tabletu (16:10) jsou tlačítka menší a hra zabere víc místa, na telefonu zůstává rozložení stejné.
- Předměty jako lektvary, jablko nebo podkova se použijí tak, že je vezmeš v batohu a klikneš jimi
  na panáčka postavy.

---

## Synchronizace uložených her

**Pro hráče:** při prvním spuštění se hra zeptá „Synchronizace uložených her". Klepni na **Vybrat složku**,
vytvoř třeba `Dokumenty/MaM8`, klepni na „Použít tuto složku" a povol přístup. Na tabletu udělej totéž.
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
  převzetí změněného save, kolize se zálohou) i na telefonu a tabletu s Google Diskem. V MaM 8 je ověřený
  v emulátoru (zkušební soubor ze složky do hry a zpět, zástupce u ikony), na telefonu zatím ne.

---

## Když upravený engine nemáš

Pak je potřeba MM8 do OpenEnroth dopsat. Není to práce na jeden prompt, ale na dlouhé vedení
(„pokračuj, dokud to nebude hotové“). Takhle vznikl tenhle port a osvědčilo se:

- **Základ z MaM 6:** všechny Android úpravy z návodu k MaM 7 (instalátor dat, dotyková tlačítka,
  klávesnice, tablet) a rozlišení her z MaM 6 platí i tady. Nejdřív postav MaM 7 a MaM 6,
  pak kopii enginu z MaM 6 rozšiřuj o MM8.
- **Rozlišení hry podle dat:** `isMm8()` v `Utility/GameVariant.h`. MM8 se pozná podle `EnglishT.lod`.
  Každá změna je větev `if (isMm8())`, chování MM6 a MM7 zůstává beze změny.
- **Pozor na stejná čísla:** MM8 používá stejná čísla předmětů, zvuků, domů, questbitů a příšer jako MM7,
  ale často pro úplně jiné věci (artefakty 500–542, zvuky od 120 výš, chrámy, hospody, usable předměty).
  Každou konstantu z MM7 v kódu je potřeba projít a pro MM8 dohledat v `MM8-Rel.exe`, co tam doopravdy je.
- **Tabulky a pravidla z MM8-Rel.exe:** co není v datech, se čte přímo z exe (`Engine/Objects/Mm8Ids`,
  `mm8ExeData`) nebo se podle disassembleru přepíše do kódu s adresou v komentáři. Vodítkem je projekt
  MMExtension (github.com/GrayFace/MMExtension), jeho `Scripts/Core` a `Misc/Paper Doll/mm8`.
- **Povolání ve slotech MM7:** 16 povolání MM8 je uloženo ve slotech tříd MM7 (rytíř MM8 = Knight,
  šampion = Champion…). Kód MM7, který se ptá na povýšení, třídy nebo rasy, proto u MM8 dává nesmysly
  (třeba barvy úrovní dovedností podle povýšení z MM7). MM8 má pro každou třídu jen jedno povýšení.
- **Rasové schopnosti:** temný elf, upír a drak mají každý svou rasovou dovednost, v enginu je to jedna
  dovednost `SKILL_MM8_RACIAL`. Název, popis a kouzla se musí brát podle rasy postavy.
- **Družina 1–5 postav:** MM8 začíná s jednou postavou, další se najímají v hostinci (roster). Všechno,
  co v enginu počítalo se čtyřmi postavami, musí zvládnout pět.
- **Vývoj na PC, ne na telefonu:** desktopový Linux build v Dockeru, spuštěný pod Xvfb s daty MM8,
  ovládaný přes xdotool, se snímky obrazovky (`tools/dev.sh`, proměnné `OE_DEV_*` pro skok na mapu,
  do domu, na událost, dání předmětů, seslání kouzel, prohlídku všech map).
- **Originál jako vzor:** původní MM8 běží pod Wine v Dockeru (`docker-wine`, kontejner `mm8ref`)
  a obrazovky se porovnávají se snímky z originálu, včetně pozic a písma.
- **Po částech:** načtení dat → hlavní menu → tvorba postavy → mapa a pohyb → domy a rozhovory →
  najímání družiny → kouzla a rasové schopnosti → knihy a ukládání → artefakty, kořist, Arcomage,
  konec hry. Po každé části kontrola stylu a unit testy (`docker-desktop/test.sh`) a commit.
- **Android build je přísnější:** APK kompiluje clang z NDK, desktop gcc. Clang navíc odmítne třeba
  zúžení čísla v `{}` (unsigned → int). Po větší změně proto sestav i APK, ne jen desktopovou verzi.

---

Pozn.: prompt odkazuje na stav repozitáře a verze nástrojů ze září 2026 (NDK 28.1, CMake 3.31, SDL 3.2.22).
