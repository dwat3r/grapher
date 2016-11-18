# Optimális dinamikus elosztott MIS

## Absztrakt

Független halmaz (IS):
  - Csúcsok olyan halmaza, ahol egyik él között sincs él, azaz szomszédos csúcsok nem szerepelnek benne
  - Átfogalmazva bármely élnek maximum az egyik végpontja lehet benne

Maximum független halmaz (MIS):
  - G gráfban a legnagyobb ilyen ponthalmaz
  - A halmazhoz tetszőleges csúcsot hozzávéve már nem lesz maximális
  - Egy G gráfnak lehet több ilyenje is (van rá felső korlát)
  - NP-teljes probléma

Lokális természete miatt statikus esetben gyorsan számolható: *logaritmikus* a csúcsok számához vagy azok fokszámaihoz. <br />

Statikus elosztott MIS lefuttatása minden topológia változáskor (csúcsok/élek hozzáadása vagy törlése) -> dinamikus elosztott modell komplexitása megegyzik a statikus esetével. <br />

Lokalitás kihasználása -> dinamikus esetben MIS változtatása *szinkron* és *aszinkron* módon. <br/>

*Determinisztikus* és *randomizált* megoldások. <br />

**Algoritmus**: Mohó *szekvenciális* MIS algoritmus a csúcsok véletlenszerű sorbarendezésével. <br />

Tulajdonságok:
  - *history-independence*: a kimeneti struktúra csakis a jelenlegi gráfból függ
  - kezeli a *graceful* és a hirtelen kieséseket