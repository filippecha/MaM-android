# MaM 7 – rozšíření „Stín nad Krajem“ (návrh scénáře)

Nová venkovní lokace **Kraj** severně od Tatalie, inspirovaná Pánem prstenů. Na jihu mapy (u vstupu z Tatalie)
leží vesnička **Hobitín** s 20 hobitími norami. Ve zbytku mapy jsou vchody do 20 dungeonů, na které navazují úkoly
z nor. Jména postav a míst jsou podle českého překladu Stanislavy Pošustové (Bilbo Pytlík, Gandalf, Sauron…), vedlejší
postavy jsou vymyšlené. Rozšíření je proto jen v soukromém repozitáři, ne ve veřejném MaM-android.

Stav: scénář odsouhlasen 29. 9. 2026, příšery budou silnější než jejich vzory.

- **Etapa 1 hotová (29. 9. 2026):** mapa Kraje (`out16.odm` v `data/kraj.lod`, generuje `tools/gen_kraj.py`) s kopci,
  řekou Brandyvínou se dvěma brody, bažinami, Starým hvozdem, klikatými cestami, vesnicí s 20 kopci pro nory
  a minimapou. Z Tatalie vede na sever nový průsmyk (engine ho vyhloubí při načtení mapy), cesta trvá 3 dny.
  Staré uložené hry mapu Kraje založí čistou. Ověřeno na PC, APK postavené.
- **Etapa 2 hotová (30. 9. 2026):** Hobitín. 20 vchodů do kopců (3D model s kulatými dveřmi a okny, textury
  kreslí `tools/art.py`), v norách 20 obyvatel s portréty z MM7, pozdravy a dvěma tématy rozhovoru (`tools/hobitin.py`).
  Nora 6 je hospoda U Zeleného draka (nocleh, jídlo, Arcomage podle pravidel Harmondale), 13 kovárna, 16 krámek
  s lektvary, 17 léčebna. Interiéry jsou statické obrázky místo videí. Domy, postavy a texty zabírají nepoužité řádky
  původních tabulek (domy 446-461, 13, 52, 88, 121, postavy 447-466), takže formát uložených her se nemění, a staré
  savy si hobity doplní z tabulky. Uložený stav Kraje z etapy 1 se zahodí a Kraj se založí znovu. Úkoly zatím
  nejsou, témata na ně jen odkazují (etapa 3).

---

## Svět a zápletka

Kraj je zelená, kopcovitá země půlčíků (hobitů), oddělená od Tatalie hraničním hvozdem. Hobité žijí v norách
vyhloubených v travnatých kopcích, s kulatými dveřmi. Stará legenda vypráví o **Jednom prstenu**, který kdysi ukul
**Temný pán Sauron**, mocný lich vyhnaný z Jámy v Deyje. Prsten se ztratil v Mlžných horách a před šedesáti lety
ho odtamtud nevědomky přinesl hobit **Bilbo Pytlík**.

Teď se do Kraje vracejí **Prstenové přízraky** a nekromanti z Deyji hledají prsten pro svého pána. Družina přichází
z Tatalie právě na Bilbovu oslavu stých jedenáctých narozenin, na které Bilbo zmizí a prsten zůstane jeho
synovci **Frodovi**. Úkolem družiny je pomoci hobitům, najít **Gandalfa Šedého**, odhalit zrádce
**Sarumana Bílého** a nakonec prsten zničit v **Hoře osudu**.

Propojení s MM7: kult Temného pána pochází z nekromantského cechu v Deyje, Saruman kdysi studoval v Bracadě
a obchodníci z Tatalie jsou první, kdo si všimnou skřetích nájezdů.

---

## Hlavní linka „Jeden prsten“ (8 kroků, 9 dungeonů)

| # | Zadává (nora) | Úkol | Dungeon | Odměna |
|---|---|---|---|---|
| 1 | Bilbo Pytlík (1) | Po oslavě se Pipin a Smíšek ztratili ve Starém hvozdu, najdi je. | D1 Starý hvozd | zkušenosti, Bilbova mapa Kraje |
| 2 | Frodo Pytlík (2) | Mohyloví přízraci unesli Froda, osvoboď ho a přines mohylový meč. | D2 Mohylové vrchy | Mohylový meč |
| 3 | Gandalf (20) | Gandalf čeká na Větrově, ale věž obsadily Prstenové přízraky. | D7 Větrov | Gandalfovo znamení |
| 4 | Gandalf (dopis v noře 20) | Gandalf je zajatcem Sarumana Bílého, osvoboď ho. | D12 Orthank | hůl, úroveň magie |
| 5 | Gandalf (20) | Jediná cesta na východ vede Skřetími doly, kde spí balrog. | D4 Morie | mitrilová košile |
| 6 | Samvěd Křepelka (3) | Froda zajala obří pavoučice Odula v Temném hvozdu. | D5 Pavoučí doupě | elfí plášť |
| 7 | Hraničář Chodec (19) | V Chrámu stínu je mapa k Černé bráně a jméno Temného pána. | D18 Chrám stínu | Chodcův meč |
| 8 | Frodo Pytlík (2) | Projdi Černou bránou a znič prsten v ohni Hory osudu. | D19 Černá brána, D20 Hora osudu | titul „Přítel Kraje“, trvalý bonus ke všem vlastnostem, zlato |

---

## Hobitín – 20 nor a jejich obyvatelé

| Nora | Obyvatel | Role | Úkol | Dungeon |
|---|---|---|---|---|
| 1 | Bilbo Pytlík | hlavní linka | krok 1 | D1 |
| 2 | Frodo Pytlík | hlavní linka | kroky 2 a 8 | D2, D19, D20 |
| 3 | Samvěd Křepelka, zahradník | hlavní linka | krok 6 | D5 |
| 4 | Pipin Bral | boční | Po návratu z hvozdu chce zpět svou ukradenou dýmku a měšec z pašerácké jeskyně. | D3 Pašerácká jeskyně |
| 5 | Smíšek Brandorád | boční | Trolové kradou ovce a sudy piva z Brandyvínského statku. | D6 Trollí sluj |
| 6 | Tlouštík Bulva, host hospody U Zeleného draka (hostinský Chmelík) | hospoda (Arcomage, jídlo, nocleh) | Ve Vlčí rokli se usadili vrrci a trhají poutníky. | D9 Vlčí rokle |
| 7 | Glóin, trpasličí host | boční | Chce se dostat do Síně trpasličích králů a pohřbít předky. | D8 Síň trpasličích králů |
| 8 | Sedlák Červík | boční | Z jeho polí mizí úroda, stopy vedou do Mlžných blat. | D11 Mlžná blata |
| 9 | Lobelie Pytlíková ze Sáčkova | boční | Její syn Lotho se zapletl s lapky, kteří obsadili starý mlýn. | D10 Opuštěný mlýn |
| 10 | Poštmistr Tukovský | boční | Pošta pro Hraniční pevnost nedorazila, pevnost je obležená. | D17 Pevnost hraničářů |
| 11 | Bába Bylinkářka | boční | Duch jejího praděda bloudí po Stezkách mrtvých. | D14 Stezky mrtvých |
| 12 | Elaniel, elfí poutnice | boční | Její bratr je vězněm v elfí pevnosti Temného hvozdu. | D16 Elfské vězení |
| 13 | Tomík Kladívko, syn kováře Kladívka | kovárna (zbraně) | Skurut-hai ukradli zbraně pro domobranu a odvlekli je do svého tábora. | D13 Tábor Skurut-hai |
| 14 | Bard Zpěvánek | boční | Legenda o drakovi Šmakovi a pokladu Osamělé hory. | D15 Šmakovo doupě |
| 15 | Starosta Vilda Bělonožka | boční + odměny | Po každém splněném úkolu dává hobití čest, za 10 čestí dar Kraje. | na mapě Kraje |
| 16 | Kopretina Proutková, žena kupce Proutka | krámek (lektvary) | Najdi na mapě tři ztracená prasátka. | na mapě Kraje |
| 17 | Mateřídouška, učednice léčitelky Heřmánkové | léčebna (chrám) | Nasbírej v Kraji pět vzácných bylin. | na mapě Kraje |
| 18 | Mistr Šipka | boční | Sestřel pět krkavců-zvědů Sarumana. | na mapě Kraje |
| 19 | Hraničář Chodec | hlavní linka | krok 7 | D18 |
| 20 | Gandalf (host v noře) | hlavní linka | kroky 3, 4 a 5 | D7, D12, D4 |

---

## 20 dungeonů

| # | Dungeon | Příšery | Hlavní protivník |
|---|---|---|---|
| D1 | Starý hvozd (jeskyně pod Starou vrbou) | Stromy, obří pavouci | Stará vrba |
| D2 | Mohylové vrchy | Stíny (mohyloví přízraci), kostlivci | Pán mohyl |
| D3 | Pašerácká jeskyně u Brandyvínu | lapkové, zloději | Bill Potměchut |
| D4 | Morie (Skřetí doly) | skřeti, trolové | Balrog |
| D5 | Pavoučí doupě | obří pavouci | Odula |
| D6 | Trollí sluj | bažinní trolové | Tom, Bert a Vilém |
| D7 | Větrov | Prstenové přízraky | Vůdce přízraků |
| D8 | Síň trpasličích králů | duchové trpaslíků, golemové | Kamenný strážce |
| D9 | Vlčí rokle | vrrci (vlci) | Vůdce smečky |
| D10 | Opuštěný mlýn | lapkové | Šarkoun |
| D11 | Mlžná blata | bažinní trolové, ještěři, shnilé mrtvoly | Blatní netvor |
| D12 | Orthank v Železném pasu | mágové, golemové, skřeti Skurut-hai | Saruman Bílý |
| D13 | Tábor Skurut-hai | skřeti Skurut-hai | Uglúk |
| D14 | Stezky mrtvých | duchové, shnilé mrtvoly | Král mrtvých |
| D15 | Šmakovo doupě | draci, gogové | drak Šmak |
| D16 | Elfské vězení Temného hvozdu | pavouci, elfí strážci (očarovaní) | Pavoučí královna |
| D17 | Pevnost hraničářů | skřeti, trolové | Velitel obléhání |
| D18 | Chrám stínu | nekromanti, lichové | Velekněz stínu |
| D19 | Černá brána | ďábelští poskoci, děsové | Strážce brány |
| D20 | Hora osudu | ohniví hlídači, ďáblové | Temný pán Sauron (lich) |

---

## Příšery inspirované Pánem prstenů

Nové příšery vzniknou z existujících druhů MM7 s novým jménem, přebarvením (hra na to má palety) a upravenými
vlastnostmi. Úplně nová grafika příšer (animované sprity z 8 stran) by byla výrazně pracnější.

| Příšera Kraje | Základ z MM7 | Poznámka |
|---|---|---|
| Skřet, Skurut-hai, skřetí šaman | Goblin | tmavší a silnější varianty |
| Troll, horský troll | Bažinný trol | kamenné a horské přebarvení |
| Stará vrba, ent | Strom | ent jako neutrální, stará vrba nepřátelská |
| Mohylový přízrak, Prstenový přízrak | Stín, Duch | černé a zelenavé varianty |
| Odula, obří pavouk | Obří pavouk | Odula jako obří boss |
| Vrrk | Obří krysa / pes | k ověření, jak vypadá přebarvený |
| Krkavec Sarumanův | Obří netopýr / Harpyje | |
| Drak Šmak | Obří drak | rudozlaté přebarvení |
| Balrog | Ďábelský poskok (nejsilnější varianta) | ohnivé přebarvení |
| Nekromant, velekněz stínu | Nekromancer | |
| Temný pán Sauron | Lich | finální boss |
| Lapka, Šarkoun | Raubíř, Zloděj | |

---

## Jak se to technicky udělá

- **Mapa Kraje:** nový soubor mapy vygenerovaný skriptem (terén s travnatými kopci, řeka Brandyvína, cesty, les,
  stromy a skály ze stávajících spritů). Z Tatalie se na sever dá dojít pěšky, z Kraje zpět na jih.
- **Hobití nory:** kopce v terénu a v nich malý 3D model kulatých dveří. Vstup otevře okno domu jako v ostatních
  vesnicích, s portrétem hobita a tématy rozhovoru.
- **Úkoly:** skripty událostí (úkoly, deník, odměny) a texty v češtině v kódování hry.
- **Dungeony:** nové 3D dungeony od nuly nejsou realistické. Půdorysy se převezmou z dungeonů MM6 a MM8, které
  máš koupené a hráč MM7 je nikdy neviděl, převedou se do formátu MM7 a osadí se novými příšerami, truhlami a texty.
- **Nová grafika** (kulaté dveře, portréty hobitů, ikony předmětů): buď ji vytvořím jednoduše skriptem, nebo
  dodáš obrázky (třeba z generátoru obrázků) a já je převedu do formátu hry.

## Etapy

1. Záloha současné verze, mapa Kraje s terénem a přechodem z Tatalie.
2. Hobitín: 20 nor s kulatými dveřmi, obyvatelé s rozhovory, hospoda, obchod, léčitelka.
3. Hlavní linka: texty, úkoly, deník, odměny.
4. Příšery Kraje.
5. Dungeony hlavní linky (9).
6. Dungeony bočních úkolů (11).
7. Vyvážení obtížnosti a dohrání celého rozšíření.
