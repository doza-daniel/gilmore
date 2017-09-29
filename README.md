# Projekat sa kursa Autmatsko Rezonovanje 2017
### Mentor: dr Filip Maric
### Studenti: Daniel Doza, Isidora Djurdjevic

# Gilmorova Procedura
Program demonstrira rad Gilmorove procedure. Glavna logika za Gilmorov algoritam, nalazi se u `gilmore.cpp`, `gilmore.h`,
`herbrand.cpp` i `herbrand.h` datotekama. Dokazivanje formule se pokrece funkciom `void prove(Signature &, const Formula &)`.
Formula se negira, a zatim se poziva u funkciji `void gilmore(Signature &, const Formula &)`. Ova funkcija najpre eliminise
kvantifikatore svodjenjem na NNF, prenex i skolemizacijom. Zatim, iz formule se izvlace sve varijable, i za svaku varijablu se radi
supstitucija nekim termom iz Erbranovog univerzuma. Posto je Erbranov univerzum beskonacan, generisacemo ga u nivoima i za
svaki nivo cemo pokusati da uradimo supstituciju. Formule dobijene supstitucijom se zatim spajaju konjunkcijom. Dobijena
formula se zatim prevodi u DNF i vrsimo Gilmorovu metodu mutliplikacije. Za svaku konjunkciju iz DNF-a i za svaki literal iz te
konjunkcije se proverava da li postoji njegov negirani literal, cime bi cela konjunkcija postala netacna. Brojimo invalidirane
konjunkcije, i ukoliko smo pokazali da su sve konjunkcije iz DNF-a netacno, prijavljujemo `UNSAT`(dakle, polazna formula je valjana)
inace nastavljamo u sledeci nivo Erbranovog univerzuma. Zbog potencijalno beskonacno koraka, i kombinatorne eksplozije Erbranovog
univerzuma, ogranicavamo se na maksimalno pet iteracija Gilmorove procedure.
# Erbranov univerzum
Logika koja predstavlja Erbranov univerzum, nalazi se u datotekama `herbrand.h` i `herbrand.cpp`. Prilikom instanciranja klase,
za datu formulu izvlace se svi funkcijski simboli i simboli konstante (predstavljene kao funkcije arnosti 0). U prvom koraku,
ubacujemo sve konstante u univerzum, a ukoliko nema konstanti u formuli, onda generisemo novu konstantu. Funkcija `void nextLevel()`
generise novi nivo, dodavanjem primene funkcijskih simbola na trenutne vrednosti univerzuma.
#### Primer:
Funkcijski simboli: {f, g} (ar(f)=1, ar(g)=1)
- Univerzum nivoa 0: {c}
- Univerzum nivoa 1: {c, f(c), g(c)}
- Univerzum nivoa 2: {c, f(c), g(c), f(f(c)), g(g(c))}
- ...

Mali problem nastaje ako funkcije imaju arnost koja je veca nego sto ima trenutno vrednosti u univerzumu, tada vrednosti koje nedostaju
menjamo novim konstantama. Takodje, ako su funkcije arnosti vece od 1, gledamo sve permutacije trenutnog nivoa, duzine x, gde je x
arnost funkcije koju primenjujemo.

## Pomocne funkcije
U datotekama `first_order_logic.h` i `first_order_logic.cpp` se pretezno nalazi kod sa vezbi, ali uz male modifikacije. Na primer,
signatura je promenjena da ima mogucnost generisanja novih konstanti, dodate su funkcije za izvlacenje funkcijskih simbola iz formule
i simbola konstanti (interpretiraju se kao funkcijski simboli arnosti 0) i jos neke sitnice.

# Pokretanje programa
U korenom direktorijumu nalazi se `Makefile`, tako da je kompilacija i pokretanje programa:
```bash
make
./gilmore
```
Uklanjanje objeknih i izvrsne datoteke:
```bash
make clean
```
U datoteci `main.cpp` mozete naci primer zapisivanja formule, i pozivanja funkcije za dokazivanje.
