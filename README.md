# Optimális dinamikus elosztott MIS

## Bevezetés + fogalmak

MIS megkeresése (igazából nem is megkeresi csak fenntartja?) gráfban elosztott rendszer esetén. <br />

Független halmaz (IS):
  - Csúcsok olyan halmaza, ahol egyik él között sincs él, azaz szomszédos csúcsok nem szerepelnek benne
  - Átfogalmazva bármely élnek maximum az egyik végpontja lehet benne (de lehet, hogy egyik sem)

Maximum független halmaz (MIS):
  - A halmazhoz tetszőleges csúcsot hozzávéve már nem lesz maximális
  - Egy G gráfnak lehet több ilyenje is (felső korlát: 3<sup>n/3</sup>)
  - https://en.wikipedia.org/wiki/Maximal_independent_set

Lokalitása miatt gyorsan számolható *statikusan* elosztott környezetben (*logaritmikus* |V|-hez vagy a csúcsok fokszámához). <br />

Dinamikus eloszott környezetben a topológia folyamatosan változik -> élek/csúcsok lépnek be vagy törlődnek ki a gráfból. Minden ilyen topológia-változás után egy (statikus) elosztott MIS algoritmus lefuttatása szükséges (ua. komplexitása, mint a statikus esetnek) <br />

Ebben a tanulmányban...
  - Lokalitást kihasználva MIS változtatása (frissítése) dinamikus környezetben
  - *Szinkron* vagy *aszinkron* módon
  - **Egy változtatással** (= várhatóan egy csúcs változtatja meg a kimenetét 1 körben), tehát jobb, mint a logaritmikus megoldás!
  - Csúcsok és élek hozzáadásának és eltűnésének bármilyen esetében (*graceful* kilépés vagy hirtelen kiesés)
  - Statikus és dinamikus elosztott modellek különválasztása
  - Determinisztikus és véletlenszerűsített megoldások különválasztása
  - *History independency*: a következő lépés csak az aktuális állapottól függ

## Az algoritmus: Greedy sequential algorithm
  - Egyenletes eloszlással (uniform distribution) véletlenszerűen sorbarendezi a csúcsokat, aztán növekvő sorrendben veszi őket
  - A csúcsot hozzáadja a MIS-hez, ha nincs a a sorrendben nála kisebb szomszédok közül egyik sem a MIS-ben
  - Csúcs *invariáns tulajdonság*: az állapota csakis a nála kisebb sorszámú szomszédoktól függ
  - Ha változás történik a gráfban akkor a csúcsoknak lehet, hogy egynél többször kell állapotot váltania

### 1. állítás: Tetszőleges változtatás a gráfban - bármilyen véletlenszerű sorrend esetén - legfeljebb egy csúcs kimenetének megváltozását okozza

## Eloszott implementáció

G = (V, E) irányítatlan gráfban E(|S|) <= 1, azaz legfeljebb egyetlen változtatás elég a MIS megtartásához dinamikus környezetben. Továbbá várhatóan egyetlen kör elég lesz ehhez. <br />
Szinkron és aszinkron esetben is igaz lesz. <br />

A csúcsoknak lokálisan csak a nála kisebb sorszámú csúcsok állapotairól kell tudnia. <br />
Konstans számú kör elég lesz a MIS helyreállításához (de mindig azt írják, hogy *várhatóan* 1 kör).<br />
Szinkron kommunikáció a csúcsok között körökre osztva (1 kör = 1 broadcast üzenet a szomszédoknak). Az üzenet mérete limitált, maximum O(log(n)) bit lehet, ahol n = |V|. <br />

A csúcsok outputja megadja a struktúrát (egy csúcs tudja magáról, hogy benne van-e a MIS-ben). <br />

Lehetséges változások a gráfban:
  - Tfh. egyszerre egy változás megy végbe a gráfban megfelelő időt hagyva a helyreállításra, azaz mindig megvan a MIS a kiinduló állapotban (stabil rendszer)
  - csúcs/él törlés:
    - graceful: az üzenetküldések után - amikor már stabil a rendszer - kilép a gráfból (addig használható kommunikációra)
    - hirtelen (*abruptly*): a szomszédai automatikusan detektálják
  - csúcs/él beszúrás:
    - új: új csúcs lehetségesen több új éllel belép (ezt majd okosan kihagyjuk és csak akkor fogjuk unmuteolni, ha már van éle)
    - *unmute*: addig láthatatlan csúcs csatlakozik, aki addig csak hallgatta a kommunikációt

Komplexitás-vizsgálat:
  - Változtatás komplexitás: csúcsok száma, aminek megváltozott az outputja
  - Kör komplexitás: hány kör alatt megy végbe
  - Broadcast komplexitás: összes broadcast üzenet száma

## Template MIS megtartásához (absztrakt)

Legyen G = (V, E) gráfban M csúcspontok halmaza úgy, hogy M bármely két csúcsa között nincs él és bármely csúcsnak, ami nincs M-ben van szomszédja M-ben (azaz M MIS?). <br />
Tekintsük ezt úgy, hogy kezdetben egy üres gráfból indultunk el végig megtartva a MIS-t. <br />

- Adott π uniform véletlenszerű permutációja v∈V csúcsoknak
- A csúcsoknak két állapota lehet: v∈M vagy v∈M̄
- v∈M ⇔ ∀u∈N(v): π(u) < π(v): u∉M, azaz v egyik kisebb sorszámú szomszédja sincs még a MIS-ben
- Ha az invariáns tulajdonság kielégített => M MIS G-ben
- Bármilyen topológia változásra maximum 1 csúcsnál sérül az invariáns tulajdonság

Legyen v\* csúcs. Él beszúrásánál vagy törlésénél v\* az a végpont, aminek nagyobb a sorszáma π szerint. Csúcs beszúrásánál vagy törlésénél v* a csúcs. Az élet érintő változásnál a másik végpontot is figyelemebe vesszük (v\*\*). Látható, hogy π(v\*\*) < π(v\*). Csúcs változásnál jelölje ugyan azt a kettő (v\* = v\*\*), ekkor π(v\*\*) = π(v\*) triviális. Tehát bármely topológia változásra π(v\*\*) <= π(v\*). <br />

[Példa] Új él beszúrása ami v\* és v\*\* élt köti össze, π(v\*\*) < π(v\*) és v\*,v\*\*∈M. Ekkor v\*-ot ki kell törölni M-ből. v\* állapotának megváltozása miatt több változtatásra is szükséges lehet. A lokális változások propagációja során egy csúcs többször is megváltoztathatja az állapotát. Ezért megadunk egy S halmazt, ami a befolyásolt csúcsok (influenced nodes) halmazát jelöli. Ebben azon csúcsok vannak benne, amik v\* változásának hatására változtatniuk kell az állapotukat. <br />

A jelölések az új G gráfon alapszanak, kivéve a csúcstörlését, amikor a régi gráfra támaszkodunk (hirtelen kilépésnél is?). Minden u csúcshoz definiálunk egy I<sub>(π)</sub>(u) = {v ∈ N (u) | π(v) < π(u)} halmazt. Ezek a csúcsok potenciálisan befolyásolhatják u állapotát a MIS invariáns szerint. S definíciója rekurzív a π sorrend szerint. <br />

Ha a topológia változás után a G gráf π sorrendezéssel a MIS invariáns fennáll v\*-ra -> S = ∅, azaz nincs befolyásolt csúcs. <br />
Egyébként S<sub>0</sub> = {v\*} és <br />
S<sub>i</sub> = {u | u ∈ M , és S<sub>i−1</sub> ∩ I<sub>π</sub>(u) ≠ ∅} ∪ {u | u ∈ M̄ , és  ∀v∈I<sub>π</sub>(u) ∩ M benne van ∪<sup>i−1</sup><sub>j=0</sub>S<sub>j</sub> )} <br />

Ezek után S = ∪<sub>i</sub>S<sub>i</sub>. Vegyük észre, hogy egy u csúcs több ilyen S<sub>i</sub>-ben is lehet. Ekkor u állapotát csak w ∈ I</sub>π</sub>(u) állapotának módosítása után frissíthetjük. Legyen i<sub>u</sub> = max{i | u ∈ S<sub>i</sub> } az az i maximális index, ahol u∈S<sub>i</sub>. <br />

**Állítás 1 (*Theorem 1*)**: Bármely két gráfra, amik csak egy élben vagy csúcsban különböznek:
E<sub>π</sub> [|S|] ≤ 1. (ez már szerepelt korábban, többször is). <br />

**Algorithm 1** -> A Template for Dynamic Correlation Clustering <br />
Initially, G = (V, E) satisfies the MIS invariant. <br />
On topology change at node v\* do: <br />

1. Update state of v\* if required for MIS to hold <br />
2. For i ← 1, until S<sub>i</sub> = ∅, do: <br />
3.   For every u ∈ S<sub>i</sub> such that i = i<sub>u</sub>: <br />
4.     Update state of u <br />
5. i ← i + 1 <br />
