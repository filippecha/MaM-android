# Might and Magic VI, VII a VIII pro Android

Neoficiální Android porty klasických RPG Might and Magic VI, VII a VIII na open-source enginu
[OpenEnroth](https://github.com/OpenEnroth/OpenEnroth). Výsledkem je jedno APK na hru, které má herní data v sobě,
po instalaci se samo rozbalí a má dotykové ovládání pro telefon i tablet (16:10), češtinu (pokud ji máš ve hře)
a volitelnou synchronizaci uložených her přes Google Disk.

**Herní data v repozitáři nejsou.** Hru si musí každý koupit sám (GOG.com) a APK si postavit ze své instalace.
Hotové APK obsahuje tvoje herní data, proto je jen pro tebe a nesmí se nikam nahrávat ani šířit.

| Hra | Složka | Engine | Návod (prompt pro Claude Code) |
|---|---|---|---|
| Might and Magic VI: The Mandate of Heaven | [`MaM 6`](MaM%206/README.md) | OpenEnroth, MM6 dopsaná přímo do enginu | [NAVOD-port-MM6-android.md](MaM%206/out-android/NAVOD-port-MM6-android.md) |
| Might and Magic VII: For Blood and Honor | [`MaM 7`](MaM%207/README.md) | OpenEnroth + Android úpravy | [NAVOD-port-MM7-android.md](MaM%207/out/NAVOD-port-MM7-android.md) |
| Might and Magic VIII: Day of the Destroyer | [`MaM 8`](MaM%208/README.md) | OpenEnroth, MM8 dopsaná přímo do enginu | [NAVOD-port-MM8-android.md](MaM%208/out-android/NAVOD-port-MM8-android.md) |

## Co tu je

- `MaM X/OpenEnroth` – upravený engine (C++ a Android část v Javě). Kopie pro MaM 6 a MaM 8 umí MM6, MM7 i MM8
  a hru pozná podle dat.
- `MaM 7/docker` – Docker image `openenroth-android` pro build (Ubuntu, JDK 17, Android SDK a NDK), sdílí ho všechny tři díly.
- `MaM 6/docker-android`, `MaM 8/docker-android`, `MaM 7/docker/build.sh` – skripty, které APK sestaví.
- `MaM 7/nase-upravy*.patch` – změny MaM 7 proti čistému OpenEnroth (commit v `MaM 7/engine-upstream.txt`).
- `MaM 6/docker-desktop`, `MaM 8/docker-desktop`, `tools`, `docker-wine` – vývojové nástroje, se kterými MM6 a MM8
  vznikaly (desktopový build pod Xvfb, originální hra pod Wine pro srovnání obrazovek, čtení LOD a exe).
- `out`, `out-android` – návody a screenshoty.

## Jak postavit APK

Nejjednodušší je otevřít návod k dané hře a jeho blok PROMPT vložit do [Claude Code](https://claude.com/claude-code).
Návod vyjmenuje, co mít připravené (nainstalovaná hra z GOG, Docker Desktop, volitelně Android Studio s emulátorem)
a Claude pak data zkopíruje, APK postaví, zkontroluje a otestuje.

Ručně (PowerShell, složka hry, třeba `MaM 8`):

1. Herní data zkopíruj z vlastní instalace do `gamedata\MM6`, `gamedata\mm7` nebo `gamedata\MM8` (seznam souborů je v návodu).
2. Licence Android SDK zkopíruj z `%LOCALAPPDATA%\Android\Sdk\licenses` do `MaM 7\docker\licenses`
   a postav image: `docker build -t openenroth-android "..\MaM 7\docker"`.
3. Spusť build podle README dané hry. Podpisový klíč `keystore.jks` si build vytvoří sám při prvním spuštění,
   schovej si ho, jinak další verze nepůjde nainstalovat přes starou.

## Licence

- OpenEnroth a naše úpravy enginu: GNU LGPL 3.0 (`MaM X/OpenEnroth/LICENSE`), knihovny v `thirdparty` mají vlastní licence.
- Might and Magic je ochranná známka jejích vlastníků. Tento projekt s nimi nijak nesouvisí a neobsahuje
  žádná herní data. Ikony aplikací jsou vytažené z ikon původních herních exe.
