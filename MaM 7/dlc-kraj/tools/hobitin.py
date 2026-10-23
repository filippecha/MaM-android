"""Hobbiton of the MaM 7 expansion "Stín nad Krajem": the 20 hobbit holes, their inhabitants and dialogues.

Everything takes unused rows of the original tables, so that saves keep their layout:
houses 446-461 (dwellings) and the unused tavern 121, weapon shop 13, alchemist 52 and temple 88,
NPCs 447-466, greetings 206-225, topics (global event ids) 600-639 and texts 800-839.
"""
import struct

FIRST_NPC = 447
FIRST_GREETING = 206
FIRST_TOPIC = 600
FIRST_TEXT = 800

# Rooms of pAnimatedRooms added by the engine patch: a still picture from icons.lod each.
ROOM_HOLE, ROOM_TAVERN, ROOM_SHOP, ROOM_TEMPLE, ROOM_SMITH = 196, 197, 198, 199, 200
ROOM_PICTURES = {ROOM_HOLE: 'krajnora', ROOM_TAVERN: 'krajhospoda', ROOM_SHOP: 'krajobchod',
                 ROOM_TEMPLE: 'krajlecitel', ROOM_SMITH: 'krajkovar'}

# Hole number of the scenario (SCENAR.md) for each hill of gen_kraj.hobbit_holes(), in its order:
# north and south side of the lane at row 84, then of the lane at row 98, columns 44, 52, 76, 84, 92.
HOLE_ORDER = (19, 1, 3, 2, 20,
              8, 4, 5, 7, 9,
              11, 15, 10, 12, 14,
              17, 6, 16, 13, 18)

# Special houses: hole -> (house id, 2devents type, room, name, proprietor, title, price, skill price, days,
# opens, closes).
SPECIAL = {
    6: (121, 'Tavern', ROOM_TAVERN, 'U Zeleného draka', 'Chmelík', 'Hostinský', '6', '1', '', '5', '2'),
    13: (13, 'Weapon Shop', ROOM_SMITH, 'Kladívkova kovárna', 'Kladívko', 'Kovář', '1.5', '1', '7', '7', '19'),
    16: (52, 'Alchemist', ROOM_SHOP, 'Proutkův krámek', 'Proutek', 'Kupec', '1.5', '1', '7', '7', '19'),
    17: (88, 'Temple', ROOM_TEMPLE, 'Heřmánkova léčebna', 'Heřmánková', 'Léčitelka', '1.5', '1', '', '1', '24'),
}

# Hole -> (house name, inhabitant, portrait, greeting 1, greeting 2, (topic, text), (topic, text)).
HOLES = {
    1: ('Dno pytle', 'Bilbo Pytlík', 54,
        'Dobré ráno! Tedy, vlastně je už skoro poledne, ale v Kraji se to nebere tak přesně. Vítejte ve Dně pytle, '
        'já jsem Bilbo Pytlík. Chystám oslavu svých stých jedenáctých narozenin a každý host je vítán!',
        'Á, zase vy! Pojďte dál, konvice je na ohni.',
        ('Oslava', 'Sto jedenáct let, to je úctyhodný věk i na hobita! Pozval jsem celý Kraj, Gandalf slíbil '
         'ohňostroj a jídla bude, až se budou prohýbat stoly. A na konci mám pro všechny malé překvapení. Ale to vám '
         'zatím neprozradím.'),
        ('Starý prsten', 'Prsten? Jaký prsten? Ach, ten. Našel jsem ho kdysi na svých cestách, v jeskyni pod Mlžnými '
         'horami. Je to jen taková drobnost, ale nějak se ho nedokážu vzdát. Je můj. Můj vlastní. Promiňte, nevím, '
         'co to do mě vjelo.')),
    2: ('Nora pod Kopcem', 'Frodo Pytlík', 597,
        'Jsem Frodo Pytlík, Bilbův synovec. Strýček je poslední dobou nějaký neklidný, pořád mluví o horách a o '
        'cestě, která vede pořád dál. Co vás přivádí do Hobitína?',
        'Rád vás zase vidím. Strýček Bilbo se na vás ptal.',
        ('Strýček Bilbo', 'Strýček mě vychoval jako vlastního. Naučil mě elfí jazyky a vyprávěl mi o drakovi Šmakovi '
         'a o trpaslících. Poslední dobou ale špatně spí a mluví ze spaní o nějakém prstenu.'),
        ('Černý jezdec', 'Sam říká, že včera večer viděl na cestě u Povodí černého jezdce. Ptal se na Pytlíka. '
         'Zahalený, beze slova, jen větřil jako pes. Ani nevím proč, ale běhá mi z toho mráz po zádech.')),
    3: ('Křepelkova nora', 'Samvěd Křepelka', 236,
        'Samvěd Křepelka, k službám. Většina mi říká Sam. Starám se o zahradu na Dně pytle, jako předtím můj táta. '
        'Neušlapejte mi prosím ty petrklíče.',
        'Dobrý den vespolek. Zrovna jsem zaléval.',
        ('Zahrada', 'Kdo chce mít pěknou zahradu, musí jí rozumět. Brambory, mrkev, petrklíče a trocha tabáku. Táta '
         'vždycky říká: Samvěde, zůstaň u svých záhonů a nepleť se do věcí velkých lidí. Jenže já bych moc rád '
         'jednou viděl elfy.'),
        ('Elfové', 'Viděl jsem je jen jednou, jak jdou za soumraku lesem k Šedým přístavům. Zpívali tak krásně, že '
         'jsem zapomněl dýchat. Říká se, že odcházejí za moře a už se nevrátí. To je smutné, nemyslíte?')),
    4: ('Nora Bralových', 'Pipin Bral', 240,
        'Peregrin Bral, ale Pipin stačí! Nemáte náhodou něco k jídlu? Druhá snídaně už byla, ale do svačiny je '
        'ještě daleko.',
        'Zase vy! Přinesli jste něco dobrého?',
        ('Houby', 'Nejlepší houby v Kraji rostou na poli sedláka Červíka. Nejlepší houby jsou taky nejvíc hlídané, '
         'protože sedlák Červík má tři obrovské psy. Tak to v životě chodí.'),
        ('Pašeráci', 'U Brandyvíny se usadila banda pašeráků. Ukradli mi dýmku a měšec s nejlepším tabákem z Jižní '
         'čtvrtky! Kdo by v Kraji kradl? To se tu nikdy nestávalo.')),
    5: ('Nora Brandorádových', 'Smíšek Brandorád', 233,
        'Smělmír Brandorád, pro přátele Smíšek. Pocházím z druhé strany Brandyvíny, tam jsou hobiti o něco '
        'odvážnější. Aspoň to o sobě rádi říkáme.',
        'Buďte zdrávi! Jak se vede na cestách?',
        ('Za řekou', 'Za řekou žijeme hned vedle Starého hvozdu. Postavili jsme si tam vysoký živý plot, protože '
         'z hvozdu občas vylézá něco, co nechcete potkat v noci.'),
        ('Trollové', 'Na Brandyvínském statku se ztrácejí ovce a sudy piva. Stopy jsou velké jako necky a vedou na '
         'sever, k Trollí sluji. Nikdo z hobitů si tam netroufne.')),
    6: (None, 'Tlouštík Bulva', 241,
        'Tlouštík Bulva, těší mě. Nevstávám, promiňte, tohle křeslo je nejpohodlnější v celém Kraji. U Zeleného '
        'draka se čepuje to nejlepší pivo široko daleko.',
        'Přisedněte si! Hostinský hned donese další korbel.',
        ('Novinky', 'Tady se v Kraji dozvíte všechno. Kdo s kým, kdo komu dluží a kdo viděl co. Včera tu třeba '
         'jeden povídal, že na severu chodí stromy. No, chodí! To by mě zajímalo, co pil.'),
        ('Vrrci', 'Poutníci od Vlčí rokle vyprávějí o obrovských vlcích, kterým se říká vrrci. Prý na nich jezdí '
         'skřeti. Hostinský už kvůli nim nemá odkud brát zvěřinu.')),
    7: ('Nora pro hosty', 'Glóin', 316,
        'Glóin, syn Gróinův, trpaslík z Osamělé hory. Nedívejte se tak, i trpaslík si občas potřebuje odpočinout '
        'mezi hobity. Mají tu dobré jídlo a nikdo se tu s nikým nehádá.',
        'Zdravím vás, přátelé. Sekeru mám nabroušenou, kdybyste ji potřebovali.',
        ('Osamělá hora', 'Když drak Šmak padl, vrátili jsme se do Ereboru a Osamělá hora zase zní kladivy. Ale teď '
         'k nám přicházejí poslové z temných zemí a ptají se na hobity. To se mi nelíbí.'),
        ('Síň králů', 'Hluboko pod horami leží Síň trpasličích králů. Moji předkové tam odpočívají, ale síň '
         'obsadily přízraky a kamenní strážci se obrátili proti trpaslíkům. Chtěl bych ty kosti pohřbít se ctí.')),
    8: ('Červíkův statek', 'Sedlák Červík', 232,
        'Sedlák Červík. Jestli jste přišli krást houby z mého pole, pustím na vás psy! Aha, vy nejste hobiti. Tak '
        'to se omlouvám, v poslední době se tu potlouká všelijaká chátra.',
        'To jste zase vy. No, pojďte dál, ale boty si otřete.',
        ('Psi', 'Kousavec, Tesák a Vlk. Nejlepší hlídači v Kraji. Všichni zloději mých hub je znají zblízka. '
         'Obzvlášť jeden mladý Pytlík, ale to už je dávno.'),
        ('Mlžná blata', 'Z mého pole mizí úroda. Zloději to nejsou, to by psi poznali. Něco přichází v noci '
         'z Mlžných blat a nechává za sebou sliz a zápach bahna. Psi se toho bojí a to se jim ještě nestalo.')),
    9: ('Sáčkov', 'Lobelie Pytlíková ze Sáčkova', 203,
        'Lobelie Pytlíková ze Sáčkova. Doufám, že nepatříte k těm nezvaným hostům, co se tlačí na Bilbovu oslavu. '
        'Dno pytle mělo patřit nám, a ne tomu jeho synovci!',
        'Zase vy? Co ještě chcete?',
        ('Dno pytle', 'Dno pytle je nejkrásnější nora v celém Kraji a mělo připadnout nám, Pytlíkům ze Sáčkova. '
         'Ale Bilbo si vzal do domu toho Froda a všechno odkáže jemu. Uvidíte, ještě si pro ni přijdu.'),
        ('Lotho', 'Můj syn Lotho je dobrý chlapec, jen se dal do řeči s nějakými cizinci. Obsadili starý mlýn '
         'u řeky a Lotho s nimi tráví víc času než doma. Kdyby ho někdo přivedl zpátky, byla bych mu vděčná. Ale '
         'neříkejte to nikomu.')),
    10: ('Pošta', 'Poštmistr Tukovský', 612,
         'Poštmistr Tukovský, hobitínská rychlá pošta. Rychlá je tedy hlavně ve jméně, ale dopisy doručujeme '
         'spolehlivě. Tedy doručovali jsme, než se na cestách začalo dít něco divného.',
         'Dobrý den! Žádný dopis pro vás bohužel nemám.',
         ('Pošta', 'Každý hobit rád píše dopisy, hlavně když se v nich dá pomlouvat sousedy. Pošta je proto '
          'nejdůležitější úřad v Kraji, hned po hostinci.'),
         ('Hraniční pevnost', 'Pošta pro hraničářskou pevnost na severu už tři týdny nedošla. Posel se nevrátil '
          'a od severu je vidět kouř. Bojím se, že pevnost je obležená.')),
    11: ('Bylinková nora', 'Bába Bylinkářka', 218,
         'Pojďte blíž, ať si vás prohlédnu. Hmm, unavení, pořezaní a hladoví, jak už to u dobrodruhů bývá. Jsem '
         'Bába Bylinkářka a na každou nemoc tu mám nějakou bylinku.',
         'Zase jste tady? Posaďte se, uvařím vám heřmánek.',
         ('Byliny', 'Královské listí, athelas, léčí rány, na které jiné byliny nestačí. Heřmánek na žaludek, '
          'mateřídouška na kašel. A na zlomené srdce nejlíp pomáhá dobré jídlo a přátelé.'),
         ('Praděd', 'Můj praděd byl hraničář a padl na Stezkách mrtvých. Jeho duch prý dodnes bloudí mezi hroby '
          'a nemůže najít klid. Kdyby mu někdo vrátil jeho meč, třeba by konečně usnul.')),
    12: ('Nora u lesa', 'Elaniel', 468,
         'Zdravím vás, poutníci. Jsem Elaniel z Temného hvozdu. Hobiti mi poskytli přístřeší, když jsem utíkala '
         'před stíny, které se v našich lesích probouzejí.',
         'Ať vaše cesty osvětlují hvězdy.',
         ('Temný hvozd', 'Kdysi se mu říkalo Zelený hvozd a byl nejkrásnějším lesem na světě. Pak do něj přišel '
          'stín a s ním pavouci. Teď je v něm tma i za poledne.'),
         ('Bratr', 'Můj bratr zůstal v naší pevnosti v Temném hvozdu. Pevnost padla, stráže jsou očarované a '
          'v podzemí vládne Pavoučí královna. Jestli ještě žije, drží ho tam.')),
    13: (None, 'Tomík Kladívko', 237,
         'Tomík Kladívko. Táta kuje vepředu v dílně, já mu dělám měchy a nosím uhlí. Jednou budu kovář jako on!',
         'Ahoj! Táta zrovna brousí sekery.',
         ('Kovárna', 'Táta kuje motyky, podkovy a hřebíky. Poslední dobou ale kuje i meče pro domobranu. Starosta '
          'říká, že je to jen pro jistotu.'),
         ('Ukradené meče', 'Všechny meče pro domobranu nám ukradli skurut-hai! Velcí skřeti, co se nebojí slunce. '
          'Odvlekli je do svého tábora na severu. Táta od té doby skoro nemluví.')),
    14: ('Nora u studánky', 'Bard Zpěvánek', 563,
         'Á, posluchači! Jsem Bard Zpěvánek a znám všechny písně Kraje, od pijáckých až po Píseň o Osamělé hoře. '
         'Chcete si nějakou poslechnout?',
         'Zrovna skládám novou píseň, třeba bude o vás!',
         ('Písně', 'Znám píseň o cestě, která pořád jde dál a dál, pak pijácké, koupací i ohňové. A taky jednu '
          'smutnou, o drakovi, co spí na zlatě v Osamělé hoře. Tu vám zazpívám, až budete mít čas.'),
         ('Rudozlatý drak', 'Šmak je mrtvý, to ví každý. Jenže poutníci od hor povídají o dalším drakovi, '
          'rudozlatém, co si udělal doupě ve staré Šmakově sluji. A dračí poklad přece nikdy nezůstane bez '
          'hlídače.')),
    15: ('Starostova nora', 'Starosta Vilda Bělonožka', 58,
         'Vilda Bělonožka, starosta Kraje. Úřaduji hlavně na hostinách, to je nejdůležitější povinnost starosty. '
         'Co pro vás mohu udělat?',
         'Vítejte, vítejte! Kraj je rád za každou pomoc.',
         ('Hobití čest', 'Kraj si váží každého, kdo mu pomůže. Za každý splněný úkol pro hobity vám dám hobití čest '
          'a kdo jich nasbírá deset, dostane dar Kraje. Tak to u nás chodí odnepaměti. Tedy od včerejška, ale '
          'znělo to dobře.'),
         ('Starosti', 'Na severu kouř, na cestách černí jezdci, na polích sliz a v hospodě řeči o chodících '
          'stromech. Tolik starostí Kraj nezažil od Dlouhé zimy.')),
    16: (None, 'Kopretina Proutková', 158,
         'Kopretina Proutková. Manžel prodává vepředu v krámku, já se starám o všechno ostatní. A že je toho '
         'poslední dobou dost, když nám utíkají prasátka!',
         'Dobrý den! Nenašli jste náhodou naše prasátka?',
         ('Prasátka', 'Utekla nám tři prasátka! Jedno k řece, jedno do Starého hvozdu a třetí prý viděli '
          'u Povodí. Jestli je najdete, manžel vám dá slevu v krámku. Tedy, já mu to řeknu.'),
         ('Krámek', 'Prodáváme lektvary, byliny a všechno, co hobit potřebuje na cesty. Tedy skoro nic, protože '
          'hobiti na cesty nechodí. Ale vy ano, tak si vyberte.')),
    17: (None, 'Mateřídouška', 157,
         'Jsem Mateřídouška, učednice paní Heřmánkové. Učím se poznávat byliny a vařit z nich léky. Paní '
         'léčitelka je vepředu, jestli potřebujete ošetřit.',
         'Dobrý den! Dneska jsem se naučila tři nové byliny.',
         ('Léčitelka', 'Paní Heřmánková umí vyléčit i otravu pavoučím jedem. Říká ale, že poslední dobou chodí '
          'víc raněných než za celý loňský rok.'),
         ('Vzácné byliny', 'Pro nové léky potřebujeme pět vzácných bylin, které rostou jen v Kraji. Jedna u řeky, '
          'jedna ve Starém hvozdu, jedna v blatech a dvě v kopcích. Sama si tam netroufnu.')),
    18: ('Střelnice', 'Mistr Šipka', 567,
         'Mistr Šipka, nejlepší střelec z luku v celém Kraji. Hobiti možná nevypadají jako válečníci, ale kámen '
         'nebo šíp vrhneme přesněji než kdokoli jiný.',
         'Přišli jste si zastřílet? Terče jsou za norou.',
         ('Lukostřelba', 'Nejdůležitější je klidný dech a trpělivost. A dobrá tětiva. Hobit, který umí střílet, '
          'se nemusí bát ani vlka.'),
         ('Krkavci', 'Nad Krajem krouží černí krkavci. To nejsou obyčejní ptáci, sledují každý náš krok a pak '
          'odlétají na jih, k Železnému pasu. Někdo nás špehuje.')),
    19: ('Nora na kraji vesnice', 'Chodec', 611,
         'Vysoký muž v zablácené kápi si vás chvíli mlčky prohlíží. "Říkají mi Chodec. Hlídám hranice Kraje, '
         'i když o tom hobiti nevědí. Tak je to lepší."',
         'Chodec kývne na pozdrav. "Buďte ostražití. Stíny se blíží."',
         ('Hraničáři', 'Hraničáři střeží hranice zemí, o kterých hobiti ani nevědí, že existují. Bez nás by Kraj '
          'už dávno nebyl tak klidný.'),
         ('Nepřítel', 'Na jihu se probouzí stará moc. Její služebníci hledají něco, co leží tady, v Kraji. Až '
          'přijde čas, budu potřebovat vaši pomoc.')),
    20: ('Pokoj pro poutníky', 'Gandalf', 703,
         'Jsem Gandalf, a Gandalf znamená mě! Hobiti mě znají hlavně podle ohňostrojů. Přijel jsem na Bilbovu '
         'oslavu, ale mám obavy, že se ve světě chystá něco mnohem méně veselého.',
         'Á, moji přátelé z dalekých zemí. Dobře, že jste tady.',
         ('Ohňostroj', 'Na Bilbovu oslavu jsem přivezl ty nejlepší ohňostroje. Rakety, hvězdy a jednoho ohnivého '
          'draka, ze kterého budou mít hobiti strach ještě za sto let.'),
         ('Jeden prsten', 'Bilbo má prsten, který dělá svého nositele neviditelným. Obávám se, že to není obyčejný '
          'kouzelný prsten. Musím prozkoumat staré spisy, než řeknu víc. Zatím o tom nikomu neříkejte.')),
}


def house_of(hole):
    """House id of a hole: the special houses have their own, dwellings take 446 and on in hole order."""
    if hole in SPECIAL:
        return SPECIAL[hole][0]
    return 446 + sum(1 for h in range(1, hole) if h not in SPECIAL)


def npc_of(hole):
    return FIRST_NPC + hole - 1


def enc(text):
    return text.encode('cp1250')


def houses_table():
    """k2devents.txt: rows for the houses of Hobbiton, same columns as 2devents.txt."""
    lines = ['2D Events (Stín nad Krajem)', '#\t#\tType\tMap\tPicture\tName\tProprieter Name\tTitle\tPicture\tState\t'
             'Rep\tPer\tVal\tA\tB\tC\tNotes:\tNotes(2):\tOpen\tClosed\tPic\tMap\tRestrictions\tText']
    for hole in range(1, 21):
        if hole in SPECIAL:
            house, kind, room, name, owner, title, val, a, c, opens, closes = SPECIAL[hole]
        else:
            house, kind, room, name, owner, title, val, a, c, opens, closes = \
                house_of(hole), 'H_Hobbit', ROOM_HOLE, HOLES[hole][0], '', '', '1.5', '', '', '', ''
        lines.append('\t'.join([str(house), str(hole), kind, '77', str(room), name, owner, title, '0', '0', '0', '0',
                                val, a, '', c, '', '', opens, closes, '0', '0', '0', '0']))
    return enc('\r\n'.join(lines) + '\r\n')


def npc_table():
    """knpcdata.txt: the inhabitants, in the unused NPC rows."""
    lines = ['NPC Data (Stín nad Krajem)', '#\tName\tPic\tA\tB\tC\t2D Location\tProfession\tGreet\tJoin\t'
             'Event A\tEvent B\tEvent C\tEvent D\tEvent E\tEvent F\tNotes']
    for hole in range(1, 21):
        topic = FIRST_TOPIC + 2 * (hole - 1)
        lines.append('\t'.join(map(str, [npc_of(hole), HOLES[hole][1], HOLES[hole][2], 0, 0, 0, house_of(hole), 0,
                                         FIRST_GREETING + hole - 1, 'n', topic, topic + 1, 0, 0, 0, 0,
                                         f'Kraj, nora {hole}'])))
    return enc('\r\n'.join(lines) + '\r\n')


def greet_table():
    lines = ['#\tGreeting 1\tGreeting 2\tNotes\tOwner']
    for hole in range(1, 21):
        lines.append(f'{FIRST_GREETING + hole - 1}\t{HOLES[hole][3]}\t{HOLES[hole][4]}\t\t{HOLES[hole][1]}')
    return enc('\r\n'.join(lines) + '\r\n')


def topic_table():
    lines = ['#\tTopic']
    for hole in range(1, 21):
        for k in range(2):
            lines.append(f'{FIRST_TOPIC + 2 * (hole - 1) + k}\t{HOLES[hole][5 + k][0]}')
    return enc('\r\n'.join(lines) + '\r\n')


def text_table():
    lines = ['#\tText']
    for hole in range(1, 21):
        for k in range(2):
            lines.append(f'{FIRST_TEXT + 2 * (hole - 1) + k}\t{HOLES[hole][5 + k][1]}')
    return enc('\r\n'.join(lines) + '\r\n')


def evt_record(event_id, step, opcode, params):
    body = struct.pack('<HBB', event_id, step, opcode) + params
    return bytes([len(body)]) + body


EVENT_EXIT, EVENT_SPEAK_IN_HOUSE, EVENT_MOUSE_OVER, EVENT_SHOW_MESSAGE = 1, 2, 4, 30


def global_events():
    """kglobal.evt: every topic shows its text."""
    out = b''
    for hole in range(1, 21):
        for k in range(2):
            topic = FIRST_TOPIC + 2 * (hole - 1) + k
            out += evt_record(topic, 0, EVENT_SHOW_MESSAGE, struct.pack('<I', FIRST_TEXT + 2 * (hole - 1) + k))
            out += evt_record(topic, 1, EVENT_EXIT, b'\0')
    return out


def door_events():
    """out16.evt: event n (1-20) is the door of the n-th hill, the hint is the house name.

    A hint isn't executed, so the command after it has the same step 0, as in the original maps."""
    out = b''
    for i, hole in enumerate(HOLE_ORDER):
        out += evt_record(i + 1, 0, EVENT_MOUSE_OVER, b'\0')
        out += evt_record(i + 1, 0, EVENT_SPEAK_IN_HOUSE, struct.pack('<I', house_of(hole)))
        out += evt_record(i + 1, 1, EVENT_EXIT, b'\0')
    return out


LEVEL_STRINGS = enc('Dveře\0')
